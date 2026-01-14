#ifndef USART_H_
#define USART_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

typedef struct {
    void (*init)(void);
    void (*transmit)(uint8_t c);
    unsigned char (*receive)(void);
}usart_t;

extern const usart_t USART;

#endif