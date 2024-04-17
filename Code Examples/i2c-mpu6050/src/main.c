#include <stm32f091xc.h>
#include "mci.h"
#include <stdio.h>


// This is a simple macro to print debug messages of DEBUG is defined
#define DEBUG
#ifdef DEBUG
  #define LOG( msg... ) printf( msg );
#else
  #define LOG( msg... ) ;
#endif


// Define GPIO PINs for I2C
#define PIN_SDA 9
#define PIN_SCL 8


// The MPU Address is a 7 bit address + 1 bit for read/write
#define MPU6050_BASE_ADDR 0x68
#define MPU6050_ADDR MPU6050_BASE_ADDR << 1


// MPU Specific Registers - Please refer to the Datasheet for more information
#define SMPLRT_DIV_REG 0x19
#define GYRO_CONFIG_REG 0x1B
#define ACCEL_CONFIG_REG 0x1C
#define GYRO_XOUT_H_REG 0x43
#define PWR_MGMT_1_REG 0x6B
#define WHO_AM_I_REG 0x75



/**
 * @brief Writes data to a target device via I2C communication.
 *
 * This function writes data to a target device using the I2C protocol. It configures the I2C controller,
 * sets the target address, sends the data, and waits for the transmission to complete.
 *
 * @param targetAddr The 7-bit address of the target device.
 * @param buf Pointer to the buffer containing the data to be sent.
 * @param len Number of bytes to send.
 * @return 0 on success.
 */
