#include "uart.h"
#include <avr/interrupt.h>
#include <stdint.h>

static volatile uint8_t tx_buffer[TX_BUFFER_SIZE];
static volatile uint8_t tx_head;
static volatile uint8_t tx_tail;
static volatile uint8_t tx_shift;
static volatile int8_t  tx_bit;

static void uart_init(void) {
    DDRB |= (1 << PB4);
    PORTB |= (1 << PB4);
    tx_head = 0;
    tx_tail = 0;
    tx_bit  = -1;
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS11);
    OCR1A  = (11059200UL / 8 / 9600) - 1;
    TCNT1 = 0;
    TIMSK |= (1 << OCIE1A);
}

static void uart_send(uint8_t c) {
    uint8_t next = (tx_head + 1) & TX_MASK;
    while (next == tx_tail);
    tx_buffer[tx_head] = c;
    tx_head = next;
    if (tx_bit == -1)
        tx_bit = 0;
}

static void uart_hex(uint8_t value){
    char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    uart_send(hex[value >> 4]);
    uart_send(hex[value & 0x0F]);
}

ISR(TIMER1_COMPA_vect){
    if (tx_bit == -1) 
        return;
    if (tx_bit == 0) {
        PORTB &= ~(1 << PB4);
        tx_shift = tx_buffer[tx_tail];
        tx_bit++;
    } else if (tx_bit >= 1 && tx_bit <= 8) {
        if (tx_shift & 0x01) 
        	PORTB |= (1 << PB4);
        else 
        	PORTB &= ~(1 << PB4);
        tx_shift >>= 1;
        tx_bit++;
    } else if (tx_bit == 9) {
        // Stop bit
        PORTB |= (1 << PB4);
        tx_bit = -1;
        if (tx_head != tx_tail) {
            tx_tail = (tx_tail + 1) & TX_MASK;
            tx_bit = 0;
        }
    }
}

const uart_t UART = {
	.init = uart_init,
	.send = uart_send,
    .hex = uart_hex
};