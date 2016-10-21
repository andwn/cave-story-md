#ifndef VDP_EXT_H_
#define VDP_EXT_H_

#include <genesis.h>

/*
 * This contains some helper functions that don't exist in SGDK
 */

// 32 bytes of zero, can be sent to VDP to clear any tile
extern const u32 TILE_BLANK[8];
// FadeOut is almost completely black, except index 15 which is white
// This allows text to still be displayed after the screen fades to black
extern const u16 PAL_FadeOut[64];
// FullWhite is used for a TSC instruction that flashes the screen white
extern const u16 PAL_FullWhite[64];
// Palette data
u16 cachedPalette[64];
//u16 waterPalette[64];
//u16 waterPending;

// Window plane functions
void VDP_setWindowPos(u8 x, u8 y);
u8 VDP_getWindowPosX();
u8 VDP_getWindowPosY();
void VDP_drawTextWindow(const char *str, u16 x, u16 y);
void VDP_clearTextWindow(u16 x, u16 y, u16 w);

// Palette functions
// The "cached palette" is an in-memory copy of the full 64 colors in CRAM so 
// that they can be remembered during/after fades
void VDP_setCachedPalette(u16 pindex, u16 *colors);
void VDP_setCachedColor(u16 cindex, u16 color);
u16* VDP_getCachedPalette();

// Flash the screen white
void VDP_flashWhite();

// Water palette
//void VDP_genWaterPalette();
//void vblank_water();
//void hblank_water();

// Number draw functions
void VDP_drawInt(u32 n, u16 x, u16 y);
void VDP_drawByte(u8 n, u16 x, u16 y);
void VDP_drawWord(u16 n, u16 x, u16 y);

#endif
