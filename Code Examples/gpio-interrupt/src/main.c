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

    // Enable clock for SYSCFG
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    // Enable clock for GPIOC
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

    // Set GPIOC pin X as input
    GPIOC->MODER &= ~(GPIO_MODER_MODER0 << 2*gpio_inputPin);
    GPIOC->PUPDR |= (GPIO_PUPDR_PUPDR0_1 << 2*gpio_inputPin);


    // Configure the external interrupt configuration register, for the selected pin.
    SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PC;
    // Set the interrupt mask register to enable the interrupt for the selected pin. 
    EXTI->IMR |= EXTI_IMR_MR0 << gpio_inputPin;
    // Set the interrupt trigger selection register to select the rising edge trigger for the selected pin.
    EXTI->RTSR |= EXTI_RTSR_TR0 << gpio_inputPin;
    EXTI->FTSR &= ~EXTI_FTSR_TR0 << gpio_inputPin;

    // Enable the interrupt for the selected pin.
    NVIC_SetPriority(EXTI4_15_IRQn, 0x3);
    NVIC_EnableIRQ(EXTI4_15_IRQn);
    

    for(;;){
        asm("nop");
    }
}




