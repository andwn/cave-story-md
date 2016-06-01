#ifndef VDP_EXT_H_
#define VDP_EXT_H_

#include <genesis.h>

extern const u32 TILE_BLANK[8];

// Window plane functions
void VDP_setWindowPos(u8 x, u8 y);
u8 VDP_getWindowPosX();
u8 VDP_getWindowPosY();
void VDP_drawTextWindow(const char *str, u16 x, u16 y);
void VDP_clearTextWindow(u16 x, u16 y, u16 w);

// Palette functions
extern const u16 PAL_FadeOut[64], PAL_FullWhite[64];

void VDP_setCachedPalette(u16 pindex, u16 *colors);
void VDP_setCachedColor(u16 cindex, u16 color);
u16* VDP_getCachedPalette();

// Number draw functions
void draw_byte(u8 n, u16 x, u16 y);
void draw_word(u16 n, u16 x, u16 y);

#endif
