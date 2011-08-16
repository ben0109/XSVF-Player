/*******************************************************/
/* file: ports.c					*/
/*******************************************************/

#include "ports.h"

unsigned char buffer[0x100];
int buffer_pos;
int buffer_size;


#ifdef DEBUG_MODE
/* if in debugging mode, use variables instead of setting the ports */
short pTCK,pTMS,pTDI;
#endif

#ifdef AVR
#define F_CPU 16000000UL  // 16 MHz
#define UART_BAUD 9600  // baudrate
#include <avr/io.h>
#include <avr/delay.h>
void uart_putchar(char c);
char uart_getchar(void);
FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

char in_word = 0;
char out_word = 0;
#endif

#include <stdio.h>
FILE *_stdin;
FILE *_stdout;

#ifdef TTY
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

FILE *in;
int tty_fd;

char in_word = 0;
char out_word = 0;
#endif

char* readLine(void);
int readBit(void);

void ports_init(void)
{
#ifdef AVR
	DDRA = TMS|TDI|TCK;
	UBRR = (F_CPU / (16UL * UART_BAUD)) - 1;
	UCR = _BV(TXEN) | _BV(RXEN); /* tx/rx enable */

	_stdin = &uart_str;
	_stdout = &uart_str;
#endif
#ifdef TTY
	struct termios tio;
        memset(&tio,0,sizeof(tio));
        tio.c_iflag=0;
        tio.c_oflag=0;
        tio.c_cflag=CS8|CREAD|CLOCAL;	// 8n1
        tio.c_lflag=0;
        tio.c_cc[VMIN]=1;
        tio.c_cc[VTIME]=5;
        cfsetospeed(&tio,B115200);	// bauds
	cfsetispeed(&tio,B115200);

	tty_fd = open("/dev/ttyUSB1", O_RDWR);
	tcsetattr(tty_fd,TCSANOW,&tio);

	_stdin = fopen("to_papilio","rb");
	_stdout = fopen("from_papilio","wb");
//	_stdin = stdin;
//	_stdout = stdout;
#endif

	int tms,tdo,tdi,tck;
	int waitStart;
	waitStart = 1; 
	while (waitStart) {
		char c = fgetc(_stdin);
		switch (c) {
		case 'R': break;			// RESET (ignored)
		case 'M': tms = readBit(); break;	// set TMS
		case 'O': tdo = readBit(); break;	// set TDO
		case 'I': tdi = readBit(); break;	// set TDI
		case 'C': tck = readBit(); break;	// set TCK
		case 'S': waitStart = 0; break;		// start upload
		default: fprintf(_stdout,"U%02x\n", c);
		}
		fgetc(_stdin);	// read new line
	}

	buffer_pos = 0;
	buffer_size = 0;
}

#ifdef AVR

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
#endif

int readBit()
{
	int c = fgetc(_stdin)-'0';
	return 1<<c;
}

static char line_buffer[0x100];

char* readLine()
{
	int i = 0;
	while (1) {
		char c = fgetc(_stdin);
		if (c=='\n' || i==0xfe) {
			break;
		}
		line_buffer[i++] = c;
	}
	line_buffer[i] = 0;
	return line_buffer;
}

void fail(void)
{
	fprintf(_stdout,"f\n");
}

void success(void)
{
	fprintf(_stdout,"s\n");
}

int getHexValue(char c)
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

int readChunk(void)
{
	char c;
	unsigned char size;

	fprintf(_stdout, "+ff\n");
	
	c = fgetc(_stdin);
	if (c!='+') {
		return 1;
	}

	size = getHexValue(fgetc(_stdin));
	size <<= 4;
	size += getHexValue(fgetc(_stdin));
	fgetc(_stdin);	// read new line

	buffer_size = 0;
	while (buffer_size<size) {
		buffer[buffer_size++] = uart_getchar();
	}
	LOG_DEBUG("read ok %02x",buffer_size);
	return 0;
}

int readByte(uint8_t *data)
{
	if (buffer_pos>=buffer_size) {
		LOG_DEBUG("need more data");
		if (readChunk()) {
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

int readWord(uint16_t *data)
{
	uint8_t l,h;
	if (readByte(&h)) {
		return 1;
	}
	if (readByte(&l)) {
		return 1;
	}
	((uint8_t*)data)[0] = l;
	((uint8_t*)data)[1] = h;
	return 0;
}

int readLong(uint32_t *data)
{
	uint16_t l,h;
	if (readWord(&h)) {
		return 1;
	}
	if (readWord(&l)) {
		return 1;
	}
	((uint16_t*)data)[0] = l;
	((uint16_t*)data)[1] = h;
	return 0;
}

int readBytes(uint8_t *data, int len)
{
	int i;
//	LOG_DEBUG("reading %d bytes",len);
	for (i=len-1; i>=0; --i) {
		readByte(&data[i]);
	}
}





char setBit(char in, char bit, char val)
{
	if (val) {
		return (in | bit);
	} else {
		return (in & ~bit);
	}
}

void setPort(short p,short val)
{
	static int once=0;
	/* if in debugging mode, then just set the variables */
	#ifdef DEBUG_MODE
	if (p==TCK)
		pTCK=val;
	if (p==TMS)
		pTMS=val;
	if (p==TDI)
		pTDI=val;
	#endif
	out_word = setBit(out_word, p, val);
	#ifdef AVR
	if (p==TCK) {
		PORTA = out_word;
	}
	#endif
	#ifdef TTY
	if (p==TCK) {
		// send data
		out_word |= 0x30;
		write(tty_fd,&out_word,1);
		// wait for answer
		while (read(tty_fd,&in_word,1)==0) {}
		#ifdef DEBUG_MODE
			printf("%c->%c\n",out_word,in_word);
		#endif
	}
	#endif
}

/* toggle tck LHL */
void pulseClock()
{
	setPort(TCK,0); /* set the TCK port to low */
	setPort(TCK,1); /* set the TCK port to high */
	#ifdef DEBUG_MODE
		/* if in debugging mode, print the ports on the rising clock edge */
		LOG_DEBUG("M=%d I=%d",pTMS,pTDI);
	#endif
	setPort(TCK,0); /* set the TCK port to low */
}

/* read the TDO bit from port */
unsigned char readTDOBit()
{
	#ifdef AVR
		in_word = (unsigned char)PINA;
	#endif
	#ifdef TTY
		setPort(TCK,0); /* set the TCK port to low */
	#endif
	#ifdef DEBUG_MODE
		LOG_DEBUG("O=%d",(in_word&TDO)?1:0);
	#endif
	return (in_word&TDO)?1:0;
}

/* Wait at least the specified number of microsec. */
#ifdef AVR
void waitTime(long microsec)
{
//	_delay_ms(microsec>>12);
	setPort(TCK,0); /* set the TCK port to low */
	while (--microsec > 0) {
		setPort(TCK,1); /* set the TCK port to high */
		_delay_us(1);
		setPort(TCK,0); /* set the TCK port to low */
		_delay_us(1);
	}
}
#endif
#ifdef TTY
void waitTime(long microsec)
{
	usleep(microsec);
}
#endif

