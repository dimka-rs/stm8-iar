// NRF24L01P+ functions
// nCS is PB4
// CE is ??

#include "nrf24.h"
#include "iostm8l152c6.h"
#include "delay.h"

//helpers
void PrintString(char* data){
  unsigned int i = 0;
  while(data[i] != 0){
    while(!(USART1_SR_bit.TXE));
    USART1_DR=data[i];
    i++;
  }
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

void PrintBuffer(char* buffer, int size){
  for (unsigned int i=size; i>0; i--) {
    PrintByte(buffer[i-1]);
  }
}

//SPI functions
void SpiStart(){
  SPI1_CR1_bit.SPE = 1; //spi enable
  PB_ODR_bit.ODR4 = 0; //cs low  
}

void SpiStop(){
  PB_ODR_bit.ODR4 = 1; //cs high
  SPI1_CR1_bit.SPE = 0; //spi disable  
}

unsigned char SpiSendByte(unsigned char data){
  SPI1_DR = data;
  while(!(SPI1_SR_bit.RXNE));
  unsigned char read = SPI1_DR;
  return read;
}

// NRF functions
unsigned char NrfReadReg(unsigned char reg){
  unsigned char read;
  SpiStart();
  read = SpiSendByte(reg);
  read = SpiSendByte(0xFF);
  SpiStop();
  return read;
}

void NrfWriteReg(unsigned char reg, unsigned char data){
  volatile unsigned char read;
  reg += 0x20; //add write flag
  SpiStart();
  read = SpiSendByte(reg);
  read = SpiSendByte(data);
  SpiStop();
}

void NrfReadAddr(unsigned char reg, char* addr, unsigned char addr_size){
  //only 0x0A, 0x0B, 0x10 are valid addresses
  volatile unsigned char read;
  PrintString("\nReading from ");
  PrintByte(reg);
  PrintString(": ");
  SpiStart();
  SpiSendByte(reg);
  while(addr_size--){
    addr[addr_size]=SpiSendByte(0xFF);
    PrintByte(addr[addr_size]);
  }
  SpiStop();
}

void NrfWriteAddr(unsigned char reg, char* addr, unsigned char addr_size){
  //only 0x0A, 0x0B, 0x10 are valid addresses
  volatile unsigned char read;
  reg += 0x20; //add write flag
  PrintString("\nWriting to ");
  PrintByte(reg);
  PrintString(": ");
  SpiStart();
  SpiSendByte(reg);
  while(addr_size--){
    PrintByte(addr[addr_size]);
    SpiSendByte(addr[addr_size]);
  }
  SpiStop();
}

void NrfReadPayload(char* buffer, unsigned char len){
  // R_RX_PAYLOAD: 0110 0001, 1-32 LSByte first
}

void NrfWritePayload(char* buffer, unsigned char len){
  // W_TX_PAYLOAD: 1010 0000, 1 to 32 LSByte first
  SpiStart();
  SpiSendByte(0xA0);
  PrintString("\nPayload: ");
  while(len--){
    PrintByte(buffer[len]);
    SpiSendByte(buffer[len]);
  }
  SpiStop();
}

void NrfFlushTx(){
  // FLUSH_TX: 1110 0001
  SpiStart();
  SpiSendByte(0xE1);
  SpiStop();
}

void NrfFlushRx(){
  // FLUSH_RX: 1110 0010
  SpiStart();
  SpiSendByte(0xE2);
  SpiStop();
}

void NrfEnable(){
    PB_ODR_bit.ODR3 = 1;
}
void NrfDisable(){
    PB_ODR_bit.ODR3 = 0;
}