int8_t I2C_Controller_Write(uint8_t targetAddr, uint8_t* buf, size_t len){

    // Reset the CR2 Control Register
    I2C1->CR2 = 0x00000000;

    // Check if the I2C bus is busy
    while((I2C1->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);

    // Set the target address and number of bytes to send
    I2C1->CR2 |= (targetAddr + 0x0) << I2C_CR2_SADD_Pos;
    I2C1->CR2 |= (len) << I2C_CR2_NBYTES_Pos; 

    // Start the transmission
    I2C1->CR2 |= I2C_CR2_START;
    
    // Send the data
    for(uint8_t i = 0; i < len; i++){
        if ((I2C1->ISR & I2C_ISR_TXE) == (I2C_ISR_TXE)){
            I2C1->TXDR = buf[i];
        }
        // Wait for the TXE to complete to send the next byte
        while(!((I2C1->ISR & I2C_ISR_TXE) == (I2C_ISR_TXE)));
    }

    // Check if the transmission is complete
    while(!((I2C1->ISR & I2C_ISR_TC) == (I2C_ISR_TC))); 

    // Stop the transmission
    I2C1->CR2 |= I2C_CR2_STOP;

    // Wait for the bus to be free
    while((I2C1->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);
    I2C1->ICR |= I2C_ICR_STOPCF;

    return 0;
}

/**
 * @brief Reads data from a target device via I2C communication.
 *
 * This function reads data from a target device using the I2C protocol. It configures the I2C controller,
 * sets the target address, sends the register address to read from, receives the data, and waits for the
 * reception to complete.
 *
 * @param targetAddr The 7-bit address of the target device.
 * @param reg The register address to read from.
 * @param data Pointer to the buffer where the received data will be stored.
 * @param len Number of bytes to read.
 * @return 0 on success.
 */
int8_t I2C_Target_Read(uint8_t targetAddr, uint8_t reg, uint8_t *data, size_t len){

    uint8_t count = 0;

    // Reset the CR2 Control Register
    I2C1->CR2 = 0x00000000;
    //Set the target address and number of bytes to read
    I2C1->CR2 |= (targetAddr) << I2C_CR2_SADD_Pos;
    I2C1->CR2 |= 1 << I2C_CR2_NBYTES_Pos; 

    // Start the transmission
    I2C1->CR2 |= I2C_CR2_START; 

    // Send the register address to read from
    while(!((I2C1->ISR & I2C_ISR_TXE) == (I2C_ISR_TXE))); 
    I2C1->TXDR = (uint8_t)reg;
    while(!((I2C1->ISR & I2C_ISR_TC) == (I2C_ISR_TC))); 

    // Reset the CR2 Control Register
    I2C1->CR2 = 0x00000000;

    // Set the target address and number of bytes to read, and set the read bit
    I2C1->CR2 |= (targetAddr + 0x1) << I2C_CR2_SADD_Pos;
    I2C1->CR2 |= (len)<<I2C_CR2_NBYTES_Pos; 
    // Put the controller in read mode
    I2C1->CR2 |= (I2C_CR2_RD_WRN); 
    // Enable automatic end mode
    I2C1->CR2 |= I2C_CR2_AUTOEND; 
 
    // Start the transmission
    I2C1->CR2 |= I2C_CR2_START; 
    
    // Receive the data
    while(count < len){
        while(!((I2C1->ISR & I2C_ISR_RXNE) == (I2C_ISR_RXNE)));
        data[count++]= I2C1->RXDR;
    }

    // Autoend will stop the transmission

    return 0;
}


/**
 * @brief Writes data to a register of the MPU6050 sensor via I2C communication.
 *
 * This function writes data to a specific register of the MPU6050 sensor using the I2C protocol. It constructs
 * a buffer containing the register address and data, then calls the I2C_Controller_Write function to perform
 * the actual write operation.
 *
 * @param targetAddr The 7-bit address of the MPU6050 sensor.
 * @param reg The register address to write to.
 * @param data The data to write into the specified register.
 */
void MPU6050_Write (uint8_t targetAddr, uint8_t reg, uint8_t data){
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = data;
    I2C_Controller_Write(targetAddr, buf, 2);
}


/**
 * @brief Reads data from a register of the MPU6050 sensor via I2C communication.
 *
 * This function reads data from a specific register of the MPU6050 sensor using the I2C protocol. It calls the
 * I2C_Target_Read function to perform the read operation.
 *
 * @param targetAddr The 7-bit address of the MPU6050 sensor.
 * @param reg The register address to read from.
 * @param buffer Pointer to the buffer where the received data will be stored.
 * @param size Number of bytes to read.
 */
void MPU6050_Read (uint8_t targetAddr, uint8_t reg, uint8_t *buffer, uint8_t size){
    I2C_Target_Read(targetAddr, reg, buffer, size);
}


/**
 * @brief Configures the I2C peripheral and GPIO pins for I2C communication.
 *
 * This function configures the I2C peripheral and GPIO pins for I2C communication. It enables the necessary
 * GPIO and I2C clock, sets the pin modes, types, speeds, and pull-up resistors. Additionally, it configures
 * the GPIO alternate function registers and sets the I2C timing register. Finally, it enables the I2C peripheral.
 */
void I2C_config(){

    // Enable the GPIOB and I2C1 clock
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    // Set the GPIO pin modes to alternate function
    GPIOB->MODER |= 0b10 << (PIN_SDA<<1);
    GPIOB->MODER |= 0b10 << (PIN_SCL<<1); 

    // I2C Needs open-drain
    GPIOB->OTYPER |=  1 << (PIN_SDA) | 1 << (PIN_SCL);

    // GPIO Needs to be high speed
    GPIOB->OSPEEDR |= 0b11 << (PIN_SDA << 1);
    GPIOB->OSPEEDR |= 0b11 << (PIN_SCL << 1);


    // Enable pull-up resistors
    GPIOB->PUPDR |= 0b01 << (PIN_SDA << 1);
    GPIOB->PUPDR |= 0b01 << (PIN_SCL << 1);

    // Set the GPIO pins to alternate function 1
    GPIOB->AFR[PIN_SDA / 8] |= 0b0001 << ((PIN_SDA % 8) * 4); 
    GPIOB->AFR[PIN_SCL / 8] |= 0b0001 << ((PIN_SCL % 8) * 4); 

    // Set the I2C timing register - please refer to the datasheet for more information
    I2C1->TIMINGR =  (uint32_t)0x00B01A4B;
    // Enable the I2C peripheral
    I2C1->CR1 |= I2C_CR1_PE; //enable peripheral

}



/**
 * @brief Initializes the MPU6050 sensor.
 *
 * This function initializes the MPU6050 sensor by performing the following steps:
 * - Reads the WHO_AM_I register to verify communication with the sensor.
 * - Waits for a short delay.
 * - If communication is successful (sensor's WHO_AM_I value is 104), configures the sensor:
 *   - Sets the power management register to wake the sensor up.
 *   - Sets the data rate to 1kHz.
 *   - Sets the accelerometer configuration to 2g.
 *   - Sets the gyroscopic configuration to 250°/s.
 *
 * @note This function assumes that the APB_FREQ and other register values/constants are defined elsewhere.
 */
void MPU6050_Init(){
    uint8_t check[] = {0x00};
	uint8_t Data;

    // Read the WHO_AM_I register to verify communication with the sensor
    MPU6050_Read(MPU6050_ADDR, 0x75, check, 1);

    // Give the sensor some time
    EPL_delay(APB_FREQ/100);

    // Check if the sensor responded with the correct value
    if(check[0] == 104){

    	// Wake up the sensor by writing all 0's to the power management register
    	Data = 0;
    	MPU6050_Write(MPU6050_ADDR, PWR_MGMT_1_REG, Data);
    	// Set the data rate to 1kHz by writing the SMPLRT_DIV register
    	Data = 0x07;
    	MPU6050_Write(MPU6050_ADDR, SMPLRT_DIV_REG, Data);
    	// Set accelerometer configuration in ACCEL_CONFIG Register
    	Data = 0x00;
    	MPU6050_Write(MPU6050_ADDR, ACCEL_CONFIG_REG, Data);
    	// Set gyroscopic configuration in GYRO_CONFIG Register
    	Data = 0x00;
    	MPU6050_Write(MPU6050_ADDR, GYRO_CONFIG_REG, Data);
    }
}



int main(void){

    // Initialize the system clock and debug UART
    EPL_SystemClock_Config();
    EPL_init_Debug_UART();

    // Configure the I2C peripheral
    I2C_config();

    // Initialize the MPU6050 sensor
    MPU6050_Init();


    int16_t GX = 0;
    int16_t GY = 0;
    int16_t GZ = 0;

    uint8_t rx_data[6];

    // Main loop
    for(;;){

        // Read the gyroscope data
	    MPU6050_Read(MPU6050_ADDR, GYRO_XOUT_H_REG , rx_data, 6);

        // Convert the raw data to 16-bit values
        GX = (int16_t)(rx_data[0] << 8 | rx_data [1]);
	    GY = (int16_t)(rx_data[2] << 8 | rx_data [3]);
	    GZ = (int16_t)(rx_data[4] << 8 | rx_data [5]);

        // Send the data to the debug UART
        LOG("[DEBUG-LOG] AX: %d | AY: %d | AZ: %d |  \r\n", GX,GY,GZ );

        // Wait for a short delay
        EPL_delay(APB_FREQ/1000);

    }

    return 0;
}

