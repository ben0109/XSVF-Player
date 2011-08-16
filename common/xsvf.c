#include "xsvf.h"

uint32_t sdr_size;

#define READ(m) for (i=0;i<m;i++) { buffer[n++] = fgetc(stream); }

int load_next_instr(uint8_t *buffer, FILE *stream)
{
	int i,n;

	n = 0;
	READ(1);

	switch (buffer[0]) {
	case XTDOMASK:
		READ(sdr_size);
		break;
	case XREPEAT:
		READ(1);
		break;
	case XRUNTEST:
		READ(4);
		break;
	case XSIR:
		READ(1);
		READ(BYTES(buffer[1]));
		break;
	case XSDR:
		READ(sdr_size);
		break;
	case XSDRSIZE:
		READ(4);
		sdr_size = 0;
		sdr_size |= (buffer[1])<<24;
		sdr_size |= (buffer[2])<<16;
		sdr_size |= (buffer[3])<<8;
		sdr_size |= (buffer[4])<<0;
		sdr_size = BYTES(sdr_size);
		break;
	case XSDRTDO:
		READ(sdr_size);
		READ(sdr_size);
		break;
	case XSDRB:
		READ(sdr_size);
		break;
	case XSDRC:
		READ(sdr_size);
		break;
	case XSDRE:
		READ(sdr_size);
		break;
	case XSDRTDOB:
		READ(sdr_size);
		READ(sdr_size);
		break;
	case XSDRTDOC:
		READ(sdr_size);
		READ(sdr_size);
		break;
	case XSDRTDOE:
		READ(sdr_size);
		READ(sdr_size);
		break;
	case XSETSDRMASKS:
		READ(sdr_size);
		READ(sdr_size);
		break;
	case XCOMPLETE:
		break;
	case XSTATE:
		READ(1);
		break;

	case XSDRINC:
	default:
		return -1;
	}

	return n;
}
