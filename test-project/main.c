#include "iostm8s103f3.h"

//PORT C Anodes, active high
#define seg_H 7  //HS
#define seg_G 6  //HS
#define seg_F 5  //HS
#define seg_E 4  //HS
#define seg_D 3  //HS

//PORT A
#define seg_C 3  //HS
#define seg_B 2
#define seg_A 1

//PORT D left to right! Cathodes, active low
//#define SWIM 1  //HS SWIM! do not use
#define dig_0 2  //HS
#define dig_1 3  //HS
#define dig_2 4  //HS
#define dig_3 5  //HS
#define dig_4 6  //HS

//PORT B - usonic
#define usin  4  // T, also SCL
#define usout 5  // T, also SDA - will not work, use seg_H for now


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

void Delay(unsigned long max_delay) {
  for (unsigned long delay_count=0; delay_count<max_delay; delay_count++);
  };

void WriteChar(unsigned char pos, unsigned char val) {
  //turn segs off for a while
  PD_ODR = PD_ODR  | 0x7C ;
  Delay(20);
  PD_ODR = pd[pos];
  PC_ODR = pc[val];
  PA_ODR = pa[val]; 
  };

unsigned int usmeasure()
{
  unsigned int cnt, cm = 0;
  unsigned char pin = 0;
  //send pulse
  unsigned char pcodr = PC_ODR;
  PC_ODR = (1 << seg_H);
  Delay(1);
  PC_ODR = pcodr;
  //recv echo. max 4 m = 23200 us
  //wait for HIGH
  for (unsigned int i=0; i<60000; i++) {
    pin = PB_IDR & (1 << usin);
    if (pin != 0) { break ;}
  };
  //count HIGH length
  for (unsigned int i=0; i<3000; i++) {
    pin = PB_IDR & (1 << usin);
    cnt = cnt + 1;
    if (pin == 0) { break ;}
  };
  // cm = us/58, eq / 29 / 2
  // my guess is * 8.1 -60 additionnaly
  // which results in / 7.16 - 60
  cm = cnt / 7.16 - 60 ;
  return cm;
};

int main( void )
{
  PC_DDR = 0xF8; // Set to out
  PC_CR1 = 0xF8; // Push-Pull
  PC_CR2 = 0x00; // Slow mode
  PA_DDR = 0x0E;
  PA_CR1 = 0x0E;
  PA_CR2 = 0x00;
  PD_DDR = 0x7C; // Set to out bits 2-6
  PD_CR1 = 0x00; // Open drain
  PD_CR2 = 0x00;
  unsigned int a = 0;
  
  while(1) {
    unsigned int cm = usmeasure();
    unsigned int d100 = cm / 100;
    unsigned int d10 = (cm % 100) / 10;
    unsigned int d1 = cm % 10;
    //Display
    for (unsigned int i=0; i<300; i++) {
      WriteChar(0, d100);
      Delay(10);
      WriteChar(1, d10);
      Delay(10);
      WriteChar(2, d1);
      Delay(10);
      WriteChar(4, a);
      Delay(10);
      PD_ODR = PD_ODR  | 0x7C ; //turn off while measure
    };
    //increment indicator counter
    if (a < 9) { a = a + 1 ;} else { a = 0 ;}

    // Running digits test
    //for (unsigned int i=0; i<16; i++) {
    //  WriteChar((i%5), i);
    //  Delay(30000);
    //  };
  }
  
  //return 0; never reached
}
