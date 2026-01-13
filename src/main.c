#include <stdint.h>
#define F_CPU 11059200UL
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>


// uart bitbang
#define TX_BUFFER_SIZE 32

volatile uint8_t tx_buffer[TX_BUFFER_SIZE];
volatile uint8_t tx_head = 0;
volatile uint8_t tx_tail = 0;
volatile uint8_t tx_shift;
volatile int8_t tx_bit = -1; 

void uart_bitbang_init(void) {
    DDRB |= (1 << PB4); 
    PORTB |= (1 << PB4);
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS11); 
    OCR1A = (F_CPU / 8 / 9600) - 1; 
    TCNT1 = 0;
    TIMSK |= (1 << OCIE1A);
}

void uart_send_8n1(uint8_t data){
    uint8_t next = (tx_head + 1) % TX_BUFFER_SIZE;
    while (next == tx_tail);
    tx_buffer[tx_head] = data;
    tx_head = next;
    if (tx_bit == -1) tx_bit = 0; 
}

ISR(TIMER1_COMPA_vect){
    if (tx_bit == -1) 
        return;
    if (tx_bit == 0) {
        PORTB &= ~(1 << PB4);
        tx_shift = tx_buffer[tx_tail];
        tx_bit++;
    } else if (tx_bit >= 1 && tx_bit <= 8) {
        if (tx_shift & 0x01) PORTB |= (1 << PB4);
        else PORTB &= ~(1 << PB4);
        tx_shift >>= 1;
        tx_bit++;
    } else if (tx_bit == 9) {
        // Stop bit
        PORTB |= (1 << PB4);
        tx_bit = -1;
        if (tx_head != tx_tail) {
            tx_tail = (tx_tail + 1) % TX_BUFFER_SIZE;
            tx_bit = 0;
        }
    }
}

// uasrt
void usart_init(unsigned int baudrate){
    UBRRH = 0;
    UBRRL = (F_CPU / 16 / baudrate) - 1;
    UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
    UCSRC = (1 << UCSZ1) | (1 << UCSZ0); 
}

void USART_Transmit(unsigned char data ){
    while (!( UCSRA & (1 << UDRE)));
    UDR = data;
}

void USART_Transmit_printf(const char *s, ...){
    while (*s) {
        USART_Transmit(*s++);
    }
}

unsigned char USART_Receive(void){
    while (!(UCSRA & (1 << RXC)));
    return UDR;
}

ISR(USART_RX_vect){
    uint8_t c = UDR;
    uart_send_8n1(c);
}

// rtc ds1302
#define DS1302_CE   PD3
#define DS1302_DATA PD4
#define DS1302_SCLK PD5

#define CE_HIGH()   (PORTD |=  (1 << DS1302_CE))
#define CE_LOW()    (PORTD &= ~(1 << DS1302_CE))
#define CLK_HIGH()  (PORTD |=  (1 << DS1302_SCLK))
#define CLK_LOW()   (PORTD &= ~(1 << DS1302_SCLK))
#define DATA_HIGH() (PORTD |=  (1 << DS1302_DATA))
#define DATA_LOW()  (PORTD &= ~(1 << DS1302_DATA))

void ds1302_init(void) {
    DDRD |= (1 << DS1302_CE) | (1 << DS1302_SCLK);
    DDRD &= ~(1 << DS1302_DATA);
    CE_LOW();
    CLK_LOW();
}

void ds1302_write_byte(uint8_t data) {
    DDRD |= (1 << DS1302_DATA); 
    for (uint8_t i = 0; i < 8; i++) {
        if (data & 1) 
            DATA_HIGH();
        else
            DATA_LOW();
        CLK_HIGH();
        _delay_us(2);
        CLK_LOW();
        _delay_us(2);
        data >>= 1; 
    }
}

uint8_t ds1302_read_byte(void) {
    uint8_t data = 0;
    DDRD &= ~(1 << DS1302_DATA); // INPUT
    for (uint8_t i = 0; i < 8; i++) {
        data >>= 1;
        if (PIND & (1 << DS1302_DATA))
            data |= 0x80;
        CLK_HIGH();
        _delay_us(2);
        CLK_LOW();
        _delay_us(2);
    }
    return data;
}

uint8_t ds1302_read_reg(uint8_t addr) {
    uint8_t val;
    CE_HIGH();
    ds1302_write_byte((addr << 1) | 0x81); // READ
    val = ds1302_read_byte();
    CE_LOW();
    return val;
}

void ds1302_write_reg(uint8_t addr, uint8_t data) {
    CE_HIGH();
    ds1302_write_byte((addr << 1) | 0x80); // WRITE
    ds1302_write_byte(data);
    CE_LOW();
}

uint8_t bcd_to_dec(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

uint8_t dec_to_bcd(uint8_t dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

void ds1302_set_time(uint8_t hour, uint8_t min, uint8_t sec) {
    ds1302_write_reg(0x07, 0x00);
    ds1302_write_reg(0x00, dec_to_bcd(sec) & 0x7F); // seconds, CH = 0
    ds1302_write_reg(0x01, dec_to_bcd(min));
    ds1302_write_reg(0x02, dec_to_bcd(hour));       // 24h mode
    ds1302_write_reg(0x07, 0x80);   // enable write protect (optional)
}

void uart_print_2d(uint8_t v) {
    uart_send_8n1('0' + (v / 10));
    USART_Transmit('0' + v / 10);
    uart_send_8n1('0' + (v % 10));
    USART_Transmit('0' + v % 10);
}

int main(void){
    DDRB |= (1 << PB0);
    DDRB |= (1 << PB1);
    uart_bitbang_init();
    usart_init(9600);
    sei(); 
    ds1302_init();
    /* Disable write protect */
    ds1302_write_reg(0x07, 0x00);
    /* Reset seconds, clear CH bit */
    ds1302_write_reg(0x00, 0x00);
    ds1302_set_time(15, 01, 0);
    while (1){
        PORTB ^= (1 << PB0);
        PORTB ^= (1 << PB1);
        uint8_t sec_bcd  = ds1302_read_reg(0x00) & 0x7F;
        uint8_t min_bcd  = ds1302_read_reg(0x01);
        uint8_t hour_bcd = ds1302_read_reg(0x02) & 0x3F;
        uint8_t sec  = bcd_to_dec(sec_bcd);
        uint8_t min  = bcd_to_dec(min_bcd);
        uint8_t hour = bcd_to_dec(hour_bcd);
        uart_print_2d(hour);
        USART_Transmit(':');
        uart_send_8n1(':');
        uart_print_2d(min);
        USART_Transmit(':');
        uart_send_8n1(':');
        uart_print_2d(sec);
        uart_send_8n1('\r');
        USART_Transmit('\r');
        uart_send_8n1('\n');
        USART_Transmit('\n');
        _delay_ms(1000);
    }
}
