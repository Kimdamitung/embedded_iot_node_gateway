#include "hc12.h"
#include "usart.h"
#include "uart.h"
#include <stdint.h>

static void hc12_send(package_t package){
	USART.transmit(package.start);
	USART.transmit(package.header);
	USART.transmit(package.id);
	USART.transmit(package.commands);
	//data
	USART.transmit(package.data[0]);
	USART.transmit(package.data[1]);
	USART.transmit(package.data[2]);
	USART.transmit(package.data[3]);
	USART.transmit(package.data[4]);
	USART.transmit(package.data[5]);
	//
	USART.transmit(package.length);
	USART.transmit(package.end);
}

const hc12_t HC12 = {
	.send = hc12_send
};