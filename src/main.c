#include <stdint.h>
#include <stdio.h>
#define F_CPU 11059200UL
#include <util/delay.h>
#include <avr/io.h>	
#include <avr/interrupt.h>

// Struct Bits LSB UART
/*
IDLE	START	DATA	PARTY	STOP
				LSB
9600	104us
Tick = F_CPU / 8 = 1,382,400 Hz
OCR0A = 1,382,400 / 9600 − 1 ≈ 143
*/

volatile uint8_t tx_shift;
volatile int8_t  tx_bit = -1;

void uart_bitbang_init(){
	DDRB |= (1 << PB4);
	PORTB |= (1 << PB4);
	TCCR0A = (1 << WGM01);
	TCCR0B = (1 << CS01);
	OCR0A = (F_CPU / 8 / 9600) - 1;
}

ISR(TIMER0_COMPA_vect){
	// start bits
	if(tx_bit == -1){
		PORTB &= ~(1 << PB4);
		tx_bit = 0;
	}else if(tx_bit < 8){ //data bit LSB
		if(tx_shift & 1)
			PORTB |= (1 << PB4);
		else 
			PORTB &= ~(1 << PB4);
		tx_shift >>= 1;
		tx_bit++;
	}else if(tx_bit == 8){ // stop bitss
		PORTB |= (1 << PB4);
		tx_bit++;
	}else{
		TIMSK &= ~(1 << OCIE0A);
		tx_bit = -1;
	}
}

void uart_send_8n1(uint8_t character){
	while (tx_bit != -1);
	tx_shift = character;
	TCNT0 = 0; 
	TIMSK |= (1 << OCIE0A); 
}

// usart 

void usart_init(unsigned int baudrate){
	UBRRH = (unsigned char)(baudrate >> 8);
	UBRRL = (unsigned char)baudrate;
	/* Enable receiver and transmitter */
	UCSRB = (1 << RXEN ) | (1 << TXEN);
	/* Set frame format: 8data, 2stop bit */
	UCSRC = (1 << USBS ) | (3 << UCSZ0);
}

void USART_Transmit( unsigned char data ){
	while (!( UCSRA & (1 << UDRE)));	
	/* Put data into buffer, sends the data */
	UDR = data;
}

unsigned char USART_Receive(void){
	/* Wait for data to be received */
	while ( !(UCSRA & (1 << RXC)) );
	/* Get and return received data from buffer */
	return UDR;
}

int main(void){
	/* code */
	DDRD |= (1 << DD6);
	uart_bitbang_init();
	sei();
	usart_init((unsigned int)115200);
	while (1) {
		PORTD ^= (1 << DD6);
		if (UCSRA & (1 << RXC)) {  
            uint8_t c = USART_Receive();
            uart_send_8n1(c);
        }
		_delay_ms(1000);
	}
	return 0;
}