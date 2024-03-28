#define STM32F091xC
#include <stm32f0xx.h>
#include "mci_clock.h"

#define DEBUG

// This is a simple macro to print debug messages of DEBUG is defined
#ifdef DEBUG
  #define LOG( msg... ) printf( msg );
#else
  #define LOG( msg... ) ;
#endif

// Select the Baudrate for the UART
#define BAUDRATE 9600


// For supporting printf function we override the _write function to redirect the output to UART
int _write( int handle, char* data, int size ) {
    int count = size;
    while( count-- ) {
        while( !( USART2->ISR & USART_ISR_TXE ) ) {};
        USART2->TDR = *data++;
    }
    return size;
}

int main(void){
    // Configure the system clock to 48MHz
    EPL_SystemClock_Config();

    // Enable peripheral  GPIOA clock
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    // Enable peripheral  USART2 clock
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    // Configure PA2 as USART2_TX using alternate function 1
    GPIOA->MODER |= GPIO_MODER_MODER2_1;
    GPIOA->AFR[0] |= 0b0001 << (4*2);


    // Configure PA3 as USART2_RX using alternate function 1
    GPIOA->MODER |= GPIO_MODER_MODER3_1;
    GPIOA->AFR[0] |= 0b0001 << (4*3);

    // Configure the UART Baude rate Register 
    USART2->BRR = (APB_FREQ / BAUDRATE);
    // Enable the UART using the CR1 register
    USART2->CR1 |= ( USART_CR1_RE | USART_CR1_TE | USART_CR1_UE );


    uint8_t rxb = '\0';

    for(;;){
    // Wait for the data to be received
    while( !( USART2->ISR & USART_ISR_RXNE ) ){

    };
    // Read the data from the RX buffer
    rxb = USART2->RDR;
    
    // Print the data to the console using the LOG macro which is defined above and calls printf which uses _write, which is overriden to redirect the output to UART
    LOG("[DEBUG-LOG]: %d\r\n", rxb );
    }
}
