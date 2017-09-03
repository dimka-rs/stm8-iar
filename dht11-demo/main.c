#include <iostm8l152c6.h>

#define DHT_PIN 0
#define DHT_ODR PC_ODR
#define DHT_IDR PC_IDR
#define DHT_DDR PC_DDR
#define DHT_CR1 PC_CR1
#define DHT_CR2 PC_CR2

#define DHT_SIZE 5
char DhtData[DHT_SIZE];
unsigned char TimerOverflow;
unsigned char result;

void Init(void) {
  //rcc
  CLK_CKDIVR_bit.CKM = 0; //SYSCLK = 16 MHz from HSI
  //init timer to 1 MHz
  CLK_PCKENR1_bit.PCKEN10 = 1; //enable TIM2 clock
  TIM2_PSCR_bit.PSC = 4; //prescaler = 16 = 2^4
  
}

unsigned char WaitBit(unsigned char bit, unsigned int timeout) {
  TimerOverflow = 0;
  TIM2_ARRH = timeout/256; // higher byte first
  TIM2_ARRL = timeout%256; // lower byte last
  //TIM2_EGR_bit.UG = 1; //Update event to update regs
  TIM2_IER_bit.UIE = 1; //interrupt on update/overflow
  asm("RIM"); //enable interrupts
  TIM2_CR1_bit.CEN = 1; //start timer
    
  if (bit) {
    //wait for 1
    while(!TimerOverflow) {
      if((DHT_IDR & (1 << DHT_PIN)) != 0) break;
    }
  } else {
    //wait for 0
    while(!TimerOverflow) {
      if((DHT_IDR & (1 << DHT_PIN)) == 0) break;
    }
  }
  if (TimerOverflow == 0) {
    //timer not out, we had desired bit
    TIM2_CR1_bit.CEN = 0; //stop timer
    return 1;
  }
  //timer overflow - no bit
  return 0;
}  


unsigned char ReadDht(char* buf){
  unsigned char BitOffset = 0;
  unsigned char ByteOffset = 0;
  unsigned int count = 0;
  
  //start pulse >18ms
  //init timer to 20 ms
  TimerOverflow = 0;
  unsigned int arr = 20000; //preload defines cycle
  TIM2_ARRH = arr/256; // higher byte first
  TIM2_ARRL = arr%256; // lower byte last
  //TIM2_EGR_bit.UG = 1; //Update event to update regs
  
  //set pin out low DDR=1 CR1=0 CR2=0 ODR=0
  DHT_DDR = DHT_DDR | (1 << DHT_PIN);
  DHT_CR1 = DHT_CR1 & ~(1 << DHT_PIN);
  DHT_CR2 = DHT_CR2 & ~(1 << DHT_PIN);
  DHT_ODR = DHT_ODR & ~(1 << DHT_PIN);
  TIM2_IER_bit.UIE = 1; //interrupt on update/overflow
  TIM2_CR1_bit.CEN = 1; //start timer
  //asm("WFI"); //wait for timer interrupt
  asm("RIM");
  while(!TimerOverflow) {};
    
  //set pin in wopu woint DDR=0 CR1=0 CR2=0
  DHT_DDR = DHT_DDR & ~(1 << DHT_PIN);
  DHT_CR1 = DHT_CR1 & ~(1 << DHT_PIN);
  DHT_CR2 = DHT_CR2 & ~(1 << DHT_PIN);
    
  //wait for presence (low) for 0.1 ms
  //should come up in 20-40us
  if(WaitBit(0, 100) == 0) {
    return 1; //no presence low
  }

  //wait for high - presence finished
  //should come up in 80 us
  if(WaitBit(1, 200) == 0) {
    return 2; //no presence high
  }
  
  //wait for low - bit start
  //should come up in 80 us
  if(WaitBit(0, 200) == 0) {
    return 3; //no bit start
  }
  
  while(1){
    //wait for high - bit value begin
    //should come up in 50 us
    if(WaitBit(1, 100) == 0) return 4; //no bit value
    
    //wait for low - bit value end
    if(WaitBit(1, 100) == 0) return 4; //no bit value
    //read length
    count = TIM2_CNTRH << 8;
    count += TIM2_CNTRL;

    //add result to array
    if(count > 30) {
      //70 us high is 1
      buf[ByteOffset] = buf[ByteOffset] | (1 << BitOffset);
    } else {
      //26-28us high is 0
      buf[ByteOffset] = buf[ByteOffset] & ~(1 << BitOffset);
    }
    BitOffset++;
    if(BitOffset > 7) {
      BitOffset = 0;
      ByteOffset++;
    }
    if(ByteOffset >= DHT_SIZE) break;
  }

  return 0;
}


int main( void )
{

  PC_DDR = PC_DDR | (1 << 7);
  PC_CR1 = PC_CR1 | (1 << 7);
  PC_CR2 = PC_CR2 | (1 << 7);
  PC_ODR = PC_ODR | (1 << 7); //led on  
  Init();
  result = ReadDht(DhtData);
  PC_ODR = PC_ODR & ~(1 << 7); //led off

  while(1) {   };
}



#pragma vector = TIM2_OVR_UIF_vector
__interrupt void TIM2_OVR_UIF_handler(void) {
  TIM2_CR1_bit.CEN = 0; //stop timer
  asm("SIM");
  TIM2_IER_bit.UIE = 0; //disable interrupt on update/overflow
  TIM2_SR1 = 0; //clear all interrupt flags
  TimerOverflow = 1;
}