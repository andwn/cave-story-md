// Reset VRAM cursor and clear message tilemap cache
void cjk_reset(uint16_t vramIndex);

void cjk_newline();
// Scroll the message window (redraw row 2-3 in row 1-2 rows and clear row 3)
void cjk_winscroll(uint16_t win_x, uint16_t win_y);

// Render a 16x16 text glyph to the screen
void cjk_draw(uint16_t plan, uint16_t chr, uint16_t x, uint16_t y, uint16_t backCol, uint16_t shadow);

// Wrapper for cjk_draw to generate sprite tiles for map name (3x2 sprite = 2 glyphs)
// chr2 may be 0 if only 1 glyph is needed
void cjk_drawsprite(uint16_t offset, uint16_t chr1, uint16_t chr2);
