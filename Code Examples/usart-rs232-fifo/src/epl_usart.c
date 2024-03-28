#include "main.h"
#include "epl_clock.h"
#include "epl_usart.h"



ringbuffer_handler_t *epl_tx_buffer;
ringbuffer_handler_t *epl_rx_buffer;


ringbuffer_handler_t *createFIFO(int size)
{

    ringbuffer_handler_t *buffer = 
	(ringbuffer_handler_t *)malloc(sizeof(ringbuffer_handler_t));
    
    buffer->readIndex=0;
    buffer->writeIndex=0;
    buffer->fifo = (userdata_t *)malloc(sizeof(userdata_t) * (size + 1));
    buffer->size = size;

    return buffer;
}

void appendFIFO(userdata_t data, ringbuffer_handler_t *buffer)
{

    if(buffer){

	buffer->fifo[buffer->writeIndex] = data;
	buffer->writeIndex = buffer->writeIndex++ % (buffer->size+1);

	if(buffer->readIndex == buffer->writeIndex)
	    buffer->readIndex = buffer->readIndex ++ % (buffer->size+1);
    }
}

int readFIFO(userdata_t *data, ringbuffer_handler_t *buffer)
{
    if(buffer){
	if(buffer->readIndex != buffer->writeIndex){
	    *data = buffer->fifo[buffer->readIndex];
	    buffer->readIndex = buffer->readIndex++ % (buffer->size+1);
	    return 1;
	}
	else
	    return -1;
    }
    else return -1;
}

void EPL_usart_read_n_bytes(uint8_t *data, uint16_t size, ringbuffer_handler_t *buffer)
{
    for(int i = 0; i < size; i++){
    readFIFO(&data[i], buffer);
    }
}

void EPL_usart_write_n_bytes(uint8_t *data, uint16_t size)
{
    int count = size;
    while( count-- ) {
        while( !( USART2->ISR & USART_ISR_TXE ) ) {};
        USART2->TDR = *data++;
    }
}

uint8_t EPL_init_usart(epl_usart_t *init){

    /*This is a static implementation of the USART2 peripheral*/

    // Use 4 bits for 'priority' and 0 bits for 'subpriority'.
    NVIC_SetPriorityGrouping( 0 );
    // UART receive interrupts should be high priority.
    uint32_t uart_pri_encoding = NVIC_EncodePriority( 0, 1, 0 );
    NVIC_SetPriority( USART2_IRQn, uart_pri_encoding );
    NVIC_EnableIRQ( USART2_IRQn );

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
    USART2->BRR = (APB_FREQ / init->baudrate);
    // Enable the UART using the CR1 register
    USART2->CR1 |= ( USART_CR1_RE | USART_CR1_TE | USART_CR1_UE | USART_CR1_RXNEIE);

    // Initialize the FIFO buffers
    init->rx_fifo = createFIFO(init->fifo_size);
    epl_rx_buffer = init->rx_fifo;

    return 0;
}

void USART2_IRQHandler( void ){
    if ( USART2->ISR & USART_ISR_RXNE ) {
        uint8_t c = USART2->RDR;
        appendFIFO(c, epl_rx_buffer);
    }
}

// For supporting printf function we override the _write function to redirect the output to UART
int _write( int handle, char* data, int size ) {
    int count = size;
    while( count-- ) {
        while( !( USART2->ISR & USART_ISR_TXE ) ) {};
        USART2->TDR = *data++;
    }
    return size;
}