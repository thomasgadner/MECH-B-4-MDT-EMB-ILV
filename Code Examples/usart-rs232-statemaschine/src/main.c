#define STM32F091xC
#include <stm32f0xx.h>
#include "epl_clock.h"
#include "epl_usart.h"

#define DEBUG


/*This software is under construction and is not yet finished. It is not intended for use in its current state.*/


int main(void){

 
    // Configure the system clock to 48MHz
    EPL_SystemClock_Config();

    


    uint8_t rxb = 42;

    for(;;){
    LOG("[DEBUG-LOG]: %d\r\n", rxb );
    }
}





