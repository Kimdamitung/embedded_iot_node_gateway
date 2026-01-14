#include "usart.h"

static void usart_init(void){
	unsigned int baudrate = 9600;
    UBRRH = 0;
    UBRRL = (11059200UL / 16 / baudrate) - 1;
    UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
    UCSRC = (1 << UCSZ1) | (1 << UCSZ0); 
}

static void usart_transmit(unsigned char c){
    while (!( UCSRA & (1 << UDRE)));
    UDR = c;
}


static unsigned char usart_receive(void){
    while (!(UCSRA & (1 << RXC)));
    return UDR;
}

ISR(USART_RX_vect){
    // uint8_t c = UDR;
}

const usart_t USART = {
	.init = usart_init,
	.transmit = usart_transmit,
	.receive = usart_receive
};