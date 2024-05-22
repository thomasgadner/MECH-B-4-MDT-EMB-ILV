#include <main.h>
#include <gpio.h>



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


/**
 * @brief Main function where the program execution starts.
 */
int main(void){   

    // Use new Things

    for(;;){
        asm("nop");
    }
}




