/* Includes ------------------------------------------------------------------*/
#include <intrinsics.h>
#include <string.h>
#include "iostm8s103f3.h"
#include "ds18b20.h"
#include "delay.h"
#include "nrf24.h"

#define debug

// NRF24 pipe address
#define ADDR_LEN 5
char pipe_addr[]={0xB5,0xB4,0xB3,0xB2,0xB1};
//payload buffer size
#define PAYLOAD_LEN 32
//MQTT topic _and_ empty space for value
#define TOPIC_LEN 23
char mqtt_topic_value[PAYLOAD_LEN]="/nrf24/B5B4B3B2B1/temp:"; //23 of 32 bytes
//MQTT value
#define VALUE_LEN 6
char value[VALUE_LEN]="+123.4";

void init_hw(){
  // SYS: HSI/2 = 8 MHz TODO!
  CLK_CKDIVR=0x00;//16 MHz at the moment
  
  //Enable SPI Clock
  CLK_PCKENR1 = CLK_PCKENR1 | (1 << 1); //bit 11 - SPI clock
  //PB7: MISO, PU OK?
  PC_DDR_bit.DDR7 = 0;
  PC_CR1_bit.C17 = 1;
  PC_CR2_bit.C27 = 0;
  //PB6: MOSI
  PC_DDR_bit.DDR6 = 1;
  PC_CR1_bit.C16 = 1;
  PC_CR2_bit.C26 = 1;
  //PB5: SCK
  PC_DDR_bit.DDR5 = 1;
  PC_CR1_bit.C15 = 1;
  PC_CR2_bit.C25 = 1;
  //PB4: NSS
  PC_DDR_bit.DDR4 = 1;
  PC_CR1_bit.C14 = 1;
  PC_CR2_bit.C24 = 1;
  PC_ODR_bit.ODR4 = 1;
  //PB3: CE
  PC_DDR_bit.DDR3 = 1;
  PC_CR1_bit.C13 = 1;
  PC_CR2_bit.C23 = 1;
  SPI_CR1_bit.MSTR = 1; //master
  SPI_CR2_bit.SSI = 1; //master
  SPI_CR2_bit.SSM = 1; //soft management
  
  //Enable USART clock
  CLK_PCKENR1 = CLK_PCKENR1  | (1 << 3); //bit 3 is USART clk
  
  //PD6: USART1 RX, In, PU, NoInt
  PD_DDR_bit.DDR6 = 0;
  PD_CR1_bit.C16 = 1;
  PD_CR2_bit.C26 = 0;
  //PD5: USART1 TX, Out, PP, Fast
  PD_DDR_bit.DDR5 = 1;
  PD_CR1_bit.C15 = 1;
  PD_CR2_bit.C25 = 1;
  // 16000000/9600=1667=0x0683
  UART1_BRR2 = 0x03;
  UART1_BRR1 = 0x68;
  UART1_CR2_bit.REN=1;
  UART1_CR2_bit.TEN=1;
//  UART1_CR2_bit.RIEN=1; //rx int
  __enable_interrupt ();
 
  // PB5 - LED
  PB_DDR_bit.DDR5 = 1;
  PB_CR1_bit.C15 = 1;
  PB_CR2_bit.C25 = 0;
  PB_ODR_bit.ODR5 = 1;
}

void init_common(){
  NrfWriteReg(SETUP_AW, ADDR_LEN-2); // address width (1 - 3 bytes, 2 - 4 bytes, 3 - 5 bytes)
  NrfWriteReg(SETUP_RETR, 0x5A); // retr in 1500 us, up to 10 attempts
  NrfWriteReg(RF_CH, 0x4C); // channel 2400 + 76 MHz
  NrfWriteReg(RF_SETUP, 0x03); // 1 Mbit, -12 dBm, dontcare=1
  //NrfWriteReg(0x50, 0x73); // some magic dumped from arduino
  NrfWriteReg(FEATURE, 0x00); // disable all features
  NrfWriteReg(DYNPD, 0x00); // disable dynamic payload
  NrfWriteReg(EN_AA, 0x03); // enable autoack on pipe 0,1
}

