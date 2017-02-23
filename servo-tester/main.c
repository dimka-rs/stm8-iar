#include "iostm8s103f3.h"

/*
Ports:
A1 OSCIN
A2 OSCOUT
A3 BTN (IPU)
B4 ENCA (EPU)
B5 ENCB (EPU)
C3 Dig1
C4 Dig2
C5 Dig3
C6 Dig4
C7 Dig5
D1 SWIM
D2 ~OE
D3 PWM out (TIM2_CH2)
D4 SHCP
D5 DS
D6 STCP
*/

#define ClockDelay 1

extern const char DigitsTable[0x28] = {
/*00 - 0*/ 0xFC,
/*01 - 1*/ 0x60,
/*02 - 2*/ 0xDA,
/*03 - 3*/ 0xF2,
/*04 - 4*/ 0x66,
/*05 - 5*/ 0xB6,
/*06 - 6*/ 0xBE,
/*07 - 7*/ 0xE0,
/*08 - 8*/ 0xFE,
/*09 - 9*/ 0xF6,
/*0a - a*/ 0xEE,
/*0b - b*/ 0x3E,
/*0c - c*/ 0x9C,
/*0d - d*/ 0x7A,
/*0e - e*/ 0x9E,
/*0f - f*/ 0x8E,
/*10 - g*/ 0xBC,
/*11 - h*/ 0x6E,
/*12 - i*/ 0x0C,
/*13 - j*/ 0x70,
/*14 - k*/ 0x6E,
/*15 - l*/ 0x1C,
/*16 - m*/ 0x2B,
/*17 - n*/ 0x2A,
/*18 - o*/ 0x3A,
/*19 - p*/ 0xCE,
/*1a - q*/ 0xE6,
/*1b - r*/ 0x0A,
/*1c - s*/ 0xB6,
/*1d - t*/ 0x1E,
/*1e - u*/ 0x38,
/*1f - v*/ 0x7C,
/*20 - w*/ 0x39,
/*21 - x*/ 0x01,
/*22 - y*/ 0x76,
/*23 - z*/ 0xDA,
/*24 -  */ 0x00,
/*25 - -*/ 0x02,
/*26 - .*/ 0x01,
/*27 - _*/ 0x10
};




void Delay(unsigned long max_delay) {
  for (unsigned long delay_count=0; delay_count<max_delay; delay_count++);
};

void SendByte(unsigned char ByteToSend) {
  unsigned char CurrBit = 0;
  for(unsigned char i=0;i<8;i++) {
    CurrBit = ByteToSend & (1 << i);
    if (CurrBit > 0) {
      PD_ODR_bit.ODR5 = 1; //DS high
    } else {  
      PD_ODR_bit.ODR5 = 0; //DS low
    };
    PD_ODR_bit.ODR4 = 1; //SHCP high
    Delay(ClockDelay);
    PD_ODR_bit.ODR4 = 0; //SHCP low
    Delay(ClockDelay);
  };
  PD_ODR_bit.ODR6 = 1; //STCP high
  Delay(ClockDelay);
  PD_ODR_bit.ODR6 = 0; //STCP low
  Delay(ClockDelay);
};

void BlinkLed() {
  PB_ODR_bit.ODR5 = 0; //led on
  Delay(10000);
  PB_ODR_bit.ODR5 = 1; //led off
  Delay(10000);
};

/* timer interrups handler */
/*
  TIM2_SR1
  Bit 2 CC2IF: Capture/compare 2 interrupt flag - clear writing 0
  Bit 0 UIF: Update interrupt flag - clear writing 0

  #pragma vector = TIM1_OVR_UIF_vector
  __interrupt void TIM1_OVR_UIF_handler(void)
  {
    if (TIM1_SR1_UIF==1){
      TIM1_SR1_UIF = 0;             // Очистка флага прерывания по обновлению
      PD_ODR ^= MASK_PD_ODR_ODR0;   // Переключение уровня напряжения на ножке на противоположное
    }  
  }

*/

int main( void )
{
  /* clock setup
  1. Enable the switching mechanism by setting the SWEN bit in the Switch control
  register (CLK_SWCR).
  2.  Write the 8-bit value used to select the target clock source in the Clock
  master switch register (CLK_SWR). The SWBSY bit in the CLK_SWCR register is set
  by hardware, and the target source oscillator starts. The old clock source
  continues to drive the CPU and peripherals. 
  */
  CLK_SWCR_bit.SWEN = 1; //Autoswitch clock source
  CLK_SWR = 0xB4; // HSE selected as master clock source

  /* TODO: CSS function enabled: (CSSEN = 1 in the CLK_CSSR register) */
  
  /* ports setup */
  PB_DDR = 0x20; //out
  PB_CR1 = 0x20; //push-pull
  PB_CR2 = 0x00; //slow
  PC_ODR = 0xFF; //LED cathodes off
  PC_DDR = 0xFF; //out
  PC_CR1 = 0xFF; //push-pull
  PC_CR2 = 0x00; //slow
  PD_DDR = 0xFE; //out
  PD_CR1 = 0xFE; //push-pull
  PD_CR2 = 0x00; //slow
  PD_ODR_bit.ODR2 = 1; //~OE high for now

  /* timer setup */
  TIM2_CR1_bit.ARPE = 1; //auto preload to keep frequency
  unsigned int arr = 20000; //preload = 20k ticks = 20 ms = 50 Hz
  TIM2_ARRH = arr/256; // higher byte first
  TIM2_ARRL = arr%256; // lower byte last
  //TIM2_IER_bit.CC2IE = 1; //CH2 Capture/compare interrupt enabled
  //TIM2_IER_bit.UIE = 1; //Update interrupt enabled
  TIM2_CCMR2_bit.CC2S = 0; //output, writable only if CC2E = 0 in TIM2_CCER1
  TIM2_CCMR2_bit.OC2M = 6; //pwm1
  TIM2_CCMR2_bit.OC2PE = 1; //preload enable
  unsigned int ccr = 1500; //compare value = impulse length = 1.5 ms
  TIM2_CCR2H = ccr/256; // higher byte first
  TIM2_CCR2L = ccr%256; // lower byte last
  TIM2_PSCR_bit.PSC = 4; //prescaler = 16 = 2^4 
  TIM2_CCER1_bit.CC2E = 1; //compare ch2 enable
  TIM2_EGR_bit.UG = 1; //Update event to update regs
  
  TIM2_CR1_bit.CEN = 1; //counter enabled
  //The UEV can be disabled by setting the UDIS bit in the TIM1_CR1 ?

  
  
  char DisplayString[5];
  DisplayString[0] = DigitsTable[0x01];
  DisplayString[1] = DigitsTable[0x26];
  DisplayString[2] = DigitsTable[0x02];
  DisplayString[3] = DigitsTable[0x01];
  DisplayString[4] = DigitsTable[0x16];
  
  // blink led on PB5 twice when ready
  BlinkLed();
  BlinkLed();
  while(1) {
    for(unsigned char CurrDigit=3;CurrDigit<8;CurrDigit++) {
      PD_ODR_bit.ODR2 = 1;
      DisplayString[0] = DigitsTable[CurrDigit];
      SendByte(DisplayString[CurrDigit-3]); //Set segs
      PC_ODR = 0xff & ~(1 << CurrDigit); //Select digit
      PD_ODR_bit.ODR2 = 0; //~OE low, 595 on, show digit
      Delay(100);
    };
  };
  //return 0;
}
