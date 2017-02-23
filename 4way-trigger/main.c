#include "iostm8s103f3.h"

void SomeDelay() {
  for (unsigned long delay_count=0; delay_count<2000; delay_count++);
};

int main( void )
{
  // both ports use bits 6 to 3
  // port D as output
  PD_ODR = 0x00; // clear all
  PD_DDR = 0x78; // Set to out
  PD_CR1 = 0x78; // Push-Pull
  PD_CR2 = 0x00; // Slow mode
  // port C as inputs
  PC_DDR = 0x00; // Set to input
  PC_CR1 = 0x00; // Open drain
  PC_CR2 = 0x00; // Slow mode
  // init vars
  unsigned int outdata = 255; // high - relay off
  unsigned int indata = 0; // low - no transmission
  unsigned int flags = 0; // low - no signal locked
  unsigned int in1 = 0;
  unsigned int in2 = 0;
  unsigned int in3 = 0;
 
  while(1) {
    while(1) {
      //read inputs 3 times with some delay and compare
      //in they are equal. If not - read again
      in1 = PC_IDR;
      SomeDelay();
      in2 = PC_IDR;
      SomeDelay();
      in3 = PC_IDR;
      if ((in1 == in2 ) && (in2 == in3)) break;
      SomeDelay();
    }
      indata = in1;
      for (unsigned int i=0; i<8; i++) {
        if ((indata & (1 << i) ) > 0)  {
          // check if flag not set 
          if (( flags & (1 << i) ) == 0)  {
            // set flag
            flags = flags | (1 << i);
            // invert out bit
            if ((outdata & (1 << i)) == 0)  {
              // set bit (relay off)
              outdata = outdata | (1 << i);
            } else {
              // clear bit (relay on)
              outdata = outdata & ~(1 << i);
            }
          }
        } else {
          // clear flag
          flags = flags & ~(1 << i);
        }
      }
      PD_ODR = outdata;
  }
  
  return 0;
}
