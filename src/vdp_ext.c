#include "vdp_ext.h"

// Window functions
void VDP_setWindowPos(u8 x, u8 y) {
	VDP_setReg(0x11, x);
	VDP_setReg(0x12, y);
}

u8 VDP_getWindowPosX() {
	return VDP_getReg(0x11);
}

u8 VDP_getWindowPosY() {
	return VDP_getReg(0x12);
}

void VDP_drawTextWindow(const char *str, u16 x, u16 y) {
	VDP_drawTextBG(PLAN_WINDOW, str, x, y);
}

void VDP_clearTextWindow(u16 x, u16 y, u16 w) {
	VDP_clearTextBG(PLAN_WINDOW, x, y, w);
}
