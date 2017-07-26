#include <intrinsics.h>
#include "iostm8l152c6.h"

void Delayus(unsigned int usec){
  CLK_PCKENR1_bit.PCKEN11 = 1; //enable timer3 clock
  TIM3_CR1_bit.OPM = 1; //one pulse mode
  //TIM3_CR1_bit.URS = 1; //only hw causes UEV
  TIM3_IER_bit.CC1IE = 1; //enable interrupt on compare 1
  TIM3_CCR1H = usec/256; //load high byte
  TIM3_CCR1L = usec%256; //load low byte
  TIM3_CNTRH = 0; // reset counter
  TIM3_CNTRL = 0; // reset counter
  TIM3_SR1_bit.CC2IF = 0; //clear CC2 interrupt flag
  TIM3_PSCR = 0x04; // Divide by 2^4 to get 1MHz clock
  TIM3_EGR_bit.UG = 1; //generate UEV to update actual prescaler
  ITC_SPR6_bit.VECT22SPR = 3; //tim3 cc int priority
  TIM3_CR1_bit.CEN = 1; //start timer
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


#pragma vector=TIM3_CAPCOM_CC1IF_vector
__interrupt void TIM3_CAPCOM_CC1IF_handler(void)
{
  TIM3_CR1_bit.CEN = 0;   //stop timer just in case
  TIM3_SR1_bit.CC1IF = 0; //clear CC1 interrupt flag
  TIM3_SR1_bit.UIF = 0;   //clear update interrupt flag
  __disable_interrupt();
}
