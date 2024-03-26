#include <stm32f091xc.h>


#define CLOCK_CYCLES_PER_SECOND  48000000
#define MAX_RELOAD               0xFFFF




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

    // Set the frequency of the PWM signal
    uint32_t freq = 1000; // 1 Khz
    // Calculate the period of the PWM signal
    uint32_t period_cycles = CLOCK_CYCLES_PER_SECOND / freq;

    // Calculate the prescaler and overflow values
    uint16_t prescaler = (uint16_t)(period_cycles / MAX_RELOAD + 1);
    uint16_t overflow = (uint16_t)((period_cycles + (prescaler / 2)) / prescaler);

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
    TIM2->PSC = prescaler;
    TIM2->ARR = overflow;
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

    
    uint32_t duty_c = 0;
    uint16_t duty_step = 100;

    for(;;){
        // Increase the duty cycle
        while(duty_c < MAX_RELOAD)
        {
            TIM2->CCR1 = duty_c;
            duty_c += duty_step;
            delay(10000);
        }
        // Decrease the duty cycle
        while(duty_c > 0)
        {
            TIM2->CCR1 = duty_c;
            duty_c -= duty_step;
            delay(10000);
        }
    }   
}