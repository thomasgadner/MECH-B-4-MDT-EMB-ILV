#include <stm32f091xc.h>
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))



uint8_t gpio_outputPin = 5;
uint8_t gpio_inputPin = 13;


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


void EXTI4_15_IRQHandler(void)
{
  if (EXTI->PR & (1 << gpio_inputPin)) {
    // Clear the EXTI status flag.
    // Put a breakpoint here and check the value of the flag.
    EXTI->PR |= (1 << gpio_inputPin);
  }
 
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




