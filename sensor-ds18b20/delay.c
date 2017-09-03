#include <intrinsics.h>
#include "iostm8s103f3.h"

void Delayus(unsigned int usec){
  CLK_PCKENR1 = CLK_PCKENR1 | (1 << 5); //bit 5 - timer2 clock
  TIM2_CR1_bit.OPM = 1; //one pulse mode
  //TIM2_CR1_bit.URS = 1; //only hw causes UEV
  TIM2_IER_bit.CC1IE = 1; //enable interrupt on compare 1
  TIM2_CCR1H = usec/256; //load high byte
  TIM2_CCR1L = usec%256; //load low byte
  TIM2_CNTRH = 0; // reset counter
  TIM2_CNTRL = 0; // reset counter
  TIM2_SR1_bit.CC2IF = 0; //clear CC2 interrupt flag
  TIM2_PSCR = 0x04; // Divide by 2^4 to get 1MHz clock
  TIM2_EGR_bit.UG = 1; //generate UEV to update actual prescaler
  ITC_SPR4 = ITC_SPR4 | 0x30; //IRQ14 Vector14 is TIM2 cc int priority   ///???
  TIM2_CR1_bit.CEN = 1; //start timer
  __enable_interrupt();
  __wait_for_interrupt();
}

void Delayms(unsigned int msec){
  if (msec <= 65) {
    Delayus(msec*1000);
  } else {
    unsigned int cycles = msec /64;
    unsigned int finale = msec %64;
    for (unsigned int i=0; i < cycles ; i++) Delayus(64000);
    Delayus(finale*1000);
  }
}


#pragma vector=TIM2_CAPCOM_CC1IF_vector
__interrupt void TIM2_CAPCOM_CC1IF_handler(void)
{
  TIM2_CR1_bit.CEN = 0;   //stop timer just in case
  TIM2_SR1_bit.CC1IF = 0; //clear CC1 interrupt flag
  TIM2_SR1_bit.UIF = 0;   //clear update interrupt flag
  __disable_interrupt();
}
