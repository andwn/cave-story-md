#include "types.h"
#include "error.h"
#include "gamemode.h"
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

static const char STR_ERROR[5][16] = {
        "Fatal Error", "Address Error", "Bad Instruction", "Divide by Zero", "Out of Memory"
};
static const char hexchars[16] = "0123456789ABCDEF";

void print_reg(char *str, char kind, uint16_t index, uint16_t x, uint16_t y) {
    index &= 7;
    str[0] = kind;
    str[1] = '0' + index;
    str[2] = '=';
    str[3] = 0;
    vdp_puts(VDP_PLANE_A, str, x, y);
}

void print_hex(char *str, uint32_t val, uint16_t digits, uint16_t x, uint16_t y) {
    if(digits > 8) digits = 8;
    for(uint16_t i = 0; i < digits; i++) {
        str[digits - i - 1] = hexchars[(val >> (i << 2)) & 0xF];
    }
    str[digits] = 0;
    vdp_puts(VDP_PLANE_A, str, x, y);
}

__attribute__((noreturn))
void _error() {
	char buf[16];
	// Don't completely clear the screen or rewrite the palettes
    // Only what is needed to display the error
    disable_ints();

    vdp_sprites_clear();
    vdp_sprites_update();
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
#ifndef PROFILE
	switch(v_err_type) {
		case 1: vdp_color(0, 0x800); break;
		case 2: vdp_color(0, 0x008); break;
		case 3: vdp_color(0, 0x046); break;
		case 4: vdp_color(0, 0x080); break;
		default: vdp_color(0, 0x404);
	}
#endif
    if(v_err_type > 4) v_err_type = 0;
    vdp_puts(VDP_PLANE_A, STR_ERROR[v_err_type], 2, 2);
	
	if(v_err_type < 4) {
		// Registers
        uint16_t x = 2, y = 4;
        for(uint16_t i = 0; i < 8; i++) {
            print_reg(buf, 'D', i, x, y);
            print_hex(buf, v_err_reg[i], 8, x + 3, y);
            y++;
        }
        for(uint16_t i = 0; i < 8; i++) {
            print_reg(buf, 'A', i, x, y);
            print_hex(buf, v_err_reg[i+8], 8, x + 3, y);
            y++;
        }
        // Other values
        x = 15;
        switch(v_err_type) {
            case 0:
            case 1: // Bus / Address Error
                y = 20;
                vdp_puts(VDP_PLANE_A, "FUNC=", x, y);
                print_hex(buf, v_err_ext1, 4, x + 5, y);
                vdp_puts(VDP_PLANE_A, "INST=", x + 12, y);
                print_hex(buf, v_err_ext2, 4, x + 17, y);
                y++;
                vdp_puts(VDP_PLANE_A, "ADDR=", x, y);
                print_hex(buf, v_err_addr, 6, x + 5, y);
                break;
            case 2: // Illegal
                y = 21;
                vdp_puts(VDP_PLANE_A, "OV=", x, y);
                print_hex(buf, v_err_ext1, 4, x + 3, y);
                break;
        }
        x = 2; y = 20;
        vdp_puts(VDP_PLANE_A, "PC=", x, y);
        print_hex(buf, v_err_pc, 6, x + 3, y);
        y++;
        vdp_puts(VDP_PLANE_A, "SR=  ", x, y);
        print_hex(buf, v_err_sr, 4, x + 5, y);

        // Stack dump
        x = 25; y = 4;
        uint32_t *sp = (uint32_t*) v_err_reg[15];
        for(uint16_t i = 0; i < 16; i++) {
            // Prevent wrapping around after reaching top of the stack
            if((uint32_t) sp < 0xFFF000) break;
            x = 15;
            vdp_puts(VDP_PLANE_A, "SP+", x, y);
            print_hex(buf, i << 3, 2, x + 3, y);
            vdp_puts(VDP_PLANE_A, "=", x + 5, y);
            print_hex(buf, *sp, 8, x + 6, y);
            sp++;

            if((uint32_t) sp < 0xFFF000) break;
            x = 30;
            print_hex(buf, *sp, 8, x, y);
            sp++;

            y++;
        }
	} else {
		uint8_t y = 4;
        vdp_puts(VDP_PLANE_A, dieFile, 2, y);
        vdp_puts(VDP_PLANE_A, ":", 32, y);
        uint8_t x = 36;
        do {
            vdp_map_xy(VDP_PLANE_A, TILE_FONTINDEX + '0' + (dieLine % 10) - 0x20, x--, y);
            dieLine /= 10;
        } while(dieLine);
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
	
    print_version();
	
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
