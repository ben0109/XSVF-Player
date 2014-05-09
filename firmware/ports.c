/********************************************************/
/* file: ports.c					*/
/********************************************************/

#include <avr/io.h>
#include <avr/delay.h>

#include "ports.h"
#include "uart.h"

uint8_t in_word = 0;
uint8_t out_word = 0;

void ports_init(void)
{
	DDRA = TMS|TDI|TCK;
}

uint8_t set_bit(uint8_t in, uint8_t bit, int val)
{
	if (val) {
		return (in | bit);
	} else {
		return (in & ~bit);
	}
}

void set_port(uint8_t p,int val)
{
	out_word = set_bit(out_word, p, val);

	/* Update port on falling edge of TCK */
	if (p==TCK) {
		if (val==0) {
			PORTA = out_word;
		} else {
			PORTA |= TCK;
		}
	}
}

void pulse_clock()
{
	set_port(TCK,0);
	set_port(TCK,1);
	_delay_us(1000);
	#ifdef DEBUG_MODE
//		LOG_DEBUG("M=%d I=%d",(out_word&TMS)?1:0,(out_word&TDI)?1:0);
	#endif
	set_port(TCK,0);
}

int read_tdo()
{
//	_delay_us(1000);
	in_word = (uint8_t)PINA;
	#ifdef DEBUG_MODE
//		LOG_DEBUG("O=%d",(in_word&TDO)?1:0);
	#endif
	return (in_word&TDO)?1:0;
}

/* Wait at least the specified number of microsec. */
void delay(long microsec)
{
//	_delay_ms(microsec>>12);
	set_port(TCK,0);
	while (--microsec > 0) {
		set_port(TCK,1);
		_delay_us(1);
		set_port(TCK,0);
		_delay_us(1);
	}
}

