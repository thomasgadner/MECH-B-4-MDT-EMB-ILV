#include "gpio.h"
#include <stm32f091xc.h>




/* Interrupt dispatcher for the GPIO Driver. The STM32 is limited to 16 GPIO Interrupts */
gpio_callback gpio_callback_pinx[16];



int GPIO_Init(GPIO_Handle_t* pGPIOHandle){
    uint32_t mask = 0;

    if (pGPIOHandle->pGPIOx == GPIOA){
        RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    }
    else if (pGPIOHandle->pGPIOx == GPIOB){
        RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    }

    /*TODO: Expand for all the other Ports*/
    
    if (pGPIOHandle->GPIO_PinConfig.pin_mode <= GPIO_ANALOG_MODE){ 
        mask = (pGPIOHandle->GPIO_PinConfig.pin_mode << (2 * pGPIOHandle->GPIO_PinConfig.pin_number));
        pGPIOHandle->pGPIOx->MODER &= ~(0x3 << pGPIOHandle->GPIO_PinConfig.pin_number*2);
        pGPIOHandle->pGPIOx->MODER |= mask;
        mask = 0;
    } else{
        //Init for Interrupt Mode. 

        // Attach callback functions e.g:

        gpio_callback_pinx[pGPIOHandle->GPIO_PinConfig.pin_number] = pGPIOHandle->GPIO_InterruptConfig.gpio_callback;

    }

    /* TODO: Continue with the rest of the configuration for speed pupd type ... on your own*/

    /*Check the Examples on I2C and UART to understand the Configuration*/
	if (pGPIOHandle->GPIO_PinConfig.pin_mode == GPIO_ALTFN_MODE) {

		uint8_t temp1 , temp2 ;

		temp1 = pGPIOHandle->GPIO_PinConfig.pin_number  / 8 ;
		temp2 = pGPIOHandle->GPIO_PinConfig.pin_number % 8 ;
		pGPIOHandle->pGPIOx->AFR[temp1] &= ~(0xF << (4 * temp2)) ;
		pGPIOHandle->pGPIOx->AFR[temp1] |= (pGPIOHandle->GPIO_PinConfig.pin_alt_fun_mode << (4 * temp2)) ;
	}

    return 0;
}

int GPIO_Write(GPIO_Handle_t *GPIO_Handle, uint8_t value){
    if (value == PIN_SET){
        GPIO_Handle->pGPIOx->ODR |= (0x1 << GPIO_Handle->GPIO_PinConfig.pin_number);
    } else if(value == PIN_RESET){
		GPIO_Handle->pGPIOx->ODR &= ~(1 << GPIO_Handle->GPIO_PinConfig.pin_number) ;
    }

    return 0;
}
int GPIO_Read(GPIO_Handle_t *GPIO_Handle, uint8_t *value){
    /*TODO: Program the Read Function with the example of the write function in mind.*/

    return 0;
}

void EXTI4_15_IRQHandler(void)
{
  if (EXTI->PR & (1 << PIN_0)) {
        gpio_callback_pinx[PIN_0](PIN_0);
  }
  /*TODO: Expand this for all Pins - Have a look at the gpio-Interrupt example*/
}






