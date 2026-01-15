#ifndef HC12_H_
#define HC12_H_ 

#include <stdint.h>
#include <stdbool.h>

#define BUFFER	12
#define START	0x3C
#define NODE	0x4E
#define GATEWAY	0x47
#define END 	0x24
#define NONE	0x00
#define CID		0x01
#define CTIME	0x02
#define HC12_RX_MASK    (BUFFER - 1)


typedef struct {
	uint8_t start; // 0x3C (60)
	uint8_t header; // 0x4E (N), 0x47 (G)
	uint8_t id; // STT 0x01 to Node, 0x00 to Gateway 
	uint8_t commands;
	uint8_t data[6]; // temp 1 byte, humi 1 byte, hour 1 byte, minutes 1 byte, seconds 1 byte, reponse 1byte
	uint8_t length; //
	uint8_t end; // 0x24 (36)
} package_t;

typedef struct {
	void (*send)(package_t package);
} hc12_t;

const extern hc12_t HC12;

#endif