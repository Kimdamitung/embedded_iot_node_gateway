#include <stdio.h>
#define F_CPU 11095200UL
#include <util/delay.h>
#include <avr/io.h>	

// Struct Bits LSB UART


int main(int argc, char const *argv[]){
	/* code */
	DDRD |= (1 << DD6);
	// DDRB |== 
	while (1) {
		PORTD |= (1 << DD6);
		_delay_ms(1000);
		PORTD &= ~(1 << DD6);
		_delay_ms(1000);
	}
	return 0;
}