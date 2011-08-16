/********************************************************/
/* file: ports.h					*/
/********************************************************/

#ifndef ports_dot_h
#define ports_dot_h

#include <inttypes.h>

#define TMS 0x10
#define TDO 0x20
#define TDI 0x40
#define TCK 0x80

void ports_init(void);

extern void set_port(uint8_t port, int value);
extern int read_tdo(void);
extern void pulse_clock(void);
extern void delay(long microsec);

#endif

