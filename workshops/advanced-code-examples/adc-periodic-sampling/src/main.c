#include "epl.h"
#define ADC_TIMEOUT 100000


void init_adc(void);
void init_tim15(void);



int main(void){

    EPL_SystemClock_Config();

    init_tim15();
    init_adc();

    NVIC_SetPriority(TIM15_IRQn, 10);
    NVIC_EnableIRQ(TIM15_IRQn);
    NVIC_EnableIRQ(ADC1_COMP_IRQn); 
    NVIC_SetPriority(ADC1_COMP_IRQn,0); 

    __enable_irq();

    for(;;){
        asm("nop");
    }

    return 0;
}


void init_tim15(void){
    // Enable the TIM2 clock
    RCC->APB2ENR |= RCC_APB2ENR_TIM15EN; 
    // Disable the timer
    TIM15->CR1 &= ~TIM_CR1_CEN;
    
    TIM15->PSC = 48-1;
    TIM15->ARR = 1000-1;
    TIM15->CR1 |= TIM_CR1_ARPE;
    TIM15->CR1 |= TIM_CR1_URS;
    TIM15->EGR |= TIM_EGR_UG;

    // Output a TRGO event on update for ADC
    TIM15->CR2 |= 0x2 << TIM_CR2_MMS_Pos;

    //TIM15->DIER |= TIM_DIER_UIE;
	TIM15->SR &= ~TIM_SR_UIF;    // Clear the interrupt

    TIM15->EGR |= TIM_EGR_UG;
    TIM15->CR1 |= TIM_CR1_CEN;

    // Stop the timer when the core is halted for debugging
    DBGMCU->APB2FZ |= DBGMCU_APB2_FZ_DBG_TIM15_STOP;
}

void init_adc(void){ 
    // Enable the GPIOA and ADC clock
    RCC->AHBENR  |= RCC_AHBENR_GPIOAEN; 
    RCC->APB2ENR |= RCC_APB2ENR_ADCEN; 

    // Set the GPIOA pin 0 to analog mode
    GPIOA->MODER |= GPIO_MODER_MODER0;  

    ADC1->CFGR1 = 0x00000000;
    ADC1->CFGR2 = 0x00000000;

    ADC1->CFGR1 &= ~ADC_CFGR1_RES;
    ADC1->CFGR2 &= ~ADC_CFGR2_CKMODE;

    // Set the ADC to continuous mode and select scan direction
    ADC1->CHSELR |= ADC_CHSELR_CHSEL0;
    ADC1->CFGR1  &= ~ADC_CFGR1_CONT; 
    ADC1->CFGR1  |= ADC_CFGR1_SCANDIR;
    ADC1->CFGR1 |= ADC_CFGR1_EXTEN_0 | ADC_CFGR1_EXTSEL_2;

    ADC1->SMPR   |= ADC_SMPR_SMP_0;
    ADC1->IER = ADC_IER_EOCIE | ADC_IER_EOSEQIE | ADC_IER_OVRIE;
    
    // If ADC is not ready set the ADC ready bit
    if ((ADC1->ISR & ADC_ISR_ADRDY) != 0){   
        ADC1->ISR |= ADC_ISR_ADRDY; 
    }
    // Enable the ADC
    ADC1->CR |= ADC_CR_ADEN; 
    // Wait for the ADC to be ready
    while ((ADC1->ISR & ADC_ISR_ADRDY) == 0){
    }

    ADC1->CR |= ADC_CR_ADSTART;
}


void ADC1_COMP_IRQHandler(void){
    if (ADC1->ISR & ADC_ISR_EOC){
        uint32_t adc_val = ADC1->DR;
        ADC1->ISR |= ADC_ISR_EOC;
    }
    if (ADC1->ISR & ADC_ISR_EOS){
        ADC1->ISR |= ADC_ISR_EOS;
    }
}

/* For debugging purposes only */
void TIM15_IRQHandler(void) {
    if (TIM15->SR & TIM_SR_UIF){
        TIM15->SR &= ~TIM_SR_UIF;
    }
}