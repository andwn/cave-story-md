/*
 * This contains some helper functions that don't exist in SGDK
 */

// 32 bytes of zero, can be sent to VDP to clear any tile
extern const uint32_t TILE_BLANK[8];
// FadeOut is almost completely black, except index 15 which is white
// This allows text to still be displayed after the screen fades to black
extern const uint16_t PAL_FadeOut[64];
extern const uint16_t PAL_FadeOutBlue[64];
// FullWhite is used for a TSC instruction that flashes the screen white
extern const uint16_t PAL_FullWhite[64];
// Palette data
uint16_t cachedPalette[64];

// Window plane functions
void VDP_setWindowPos(uint8_t x, uint8_t y);
uint8_t VDP_getWindowPosX();
uint8_t VDP_getWindowPosY();
void VDP_drawTextWindow(const char *str, uint16_t x, uint16_t y);
void VDP_clearTextWindow(uint16_t x, uint16_t y, uint16_t w);

// Palette functions
// The "cached palette" is an in-memory copy of the full 64 colors in CRAM so 
// that they can be remembered during/after fades
void VDP_setCachedPalette(uint16_t pindex, uint16_t *colors);
void VDP_setCachedColor(uint16_t cindex, uint16_t color);
uint16_t* VDP_getCachedPalette();

// Flash the screen white
void VDP_flashWhite();

// Number draw functions
void VDP_drawInt(uint32_t n, uint16_t x, uint16_t y);
void VDP_drawByte(uint8_t n, uint16_t x, uint16_t y);
void VDP_drawWord(uint16_t n, uint16_t x, uint16_t y);
