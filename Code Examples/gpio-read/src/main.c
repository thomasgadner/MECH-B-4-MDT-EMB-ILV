#include <stm32f091xc.h>
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))


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
    uint8_t gpio_outputPin = 0;
    uint8_t gpio_inputPin = 0;
    
    // Enable clock for GPIOA
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    
    // Set GPIOA pin X as output
    GPIOA->MODER |= GPIO_MODER_MODER0_0 << gpio_outputPin;
    // Set GPIOA pin X as push-pull output
    GPIOA->OTYPER &= ~(GPIO_OTYPER_OT_0 << gpio_outputPin);

    // Set GPIOA pin X as input
    GPIOA->MODER &= ~(GPIO_MODER_MODER0_0 << gpio_inputPin);
    GPIOA->MODER &= ~(GPIO_MODER_MODER0_1 << gpio_inputPin);

    uint8_t is_led_set = 0;

    for(;;){

        if(CHECK_BIT(GPIOA->IDR, gpio_inputPin)){
            if(is_led_set){
                // Set GPIOA pin X to low
                GPIOA->BSRR |= GPIO_BSRR_BR_0 << gpio_outputPin;
                
            }else{
                // Set GPIOA pin X to high
                GPIOA->BSRR |= GPIO_BSRR_BS_0 << gpio_outputPin;
                is_led_set = 0;
            }
        }
          
        // Delay for 100 cycles
        delay(100);
        

    }
}