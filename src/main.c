#include <stdint.h>
#define F_CPU 11059200UL
#include <util/delay.h>
#include <avr/io.h>
#include "uart.h"
#include "usart.h"
#include "ds1302.h"
#include "hc12.h"

void  show_and_send_ds1302(uint8_t data);

int main(int argc, char const *argv[]){
    /* code */
    DDRB |= (1 << PB0) | (1 << PB1);
    UART.init();
    USART.init();
    sei();
    DS1302.init();
    DS1302.reset();
    uint8_t rx_buf[12];
    time_t time = {0};
    package_t frame = {0};
    frame.start = START;
    frame.header = NODE;
    frame.id = NONE;
    frame.commands = (CID | CTIME);
    frame.data[0] = NONE;
    frame.data[1] = NONE;
    frame.data[2] = NONE;
    frame.data[3] = NONE;
    frame.data[4] = NONE;
    frame.data[5] = NONE;
    frame.length = NONE;
    frame.end = END;
    HC12.send(frame);
    for(uint8_t i = 0; i < 12; i++){
        rx_buf[i] = (uint8_t) USART.receive();
        UART.hex(rx_buf[i]);
        UART.send(' ');
    }
    uint8_t id = rx_buf[2];
    UART.send('\r');
    UART.send('\n');
    time.hours = rx_buf[6];
    time.minutes = rx_buf[7];
    time.seconds = rx_buf[8];
    DS1302.set_time(time);
    time_t value;
    while (1) {
        PORTB ^= (1 << PB0);
        PORTB ^= (1 << PB1);
        value = DS1302.read_time();
        frame.start = START;
        frame.header = NODE;
        frame.id = id;
        frame.commands = NONE;
        frame.data[0] = NONE;
        frame.data[1] = NONE;
        frame.data[2] = value.hours;
        frame.data[3] = value.minutes;
        frame.data[4] = value.seconds;
        frame.data[5] = NONE;
        frame.length = 6;
        frame.end = END;
        HC12.send(frame);
        for(uint8_t i = 0; i < 12; i++){
            rx_buf[i] = (uint8_t) USART.receive();
            UART.hex(rx_buf[i]);
            UART.send(' ');
        }
        UART.send('\r');
        UART.send('\n');
        _delay_ms(2000);
    }
    return 0;
}
