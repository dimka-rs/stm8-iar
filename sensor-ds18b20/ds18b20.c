#include "iostm8s103f3.h"
#include "ds18b20.h"
#include "delay.h"

//PIN is PB4
//TODO: make code less hw dependant

/* --- 1-wire functions --- */
unsigned char OneWire_Reset() {
  /*
  - held low for more than 480µs
  - release for 480 us
  -- slave waits 15-60 us
  -- presence pulse 60-240 us
  */
  //PB4 as OpenDrain output fast mode
  PB_DDR_bit.DDR4 = 1;
  PB_CR1_bit.C14 = 0;
  PB_CR2_bit.C24 = 1;
  //reset impulse
  PB_ODR_bit.ODR4 = 0;
  Delayus(500);
  PB_ODR_bit.ODR4 = 1;
  Delayus(100);
  unsigned char presence = PB_IDR_bit.IDR4;
  Delayus(400);
  return presence;
}

unsigned char OneWire_ReadBit() {
  /*
  - low for 10 us
  - release and read
  - wait 50 us
  */
  PB_ODR_bit.ODR4 = 0;
  Delayus(10);
  PB_ODR_bit.ODR4 = 1;
  Delayus(1);
  unsigned char ow_bit = PB_IDR_bit.IDR4;
  Delayus(60);
  return ow_bit;
}
 
void OneWire_WriteBit(unsigned char ow_bit) {
  /*
  60+ us slot + 1+ us recovery
  1: release in 15 us (prefer 10)
  0: hold low whole time
  */
  PB_ODR_bit.ODR4 = 0;
  Delayus(10);
  if (ow_bit == 0) {
    Delayus(50);
    PB_ODR_bit.ODR4 = 1;
  } else {
    PB_ODR_bit.ODR4 = 1;
    Delayus(50);
  }
}

//wait for 1 function ???


/* --- ds18b20 functions --- */
/*
Step 1. Initialization (reset + presence if any)
Step 2. ROM Command (followed by any required data exchange) 
Step 3. DS18B20 Function Command (followed by any required data exchange) 
Exceptions are the Search ROM  [F0h]  and  Alarm  Search  [ECh]  commands

Init - Read rom - read scratch for conf (up to 9 bytes)
   Init - Read rom - write scratch (3 bytes)
   Init - Read rom - read scratch for crc (up to 9 bytes)
   Init - Read rom - copy scratch - wait for 1
Init - Read rom - start conversion - wait for 1 (100-1000 ms)
Init - Read rom - read scratch for temp (up to 9 bytes)

*/

void Ds18b20_SendByte(unsigned char byte){
  for (int i = 0; i < 8; i++) {
    //write LSBit first!
    OneWire_WriteBit(byte%2);
    byte = byte >> 1;
  }
}
unsigned char Ds18b20_ReadByte(){
  unsigned char byte = 0;
  for (int i = 0; i < 8; i++) {
    //read LSBit first!
    byte += OneWire_ReadBit() << i;
  }
  return byte;
}

void Ds18b20_ReadScratchpad(char* buffer, unsigned char len){
  //byte 0 LSBit first
  //Init - Read rom - read scratch (up to 9 bytes)
  //TODO: len must be <=9
  OneWire_Reset();
  Ds18b20_SendByte(SKIP_ROM);
  Delayus(100);
  Ds18b20_SendByte(READ_SCRATCHPAD);
  for (unsigned int i = 0; i < len; i++){
    buffer[i] = Ds18b20_ReadByte();
  }
}

void Ds18b20_WriteScratchpad(char* buffer, unsigned char len){
  // Bytes 2(TH), 3(TL), 4(CONF) only, byte 2 LSBit first
  // Conf: 0x1F - 9 bit (0.5 C), 0x7F - 12 bit (0.0625 C)
  // TODO: len must be <=3
  OneWire_Reset();
  Ds18b20_SendByte(SKIP_ROM);
  Delayus(100);
  Ds18b20_SendByte(WRITE_SCRATCHPAD);
  for (unsigned int i = 0; i < len; i++){
    Ds18b20_SendByte(buffer[i]);
  }
}

void Ds18b20_CopyScratchpad() {
  OneWire_Reset();
  Ds18b20_SendByte(SKIP_ROM);
  Delayus(100);
  Ds18b20_SendByte(COPY_SCRATCHPAD);
  //TODO: wait for 1
  Delayms(1000);
}

void Ds18b20_ConvertT(){
  /* master can issue “read time slots” after the Convert T command
  DS18B20 will respond by transmitting 0 while the temperature conversion
  is in progress and 1 when the conversion is done
  Does not work with parasite power */
  OneWire_Reset();
  Ds18b20_SendByte(SKIP_ROM);
  Delayus(100);
  Ds18b20_SendByte(CONVERT_T);
  //TODO: wait for 1
  Delayms(1000);
}

//TODO: unsigned char Ds18b20_ReadPowerSupply();

//TODO void Ds18b20_RecallEE();
/*master  can  issue  read  time  slots following the Recall E2 command
and the DS18B20 will indicate the status of the recall by transmitting 0 
while the recall is in progress and 1 when the recall is done. */