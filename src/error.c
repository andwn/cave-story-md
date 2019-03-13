#include "common.h"
#include "error.h"
#include "string.h"
#include "vdp.h"

extern uint32_t v_err_reg[8+8];
extern uint32_t v_err_pc;
extern uint32_t v_err_addr;
extern uint16_t v_err_ext1;
extern uint16_t v_err_ext2;
extern uint16_t v_err_sr;
extern uint8_t v_err_type;

static const char *dieMsg = NULL;
static const char *dieFile = NULL;
static uint16_t dieLine = 0;

void _error() {
	char buf[40];
	// Reset VDP
	vdp_init();
	
	// Error type
	uint16_t x = 2, y = 2;
	switch(v_err_type) {
		case 0:
		sprintf(buf, "%s", "Bus Error");
		vdp_color(0, 0x222);
		break;
		case 1:
		sprintf(buf, "%s", "Address Error");
		vdp_color(0, 0x800);
		break;
		case 2:
		sprintf(buf, "%s", "Bad Instruction");
		vdp_color(0, 0x008);
		break;
		case 3:
		sprintf(buf, "%s", "Divide by Zero");
		vdp_color(0, 0x046);
		break;
		case 4:
		sprintf(buf, "%s", "Out of Memory");
		vdp_color(0, 0x080);
		break;
		default:
		sprintf(buf, "%s", "Fatal Error");
		vdp_color(0, 0x404);
	}
	vdp_puts(VDP_PLAN_A, buf, x, y);
	
	if(v_err_type < 4) {
		// Registers
		x = 2; y = 4;
		for(uint16_t i = 0; i < 8; i++) {
			sprintf(buf, "D%hu=%08lX", i, v_err_reg[i]);
			vdp_puts(VDP_PLAN_A, buf, x, y+i);
		}
		for(uint16_t i = 0; i < 8; i++) {
			sprintf(buf, "A%hu=%08lX", i, v_err_reg[i+8]);
			vdp_puts(VDP_PLAN_A, buf, x, y+i+8);
		}
		
		// Other values
		x = 15; 
		switch(v_err_type) {
			case 0:
			case 1: // Bus / Address Error
			y = 20;
			sprintf(buf, "FUNC=%04hX   INST=%04hX", v_err_ext1, v_err_ext2);
			vdp_puts(VDP_PLAN_A, buf, x, y++);
			sprintf(buf, "ADDR=%06lX", v_err_addr);
			vdp_puts(VDP_PLAN_A, buf, x, y++);
			break;
			case 2: // Illegal
			y = 21;
			sprintf(buf, "OV=%04hX", v_err_ext1);
			vdp_puts(VDP_PLAN_A, buf, x, y++);
			break;
		}
		x = 2; y = 20;
		sprintf(buf, "PC=%06lX", v_err_pc);
		vdp_puts(VDP_PLAN_A, buf, x, y++);
		sprintf(buf, "SR=  %04hX", v_err_sr);
		vdp_puts(VDP_PLAN_A, buf, x, y++);
		
		// Stack dump
		x = 25; y = 2;
		vdp_puts(VDP_PLAN_A, "Stack", x, y);
		y = 4;
		uint32_t *sp = (uint32_t*) v_err_reg[15];
		for(uint16_t i = 0; i < 16; i++) {
			// Prevent wrapping around after reaching top of the stack
			if((uint32_t) sp < 0xFFF000) break;
			x = 15;
			sprintf(buf, "SP+%02X=%08lX", i << 3, *sp);
			vdp_puts(VDP_PLAN_A, buf, x, y);
			sp++;
			
			if((uint32_t) sp < 0xFFF000) break;
			x = 30;
			sprintf(buf, "%08lX", *sp);
			vdp_puts(VDP_PLAN_A, buf, x, y);
			sp++;
			
			y++;
		}
	} else {
		x = 2; y = 4;
		sprintf(buf, "%.32s:%hu", dieFile, dieLine);
		vdp_puts(VDP_PLAN_A, buf, x, y);
		if(dieMsg) {
			// Custom message
			x = 2; y = 6;
			for(uint16_t i = 0; i < 36*16; i++) {
				if(dieMsg[i] == '\n') {
					y++; x = 2; // Newline
				} else if(dieMsg[i] < 0x20) {
					break; // Invalid or nullchar
				} else {
					vdp_map_xy(VDP_PLAN_A, TILE_FONTINDEX+dieMsg[i]-0x20, x++, y);
				}
			}
		}
	}
	
	// Message
	x = 2; y = 23;
	//vdp_puts(VDP_PLAN_A, "This shouldn't happen. Report it!", x, y++);
	//vdp_puts(VDP_PLAN_A, "andy@skychase.zone", x + 8, y++);
	//vdp_puts(VDP_PLAN_A, "twitter.com/donutgrind", x + 8, y++);
	
	// R.I.P
	while(TRUE);
}

void _error_oom(const char* file, const uint16_t line) {
	dieFile = file;
	dieLine = line;
	dieMsg = NULL;
	v_err_type = 4;
	_error();
}

void _error_other(const char *err, const char *file, const uint16_t line) {
	dieFile = file;
	dieLine = line;
	dieMsg = err;
	v_err_type = 99;
	_error();
}
