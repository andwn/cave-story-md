#include "common.h"

#include "memory.h"
#include "system.h"
#include "vdp.h"

#include "joy.h"

#define JOY_TYPE_SHIFT          12

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

static volatile uint8_t joyType[JOY_NUM];
static volatile uint16_t joyState[JOY_NUM];

void JOY_init() {
	/*
     * Initialize ports for peripheral interface protocol - default to
     * TH Control Method for pads
     */
	volatile uint8_t *pb;
    /* set the port bits direction */
    pb = (volatile uint8_t *)0xa10009;
    *pb = 0x40;
    pb += 2;
    *pb = 0x40;
    pb += 2;
    *pb = 0x40;
    /* set the port bits value */
    pb = (volatile uint8_t *)0xa10003;
    *pb = 0x40;
    pb += 2;
    *pb = 0x40;
    pb += 2;
    *pb = 0x40;

    vsync();
    JOY_update();
    vsync();
    JOY_update();

    for (uint8_t i=JOY_1; i<JOY_NUM; i++) {
        joyType[i] = JOY_TYPE_PAD6;
        joyState[i] = 0;
    }
    /* wait a few vblanks for JOY_update() to get valid data */
    vsync();
    JOY_update();
    vsync();
    JOY_update();
    vsync();
    JOY_update();
}

uint8_t JOY_getJoypadType(uint16_t joy) {
    return joyType[joy];
}

uint16_t JOY_readJoypad(uint16_t joy) {
    if (joy == JOY_ALL) {
        uint16_t i;
        uint16_t res;

        res = 0;
        i = JOY_NUM;
        while(i--) res |= joyState[i] &  BUTTON_ALL;

        return res;
    }
    else if (joy < JOY_NUM)
        return joyState[joy];

    return 0;
}

static uint16_t TH_CONTROL_PHASE(volatile uint8_t *pb) {
    uint16_t val;

    *pb = 0x00; /* TH (select) low */
    __asm__("nop");
    __asm__("nop");
    val = *pb;

    *pb = 0x40; /* TH (select) high */
    val <<= 8;
    val |= *pb;

    return val;
}

static uint16_t read6Btn() {
    uint16_t val, v1, v2;
    volatile uint8_t *pb = (volatile uint8_t *)0xa10003;
    v1 = TH_CONTROL_PHASE(pb);                    /* - 0 s a 0 0 d u - 1 c b r l d u */
    val = TH_CONTROL_PHASE(pb);                   /* - 0 s a 0 0 d u - 1 c b r l d u */
    v2 = TH_CONTROL_PHASE(pb);                    /* - 0 s a 0 0 0 0 - 1 c b m x y z */
    val = TH_CONTROL_PHASE(pb);                   /* - 0 s a 1 1 1 1 - 1 c b r l d u */

    if ((val & 0x0F00) != 0x0F00) v2 = (JOY_TYPE_PAD3 << JOY_TYPE_SHIFT) | 0x0F00; /* three button pad */
    else v2 = (JOY_TYPE_PAD6 << JOY_TYPE_SHIFT) | ((v2 & 0x000F) << 8); /* six button pad */

    val = ((v1 & 0x3000) >> 6) | (v1 & 0x003F);   /* 0 0 0 0 0 0 0 0 s a c b r l d u  */
    val |= v2;                                    /* 0 0 0 1 m x y z s a c b r l d u  or  0 0 0 0 1 1 1 1 s a c b r l d u */
    val ^= 0x0FFF;                                /* 0 0 0 1 M X Y Z S A C B R L D U  or  0 0 0 0 0 0 0 0 S A C B R L D U */

    return val;
}

void JOY_update() {
	uint16_t val = read6Btn();
	joyType[JOY_1] = cfg_force_btn == 0 ? (val >> JOY_TYPE_SHIFT) :
                     cfg_force_btn == 1 ? JOY_TYPE_PAD3 : JOY_TYPE_PAD6;
	joyState[JOY_1] = val & BUTTON_ALL;
}
