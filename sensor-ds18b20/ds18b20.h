#define ALARM_SEARCH 0xEC // like search, but only alarmed respond
#define MATCH_ROM    0x55 // select slave
#define READ_ROM     0x33 // only if 1 slave
#define SEARCH_ROM   0xF0 // identify devices on bus
#define SKIP_ROM     0xCC // broadcast next command

#define CONVERT_T         0x44
#define COPY_SCRATCHPAD   0x48
#define READ_POWER_SUPPLY 0xB4
#define READ_SCRATCHPAD   0xBE
#define RECAL_EE          0xB8
#define WRITE_SCRATCHPAD  0x4E

unsigned char OneWire_Reset();
unsigned char OneWire_ReadBit();
void OneWire_WriteBit(unsigned char ow_bit);

unsigned char Ds18b20_ReadByte();
void Ds18b20_SendByte(unsigned char byte);
void Ds18b20_ReadScratchpad(char* buffer, unsigned char len);
void Ds18b20_WriteScratchpad(char* buffer, unsigned char len);
void Ds18b20_CopyScratchpad();
void Ds18b20_ConvertT();

// TODO: ReadPowerSupply
// TODO: RecallEE