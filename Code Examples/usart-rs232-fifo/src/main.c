#include <stm32f0xx.h>
#include "epl_clock.h"
#include "epl_usart.h"

#define DEBUG

/**
 * @brief Delays the program execution for a specified amount of time.
 * @param time The amount of time to delay in number of cycles.
 * @return 0 when the delay is completed.
 */
int delay(uint32_t time){
    for(uint32_t i = 0; i < time; i++ ){
        asm("nop"); // No operation, used for delaying
    }
    return 0;
}


int main(void){

    /*Active Development:
        tx: tested and working
        rx: rx fifo not tested
    */

    epl_usart_t myusart;
    myusart.baudrate = 9600;
    myusart.fifo_size = 20;

    // Configure the system clock to 48MHz
    EPL_SystemClock_Config();
    EPL_init_usart(&myusart);

    
    for(;;){
        uint8_t msg[] = "Hello, World \r\n";
        EPL_usart_write_n_bytes(msg, sizeof(msg));
        delay(1000000);



    }
}





