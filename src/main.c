#include <stdint.h>
#define F_CPU 11059200UL
#include <util/delay.h>
#include <avr/io.h>
#include "uart.h"
#include "usart.h"
#include "ds1302.h"

void  show_and_send_ds1302(uint8_t data);

int main(int argc, char const *argv[]){
    /* code */
    DDRB |= (1 << PB0) | (1 << PB1);
    UART.init();
    USART.init();
    sei();
    DS1302.init();
    DS1302.reset();
    time_t time = {
        .hours = 23,
        .minutes = 59,
        .seconds = 00
    };
    DS1302.set_time(time);
    time_t value;
    while (1) {
        PORTB ^= (1 << PB0);
        PORTB ^= (1 << PB1);
        value = DS1302.read_time();
        show_and_send_ds1302(value.hours);
        UART.send(':');
        USART.transmit(':');
        show_and_send_ds1302(value.minutes);
        UART.send(':');
        USART.transmit(':');
        show_and_send_ds1302(value.seconds);
        UART.send('\r');
        UART.send('\n');
        USART.transmit('\n');
        _delay_ms(1000);
    }
    return 0;
}

void  show_and_send_ds1302(uint8_t value){
    UART.send('0' + (value / 10));
    USART.transmit('0' + (value / 10));
    UART.send('0' + (value % 10));
    USART.transmit('0' + (value % 10));
}