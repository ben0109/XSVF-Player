#define F_CPU 16000000UL	// 16 MHz
#define UART_BAUD 9600		// baudrate

#include <avr/io.h>
#include "../common/xsvf.h"

unsigned char buffer[0x100];
int buffer_size;

void uart_putchar(char c)
{
	loop_until_bit_is_set(USR, UDRE);
	UDR = c;
}

char uart_getchar(void)
{
	loop_until_bit_is_set(USR, RXC);
	return UDR;
}

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

FILE *uart;

void main()
{
	// setup tx/rx
	UBRR = (F_CPU / (16UL * UART_BAUD)) - 1;
	UCR = _BV(TXEN) | _BV(RXEN);

	uart = &uart_str;

	buffer_size = 0;

	while (1) {
		char c;
		c = uart_getchar();
		uart_putchar(c);
		uart_putchar(c);
	}
}

