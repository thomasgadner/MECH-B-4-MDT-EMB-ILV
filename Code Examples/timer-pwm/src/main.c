#include "main.h"
#include "mci_clock.h"

#define C


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
 * @brief Main function
 * @return 0
 */
int main(void){

    SystemClock_Config();


    // Enable the GPIOA and TIM2 peripherals
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; 

    // Set the mode of the GPIOA pin 5 to alternate function mode 2
    GPIOA->MODER |= GPIO_MODER_MODER5_1;
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR5;
    GPIOA->AFR[0] |= 0b0010 << 20;
    
    // Disable the timer before setting the prescaler and overflow values
    TIM2->CR1 &= ~TIM_CR1_CEN;
    
    // Set the prescaler and overflow values
    TIM2->PSC = 0;
    TIM2->ARR = 47999/2;
    TIM2->CCR1 = 0;

    // Set the PWM mode 1
    TIM2->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
    TIM2->CCMR1 |= TIM_CCMR1_OC1PE;
    // Set the output polarity to active high
    TIM2->CCER |= TIM_CCER_CC1E;

    // Enable the main output
    TIM2->BDTR |= TIM_BDTR_MOE;
    
    // Enbable the timer
    // Set the center-aligned mode 
    TIM2->CR1 |= TIM_CR1_CMS_0 | TIM_CR1_CEN;

    uint16_t duty = 5999/2;


    for(;;){

        TIM2->CCR1 = duty;
    }
}