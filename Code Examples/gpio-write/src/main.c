#include <stm32f091xc.h>

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
    uint8_t gpio_pin = 0;
    
    // Enable clock for GPIOA
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    
    // Set GPIOA pin 0 as output
    GPIOA->MODER |= GPIO_MODER_MODER0_0 << gpio_pin;
    
    // Set GPIOA pin 0 as push-pull output
    GPIOA->OTYPER &= ~(GPIO_OTYPER_OT_0 << gpio_pin);

    for(;;){
        // Set GPIOA pin 0 to high
        GPIOA->BSRR |= GPIO_BSRR_BS_0 << gpio_pin;
        
        // Delay for 10000 cycles
        delay(10000);
        
        // Set GPIOA pin 0 to low
        GPIOA->BSRR |= GPIO_BSRR_BR_0 << gpio_pin;
    }
}
