enum { CJK_TITLE, CJK_MESSAGE, CJK_CREDITS };

// Reset VRAM cursor and clear message tilemap cache
// Possible VRAM Mode values are in the above enum
// Tiles will get allocated to different places depending on this "mode"
void cjk_reset(uint16_t vramMode);

// Call this at the end of a line or string. It will reset the shift value,
// preventing the next char from overlapping the previous one
void cjk_newline();

// Scroll the message window (redraw row 2-3 in row 1-2 rows and clear row 3)
void cjk_winscroll(uint16_t win_x, uint16_t win_y);

// Render a 12x16 text glyph to the screen
void cjk_draw(uint16_t plan, uint16_t chr, uint16_t x, uint16_t y, uint16_t backCol, uint16_t shadow);

// Wrapper for cjk_draw to generate sprite tiles for map name (3x2 sprite = 2 glyphs)
// chr2 may be 0 if only 1 glyph is needed
void cjk_drawsprite(uint16_t offset, uint16_t chr1, uint16_t chr2);
