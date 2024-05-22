#ifndef EPL_USART_H_
#define EPL_USART_H_
#include <string.h>
#include <stdlib.h>



typedef uint8_t userdata_t;

typedef struct
{
    //Index zum Lesen
    int readIndex;
    //Index zum Schreiben
    int writeIndex;
    //Platz fuer Speicherelemente, eigentlicher Buffer
    userdata_t *fifo;
    //Groesse des Buffers, d.h. Anzahl der Elemente
    int size;
} ringbuffer_handler_t;



typedef struct 
{
    uint32_t baudrate;
    uint8_t fifo_size;
    ringbuffer_handler_t *rx_fifo;
}epl_usart_t;


void EPL_usart_write_n_bytes(uint8_t *data, uint16_t size);
uint8_t EPL_init_usart(epl_usart_t *init);
void EPL_usart_read_n_bytes(uint8_t *data, uint16_t size, ringbuffer_handler_t *buffer);

#ifdef DEBUG
  #define LOG( msg... ) printf( msg );
#else
  #define LOG( msg... ) ;
#endif





#endif /* EPL_USART_H_ */
