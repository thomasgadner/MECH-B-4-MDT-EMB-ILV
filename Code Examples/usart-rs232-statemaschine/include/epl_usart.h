#ifndef EPL_USART_H_
#define EPL_USART_H_
#include <string.h>



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
    ringbuffer_handler_t *rx_fifo;
    ringbuffer_handler_t *tx_fifo;
}epl_usart_t;



#ifdef DEBUG
  #define LOG( msg... ) printf( msg );
#else
  #define LOG( msg... ) ;
#endif





#endif /* EPL_USART_H_ */
