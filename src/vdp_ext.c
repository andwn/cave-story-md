#include "common.h"
#include "memory.h"
#include "string.h"
#include "vdp.h"
#include "vdp_bg.h"
#include "vdp_pal.h"

#include "vdp_ext.h"

const uint32_t TILE_BLANK[8] = {0,0,0,0,0,0,0,0};

// Window functions
void VDP_setWindowPos(uint8_t x, uint8_t y) {
	VDP_setReg(0x11, x);
	VDP_setReg(0x12, y);
}

uint8_t VDP_getWindowPosX() {
	return VDP_getReg(0x11);
}

uint8_t VDP_getWindowPosY() {
	return VDP_getReg(0x12);
}

void VDP_drawTextWindow(const char *str, uint16_t x, uint16_t y) {
	VDP_drawTextBG(PLAN_WINDOW, str, x, y);
}

void VDP_clearTextWindow(uint16_t x, uint16_t y, uint16_t w) {
	VDP_clearTextBG(PLAN_WINDOW, x, y, w);
}

// Palette functions
const uint16_t PAL_FadeOut[64] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0xEEE,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
const uint16_t PAL_FullWhite[64] = {
	0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,
	0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,
	0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,
	0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE};

void VDP_setCachedPalette(uint16_t pindex, uint16_t *colors) {
	if(pindex >= 4) return;
	memcpy(&cachedPalette[pindex * 16], colors, 16 * 2);
}

void VDP_setCachedColor(uint16_t cindex, uint16_t color) {
	if(cindex >= 64) return;
	cachedPalette[cindex] = color;
}

uint16_t* VDP_getCachedPalette() {
	return cachedPalette;
}

void VDP_flashWhite() {
	
	VDP_setPaletteColors(0, PAL_FullWhite, 64);
	VDP_fadeTo(0, 63, VDP_getCachedPalette(), 10, TRUE);
	
}

// Number draw functions
void VDP_drawInt(uint32_t n, uint16_t x, uint16_t y) {
	char str[12];
	intToStr(n, str, 1);
	VDP_drawText(str, x, y);
}

void VDP_drawByte(uint8_t n, uint16_t x, uint16_t y) {
	char b[2] = { (n>>4)+0x30, (n&0xF)+0x30 };
	for(uint8_t i=0;i<2;i++) { 
		if(b[i] > 0x39) b[i] += 0x7;
	}
	VDP_drawText(b, x, y);
}

void VDP_drawWord(uint16_t n, uint16_t x, uint16_t y) {
	VDP_drawByte(n >> 8, x, y); 
	VDP_drawByte(n & 0xFF, x + 2, y);
}