void init_send(){
  init_common();
  NrfWriteReg(CONFIG, 0x0E); // EN_CRC 2 Bytes, PWRUP, PTX
  NrfWriteAddr(TX_ADDR, pipe_addr, ADDR_LEN); // tx addr
  NrfWriteAddr(RX_ADDR_P0, pipe_addr, ADDR_LEN); // pipe 0 rx addr
  NrfWriteReg(RX_PW_P0, PAYLOAD_LEN); // payload len for pipe 0
  NrfWriteReg(EN_RXADDR, 0x01); // enable pipe 0
  NrfFlushTx();
}

void init_ds18b20(){
  /* TL=0, TH=0, CONF=9bit resolution */
  char buffer[] = {0x00, 0x00, 0x1F};
  Ds18b20_WriteScratchpad(buffer, 3);
  Ds18b20_CopyScratchpad();
}

void read_ds18b20(char *buffer) {
  char temp, half, sign;
  Ds18b20_ConvertT();
  Ds18b20_ReadScratchpad(buffer, 2);
#ifdef debug
  PrintString("\nDS18b20: 0x");
  PrintByte(buffer[1]);
  PrintByte(buffer[0]);
#endif
  half = buffer[0] & 0x08;  //save 1/2 degree value
  sign = buffer[1] & 0x80;  //save sign
  buffer[0]=buffer[0] >> 4; //get rid of decimal part, move low bits to low nibble
  buffer[1]=buffer[1] << 4; //move high bits to high nibble
  temp=buffer[0]+buffer[1]; //puth both nibbles into one byte
  if (sign) {
    buffer[0]='-';
    temp = ~temp; //invert to positive
    temp += 1;
  } else {
    buffer[0]='+';
  }
  buffer[1]=temp/100+0x30;  //write figures to buffer in ascii
  temp=temp%100;
  buffer[2]=temp/10+0x30;
  temp=temp%10;
  buffer[3]=temp+0x30;
  buffer[4]='.'; //add decimal point
  //if (half) buffer[5]='5'; else buffer[5]='0'; //and half value
  buffer[5] = half ? '5':'0'; //is it similar to line above?
#ifdef debug
  PrintString(" temp: ");
  PrintBuffer(value, VALUE_LEN);
  PB_ODR_bit.ODR5 = ~PB_ODR_bit.ODR5; // blink LED
#endif
}

void main(void)
{
  //PrintString("\nStarted");
  char status = 0;
  init_hw();
  init_ds18b20();
  init_send();  
  /* Infinite loop */
  while (1)
  {
    read_ds18b20(value);
    memcpy(mqtt_topic_value+TOPIC_LEN, value, VALUE_LEN);
    PrintString("\nData: ");
    PrintString(mqtt_topic_value);
    NrfWritePayload(mqtt_topic_value, PAYLOAD_LEN);
    //starting tx
    PrintString("\nBefore: STATUS: ");
    PrintByte(NrfReadReg(STATUS));
    PrintString(". FIFO_STATUS: ");
    PrintByte(NrfReadReg(FIFO_STATUS));
    NrfEnable();
    while(!((NrfReadReg(STATUS))&0xF0)); //wait for some flag to be set
    NrfDisable();
    status = NrfReadReg(STATUS);
    PrintString("\nAfter:  STATUS: ");
    PrintByte(status);
    PrintString(". FIFO_STATUS: ");
    PrintByte(NrfReadReg(FIFO_STATUS));
    //check flags
    if (status & (1<<5)) {
      PrintString("\nTX_DS is set. Resetting");
      NrfWriteReg(STATUS, NrfReadReg(STATUS)|(1<<5));
    }
    if (status & (1<<4)) {
      PrintString("\nMAX_RT is set. Resetting");
      NrfWriteReg(STATUS, NrfReadReg(STATUS)|(1<<4));
    }
    PB_ODR_bit.ODR5 = ~PB_ODR_bit.ODR5; // blink LED
    PrintString("\n");
    Delayms(5000);
  }
}

#pragma vector=UART1_R_OR_vector
__interrupt void UART1_RXNE(void)
{
  UART1_DR=UART1_DR; //echo back
}

