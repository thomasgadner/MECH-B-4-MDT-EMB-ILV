#include <stm32f091xc.h>
#define TIMEOUT 100000




/**
* @brief Check if timeout is reached. This is a function to be used in conjunction with timer_add ()
* @param time Time in cycles to check
* @return 1 if timeout reached 0 if not ( 0 is returned on timeout ) Note : It's not possible to use timer_add () in this
*/
uint8_t timeout(uint32_t time){
    static uint32_t cnt = 0;
    cnt++;
    if (cnt > time){
        return 1;
    }
    return 0;
}


/**
* @brief Error handler for program exit. We don't have a way to tell the user what went wrong
*/
void error(void){
    for(;;){}
}



/**
* @brief Main function for ADC. This is the entry point for the application. It sets up the GPIO and the ADC channels and starts the ADC. ADC is connected to GPIOA pin 0
* @return 0 on success non - zero on failure to initialize the hardware. In case of error the error code is returned
*/
int main(void){ 

    // Enable the GPIOA and ADC clock
    RCC->AHBENR  |= RCC_AHBENR_GPIOAEN; 
    RCC->APB2ENR |= RCC_APB2ENR_ADCEN; 

    // Set the GPIOA pin 0 to analog mode
    GPIOA->MODER |= GPIO_MODER_MODER0;  

    // Set the ADC to continuous mode and select scan direction
    ADC1->CHSELR |= ADC_CHSELR_CHSEL0 ; 
    ADC1->CFGR1  |= ADC_CFGR1_CONT | ADC_CFGR1_SCANDIR;
    // Set Sample time 
    ADC1->SMPR   |= ADC_SMPR_SMP_0;
    

    // If ADC is not ready set the ADC ready bit
    if ((ADC1->ISR & ADC_ISR_ADRDY) != 0){   
        ADC1->ISR |= ADC_ISR_ADRDY; 
    }
    // Enable the ADC
    ADC1->CR |= ADC_CR_ADEN; 
    // Wait for the ADC to be ready
    while ((ADC1->ISR & ADC_ISR_ADRDY) == 0){
        if (timeout(TIMEOUT)){
            error();
        }
    }
    

    uint32_t adc_val = 0;
    // Start the ADC
    ADC1->CR |= ADC_CR_ADSTART;

    for(;;){
        // Wait for the ADC to complete
        while ((ADC1->ISR & ADC_ISR_EOC) == 0) {
            if (timeout(TIMEOUT)){
                error();
            }
        }
        // Read the ADC value - set Breakpoint here to see the value
        // Please note in order zo stop the compiler from optimizing the code, debug_build_flags = -O0 -g -ggdb  is set in the platformio.ini file
        adc_val = ADC1->DR;

    }
}







