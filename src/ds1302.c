#include "ds1302.h"
#include <stdint.h>
#include <util/delay.h>

static void ds1302_init(void){
	DDRD |= (1 << CE) | (1 << SCL);
	DDRD &= ~(1 << IO);
	PORTD &= ~(1 << CE);
	PORTD &= ~(1 << SCL);
}

static void ds1302_write_byte(uint8_t data){
	DDRD |= (1 << IO);
	for(uint8_t i = 0; i < 8; i++){
		(data & 0x01) ? (PORTD |= (1 << IO)) : (PORTD &= ~(1 << IO));
		PORTD |= (1 << SCL);
		_delay_us(2);
		PORTD &= ~(1 << SCL);
		_delay_us(2);
		data >>= 1;
	}
}

static uint8_t ds1302_read_byte(void){
	uint8_t data = 0;
	DDRD &= ~(1 << IO);
	for(uint8_t i = 0; i < 8; i++){
		data >>= 1;
		if(PIND & (1 << IO))
			data |= 0x80;
		PORTD |= (1 << SCL);
		_delay_us(2);
		PORTD &= ~(1 << SCL);
		_delay_us(2);
	}
	return data;
}

static void ds1302_write_reg(uint8_t address, uint8_t data){
	PORTD |= (1 << CE);
	ds1302_write_byte((address << 1) | 0x80);
	ds1302_write_byte(data);
	PORTD &= ~(1 << CE);
}

static uint8_t ds1302_read_reg(uint8_t address){
	uint8_t value;
	PORTD |= (1 << CE);
	ds1302_write_byte((address << 1) | 0x81);
	value = ds1302_read_byte();
	PORTD &= ~(1 << CE);
	return value;
}

static uint8_t bcd_convert_dec(uint8_t bcd){
	return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

static uint8_t dec_covert_bcd(uint8_t dec){
	return ((dec / 10) << 4) | (dec % 10);
}

static void ds1302_reset_ch(void){
	ds1302_write_reg(CONTROL, 0x00);
	ds1302_write_reg(SEC, 0x00);
}

static void ds1302_set_time(time_t time){
	ds1302_write_reg(CONTROL, 0x00);
	ds1302_write_reg(SEC, dec_covert_bcd(time.seconds & 0x7F));
	ds1302_write_reg(MIN, dec_covert_bcd(time.minutes));
	ds1302_write_reg(HOUR, dec_covert_bcd(time.hours));
	ds1302_write_reg(CONTROL, 0x80);
}

static void ds1302_set_date(date_t date){
	ds1302_write_reg(CONTROL, 0x00);
	ds1302_write_reg(DATE, dec_covert_bcd(date.date));
	ds1302_write_reg(DAY, dec_covert_bcd(date.day));
	ds1302_write_reg(MONTH, dec_covert_bcd(date.month));
	ds1302_write_reg(YEAR, dec_covert_bcd(date.year));
	ds1302_write_reg(CONTROL, 0x80);
}

static time_t ds1302_read_time(void){
	uint8_t seconds = bcd_convert_dec(ds1302_read_reg(SEC) & 0x7F);
	uint8_t minutes = bcd_convert_dec(ds1302_read_reg(MIN));
	uint8_t hours = bcd_convert_dec(ds1302_read_reg(HOUR) & 0x3F);
	time_t time = {
		.seconds = seconds,
		.minutes = minutes,
		.hours = hours
	};
	return time;
}

static date_t ds1302_read_date(void){
	uint8_t date = bcd_convert_dec(ds1302_read_reg(DATE));
	uint8_t day = bcd_convert_dec(ds1302_read_reg(DAY));
	uint8_t month = bcd_convert_dec(ds1302_read_reg(MONTH));
	uint8_t year = bcd_convert_dec(ds1302_read_reg(YEAR));
	date_t calendar = {
		.date = date,
		.day = day,
		.month = month,
		.year = year
	};
	return calendar;
}

const ds1302_t DS1302 = {
	.init = ds1302_init,
	.set_time = ds1302_set_time,
	.set_date = ds1302_set_date,
	.reset = ds1302_reset_ch,
	.read_time = ds1302_read_time,
	.read_date = ds1302_read_date
};