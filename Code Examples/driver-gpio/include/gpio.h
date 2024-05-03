#ifndef STM32_GPIO_H_
/**
 * @file stm32_gpio.h
 * @brief Header file for STM32 GPIO operations.
 *
 * This file provides declarations for STM32 GPIO operations.
 * It defines macros, structs, and function prototypes related to GPIO.
 *
 * @note This header file should be included in any source file that
 * needs to access GPIO functionality on STM32 microcontrollers.
 *
 * @details
 * The inclusion guard `STM32_GPIO_H_` is used to prevent multiple inclusions
 * of this header file within the same translation unit. If this file is included
 * multiple times in the same translation unit, it can lead to duplicate symbol
 * definitions and compilation errors. When a source file includes this header,
 * the preprocessor checks whether `STM32_GPIO_H_` is already defined. If it's not
 * defined, the preprocessor defines it and proceeds with including the contents
 * of this file. If it's already defined, the preprocessor skips the contents of
 * this file, preventing duplicate definitions.
 */
#define STM32_GPIO_H_

#include <main.h> 


/* TODO: Expand this list to include all options */
#define GPIO_INPUT_MODE  0x0
#define GPIO_OUTPUT_MODE 0x1
#define GPIO_ALTFN_MODE  0x2
#define GPIO_ANALOG_MODE 0x3


/* TODO: Expand this list to include all options */
#define PIN_0 0



#define PIN_SET 1
#define PIN_RESET 0



typedef void (*gpio_callback)(uint8_t index);

typedef struct {
	uint8_t pin_number;
	uint8_t pin_mode;
	uint8_t pin_speed ;
	uint8_t pin_pupdc;
	uint8_t pin_opt;
	uint8_t pin_alt_fun_mode;
} PinConfig_t;


/* This driver does not allow for dynamic callback assignments */
typedef struct gpio{
	uint8_t interuppt_enable;
	uint8_t interrupt_mode;
	gpio_callback gpio_callback;
}InterruptConfig_t;


typedef struct {
	GPIO_TypeDef *pGPIOx; 
	PinConfig_t GPIO_PinConfig;
	InterruptConfig_t GPIO_InterruptConfig; 
} GPIO_Handle_t;



/**
 * @brief Initializes the GPIO peripheral.
 *
 * This function initializes the GPIO peripheral specified by the given GPIO handle.
 *
 * @param[in] GPIO_Handle Pointer to the GPIO handle structure.
 * @return Status code indicating the success or failure of the initialization.
 *         - 0: Initialization successful.
 *         - Other values: Initialization failed.
 */
int GPIO_Init(GPIO_Handle_t* GPIO_Handle);

/**
 * @brief Writes a value to a GPIO pin.
 *
 * This function writes the specified value to the GPIO pin associated with the given GPIO_Handle.
 *
 * @param GPIO_Handle Pointer to the GPIO_Handle_t structure.
 * @param value The value to be written to the GPIO pin.
 * @return Returns 0 on success, or a negative error code on failure.
 */
int GPIO_Write(GPIO_Handle_t *GPIO_Handle, uint8_t value);

/**
 * @brief Reads the value of a GPIO pin.
 *
 * This function reads the value of the GPIO pin associated with the given GPIO_Handle and stores it in the provided value pointer.
 *
 * @param GPIO_Handle Pointer to the GPIO_Handle_t structure.
 * @param value Pointer to a variable where the read value will be stored.
 * @return Returns 0 on success, or a negative error code on failure.
 */
int GPIO_Read(GPIO_Handle_t *GPIO_Handle, uint8_t *value);




#endif /* STM32_GPIO_H_ */
