#include <stm32f091xc.h>
typedef struct {
    GPIO_TypeDef *port; // GPIO port
    I2C_TypeDef *i2c;   // I2C peripheral
    uint8_t sca;        // SCA pin
    uint8_t scl;        // SCL pin

} epl_i2c_Config_t;



void epl_i2c_init(epl_i2c_Config_t *i2c){
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

    i2c->port->MODER |= GPIO_MODER_MODER0_1 << (i2c->sca * 2);
    i2c->port->MODER |= GPIO_MODER_MODER0_1 << (SCL * 2);

    i2c->port->OTYPER |= GPIO_OTYPER_OT_0 << SCA;
    i2c->port->OTYPER |= GPIO_OTYPER_OT_0 << SCL;

    i2c->port->OSPEEDR |= 0b11 << (SCA * 2);
    i2c->port->OSPEEDR |= 0b11 << (SCL * 2);


    GPIOB->PUPDR |= GPIO_PUPDR_PUPDR0_0 << (SCA * 2);
    GPIOB->PUPDR |= GPIO_PUPDR_PUPDR0_0 << (SCL * 2);

    if()

    i2c->port->AFR[1] |= 0b0100 << (4* i2c->sca - 32);
    i2c->port->AFR[1] |= 0b0100 << (4* i2c->scl - 32);


}

void epl_i2c_start(){

}


void epl_i2c_stop(){

}


void epl_i2c_write(){

}


void epl_i2c_read(){

}


void epl_i2c_init(){

}

void