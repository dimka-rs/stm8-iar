/* Includes ------------------------------------------------------------------*/
#include "iostm8l152c6.h"
#include <intrinsics.h>

void init(){
    // SYS: HSI/2 = 8 MHz TODO!
  CLK_CKDIVR=0x00;//16 for test
  
  //Enable SPI Clock
  CLK_PCKENR1_bit.PCKEN14 = 1;
  //PB7: MISO, PU OK?
  PB_DDR_bit.DDR7 = 0;
  PB_CR1_bit.C17 = 1;
  PB_CR2_bit.C27 = 0;
  //PB6: MOSI
  PB_DDR_bit.DDR6 = 1;
  PB_CR1_bit.C16 = 1;
  PB_CR2_bit.C26 = 1;
  //PB5: SCK
  PB_DDR_bit.DDR5 = 1;
  PB_CR1_bit.C15 = 1;
  PB_CR2_bit.C25 = 1;
  //PB4: NSS
  PB_DDR_bit.DDR4 = 1;
  PB_CR1_bit.C14 = 1;
  PB_CR2_bit.C24 = 1;
  PB_ODR_bit.ODR4 = 1;
  //PB3: CE
  PB_DDR_bit.DDR3 = 1;
  PB_CR1_bit.C13 = 1;
  PB_CR2_bit.C23 = 1;
  //PB1: IRQ, PU
  PB_DDR_bit.DDR1 = 0;
  PB_CR1_bit.C11 = 1;
  PB_CR2_bit.C21 = 0;
  SPI1_CR1_bit.MSTR = 1; //master
  SPI1_CR2_bit.SSI = 1; //master
  SPI1_CR2_bit.SSM = 1; //soft management
  
  //PB2: ADC1_IN16
  
  // RTC: LSI for power economy
  CLK_CRTCR = 0x04;
  CLK_PCKENR2_bit.PCKEN22 = 1; //enable clock
  if(RTC_ISR1_bit.INITS == 0) {
    //unlock
    RTC_WPR=0xCA;
    RTC_WPR=0x53;
    RTC_ISR1_bit.INIT = 1;
    while(!(RTC_ISR1_bit.INITF));
    RTC_CR1_bit.FMT = 0; //24h
    //TODO: make corrections for LSI later
    RTC_SPRERH=0x00; // 8:12
    RTC_SPRERL=0xFF; // 0:7
    RTC_APRER=0x7F; // 0:6
    //set time and date
    RTC_TR1=0x11; //sec
    RTC_TR2=0x45; //min
    RTC_TR3=0x22; //hrs
    RTC_DR1=0x09; //day
    RTC_DR2=0x45; //dow+month
    RTC_DR3=0x17; //year
    RTC_ISR1_bit.INIT = 0;
    RTC_WPR=0x00; //lock
    while(!(RTC_ISR1_bit.RSF));
  }

  //Enable USART clock
  CLK_PCKENR1_bit.PCKEN15 = 1;
  
  //PC2: USART1 RX, In, PU, NoInt
  PC_DDR_bit.DDR2 = 0;
  PC_CR1_bit.C12 = 1;
  PC_CR2_bit.C22 = 0;
  //PC3: USART1 TX, Out, PP, Fast ot PA2?
  PC_DDR_bit.DDR3 = 1;
  PC_CR1_bit.C13 = 1;
  PC_CR2_bit.C23 = 1;
  // 16000000/9600=1667=0x0683
  USART1_BRR2 = 0x03;
  USART1_BRR1 = 0x68;
  USART1_CR2_bit.REN=1;
  USART1_CR2_bit.TEN=1;
  USART1_CR2_bit.RIEN=1; //Прерывание по приему
  __enable_interrupt (); // Разрешаем прерывания.
 
  // PC7 - Blue LED
  PC_DDR_bit.DDR7 = 1;
  PC_CR1_bit.C17 = 1;
  PC_CR2_bit.C27 = 0;
  // PE7 - Green LED
  PE_DDR_bit.DDR7 = 1;
  PE_CR1_bit.C17 = 1;
  PE_CR2_bit.C27 = 0;
}


void Delayms(unsigned int n){
  while (n-- > 0) {
    unsigned int m = 2000;
    while (m-- > 0);
  }
}

void PrintChar(unsigned char data){
  while(!(USART1_SR_bit.TXE));
  USART1_DR=data;
}

void PrintByte(unsigned char data){
  unsigned char high, low;
  high = data / 16;
  if (high < 10) {
    high = high + 48; //digit
  } else {
    high = high + 55; //letter
  }
  while(!(USART1_SR_bit.TXE));
  USART1_DR=high;
  low = data % 16;
  if (low < 10) {
    low = low + 48; //digit
  } else {
    low = low + 55; //letter
  }
  while(!(USART1_SR_bit.TXE));
  USART1_DR=low;
}

unsigned char SpiReadReg(unsigned char reg){
  PC_ODR_bit.ODR7 = 1; //blue led on
  unsigned char data;
  unsigned char status;
  SPI1_CR1_bit.SPE = 1; //enable
  PB_ODR_bit.ODR4 = 0; //cs low
  SPI1_DR = reg;
  while(!(SPI1_SR_bit.RXNE));
  status = SPI1_DR;
  SPI1_DR = 0xFF;
  while(!(SPI1_SR_bit.RXNE));
  data = SPI1_DR;
  PB_ODR_bit.ODR4 = 1; //cs high
  PC_ODR_bit.ODR7 = 0; //blue led on
  SPI1_CR1_bit.SPE = 0; //disable
  return data;
}

void SpiWriteReg(unsigned char reg, unsigned char data){
  reg += 0x20;
  PC_ODR_bit.ODR7 = 1; //blue led on
  unsigned char status;
  SPI1_CR1_bit.SPE = 1; //enable
  PB_ODR_bit.ODR4 = 0; //cs low
  SPI1_DR = reg;
  while(!(SPI1_SR_bit.RXNE));
  status = SPI1_DR;
  SPI1_DR = data;
  while(!(SPI1_SR_bit.RXNE));
  status = SPI1_DR;
  PB_ODR_bit.ODR4 = 1; //cs high
  PC_ODR_bit.ODR7 = 0; //blue led on
  SPI1_CR1_bit.SPE = 0; //disable
}

void main(void)
{
  init();
  PrintChar('0');
  PrintChar('x');
  PrintByte(0x5B);
  PrintChar('\n');
  
 
  /* Infinite loop */
  unsigned char i = 0;
  while (1)
  {
    PE_ODR_bit.ODR7 = ~PE_ODR_bit.ODR7;
    PrintChar('s');
    PrintChar('s');
    PrintChar('m');
    PrintChar('m');
    PrintChar('h');
    PrintChar('h');
    PrintChar('D');
    PrintChar('D');
    PrintChar('M');
    PrintChar('M');
    PrintChar('Y');
    PrintChar('Y');
    PrintChar('-');
    PrintChar('W');
    PrintChar('\n');
    PrintByte(RTC_TR1);
    PrintByte(RTC_TR2);
    PrintByte(RTC_TR3);
    PrintByte(RTC_DR1);
    PrintByte(RTC_DR2&0x1F);
    PrintByte(RTC_DR3);
    PrintByte(RTC_DR2/32);
    PrintChar('\n');
    Delayms(1000);
  }
}

#pragma vector=USART_R_OR_vector
__interrupt void USART1_RXE(void)
{
  USART1_DR=USART1_DR; //Отправляем тоже, что и приняли (эхо)
}

