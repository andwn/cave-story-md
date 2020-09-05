// Render a 16x16 text glyph to the screen
// Whether this glyph is kanji, kana, or ascii they are all referred to as 'kanji' in the code
void kanji_draw(uint16_t plan, uint16_t vramIndex, uint16_t chr, uint16_t x, uint16_t y, uint16_t backCol, uint8_t shadow);
// Similar to kanji_draw, but renders the tiles in sprite order and doesn't touch the nametables
void kanji_loadtilesforsprite(uint16_t vramIndex, uint16_t chr1, uint16_t chr2);
