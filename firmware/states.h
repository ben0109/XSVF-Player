#ifndef state_dot_h
#include <inttypes.h>
#include "ports.h"

#define STATE_TLR		0x00
#define STATE_RTI		0x01
#define STATE_SELECT_DR_SCAN	0x02
#define STATE_CAPTURE_DR	0x03
#define STATE_SHIFT_DR		0x04
#define STATE_EXIT1_DR		0x05
#define STATE_PAUSE_DR		0x06
#define STATE_EXIT2_DR		0x07
#define STATE_UPDATE_DR		0x08
#define STATE_SELECT_IR_SCAN	0x09
#define STATE_CAPTURE_IR	0x0a
#define STATE_SHIFT_IR		0x0b
#define STATE_EXIT1_IR		0x0c
#define STATE_PAUSE_IR		0x0d
#define STATE_EXIT2_IR		0x0e
#define STATE_UPDATE_IR		0x0f

extern int current_state;

void state_ack(int tms);
void state_step(int tms);

void set_state(int state);
void state_goto(int state);

#endif
