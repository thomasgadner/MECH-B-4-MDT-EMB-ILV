#include "mci.h"

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSI48)
  *            SYSCLK(Hz)                     = 48000000
  *            HCLK(Hz)                       = 48000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            HSI Frequency(Hz)              = 48000000
  *            Flash Latency(WS)              = 1
  * @param  None
  * @retval None
  */
void EPL_SystemClock_Config(void)
{
    // Reset the Flash 'Access Control Register', and
    // then set 1 wait-state and enable the prefetch buffer.
    // (The device header files only show 1 bit for the F0
    //  line, but the reference manual shows 3...)
    FLASH->ACR &= ~(FLASH_ACR_LATENCY_Msk | FLASH_ACR_PRFTBE_Msk);
    FLASH->ACR |=  (FLASH_ACR_LATENCY |
                    FLASH_ACR_PRFTBE);

    // activate the internal 48 MHz clock
    RCC->CR2 |= RCC_CR2_HSI48ON;

    // wait for clock to become stable before continuing
    while (!(RCC->CR2 & RCC_CR2_HSI48RDY))
        ;

    // configure the clock switch
    RCC->CFGR = RCC->CFGR & ~RCC_CFGR_HPRE_Msk;
    RCC->CFGR = RCC->CFGR & ~RCC_CFGR_PPRE_Msk;
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | (0b11 << RCC_CFGR_SW_Pos);

    // wait for clock switch to become stable
    while((RCC->CFGR & RCC_CFGR_SWS) != (0b11 << RCC_CFGR_SWS_Pos))
        ;
}


void EPL_init_Debug_UART(){
     // Enable peripheral  GPIOA clock
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    // Enable peripheral  USART2 clock
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    // Configure PA2 as USART2_TX using alternate function 1
    GPIOA->MODER |= GPIO_MODER_MODER2_1;
    GPIOA->AFR[0] |= 0b0001 << (4*2);


    // Configure PA3 as USART2_RX using alternate function 1
    GPIOA->MODER |= GPIO_MODER_MODER3_1;
    GPIOA->AFR[0] |= 0b0001 << (4*3);

    // Configure the UART Baude rate Register 
    USART2->BRR = (APB_FREQ / BAUDRATE);
    // Enable the UART using the CR1 register
    USART2->CR1 |= ( USART_CR1_RE | USART_CR1_TE | USART_CR1_UE );
}



/**
 * @brief Delays the program execution for a specified amount of time.
 * @param time The amount of time to delay in number of cycles.
 * @return 0 when the delay is completed.
 */
void EPL_delay(uint32_t time){
    for(uint32_t i = 0; i < time; i++ ){
        asm("nop"); // No operation, used for delaying
    }
    return 0;
}



// For supporting printf function we override the _write function to redirect the output to UART
int _write( int handle, char* data, int size ) {
    int count = size;
    while( count-- ) {
        while( !( USART2->ISR & USART_ISR_TXE ) ) {};
        USART2->TDR = *data++;
    }
    return size;
}


