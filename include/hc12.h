#ifndef HC12_H_
#define HC12_H_ 

#include <stdint.h>

typedef struct {
	uint8_t star; // 0x3C (60)
	uint8_t header; // 0x4E (N), 0x47 (G)
	uint8_t id; // STT 0x01 to Node, 0x00 to Gateway 
	uint8_t data[8]; // temp 2 byte, humi 2 byte, hour 1 byte, minutes 1 byte, seconds 1 byte, reponse 1byte
	uint8_t length; //
	uint8_t crc8;
	uint8_t end; // 0x24 (36)
} package_t;

typedef struct {
	
} hc12_t;

#endif