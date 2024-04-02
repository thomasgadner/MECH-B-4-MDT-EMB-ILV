#include "mci_clock.h"

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
