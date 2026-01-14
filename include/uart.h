#ifndef UART_H_
#define UART_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define TX_BUFFER_SIZE 			32
#define TX_MASK 				(TX_BUFFER_SIZE - 1)

typedef struct {
    void (*init)(void);
    void (*send)(uint8_t c);
}uart_t;

extern const uart_t UART;

#endif