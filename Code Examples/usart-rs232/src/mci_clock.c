#include "main.h"
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
  *            PREDIV                         = 2
  *            PLLMUL                         = 2
  *            Flash Latency(WS)              = 1
  * @param  None
  * @retval None
  */
void EPL_SystemClock_Config(void){

    // Reset the Flash 'Access Control Register', and
    // then set 1 wait-state and enable the prefetch buffer.
    // (The device header files only show 1 bit for the F0
    //  line, but the reference manual shows 3...)

    FLASH->ACR &= ~(0x00000017);
    FLASH->ACR |=  (FLASH_ACR_LATENCY |
                    FLASH_ACR_PRFTBE);


    RCC->CR2 |= RCC_CR2_HSI48ON;

    while(!(RCC->CR2 & RCC_CR2_HSI48RDY)){
    }



    MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, 0x0);
    MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE, 0x0);
    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, (0x00000003U));

    while((RCC->CFGR & RCC_CFGR_SWS) != (0x0000000CU)){
    }
}

