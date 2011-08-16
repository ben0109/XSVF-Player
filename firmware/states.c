#include "states.h"
#include "ports.h"

/* high nibble: next state if TMS=1, low nibble: next step if TMS=0 */
const uint8_t tms_transitions[] = {
	0x01,	/* STATE_TLR		*/
	0x21,	/* STATE_RTI		*/
	0x93,	/* STATE_SELECT_DR_SCAN	*/
	0x54,	/* STATE_CAPTURE_DR	*/
	0x54,	/* STATE_SHIFT_DR	*/
	0x86,	/* STATE_EXIT1_DR	*/
	0x76,	/* STATE_PAUSE_DR	*/
	0x84,	/* STATE_EXIT2_DR	*/
	0x21,	/* STATE_UPDATE_DR	*/
	0x0a,	/* STATE_SELECT_IR_SCAN	*/
	0xcb,	/* STATE_CAPTURE_IR	*/
	0xcb,	/* STATE_SHIFT_IR	*/
	0xfd,	/* STATE_EXIT1_IR	*/
	0xed,	/* STATE_PAUSE_IR	*/
	0xfb,	/* STATE_EXIT2_IR	*/
	0x21,	/* STATE_UPDATE_IR	*/
};

/* bit 0: TMS to get to state 0 ... bit 15: value of TMS to get to state 15 */	
const uint16_t tms_map[] = {
	0x0000,	/* STATE_TLR		*/
	0xfffd,	/* STATE_RTI		*/
	0xfe03,	/* STATE_SELECT_DR_SCAN	*/
	0xffe7,	/* STATE_CAPTURE_DR	*/
	0xffef,	/* STATE_SHIFT_DR	*/
	0xff0f,	/* STATE_EXIT1_DR	*/
	0xffbf,	/* STATE_PAUSE_DR	*/
	0xff0f,	/* STATE_EXIT2_DR	*/
	0xfefd,	/* STATE_UPDATE_DR	*/
	0x01ff,	/* STATE_SELECT_IR_SCAN	*/
	0xf3ff,	/* STATE_CAPTURE_IR	*/
	0xf7ff,	/* STATE_SHIFT_IR	*/
	0x87ff,	/* STATE_EXIT1_IR	*/
	0xdfff,	/* STATE_PAUSE_IR	*/
	0x87ff,	/* STATE_EXIT2_IR	*/
	0x7ffd,	/* STATE_UPDATE_IR	*/
};

int current_state = 0;

void set_state(int state)
{
	current_state = state;
}

void state_ack(int tms)
{
	if (tms==0) {
		current_state = tms_transitions[current_state]&0xf;
	} else {
		current_state = (tms_transitions[current_state]>>4)&0xf;
	}
}

void state_step(int tms)
{
	set_port(TMS,tms);
	pulse_clock();
	state_ack(tms);
}

void state_goto(int state)
{
	if (state==STATE_TLR) {
		int i;
		for (i=0;i<5;i++) {
			state_step(1);
		}

	} else {
		while (current_state != state) {
			int tms = (tms_map[current_state]>>state) & 1;
			state_step(tms);
		}
	}
}

