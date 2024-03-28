#include "main.h"
#include "epl_clock.h"
#include "epl_usart.h"



ringbuffer_handler_t *epl_tx_buffer;
ringbuffer_handler_t *epl_rx_buffer;


ringbuffer_handler_t *createFIFO(int size)
{
    //Zeiger auf Ringbuffer-Handler deklarieren und genuegend Speicher
    //reservieren
    ringbuffer_handler_t *buffer = 
	(ringbuffer_handler_t *)malloc(sizeof(ringbuffer_handler_t));
    
    //Werte des Handler belegen
    //readIndex und WriteIndex zu Anfang 0
    //fuer fifo muss genuegend Speicher reserviert werden, 
    //um size Datenelemente aufnehmen zu koennen
    //size gibt Anzahl der Elemente im Ringbuffer an (aus Parameter)
    buffer->readIndex=0;
    buffer->writeIndex=0;
    buffer->fifo = (userdata_t *)malloc(sizeof(userdata_t) * (size + 1));
    buffer->size = size;

    //Zeiger auf Ringbuffer-Handler zurueckgeben
    return buffer;
}


void appendFIFO(userdata_t data, ringbuffer_handler_t *buffer)
{
    //Ringbuffer-Handler muss gueltig sein
    if(buffer)
    {
	//Daten in den entsprechenden Platz im Buffer schreiben
	//dafuer ist writeIndex da
	buffer->fifo[buffer->writeIndex] = data;
	//writeIndex wird fuer das naechste Schreiben auf den neuen Wert
	//gesetzt
	//Achtung: wenn der Buffer voll ist, wird wieder vorne hineinge-
	//schrieben (deshalb RINGbuffer), Daten koennen verloren gehen,
	//wenn nicht zwischendurch gelesen wird
	buffer->writeIndex = buffer->writeIndex++ % (buffer->size+1);
	//an dieser Stelle ist es deshalb sinnvoll, den readIndex auf den
	//letzen wahren Wert zu setzen, wenn NICHT zwischendurch gelesen
	//wurde
	if(buffer->readIndex == buffer->writeIndex)
	    buffer->readIndex = buffer->readIndex ++ % (buffer->size+1);
    }
}

int readFIFO(userdata_t *data, ringbuffer_handler_t *buffer)
{
    //Ringbuffer-Handler muss gueltig sein
    if(buffer)
    {
	//der writeIndex zeigt immer auf das naechste zu beschreibende Element,
	//d.h. dieses Element ist noch nicht beschrieben worden und enthaelt
	//deshalb keine gueltigen Daten
	//wenn readIndex also gleich writeIndex, darf nicht gelesen werden
	if(buffer->readIndex != buffer->writeIndex)
	{
	    //Daten kopieren
	    *data = buffer->fifo[buffer->readIndex];
	    //readIndex fuer das naechste Lesen hochsetzen
	    buffer->readIndex = buffer->readIndex++ % (buffer->size+1);
	    //Rueckgabewert 1, da gelesen wurde
	    return 1;
	}
	else
	    //es konnten keine Daten gelesen werden, da keine gueltigen Daten
	    //Rueckgabewert also -1
	    return -1;
    }
    //es konnte nicht gelesen werden, da Buffer-Handler nicht gueltig, 
    //Ruckegabewert also -1
    else return -1;
}


uint8_t EPL_init_usart(epl_usart_t *init){

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
    USART2->CR1 |= ( USART_CR1_RE | USART_CR1_TE | USART_CR1_UE | USART_CR1_RXNEIE );

    // Initialize the FIFO buffers
    init->rx_fifo = createFIFO(20);
    epl_rx_buffer = init->rx_fifo;

    init->tx_fifo = createFIFO(20);
    epl_tx_buffer = init->tx_fifo;

    return 0;
}


void USART2_IRQn_handler( void ){
    if ( USART2->ISR & USART_ISR_RXNE ) {
        uint8_t c = USART2->RDR;
        appendFIFO(c, epl_rx_buffer);
    }
}






uint8_t epl_usart_asc_read(uint8_t *fifo, uint8_t rec, uint8_t* count, uint32_t timeout){

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