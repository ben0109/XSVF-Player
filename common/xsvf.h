#ifndef __xsvf_dot_h
#define __xsvf_dot_h

#include <stdio.h>
#include <inttypes.h>

/* xsvf instructions */
#define XCOMPLETE	0
#define XTDOMASK	1
#define XSIR		2
#define XSDR		3
#define XRUNTEST	4
#define XREPEAT		7
#define XSDRSIZE	8
#define XSDRTDO		9
#define XSETSDRMASKS	10
#define XSDRINC		11
#define XSDRB		12
#define XSDRC		13
#define XSDRE		14
#define XSDRTDOB	15
#define XSDRTDOC	16
#define XSDRTDOE	17
#define XSTATE		18

#define PING		126 /* '~' */ 

/* return number of bytes necessary for "num" bits */
#define BYTES(num) ((int)((num+7)>>3))

int load_next_instr(uint8_t *buffer, FILE *stream);

#endif
