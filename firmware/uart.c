/********************************************************/
/* file: uart.c						*/
/********************************************************/

#define F_CPU 16000000UL	// 16 MHz
#define UART_BAUD 57600	// baudrate

#include <avr/io.h>
#include "uart.h"
#include "../common/xsvf.h"

unsigned char buffer[0x100];
int buffer_pos;
int buffer_size;

void uart_putchar(char c);
char uart_getchar(void);

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

FILE *uart;

void uart_init(void)
{
	// setup tx/rx
	UBRR = (F_CPU / (16UL * UART_BAUD)) - 1;
	UCR = _BV(TXEN) | _BV(RXEN);

	uart = &uart_str;

	buffer_pos = 0;
	buffer_size = 0;
}

void uart_putchar(char c)
{
	loop_until_bit_is_set(USR, UDRE);
	UDR = c;
}

char uart_getchar(void)
{
	loop_until_bit_is_set(USR, RXC);
//	LOG_DEBUG("%02x",UDR);
	return UDR;
}

uint8_t read_next_instr()
{
	int n = 0;
	LOG_DEBUG("requesting next instruction");
	uart_putchar('+');
	while (n<=0) {
		n = load_next_instr(buffer, uart);
	}
	LOG_DEBUG("received %d bytes",n);
	buffer_pos = 1;
	buffer_size = n;
	return buffer[0];
}

void fail(void)
{
	fprintf(uart,"f\n");
}

void success(void)
{
	fprintf(uart,"s\n");
}

int get_hex_value(char c)
{
	if (c>='0' && c<='9') {
		return c-'0';
	} else if (c>='a' && c<='f') {
		return c-'a'+10;
	} else if (c>='A' && c<='F') {
		return c-'A'+10;
	} else {
		return -1;
	}
}

int read_chunk(void)
{
	char c;
	unsigned char size;

	uart_putchar('+');
	
	c = uart_getchar();
	if (c!='+') {
		return 1;
	}

	size = uart_getchar()&0xff;

	buffer_size = 0;
	while (buffer_size<size) {
		buffer[buffer_size++] = uart_getchar();
	}
	LOG_DEBUG("read ok %02x",buffer_size);
	return 0;
}

int read_byte(uint8_t *data)
{
	if (buffer_pos>=buffer_size) {
		LOG_DEBUG("need more data");
		if (read_chunk()) {
			LOG_DEBUG("failed to get more data");
			return 1;
		}
		LOG_DEBUG("got more data");
		buffer_pos = 0;
	}
	*data = buffer[buffer_pos];
	buffer_pos++;
	return 0;
}

int read_word(uint16_t *data)
{
	uint8_t l,h;
	if (read_byte(&h)) {
		return 1;
	}
	if (read_byte(&l)) {
		return 1;
	}
	((uint8_t*)data)[0] = l;
	((uint8_t*)data)[1] = h;
	return 0;
}

int read_long(uint32_t *data)
{
	uint16_t l,h;
	if (read_word(&h)) {
		return 1;
	}
	if (read_word(&l)) {
		return 1;
	}
	((uint16_t*)data)[0] = l;
	((uint16_t*)data)[1] = h;
	return 0;
}

int read_bytes(uint8_t *data, int len)
{
	int i;
	for (i=len-1; i>=0; --i) {
		read_byte(&data[i]);
	}
}


