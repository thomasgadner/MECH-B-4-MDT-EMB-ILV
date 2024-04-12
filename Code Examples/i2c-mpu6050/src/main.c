/*
----------------------------------------------------------------------------
 * Notes:
 * This project is currently under construction and has not yet been fully tested.
 * Use with caution.
 * 
 * ----------------------------------------------------------------------------
 */
#include <stm32f091xc.h>

#define SCA 9
#define SCL 8




void main(){



    I2C1->CR1 |= I2C_CR1_PE;
    I2C1->CR1 &= ~I2C_CR1_PE;

    I2C1->CR2 |= (45<<0);


}
