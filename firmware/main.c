/****************************************************************/
/* file: micro.c						*/
/* abstract: This file contains the main function		*/
/* call for reading in a file from a prom			*/
/* and pumping the JTAG ports.					*/
/****************************************************************/

#include "states.h"
#include "ports.h"
#include "uart.h"
#include "../common/xsvf.h"

#define MAX_SIZE 0x20

uint8_t repeat;
uint32_t sdr_size;
uint32_t run_test;

uint8_t address_mask[MAX_SIZE];
uint8_t data_mask[MAX_SIZE];
uint8_t tdi_value[MAX_SIZE];
uint8_t tdo_mask[MAX_SIZE];
uint8_t tdo_expected[MAX_SIZE];

void print_data(uint8_t *data, int size)
{
	int i;
	for(i=size-1; i>=0; i--) {
		LOG_INFO_PRINTF("%02x",data[i]);
	}
}

#define SDR_BEGIN	0x01
#define SDR_END		0x02
#define SDR_CHECK	0x10

#define SDR_NOCHECK	0
#define SDR_CONTINUE	0
#define SDR_FULL	3

/* output dataVal onto the TDI ports; store the TDO value returned */
void shift(int flags, uint8_t *data, uint8_t *tdo, uint32_t length)
{
	int i,j;
	int n_bytes = BYTES(length);

	LOG_DEBUG("shifting %d bits, with flags %02x",length,flags);

	for (i=0; i<n_bytes; i++) {
		uint8_t byte = data[i];
		uint8_t in = 0;
		for (j=0;j<8;j++) {
			/* on the last bit, set TMS to 1 so that we go to the EXIT state */
			if ((length==1) && (flags&SDR_END)) {
				set_port(TMS,1);
				state_ack(1);
			}

			if (length>0) {
				if (tdo) {
					in |= read_tdo()<<j;
				}
				set_port(TDI, byte&1);
				byte >>= 1;

				pulse_clock();
				length--;
			}
		}
		tdo[i] = in;
	}
}

int sdr(int flags)
{
	int failTimes=0;
	uint8_t tdo_actual[MAX_SIZE];

	if (flags&SDR_BEGIN) {
		state_goto(STATE_SHIFT_DR);
//		state_step(1); /* Select-DR-Scan state */
//		state_step(0); /* Capture-DR state */
//		state_step(0); /* Shift-DR state */
	}

	/* data processing loop */
	while (1)
	{

		shift(flags, tdi_value, tdo_actual, sdr_size);

		if (flags&SDR_CHECK) {
			int i;
			int equal = 1;
#if LOG_LEVEL>3
			LOG_DEBUG_BEGIN();
			LOG_DEBUG_PRINTF("  tdo: ");
			for (i=BYTES(sdr_size)-1; i>=0; --i) {
				LOG_DEBUG_PRINTF("%02x",tdo_actual[i]);
			}
			LOG_DEBUG_END();
#endif
			for (i=0; i<BYTES(sdr_size); i++) {
				uint8_t expected,actual;
				expected = tdo_expected[i] & tdo_mask[i];
				actual = tdo_actual[i] & tdo_mask[i];
				LOG_DEBUG("%d : %02x %02x\n",i,expected,actual);
				if (expected!=actual) {
					equal = 0;
					break;
				}
			}

			/* compare the TDO value against the expected TDO value */
			if (equal) {
				/* TDO matched what was expected */
				break;
			} else {
				/* TDO did not match the value expected */
				failTimes++;
				/* update failure count */
				if (failTimes>repeat) {
					LOG_WARNING("SDR failed -- aborting");
					return 1;
				}
				LOG_WARNING("SDR failed -- trying again (%d left)",repeat-failTimes);
				/* ISP failed */
				state_step(0); /* Pause-DR state */
				state_step(1); /* Exit2-DR state */
				state_step(0); /* Shift-DR state */
				state_step(1); /* Exit1-DR state */
				state_step(1); /* Update-DR state */
				state_step(0); /* Run-Test/Idle state */

				/* wait in Run-Test/Idle state */
				delay(run_test);

				state_goto(STATE_RTI);
			}
		} else {
			/* No TDO check - exit */
			break;
		}

	}
	if (flags&SDR_END) {
		state_goto(STATE_RTI);
	}
	
	delay(run_test);
	return 0;
}

#define READ_TDI_VALUE()	read_bytes(tdi_value,BYTES(sdr_size))
#define READ_TDO_EXPECTED()	read_bytes(tdo_expected,BYTES(sdr_size))
#define READ_TDO_MASK()		read_bytes(tdo_mask,BYTES(sdr_size))

