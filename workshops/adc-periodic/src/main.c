#include "epl.h"
#define ADC_TIMEOUT 100000




void init_adc(void);
void init_gpio(void);
void init_tim2(void);


/**
* @brief Error handler for program exit. We don't have a way to tell the user what went wrong
*/
void error_trap(void);


int main(void){
    EPL_SystemClock_Config();
    init_gpio();
    init_tim2();
    init_adc();

    __enable_irq();
    for(;;){
        asm("nop");
    }

    return 0;
}


void init_gpio(void){
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    // Set the GPIOA pin 0 to analog mode
    GPIOA->MODER |= GPIO_MODER_MODER0;  

}

void init_tim2(void){
    // Enable the TIM2 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; 

    TIM2->PSC = 0;
    TIM2->ARR = 47999;

    // Disable the timer
    TIM2->CR1 &= ~TIM_CR1_CEN;
    // Edge-aligned mode
    TIM2->CR1 |= 0b00 << TIM_CR1_CMS_Pos; 
    TIM2->CR1 |= 0b00 << TIM_CR1_DIR_Pos; 
    // Output a TRGO event on update for ADC
    TIM2->CR2 |= 0b010 << TIM_CR2_MMS_Pos;

    TIM2->DIER |= TIM_DIER_UIE;

    NVIC_SetPriority(TIM2_IRQn, 0);
    NVIC_EnableIRQ(TIM2_IRQn);

    TIM2->EGR |= TIM_EGR_UG;
    TIM2->CCR1 |= TIM_CR1_CEN;
}


void init_adc(void){ 
    // Enable the GPIOA and ADC clock
    RCC->APB2ENR |= RCC_APB2ENR_ADCEN; 
    // Set the ADC to continuous mode and select scan direction
    ADC1->CHSELR |= ADC_CHSELR_CHSEL0 ; 
    // Continuous mode and scan direction
    ADC1->CFGR1  |= ADC_CFGR1_CONT | ADC_CFGR1_SCANDIR;
    // Set Sample time 
    ADC1->SMPR   |= ADC_SMPR_SMP_0;
    // Set the external trigger to TIM2_TRGO
    ADC1->CFGR1  |= ADC_CFGR1_EXTSEL_1;
    // Set the external trigger polarity to rising edge
    ADC1->CFGR1  |= 0b10 << ADC_CFGR1_EXTEN_Pos;
    // Enable the ADC interrupts for end of conversion 
    ADC1->IER = ADC_IER_EOCIE;

    // If ADC is not ready set the ADC ready bit
    if ((ADC1->ISR & ADC_ISR_ADRDY) != 0){   
        ADC1->ISR |= ADC_ISR_ADRDY; 
    }
    // Enable the ADC
    ADC1->CR |= ADC_CR_ADEN; 
    // Wait for the ADC to be ready
    while ((ADC1->ISR & ADC_ISR_ADRDY) == 0){
        if (EPL_timeout(ADC_TIMEOUT)){
            error_trap();
        }
    }
   
    NVIC_EnableIRQ(ADC1_COMP_IRQn); 
    NVIC_SetPriority(ADC1_COMP_IRQn,0); 

}


void error_trap(void){
    for(;;){}
}



void ADC1_COMP_IRQHandler(void){
    if (ADC1->ISR & ADC_ISR_EOC){
        uint32_t adc_val = ADC1->DR;
        // Do something with the ADC value
        ADC1->ISR |= ADC_ISR_EOC;
    }
}

void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF){
        TIM2->SR |= TIM_SR_UIF;
    }
}