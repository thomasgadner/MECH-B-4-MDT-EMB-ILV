#ifndef EPL_STATE_H_
#define EPL_STATE_H_


#define STM32F091xC
#include <stm32f0xx.h>


/* This macro combines four bytes into one 32 bit unsigned integer */









#define IDENTIFIER(a,b,c,d)    ((((uint32_t)a)<<(3*8)) + \
                                (((uint32_t)b)<<(2*8)) + \
                                (((uint32_t)c)<<(1*8)) + \
                                (((uint32_t)d)<<(0*8)))

typedef struct parameters {
    uint16_t param1;
    uint16_t param2;
    uint16_t param3;
    uint16_t param4;
    uint16_t param5;
} parameters;


typedef struct {
        uint32_t identifier;
        parameters param;
}serial_command;




uint8_t epl_init_standard_comm(void);


uint8_t epl_send_UART_message(serial_command *instruction);
uint8_t epl_read_UART_message(serial_command *instruction);
uint8_t epl_parse_UART_data(void);



#endif /* EPL_SERIAL_H_ */