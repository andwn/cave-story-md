#include "error.h"
#include "string.h"
#include "sys.h"
#include "vdp.h"
#include "res/system.h"

extern uint32_t v_err_reg[8+8];
extern uint32_t v_err_pc;
extern uint32_t v_err_addr;
extern uint16_t v_err_ext1;
extern uint16_t v_err_ext2;
extern uint16_t v_err_sr;
extern uint8_t v_err_type;

static const char *dieMsg;
static const char *dieFile;
static uint16_t dieLine;

__attribute__((noreturn))
void _error() {
	char buf[40];
	// Don't completely clear the screen or rewrite the palettes
    // Only what is needed to display the error
    vdp_sprites_clear();
    vdp_font_load(UFTC_SysFont);
    vdp_color(1, 0x000);
    vdp_color(15, 0xEEE);
    vdp_set_scrollmode(HSCROLL_PLANE, VSCROLL_PLANE);
    vdp_hscroll(VDP_PLANE_A, 0);
    vdp_vscroll(VDP_PLANE_A, 0);
    vdp_set_window(0, 0);

    // When profiling, the background color is changed for each phase of the game loop.
    // For situations where none of the values in the error make any sense whatsoever,
    // it is better to keep the color as is to get a rough idea of where the crash happened

	// Error type
	uint16_t x = 2, y = 2;
	switch(v_err_type) {
		case 1:
		sprintf(buf, "%s", "Address Error");
#ifndef PROFILE
		vdp_color(0, 0x800);
#endif
		break;
		case 2:
		sprintf(buf, "%s", "Bad Instruction");
#ifndef PROFILE
		vdp_color(0, 0x008);
#endif
		break;
		case 3:
		sprintf(buf, "%s", "Divide by Zero");
#ifndef PROFILE
		vdp_color(0, 0x046);
#endif
		break;
		case 4:
		sprintf(buf, "%s", "Out of Memory");
#ifndef PROFILE
		vdp_color(0, 0x080);
#endif
		break;
		default:
		sprintf(buf, "%s", "Fatal Error");
#ifndef PROFILE
		vdp_color(0, 0x404);
#endif
	}
	vdp_puts(VDP_PLANE_A, buf, x, y);
	
	if(v_err_type < 4) {
		// Registers
		x = 2; y = 4;
		for(uint16_t i = 0; i < 8; i++) {
			sprintf(buf, "D%hu=%08lX", i, v_err_reg[i]);
			vdp_puts(VDP_PLANE_A, buf, x, y + i);
		}
		for(uint16_t i = 0; i < 8; i++) {
			sprintf(buf, "A%hu=%08lX", i, v_err_reg[i+8]);
			vdp_puts(VDP_PLANE_A, buf, x, y + i + 8);
		}
		
		// Other values
		x = 15; 
		switch(v_err_type) {
			case 0:
			case 1: // Bus / Address Error
			y = 20;
			sprintf(buf, "FUNC=%04hX   INST=%04hX", v_err_ext1, v_err_ext2);
			vdp_puts(VDP_PLANE_A, buf, x, y++);
			sprintf(buf, "ADDR=%06lX", v_err_addr);
			vdp_puts(VDP_PLANE_A, buf, x, y++);
			break;
			case 2: // Illegal
			y = 21;
			sprintf(buf, "OV=%04hX", v_err_ext1);
			vdp_puts(VDP_PLANE_A, buf, x, y++);
			break;
		}
		x = 2; y = 20;
		sprintf(buf, "PC=%06lX", v_err_pc);
		vdp_puts(VDP_PLANE_A, buf, x, y++);
		sprintf(buf, "SR=  %04hX", v_err_sr);
		vdp_puts(VDP_PLANE_A, buf, x, y++);
		
		// Stack dump
		x = 25; //y = 2;
		//vdp_puts(VDP_PLANE_A, "Stack", x, y);
		y = 4;
		uint32_t *sp = (uint32_t*) v_err_reg[15];
		for(uint16_t i = 0; i < 16; i++) {
			// Prevent wrapping around after reaching top of the stack
			if((uint32_t) sp < 0xFFF000) break;
			x = 15;
			sprintf(buf, "SP+%02X=%08lX", i << 3, *sp);
			vdp_puts(VDP_PLANE_A, buf, x, y);
			sp++;
			
			if((uint32_t) sp < 0xFFF000) break;
			x = 30;
			sprintf(buf, "%08lX", *sp);
			vdp_puts(VDP_PLANE_A, buf, x, y);
			sp++;
			
			y++;
		}
	} else {
		x = 2; y = 4;
		sprintf(buf, "%.32s:%hu", dieFile, dieLine);
		vdp_puts(VDP_PLANE_A, buf, x, y);
		if(dieMsg) {
			// Custom message
			x = 2; y = 6;
			for(uint16_t i = 0; i < 36*16; i++) {
				if(dieMsg[i] == '\n') {
					y++; x = 2; // Newline
				} else if(dieMsg[i] < 0x20) {
					break; // Invalid or nullchar
				} else {
					vdp_map_xy(VDP_PLANE_A, TILE_FONTINDEX + dieMsg[i] - 0x20, x++, y);
				}
			}
		}
	}
	
	// Message
	x = 2; y = 23;
	vdp_puts(VDP_PLANE_A, "This shouldn't happen. Report it!", x, y++);
    // Github link from ROM header
	vdp_puts(VDP_PLANE_A, (char*) 0x1D0 /*"twitter.com/donutgrind"*/, x + 4, y++);
    vdp_puts(VDP_PLANE_A, "andy@skychase.zone", x + 4, y++);
	
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
