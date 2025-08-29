#ifndef CJK_H
#define CJK_H

enum { CJK_TITLE, CJK_MESSAGE, CJK_CREDITS };

// Reset VRAM cursor and clear message tilemap cache
// Possible VRAM Mode values are in the above enum
// Tiles will get allocated to different places depending on this "mode"
void cjk_reset(uint16_t vramMode);

// Set a specific tile index in the cjk vram space to draw next
// Used for menus where some items change a lot and others never change
void cjk_set_index(uint16_t vramMode, uint16_t index);

// Call this at the end of a line or string. It will reset the shift value,
// preventing the next char from overlapping the previous one
void cjk_newline(void);

// Scroll the message window (redraw row 2-3 in row 1-2 rows and clear row 3)
void cjk_winscroll(uint16_t win_x, uint16_t win_y);

// Render a 12x16 text glyph to the screen
void cjk_draw(uint16_t plan, uint16_t chr, uint16_t x, uint16_t y, uint16_t backCol, uint16_t shadow);

// Wrapper for cjk_draw to generate sprite tiles for map name (3x2 sprite = 2 glyphs)
// chr2 may be 0 if only 1 glyph is needed
void cjk_drawsprite(uint16_t offset, uint16_t chr1, uint16_t chr2);


static uint16_t GetNextChar(const uint8_t *dat, uint16_t index) {
	uint16_t chr = dat[index];
	if(chr >= 0xE0 && chr < 0xFF) {
		return (chr - 0xE0) * 0x60 + (dat[index + 1] - 0x20) + 0x100;
	} else {
		return chr;
	}
}

// Wrapper for either vdp_nputs or cjk_draw depending on the ROM's language
static uint16_t loc_vdp_nputs(uint16_t plane, const uint8_t *dat, uint16_t x, uint16_t y, uint16_t maxlen, uint16_t backCol) {
    if(cfg_language >= LANG_JA && cfg_language < LANG_RU) {
        uint16_t index = 0;
        for(uint16_t pos = 0; index < maxlen; pos++) {
            uint16_t c = GetNextChar(dat, index);
            if(c == 0) break; // End of string
            index += (c > 0xFF) ? 2 : 1;
            cjk_draw(plane, c, x, y, backCol, 1);
            x += 1 + (pos & 1);
        }
        cjk_newline();
        return index;
    } else {
        vdp_nputs(plane, (const char*) dat, x, y, maxlen);
        return maxlen;
    }
}

#endif
