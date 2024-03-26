#include "main.h"


#define flash_latency (0x1UL << (0U))
#define RCC_PLLSOURCE_HSI (0x00008000U)
#define RCC_PLL_MUL_12 (0x00280000U)
#define RCC_PREDIV_DIV_2 (0x00000001U)
#define RCC_SYS_CLKSOURCE_PLL (0x00000002U)
#define RCC_SYS_CLKSOURCE_STATUS_PLL (0x00000008U)


void SystemClock_Config(void){
    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, flash_latency);
    while(READ_BIT(FLASH->ACR, FLASH_ACR_LATENCY) != flash_latency){
    }
    SET_BIT(RCC->CR, RCC_CR_HSION);

    while (((READ_BIT(RCC->CR, RCC_CR_HSIRDY) == (RCC_CR_HSIRDY)))  != 1 ){
    }
    
    MODIFY_REG(RCC->CR, RCC_CR_HSITRIM, 16 << RCC_CR_HSITRIM_Pos);
    MODIFY_REG(RCC->CFGR, RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL, RCC_PLLSOURCE_HSI | RCC_PLL_MUL_12);
    MODIFY_REG(RCC->CFGR2, RCC_CFGR2_PREDIV, RCC_PREDIV_DIV_2);

    SET_BIT(RCC->CR, RCC_CR_PLLON);

    while(((READ_BIT(RCC->CR, RCC_CR_PLLRDY) == (RCC_CR_PLLRDY))) != 1){
    }

    MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, 0);
    MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE, 0);
    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_SYS_CLKSOURCE_PLL);

    while((uint32_t)(READ_BIT(RCC->CFGR, RCC_CFGR_SWS)) != RCC_SYS_CLKSOURCE_STATUS_PLL){

    }
    SystemCoreClock = 48000000;
}


int main(void){
    SystemClock_Config();
    for(;;){

    }
}
