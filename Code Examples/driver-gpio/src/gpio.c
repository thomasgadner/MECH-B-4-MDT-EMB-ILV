#include "gpio.h"
#include <stm32f091xc.h>



int GPIO_Init(GPIO_Handle_t* pGPIOHandle){
    uint32_t mask = 0;

    if (pGPIOHandle->pGPIOx == GPIOA){
            RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    }
    // ... Dot the Same 


    mask = (pGPIOHandle->GPIO_PinConfig.pin_mode << (2 * pGPIOHandle->GPIO_PinConfig.pin_number));
    pGPIOHandle->pGPIOx->MODER &= ~(0x3 << pGPIOHandle->GPIO_PinConfig.pin_number*2);
    pGPIOHandle->pGPIOx->MODER |= mask;
    mask = 0;

    // ... Follow scheme for


	if (pGPIOHandle->GPIO_PinConfig.pin_mode == GPIO_ALTFN_MODE) {
		// configure the alternate function registers
		uint8_t temp1 , temp2 ;

		temp1 = pGPIOHandle->GPIO_PinConfig.pin_number  / 8 ;
		temp2 = pGPIOHandle->GPIO_PinConfig.pin_number % 8 ;
		pGPIOHandle->pGPIOx->AFR[temp1] &= ~(0xF << (4 * temp2)) ;
		pGPIOHandle->pGPIOx->AFR[temp1] |= (pGPIOHandle->GPIO_PinConfig.pin_alt_fun_mode << (4 * temp2)) ;
	}
}

int GPIO_Write(GPIO_TypeDef *pGPIOx, uint32_t pin, uint32_t val){
    if (val == PIN_SET){
        pGPIOx->ODR |= (0x1 << pin);
    } else if(val = PIN_RESET){
		pGPIOx->ODR &= ~(1 << pin) ;
    }
}
int GPIO_Read(GPIO_TypeDef *pGPIOx, uint32_t pin){
    //... follow GPIO_WRITE
}








