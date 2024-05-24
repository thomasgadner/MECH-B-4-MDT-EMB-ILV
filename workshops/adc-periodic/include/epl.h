#ifndef EPL_H
#define EPL_H

#include "epl.h"
#include "stm32f091xc.h"

#define APB_FREQ 48000000
#define AHB_FREQ 48000000

#define BAUDRATE 9600




#define DEBUG

#ifdef DEBUG
  #define LOG( msg... ) printf( msg );
#else
  #define LOG( msg... ) ;
#endif




void EPL_SystemClock_Config(void);
void EPL_init_Debug_UART(void);
void EPL_delay(uint32_t time);
uint8_t EPL_timeout(uint32_t time);

#endif // EPL_CLOCK_H
