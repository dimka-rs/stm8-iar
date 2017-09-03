// registers for NRF24L01p
#define CONFIG 0x00
#define EN_AA 0x01
#define EN_RXADDR 0x02
#define SETUP_AW 0x03
#define SETUP_RETR 0x04
#define RF_CH 0x05
#define RF_SETUP 0x06
#define STATUS 0x07
#define OBSERVE_TX 0x08
#define RPD 0x09
#define RX_ADDR_P0 0x0A // 5 bytes!
#define RX_ADDR_P1 0x0B // 5 bytes!
#define RX_ADDR_P2 0x0C
#define RX_ADDR_P3 0x0D
#define RX_ADDR_P4 0x0E
#define RX_ADDR_P5 0x0F
#define TX_ADDR 0x10 // 5 bytes!
#define RX_PW_P0 0x11
#define RX_PW_P1 0x12
#define RX_PW_P2 0x13
#define RX_PW_P3 0x14
#define RX_PW_P4 0x15
#define RX_PW_P5 0x16
#define FIFO_STATUS 0x17
// ACK_PLD, TX_PLD, RX_PLD accessed by separate command, all 32 bytes
#define DYNPD 0x1C
#define FEATURE 0x1D

//helpers
void Delayms(unsigned int n);
void PrintString(char* data);
void PrintByte(unsigned char data);
void PrintBuffer(char* buffer, int size);

// SPI functions
void SpiStart();
void SpiStop();
unsigned char SpiSendByte(unsigned char data);

// NRF functions
unsigned char NrfReadReg(unsigned char reg); // R_REGISTER: 000A AAAA, 1-5 LSByte first
void NrfWriteReg(unsigned char reg, unsigned char data); // W_REGISTER: 001A AAAA, 1-5 LSByte first
void NrfReadAddr(unsigned char reg, char* addr, unsigned char addr_size); // R_REGISTER: 000A AAAA, 1-5 LSByte first
void NrfWriteAddr(unsigned char reg, char* addr, unsigned char addr_size); // W_REGISTER: 001A AAAA, 1-5 LSByte first
void NrfReadPayload(char* rx_buffer, unsigned char len); // R_RX_PAYLOAD: 0110 0001, 1-32 LSByte first
void NrfWritePayload(char* tx_buffer, unsigned char len); // W_TX_PAYLOAD: 1010 0000, 1 to 32 LSByte first
void NrfFlushTx(); // FLUSH_TX: 1110 0001
void NrfFlushRx(); // FLUSH_RX: 1110 0010
void NrfEnable();
void NrfDisable();

/* TODO:
reusetxpl() // REUSE_TX_PL: 1110 0011
rrxplwid() // R_RX_PL_WID: 0110 0000
wackpld(pipe) // W_ACK_PAYLOAD: 1010 1PPP, 1 to 32 LSByte first
wtxpldnoack() // W_TX_PAYLOAD_NOACK: 1011 0000, 1 to 32 LSByte first
nop() // NOP: 1111 1111, shifts out STATUS
*/
