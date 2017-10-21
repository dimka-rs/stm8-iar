#include "iostm8s103f3.h"

#define DefaultARR 20000 //preload = 20k ticks = 20 ms = 50 Hz
#define DefaultCCR 1500  //compare value = impulse length = 1.5 ms
#define CCR1_MIN 1000 //most right
#define CCR1_MAX 2000 //most left
#define CCR2_MIN 100 // laser min
#define CCR2_MAX 2000 // laser max
#define CCR3_MIN 800  //most up
#define CCR3_MAX 1500 //most down
#define CCR_STEP 1    //initially 100
#define TIM4_MAX 20   //initially 2000

enum states {up, right, down, left};
enum states state;
unsigned int tim4upd = 0;
unsigned char tim4flag = 0;

void init_hw(){
  CLK_CKDIVR_bit.HSIDIV = 2; //F=16/4=4MHz

  //SRV_H - horizontal servo, D4, TIM2 CH1
  //out, pp, fast: DDR=1, CR1=1, CR2=1
  PD_DDR_bit.DDR4 = 1;
  PD_CR1_bit.C14 = 1;
  PD_CR2_bit.C24 = 1;

  //SRV_V - vertical servo, A3, TIM2 CH3
  //out, pp, fast: DDR=1, CR1=1, CR2=1
  PA_DDR_bit.DDR3 = 1;
  PA_CR1_bit.C13 = 1;
  PA_CR2_bit.C23 = 1;

  //LASER - D3, TIM2 CH2
  //out, pp, fast: DDR=1, CR1=1, CR2=1
  PD_DDR_bit.DDR3 = 1;
  PD_CR1_bit.C13 = 1;
  PD_CR2_bit.C23 = 1;

  //LED - B5
  //out, pp, fast: DDR=1, CR1=1, CR2=1
  PB_DDR_bit.DDR5 = 1;
  PB_CR1_bit.C15 = 1;
  PB_CR2_bit.C25 = 1;
  PB_ODR_bit.ODR5 = 1; //LED Off

  //UART TX - PD5
  //out, pp, fast: DDR=1, CR1=1, CR2=1

  //UART RX - PD6
  //in, pu, w/o int: DDR=0, CR1=1, CR2=0

  //TIM2 - generates 50 kHz servo signal
  TIM2_CR1_bit.ARPE = 1; //use auto preload
  TIM2_CCMR1_bit.OC1M = 6; //PWM Mode 1
  TIM2_CCMR1_bit.OC1PE = 1; //OC1 preload enable
  TIM2_CCMR2_bit.OC2M = 6; //PWM Mode 1
  TIM2_CCMR2_bit.OC2PE = 1; //OC2 preload enable
  TIM2_CCMR3_bit.OC3M = 6; //PWM Mode 1
  TIM2_CCMR3_bit.OC3PE = 1; //OC3 preload enable
  TIM2_CCER1_bit.CC1E = 1; //OC1 enabled
  TIM2_CCER1_bit.CC2E = 1; //OC2 enabled
  TIM2_CCER2_bit.CC3E = 1; //OC3 enabled
  TIM2_PSCR_bit.PSC = 2; //Fcnt = Fclk/2^PSC, 4MHz/2^2=1MHz
  //ARR defines frequency
  TIM2_ARRH = DefaultARR / 256; // higher byte first
  TIM2_ARRL = DefaultARR % 256; // lower byte last
  //CCR defines duty, CH1 - SRV_H
  TIM2_CCR1H = DefaultCCR/256; // higher byte first
  TIM2_CCR1L = DefaultCCR%256; // lower byte last
  //CCR defines duty, CH3 - SRV_V
  TIM2_CCR2H = DefaultCCR/256; // higher byte first
  TIM2_CCR2L = DefaultCCR%256; // lower byte last
  //CCR defines duty, CH2 - LASER
  TIM2_CCR3H = CCR2_MIN/256; // higher byte first
  TIM2_CCR3L = CCR2_MIN%256; // lower byte last
  TIM2_CR1_bit.CEN = 1; //timer enable

  //TIM4
  TIM4_CR1_bit.ARPE = 1; //use auto preload
  TIM4_PSCR_bit.PSC = 2; //Fcnt=Fclk/2*PSC, 4MHz/2*4=0.5MHz
  TIM4_IER_bit.UIE = 1; //enable update interrupt
  TIM4_ARR = 250; //OVF freq = 0.5MHz /250 = 2 kHz
  TIM4_CR1_bit.CEN = 1; //timer enable

  //interrupts
  ITC_SPR6_bit.VECT23SPR = 0; // TIM4_OVR_UIF interrupt Level 3 (= software priority disabled)
  asm("RIM"); //enable interrupts
}

void set_led(char state){
  if (state) {
    PB_ODR_bit.ODR5 = 0; //LED On
  } else {
    PB_ODR_bit.ODR5 = 1; //LED Off
  }t
}

void toggle_led() {
  set_led(PB_ODR_bit.ODR5);
}

int main( void )
{
  init_hw();
  //start in lower left corner, then move up
  unsigned int ccr1 = CCR1_MAX;
  unsigned int ccr3 = CCR3_MAX;
  unsigned char led_psc = 0;
  state = up;
  while(1) {
    if (tim4flag) {
      tim4flag = 0; //reset flag!
      led_psc += 1;
      if (led_psc % 10 == 0) {
        led_psc = 0;
        PB_ODR_bit.ODR5 = ~PB_ODR_bit.ODR5; //blink led
      }

      switch(state) {
      case up:
        ccr3 -= CCR_STEP;
        if (ccr3 <= CCR3_MIN) state = right;
        break;
      case right:
        ccr1 -= CCR_STEP;
        if (ccr1 <= CCR1_MIN) state = down;
        break;
      case down:
        ccr3 += CCR_STEP;
        if (ccr3 >= CCR3_MAX) state = left;
        break;
      case left:
        ccr1 += CCR_STEP;
        if (ccr1 >= CCR1_MAX) state = up;
        break;
      }

      //load new ccr values
      TIM2_CCR1H = ccr1/256;
      TIM2_CCR1L = ccr1%256;
      TIM2_CCR3H = ccr3/256;
      TIM2_CCR3L = ccr3%256;
    }
  }
}


#pragma vector = TIM4_OVR_UIF_vector
__interrupt void TIM4_OVR_UIF_handler(void) {
  //this routine is called every 0.5 ms
  TIM4_SR_bit.UIF = 0; //clear interrupt flag
  tim4upd += 1;
  if (tim4upd >= TIM4_MAX) {
    tim4upd = 0;
    tim4flag = 1;
  }
}