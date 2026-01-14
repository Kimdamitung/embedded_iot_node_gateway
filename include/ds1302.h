#ifndef DS1302_H_
#define DS1302_H_

#include <avr/io.h>
#include <stdint.h>

#define CE		PD3
#define IO		PD4
#define SCL		PD5
#define SEC 	0x00
#define MIN 	0x01
#define HOUR 	0x02
#define DAY		0x03
#define DATE	0x04
#define MONTH	0x05
#define YEAR 	0x06
#define CONTROL	0x07

typedef struct {
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
} time_t;

typedef struct {
	uint8_t date;
	uint8_t day;
	uint8_t month;
	uint8_t year;
} date_t;

typedef struct {
	void (*init)(void);
	void (*reset)(void);
	void (*set_time)(time_t *time);
	void (*set_date)(date_t *date);
	time_t (*read_time)(void);
	date_t (*read_date)(void);
} ds1302_t;

const extern ds1302_t DS1302;

#endif