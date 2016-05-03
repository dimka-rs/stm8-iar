#include "iostm8s103f3.h"

//PORT C
#define seg_H 7
#define seg_G 6
#define seg_F 5
#define seg_E 4
#define seg_D 3
//PORT A
#define seg_C 3
#define seg_B 2
#define seg_A 1
//PORT D left to right!
#define dig_0 2
#define dig_1 3
#define dig_2 4
#define dig_3 5
#define dig_4 6

//Digits
extern const char pd[5] = {
/*0*/ ~(1 << dig_0),
/*1*/ ~(1 << dig_1),
/*2*/ ~(1 << dig_2),
/*3*/ ~(1 << dig_3),
/*4*/ ~(1 << dig_4)
};

// Segs a,b,c
extern const char pa[16] = {
/*0*/ (1 << seg_A) | (1 << seg_B) | (1 << seg_C),
/*1*/ (1 << seg_B) | (1 << seg_C),
/*2*/ (1 << seg_A) | (1 << seg_B),
/*3*/ (1 << seg_A) | (1 << seg_B) | (1 << seg_C),
/*4*/ (1 << seg_B) | (1 << seg_C),
/*5*/ (1 << seg_A) | (1 << seg_C),
/*6*/ (1 << seg_A) | (1 << seg_C),
/*7*/ (1 << seg_A) | (1 << seg_B) | (1 << seg_C),
/*8*/ (1 << seg_A) | (1 << seg_B) | (1 << seg_C),
/*9*/ (1 << seg_A) | (1 << seg_B) | (1 << seg_C),
/*a*/ (1 << seg_A) | (1 << seg_B) | (1 << seg_C),
/*b*/ (1 << seg_C),
/*c*/ (1 << seg_A),
/*d*/ (1 << seg_B) | (1 << seg_C),
/*e*/ (1 << seg_A),
/*f*/ (1 << seg_A)
};

// Segs d,e,f,g,h
extern const char pc[16] = {
/*0*/ (1 << seg_D) | (1 << seg_E) | (1 << seg_F),
/*1*/ 0,
/*2*/ (1 << seg_D) | (1 << seg_E) | (1 << seg_G),
/*3*/ (1 << seg_D) | (1 << seg_G),
/*4*/ (1 << seg_F) | (1 << seg_G),
/*5*/ (1 << seg_D) | (1 << seg_F) | (1 << seg_G),
/*6*/ (1 << seg_D) | (1 << seg_E) | (1 << seg_F) | (1 << seg_G),
/*7*/ 0,
/*8*/ (1 << seg_D) | (1 << seg_E) | (1 << seg_F) | (1 << seg_G),
/*9*/ (1 << seg_D) | (1 << seg_F) | (1 << seg_G),
/*a*/ (1 << seg_E) | (1 << seg_F) | (1 << seg_G),
/*b*/ (1 << seg_D) | (1 << seg_E) | (1 << seg_F) | (1 << seg_G),
/*c*/ (1 << seg_D) | (1 << seg_E) | (1 << seg_F),
/*d*/ (1 << seg_D) | (1 << seg_E) | (1 << seg_G),
/*e*/ (1 << seg_D) | (1 << seg_E) | (1 << seg_F) | (1 << seg_G),
/*f*/ (1 << seg_E) | (1 << seg_F) | (1 << seg_G),
};

void SomeDelay() {
  for (unsigned long delay_count=0; delay_count<30000; delay_count++);
  };

void WriteChar(unsigned char pos, unsigned char val) {
  PD_ODR = pd[pos];
  PC_ODR = pc[val];
  PA_ODR = pa[val]; 
  };

int main( void )
{
  //PB_DDR_bit.DDR5 = 1;
  PC_DDR = 0xF8; // Set to out
  PC_CR1 = 0xF8; // Push-Pull
  PC_CR2 = 0x00; // Slow mode
  PA_DDR = 0x0E;
  PA_CR1 = 0x0E;
  PA_CR2 = 0x00;
  PD_DDR = 0x7C; // Set to out bits 2-6
  PD_CR1 = 0x00; // Open drain
  PD_CR2 = 0x00;
  
  while(1) {
    for (unsigned int i=0; i<16; i++) {
      WriteChar((i%5), i);
      SomeDelay();
    };

    //PC_ODR ^= MASK_PC_ODR_ODR5;
  }
  
  return 0;
}
