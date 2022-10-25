#include "common.h"
#include "memory.h"
#include "system.h"
#include "vdp.h"
#include "joy.h"

const uint16_t btn[12] = {
	BUTTON_UP, BUTTON_DOWN, BUTTON_LEFT, BUTTON_RIGHT,
	BUTTON_B, BUTTON_C, BUTTON_A, BUTTON_START,
	BUTTON_Z, BUTTON_Y, BUTTON_X, BUTTON_MODE
};

const char btnName[12][4] = {
	"Up", "Dn", "Lt", "Rt",
	"B", "C", "A", "St",
	"Z", "Y", "X", "Md"
};

uint8_t joytype;
uint16_t joystate, oldstate;

void joy_init() {
	joystate = oldstate = 0;
    /* set the port bits direction */
    volatile uint8_t *pb = (volatile uint8_t*) 0xa10009;
    *pb = 0x40;
    pb += 2;
    *pb = 0x40;
    pb += 2;
    *pb = 0x40;
    /* set the port bits value */
    pb = (volatile uint8_t*) 0xa10003;
    *pb = 0x40;
    pb += 2;
    *pb = 0x40;
    pb += 2;
    *pb = 0x40;
}

void joy_update() {
	volatile uint8_t *pb = (volatile uint8_t*) 0xa10003;
    // On hardware, the C button did not work, but it did in emulators
    // Making this volatile fixed it for...reasons?
	volatile uint16_t v[4];
    for(uint16_t i = 0; i < 4; i++) {
		*pb = 0x00; /* TH (select) low */
		__asm__("nop");
		__asm__("nop");
		v[i] = *pb;
		*pb = 0x40; /* TH (select) high */
		v[i] <<= 8;
		v[i] |= *pb;
	}
    if((v[3] & 0x0F00) != 0x0F00) {
        joytype = JOY_TYPE_PAD3;
        v[2] = 0x0F00;
    } else {
        joytype = JOY_TYPE_PAD6;
        v[2] = (v[2] & 0x000F) << 8;
    }
    if(cfg_force_btn) joytype = cfg_force_btn - 1;
	oldstate = joystate;
    joystate = ((v[0] & 0x3000) >> 6) | (v[0] & 0x003F);
    joystate |= v[2];
    joystate ^= 0x0FFF;
}
