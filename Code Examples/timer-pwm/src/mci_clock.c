#include "main.h"
#include "mci_clock.h"



#define SET_BIT(REG, BIT)     ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT)    ((REG) & (BIT))
#define CLEAR_REG(REG)        ((REG) = (0x0))
#define WRITE_REG(REG, VAL)   ((REG) = (VAL))
#define READ_REG(REG)         ((REG))
#define MODIFY_REG(REG, CLEARMASK, SETMASK)  WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))

void SystemClock_Config(void){

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

