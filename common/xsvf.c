#include "xsvf.h"
#include "../firmware/uart.h"

int sdr_bytes;

//#define READ(m) for (i=0;i<m;i++) { buffer[n++] = uart_getchar(); }
#define READ(m) for (i=0;i<m;i++) { buffer[n++] = fgetc(stream); }

int load_next_instr(uint8_t *buffer, FILE *stream)
{
	int i,n;

	n = 0;
	READ(1);
	switch (buffer[0]) {
	case XTDOMASK:
		READ(sdr_bytes);
		break;
	case XREPEAT:
		READ(1);
		break;
	case XRUNTEST:
		READ(4);
		break;
	case XSIR:
		READ(1);
		READ(buffer[1]>>3);;
		break;
	case XSDR:
		READ(sdr_bytes);
		break;
	case XSDRSIZE:
		READ(4);
		sdr_bytes = 0;
		sdr_bytes |= (buffer[3])<<8;
		sdr_bytes |= (buffer[4]);
		sdr_bytes = (sdr_bytes+7)>>3;
		break;
	case XSDRTDO:
		READ(sdr_bytes);
		READ(sdr_bytes);
		break;
	case XSDRB:
		READ(sdr_bytes);
		break;
	case XSDRC:
		READ(sdr_bytes);
		break;
	case XSDRE:
		READ(sdr_bytes);
		break;
	case XSDRTDOB:
		READ(sdr_bytes);
		READ(sdr_bytes);
		break;
	case XSDRTDOC:
		READ(sdr_bytes);
		READ(sdr_bytes);
		break;
	case XSDRTDOE:
		READ(sdr_bytes);
		READ(sdr_bytes);
		break;
	case XSETSDRMASKS:
		READ(sdr_bytes);
		READ(sdr_bytes);
		break;
	case XCOMPLETE:
		break;
	case XSTATE:
		READ(1);
		break;

	/* pseudo instr to sync */
	case PING:
		break;

	case XSDRINC:
	default:
		return -1;
	}
	return n;
}