/* parse the xsvf file and pump the bits */
int main(void)
{
	uint8_t inst; /* instruction */
	uint8_t length; /* hold the length of the arguments to read in */

	ports_init();
	uart_init();

	LOG_INFO("starting up");

	while (1)
	{
		read_byte(&inst); /* read 1 byte for the instruction */
		LOG_DEBUG("instr %02x (state=%x)",inst,current_state);
		switch (inst) {
		case XCOMPLETE:
			/* return */
			LOG_INFO("XCOMPLETE: ");
			success();
			return 1;

		case XTDOMASK:
			READ_TDO_MASK();
			LOG_INFO_BEGIN();
			LOG_INFO_PRINTF("TDOMASK: ");
			print_data(tdo_mask,BYTES(sdr_size));
			LOG_INFO_END();
			break;

		case XREPEAT:
			read_byte(&repeat);
			LOG_INFO("XREPEAT: %x",repeat);
			break;

		case XRUNTEST:
			read_long(&run_test);
			LOG_INFO("XRUNTEST: %lx",run_test);
			break;

		case XSIR:
			read_byte(&length);
			read_bytes(tdi_value,BYTES(length));

			LOG_INFO("XSIR:");
			LOG_INFO("  length: %x",length);
			LOG_INFO_BEGIN();
			LOG_INFO_PRINTF("  tdi_value:");
			print_data(tdi_value,BYTES(length));
			LOG_INFO_END();

			/* send the instruction through the TDI port */
			state_step(1); /* Select-DR-Scan state */
			state_step(1); /* Select-IR-Scan state */
			state_step(0); /* Capture-IR state */
			state_step(0); /* Shift-IR state */
			shift(SDR_END, tdi_value, 0, length);
			state_step(1); /* Update-IR state */
			state_step(0); /* Run-Test/Idle state*/
			break;

		case XSDR:
			READ_TDI_VALUE();

			LOG_INFO_BEGIN();
			LOG_INFO_PRINTF("XSDR: ");
			print_data(tdi_value,BYTES(sdr_size));
			LOG_INFO_END();

			if (sdr(SDR_FULL|SDR_CHECK)) {
				fail();
				return 0;
			}
			break;

		case XSDRSIZE:
			read_long(&sdr_size);
			LOG_INFO("XSDRSize: %lx",sdr_size);
			break;

		case XSDRTDO:
			READ_TDI_VALUE();
			READ_TDO_EXPECTED();

			LOG_INFO("XSDRTDO:");
			LOG_INFO_BEGIN();
			LOG_INFO_PRINTF("  tdi_value:");
			print_data(tdi_value,BYTES(sdr_size));
			LOG_INFO_END();
			LOG_INFO_BEGIN();
			LOG_INFO_PRINTF("  tdo_expected:");
			print_data(tdo_expected,BYTES(sdr_size));
			LOG_INFO_END();

			if (sdr(SDR_FULL|SDR_CHECK)) {
				fail();
				return 0;
			}
			break;

		case XSDRB:
			READ_TDI_VALUE();

			LOG_INFO_BEGIN();
			LOG_INFO_PRINTF("XSDRB: ");
			print_data(tdi_value,BYTES(sdr_size));
			LOG_INFO_END();

			sdr(SDR_BEGIN|SDR_NOCHECK);
			break;

		case XSDRC:
			READ_TDI_VALUE();

			LOG_INFO_BEGIN();
			LOG_INFO_PRINTF("XSDRC: ");
			print_data(tdi_value,BYTES(sdr_size));
			LOG_INFO_END();

			sdr(SDR_CONTINUE|SDR_NOCHECK);
			break;

		case XSDRE:
			READ_TDI_VALUE();

			LOG_INFO_BEGIN();
			LOG_INFO_PRINTF("XSDRE: ");
			print_data(tdi_value,BYTES(sdr_size));
			LOG_INFO_END();

			sdr(SDR_END|SDR_NOCHECK);
			break;

		case XSDRTDOB:
			READ_TDI_VALUE();
			READ_TDO_EXPECTED();

			LOG_INFO("XSDRTDOB:");
			LOG_INFO_BEGIN();
			LOG_INFO_PRINTF("  tdi_value:");
			print_data(tdi_value,BYTES(sdr_size));
			LOG_INFO_END();
			LOG_INFO_BEGIN();
			LOG_INFO_PRINTF("  tdo_expected:");
			print_data(tdo_expected,BYTES(sdr_size));
			LOG_INFO_END();

			if (sdr(SDR_BEGIN|SDR_CHECK)) {
				fail();
				return 0;
			}
			break;

		case XSDRTDOC:
			READ_TDI_VALUE();
			READ_TDO_EXPECTED();

			LOG_INFO("XSDRTDOC:");
			LOG_INFO_BEGIN();
			LOG_INFO_PRINTF("  tdi_value:");
			print_data(tdi_value,BYTES(sdr_size));
			LOG_INFO_END();
			LOG_INFO_BEGIN();
			LOG_INFO_PRINTF("  tdo_expected:");
			print_data(tdo_expected,BYTES(sdr_size));
			LOG_INFO_END();

			if (sdr(SDR_CONTINUE|SDR_CHECK)) {
				fail();
				return 0;
			}
			break;

		case XSDRTDOE:
			READ_TDI_VALUE();
			READ_TDO_EXPECTED();

			LOG_INFO("XSDRTDOE:");
			LOG_INFO_BEGIN();
			LOG_INFO_PRINTF("  tdi_value:");
			print_data(tdi_value,BYTES(sdr_size));
			LOG_INFO_END();
			LOG_INFO_BEGIN();
			LOG_INFO_PRINTF("  tdo_expected:");
			print_data(tdo_expected,BYTES(sdr_size));
			LOG_INFO_END();

			if (sdr(SDR_END|SDR_CHECK)) {
				fail();
				return 0;
			}
			break;

		case XSETSDRMASKS:
			read_bytes(address_mask,BYTES(sdr_size));
			read_bytes(data_mask,BYTES(sdr_size));

			LOG_INFO_PRINTF("XSETSDRMASKS:");
			LOG_INFO_BEGIN();
			LOG_INFO_PRINTF("  address_mask:");
			print_data(address_mask,BYTES(sdr_size));
			LOG_INFO_END();
			LOG_INFO_BEGIN();
			LOG_INFO_PRINTF("  data_mask:");
			print_data(data_mask,BYTES(sdr_size));
			LOG_INFO_END();
			break;

		case XSDRINC:
			LOG_ERROR("XSDRINC is deprecated");
			fail();
			return 0;
			break;

		case XSTATE:
			read_byte(&inst);
			LOG_INFO("XSTATE: %x",inst);
			state_goto(inst);
			break;

		default:
			LOG_ERROR("Unknown Instruction encountered");
			fail();
			return 0;
		}
	}
}
