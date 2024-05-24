#include "epl.h"
#define ADC_TIMEOUT 100000


void init_adc(void);
void init_tim2(void);



int main(void){

    EPL_SystemClock_Config();

    init_tim2();
    init_adc();

    NVIC_SetPriority(TIM2_IRQn, 0);
    NVIC_EnableIRQ(TIM2_IRQn);
    NVIC_EnableIRQ(ADC1_COMP_IRQn); 
    NVIC_SetPriority(ADC1_COMP_IRQn,10); 

    __enable_irq();

    for(;;){
        asm("nop");
    }

    return 0;
}


void init_tim2(void){
    // Enable the TIM2 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; 

    // Disable the timer
    TIM2->CR1 &= ~TIM_CR1_CEN;
    
    TIM2->PSC = 48-1;
    TIM2->ARR = 1000-1;
    TIM2->CR1 |= TIM_CR1_ARPE;
    TIM2->CR1 |= TIM_CR1_URS;
    TIM2->EGR |= TIM_EGR_UG;

    // Output a TRGO event on update for ADC
    TIM2->CR2 |= 0b010 << TIM_CR2_MMS_Pos;

    TIM2->DIER |= TIM_DIER_UIE;
	TIM2->SR &= ~TIM_SR_UIF;    // Clear the interrupt

    TIM2->EGR |= TIM_EGR_UG;
    TIM2->CR1 |= TIM_CR1_CEN;

    DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_TIM6_STOP;
}

void init_adc(void){ 
    // Enable the GPIOA and ADC clock
    RCC->AHBENR  |= RCC_AHBENR_GPIOAEN; 
    RCC->APB2ENR |= RCC_APB2ENR_ADCEN; 

    // Set the GPIOA pin 0 to analog mode
    GPIOA->MODER |= GPIO_MODER_MODER0;  

    // Set the ADC to continuous mode and select scan direction
    ADC1->CHSELR |= ADC_CHSELR_CHSEL0;
    ADC1->CFGR1  &= ~ADC_CFGR1_CONT; // 12 bit resolution 
    ADC1->CFGR1  |= ADC_CFGR1_SCANDIR;

    //ADC1->CFGR1  |= 0x1 << ADC_CFGR1_EXTEN_Pos;
    //ADC1->CFGR1  |= 0x2 << ADC_CFGR1_EXTSEL_Pos;
    ADC1->SMPR   |= ADC_SMPR_SMP_0;
    ADC1->IER = ADC_IER_EOCIE |  ADC_IER_EOSIE  ;
    
    // If ADC is not ready set the ADC ready bit
    if ((ADC1->ISR & ADC_ISR_ADRDY) != 0){   
        ADC1->ISR |= ADC_ISR_ADRDY; 
    }
    // Enable the ADC
    ADC1->CR |= ADC_CR_ADEN; 
    // Wait for the ADC to be ready
    while ((ADC1->ISR & ADC_ISR_ADRDY) == 0){
    }
    
}


void ADC1_COMP_IRQHandler(void){
    if (ADC1->ISR & ADC_ISR_EOC){
        uint32_t adc_val = ADC1->DR;
        // Do something with the ADC value
        ADC1->ISR |= ADC_ISR_EOC;
    }
    if (ADC1->ISR & ADC_ISR_EOS){
        // End of sequence
        ADC1->ISR |= ADC_ISR_EOS;
    }
}

void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF){
        ADC1->CR |= ADC_CR_ADSTART;
        TIM2->SR &= ~TIM_SR_UIF;
    }
}