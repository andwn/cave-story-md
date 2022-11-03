#include "common.h"
#include "md/stdlib.h"
#include "system.h"
#include "vdp.h"
#include "md/joy.h"
#include "xgm.h"

const uint16_t btn[12] = {
        JOY_UP, JOY_DOWN, JOY_LEFT, JOY_RIGHT,
        JOY_B, JOY_C, JOY_A, JOY_START,
        JOY_Z, JOY_Y, JOY_X, JOY_MODE
};

const char btnName[12][4] = {
	"Up", "Dn", "Lt", "Rt",
	"B", "C", "A", "St",
	"Z", "Y", "X", "Md"
};

static volatile uint8_t * const io_ctrl1 = (uint8_t*) 0xA10009;
static volatile uint8_t * const io_ctrl2 = (uint8_t*) 0xA1000B;
static volatile uint8_t * const io_data1 = (uint8_t*) 0xA10003;
static volatile uint8_t * const io_data2 = (uint8_t*) 0xA10005;

uint8_t joytype;
uint16_t joystate, joystate_old;

void joy_init() {
    z80_pause_fast();
    *io_ctrl1 = 0x40;
    *io_ctrl2 = 0x40;
    *io_data1 = 0x40;
    *io_data2 = 0x40;
    z80_resume();
}

void joy_update() {
    joytype = JOY_TYPE_PAD3;
    joystate_old = joystate;
    z80_pause_fast();

    *io_data1 = 0x40;                       /* **CB.RLDU */
    __asm__("nop");
    __asm__("nop");
    joystate = *io_data1 & 0x3F;            /* 0000.00CB.RLDU */
    __asm__("": : :"memory");
    *io_data1 = 0x00;                       /* **SA.**** */
    __asm__("nop");
    __asm__("nop");
    joystate |= (*io_data1 & 0x30) << 2;    /* 0000.SACB.RLDU */

    if(cfg_force_btn != 1) {
        *io_data1 = 0x40;       /* Skip */
        __asm__("nop");
        __asm__("nop");
        *io_data1 = 0x00;       /* Skip */
        __asm__("nop");
        __asm__("nop");
        *io_data1 = 0x40;       /* Skip */
        __asm__("nop");
        __asm__("nop");
        *io_data1 = 0x00;       /* Lower 4 bits 0 if 6 button controller */
        __asm__("nop");
        __asm__("nop");
        if(!(*io_data1 & 0x0F) || cfg_force_btn == 2) {
            joytype = JOY_TYPE_PAD6;
            *io_data1 = 0x40;                       /* ****.MXYZ */
            __asm__("nop");
            __asm__("nop");
            joystate |= (*io_data1 & 0x0F) << 8;    /* MXYZ.SACB.RLDU */
        }
    }

    // Bits are 1 when the button is NOT pressed, the opposite of what we want
    // Also, keep the unused upper 4 bits cleared
    joystate = ~joystate & 0x0FFF;
    z80_resume();
}
