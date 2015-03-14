#include "vdp_ext.h"

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
	VDP_drawTextBG(WINDOW, str, 0, x, y);
}

void VDP_clearTextWindow(u16 x, u16 y, u16 w) {
	VDP_clearTextBG(WINDOW, x, y, w);
}

/*
 *  rect[0] | rect[1]
 * -------------------
 *  rect[2] | rect[3]
 *//*
void VDP_setTileMapDataRectWrap(u16 plan, const u16 *data, u16 x, u16 y, u16 w, u16 h) {
	bool tooWide = false, tooHigh = false;
	u16 planwidth = VDP_getPlanWidth(),
		planheight = VDP_getPlanHeight();
	u16 rect[4][4] = {
			{ x % planwidth, y % planheight, w, h },
			{ 0, y % planheight, 0, h },
			{ x % planwidth, 0, w, 0 },
			{ 0, 0, 0, 0 },
	};
	u16 planwidth = VDP_getPlanWidth(),
		planheight = VDP_getPlanHeight();
	if(x + w > planwidth) {
		tooWide = true;
		// Clip the width off 0 and 2
		rect[0][2] = rect[2][2] = ((x + w) % planwidth) - x;
		// Set remaining width to 1 and 3
		rect[1][2] = rect[3][2] = w - rect[0][2];
	}
	if(y + h > planheight) {
		tooHigh = true;
		// Clip the height off 0 and 1
		rect[0][3] = rect[1][3] = ((y + h) % planheight) - y;
		// Set remaining height to 2 and 3
		rect[2][3] = rect[3][3] = y - rect[0][3];
	}
	// Draw upper left

	if(tooWide) { // Draw upper right

	}
	if(tooHigh) { // draw lower left

	}
	if(tooWide && tooHigh) { // Draw lower right

	}
}
*/
