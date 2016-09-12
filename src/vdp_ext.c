#include "vdp_ext.h"
#include "ai.h"

const u32 TILE_BLANK[8] = {0,0,0,0,0,0,0,0};

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

// Palette functions
const u16 PAL_FadeOut[64] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0xEEE,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
const u16 PAL_FullWhite[64] = {
	0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,
	0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,
	0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,
	0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE};

void VDP_setCachedPalette(u16 pindex, u16 *colors) {
	if(pindex >= 4) return;
	memcpy(&cachedPalette[pindex * 16], colors, 16 * 2);
}

void VDP_setCachedColor(u16 cindex, u16 color) {
	if(cindex >= 64) return;
	cachedPalette[cindex] = color;
}

u16* VDP_getCachedPalette() {
	return cachedPalette;
}

void VDP_genWaterPalette() {
	for(u8 i = 0; i < 64; i++) {
		u16 color = cachedPalette[i];
		if((color & 0xF00) < 0xE00) color += 0x200; // Blue
		if((color & 0x00F) > 0x000) color -= 0x002; // Red
		waterPalette[i] = color;
	}
}

// Water interrupt handlers
void vblank_water() {
	if(water_screenlevel < 3) {
		// Use only water palette and disable handler when water is above screen
		VDP_setHInterrupt(FALSE);
		VDP_setPaletteColors(0, waterPalette, 64);
		waterPending = 0;
	} else {
		// Set HInt timer to the water position
		VDP_setHInterrupt(TRUE);
		VDP_setPaletteColors(0, cachedPalette, 64);
		waterPending = 1;
		VDP_setHIntCounter(water_screenlevel - 1);
	}
}

void hblank_water() {
	if(!waterPending) return;
	VDP_setPaletteColors(0, waterPalette, 64);
	waterPending = 0;
}

// Number draw functions
void VDP_drawInt(u32 n, u16 x, u16 y) {
	char str[12];
	intToStr(n, str, 1);
	VDP_drawText(str, x, y);
}

void VDP_drawByte(u8 n, u16 x, u16 y) {
	char b[2] = { (n>>4)+0x30, (n&0xF)+0x30 };
	for(u8 i=0;i<2;i++) { 
		if(b[i] > 0x39) b[i] += 0x7;
	}
	VDP_drawText(b, x, y);
}

void VDP_drawWord(u16 n, u16 x, u16 y) {
	VDP_drawByte(n >> 8, x, y); 
	VDP_drawByte(n & 0xFF, x + 2, y);
}
