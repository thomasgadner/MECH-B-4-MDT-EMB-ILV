#ifndef EPL_CLOCK_H
#define EPL_CLOCK_H

#include "mci.h"
#include "stm32f091xc.h"

#define APB_FREQ 48000000
#define AHB_FREQ 48000000

#define BAUDRATE 9600


void EPL_SystemClock_Config(void);
void EPL_init_Debug_UART(void);
void EPL_delay(uint32_t time);

#endif // EPL_CLOCK_H
