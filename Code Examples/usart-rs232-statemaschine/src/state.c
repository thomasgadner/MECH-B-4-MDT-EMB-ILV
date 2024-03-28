#define SERIAL_BAUDRATE         115200

#include "state.h"
#include "epl_usart.h"



#define ASC_TX_BUFFER_SIZE      64
#define ASC_RX_BUFFER_SIZE      64


serial_command Serial_command;

/* Global variables */
#define BUFFER_SIZE 20


uint8_t  data_buffer[BUFFER_SIZE];
uint32_t data_id;        // Data ID
uint32_t data_length;    // Data Length
uint32_t data_crc;       // Data Length


epl_usart_t *my_usart2;


/* State machine states */
static void wait_for_start(); 
static void parse_id();      
static void parse_data();     
static void parse_end();     

void (*parse_next)(void) = wait_for_start;




uint8_t epl_init_standard_comm(void){

    my_usart2->baudrate = 9600;
    EPL_init_usart(my_usart2);


    return 0;
}




uint8_t uart_readchar(){
    uint8_t ret = 0;
    uint8_t rec = 0;
    ret = readFIFO(&rec, my_usart2->rx_fifo);
    if (ret != 1){
        return 0;
    }
    return rec;
}


uint8_t uart_canread(){
 //guess we need to change this
};

uint8_t send_UART_message(serial_command *instruction)
{
    static uint8_t txData[15] = {'#','d','a','t',0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,'$'};
    uint8_t data[10];

    memcpy(&txData[4], &data[0],sizeof(data));

    return 0;
}


uint8_t parse_UART_data(void){
    if (parse_next != 0x0){ // Check for NULL pointer
        parse_next(); // Call the state function
    }
    return 0;
}


void wait_for_start(void) {
    /* If we do not have data available, we return */

    uint8_t c = uart_readchar();
    if(c == 0){
        return;
    }

    /* If the data is start character, we update the state */
    if (c == '#'){
        /* Initialize data variables, just in case */
        data_id = 0;
        data_length = 10;
        parse_next = parse_id; // Next we parse the [ID] field
    }
}
/* Reads the 3 char identifier, stores it in data_id */
void parse_id(void) {
    static uint8_t count=0; // Counts the received data bytes

    /* If we do not have data available, we return */
    if (!uart_canread()){ return; }

    /* Add the character and move to the left
       At the end we end up with integer containing the ID from left to right. Last byte zero */
    data_id += uart_readchar();
    data_id <<= 8;
    count++;

    /* State transition rule */
    if (count == 3){
        count = 0; // reset counter
        parse_next = parse_data; // Next data is the LENGTH field
    }
}
/******************************************************************************/
/* Reads the data into data_buffer[] */
void parse_data(void) {
    static uint8_t count=0; // Counts the received data bytes

    /* If we do not have data available, we return */
    if (!uart_canread()){ return; }

    /* Add received byte to the data_length variable.
       The bytes arrive in Big Endian order. */
    data_buffer[count] = uart_readchar();
    count++;

    /* If we run out of buffer memory, we cant handle that packet.
       Only thing we can do is try again with the next packet */
    if (count > BUFFER_SIZE) {
        count = 0;
        parse_next = wait_for_start; // Wait for the next packet
    }

    /* State transition rule */
    if (count == data_length){
        count = 0; // reset counter
        parse_next = parse_end; // Next data is the LENGTH field
        return;
    }
}

uint8_t process_serial_command(uint32_t id){
    // We put the received command into the desired structure
    Serial_command.identifier = id;
    // Always correct somehow
    return 0;
}



/* End of data packet parsing, handle data and start all over. */
void parse_end(void) {
    /* If we do not have data available, we return */
    if (!uart_canread()){ return; }

    /* If the data is not the end character, we bail out. */
    uint8_t test = uart_readchar();

    if (test != '$'){
        parse_next = wait_for_start; // Start all over
        return;
    }

    /* Handle data */
    if (data_id == IDENTIFIER('r','u','n',0)) {
        /* Call the function that handles this type of package */
        process_serial_command(IDENTIFIER('r','u','n',0));
    }
    else if (data_id == IDENTIFIER('b','r','k',0)) {
        /* Call the function that handles this type of package */
        process_serial_command(IDENTIFIER('b','r','k',0));
    }
    else if (data_id == IDENTIFIER('c','t','l',0)) {
        /* Call the function that handles this type of package */
        process_serial_command(IDENTIFIER('c','t','l',0));

    }
    parse_next = wait_for_start; // Start all over
}