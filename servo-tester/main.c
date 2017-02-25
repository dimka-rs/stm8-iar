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

#include "iostm8s103f3.h"
#define ClockDelay 1
#define DefaultARR 20000; //preload = 20k ticks = 20 ms = 50 Hz
#define DefaultCCR 1500; //compare value = impulse length = 1.5 ms
#define DefaultCnt 150*4; //start with 1.5 ms interval

unsigned char enc, encp;
unsigned int cnt;

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

/* Port A button interrupt handler */
#pragma vector = EXTI0_vector
  __interrupt void EXTI0_handler(void){
    cnt=DefaultCnt;
  }

/* Port B encoder interrupt handler */
#pragma vector = EXTI1_vector
  __interrupt void EXTI1_handler(void){
    enc = PB_IDR/16; //bits 4-5
    switch(enc) {
      case 0:
        if (encp == 2) {cnt++;}
        if (encp == 1) {cnt--;}
        break;
      case 1:
        if (encp == 0) {cnt++;}
        if (encp == 3) {cnt--;}
        break;
      case 2:
        if (encp == 3) {cnt++;}
        if (encp == 0) {cnt--;}
        break;
      case 3:
        if (encp == 1) {cnt++;}
        if (encp == 2) {cnt--;}
        break;
    }
    encp = enc;
  }

/* timer interrups handler */
  #pragma vector = TIM2_CAPCOM_CC2IF_vector
  __interrupt void TIM2_CAPCOM_CC2IF_handler(void) {
    TIM2_SR1_bit.CC2IF = 0; //clear interrupt flag
    //write new ccr, will be updated on overflow
    unsigned int ccr = cnt/4*10;
    TIM2_CCR2H = ccr/256; // higher byte first
    TIM2_CCR2L = ccr%256; // lower byte last
  
  }
  
  #pragma vector = TIM2_OVR_UIF_vector
  __interrupt void TIM2_OVR_UIF_handler(void) {
    TIM2_SR1_bit.UIF = 0; //clear interrupt flag
    //TODO: something
  }



int main( void )
{
  /* vars setup */
  char DisplayString[5];
  cnt=DefaultCnt;
  enc=3;
  encp=3;
  unsigned char b,c,d = 0;
  DisplayString[0] = DigitsTable[0x00];
  DisplayString[1] = DigitsTable[0x01];
  DisplayString[2] = DigitsTable[0x02];
  DisplayString[3] = DigitsTable[0x03];
  DisplayString[4] = DigitsTable[0x16];

  /* clock setup */
  CLK_SWCR_bit.SWEN = 1; //Autoswitch clock source
  CLK_SWR = 0xB4; // HSE selected as master clock source
  /* TODO: CSS function enabled: (CSSEN = 1 in the CLK_CSSR register) */
  
  /* ports setup */
  PA_DDR = 0x00; //input
  PA_CR1 = 0x08; //pull-up
  PA_CR2 = 0x08; //interrupt enabled
  PB_DDR = 0x00; //input
  PB_CR1 = 0x30; //pull-up
  PB_CR2 = 0x30; //interrupt enabled
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
  unsigned int arr = DefaultARR; //preload defines frequency
  TIM2_ARRH = arr/256; // higher byte first
  TIM2_ARRL = arr%256; // lower byte last
  TIM2_IER_bit.CC2IE = 1; //CH2 Capture/compare interrupt enabled
  //TIM2_IER_bit.UIE = 1; //Update interrupt enabled
  TIM2_CCMR2_bit.CC2S = 0; //output, writable only if CC2E = 0 in TIM2_CCER1
  TIM2_CCMR2_bit.OC2M = 6; //pwm1
  TIM2_CCMR2_bit.OC2PE = 1; //preload enable
  unsigned int ccr = DefaultCCR; //compare defines length
  TIM2_CCR2H = ccr/256; // higher byte first
  TIM2_CCR2L = ccr%256; // lower byte last
  TIM2_PSCR_bit.PSC = 4; //prescaler = 16 = 2^4 
  TIM2_CCER1_bit.CC2E = 1; //compare ch2 enable
  TIM2_EGR_bit.UG = 1; //Update event to update regs
  TIM2_CR1_bit.CEN = 1; //counter enabled
  
  /* interrupts setup */
  ITC_SPR1_bit.VECT3SPR = 1;// EXTI0/Port A priority level 1, lower - button
  ITC_SPR2_bit.VECT4SPR = 1;// EXTI1/Port B priority level 1, lower - encoder
  ITC_SPR4_bit.VECT13SPR = 0; //TIM2 upd/ovf, priority level 2, higher
  ITC_SPR4_bit.VECT14SPR = 0; //TIM2 capt/comp, priority level 2, higher
  EXTI_CR1_bit.PAIS = 2; //port A interrupt on fall only (pulled up)
  EXTI_CR1_bit.PBIS = 3; //port B interrupt on both rise and fall
  asm("RIM"); //enable interrupts
  
  while(1) {
    //TODO: move LED update to interrupt, use asm("WFI"); to wait
    for(unsigned char CurrDigit=3;CurrDigit<8;CurrDigit++) {
      PD_ODR_bit.ODR2 = 1;
      b=cnt/4/100%10;
      c=cnt/4/10%10;
      d=cnt/4%10;
      DisplayString[0] = DigitsTable[b];
      DisplayString[1] = DigitsTable[0x26];
      DisplayString[2] = DigitsTable[c];
      DisplayString[3] = DigitsTable[d];
      SendByte(DisplayString[CurrDigit-3]); //Set segs
      PC_ODR = 0xff & ~(1 << CurrDigit); //Select digit
      PD_ODR_bit.ODR2 = 0; //~OE low, 595 on, show digit
      Delay(100);
    };
  };
  //return 0; //never reached
}
