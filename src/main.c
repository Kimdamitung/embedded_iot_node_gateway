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

int main(void){
	/* code */
	DDRD |= (1 << DD6);
	uart_bitbang_init();
	sei();
	while (1) {
		PORTD |= (1 << DD6);
		uart_send_8n1('A');
		_delay_ms(1000);
		PORTD &= ~(1 << DD6);
		uart_send_8n1('F');
		_delay_ms(1000);
	}
	return 0;
}