#include "stage.h"

#include "entity.h"
#include "resources.h"
#include "tables.h"
#include "sprite.h"
#include "camera.h"
#include "tsc.h"
#include "player.h"
#include "input.h"
#include "system.h"
#include "effect.h"
#include "hud.h"

#define TS_WIDTH 32
#define TS_HEIGHT 20
#define TILE_BACKINDEX (TILE_USERINDEX + (TS_WIDTH * TS_HEIGHT))

u8 stageBackground;
s16 backScrollTable[32];
s8 morphingRow, morphingColumn;

void stage_load_tileset();
void stage_load_blocks();
void stage_load_tileflags();
void stage_load_entities();

void stage_draw_column(s16 _x, s16 _y);
void stage_draw_row(s16 _x, s16 _y);
void stage_draw_background();
void stage_draw_background2();
void stage_update_back();

void stage_load(u16 id) {
	SYS_disableInts();
	VDP_setEnable(false); // Turn the screen off, speeds up writes to VRAM
	player_lock_controls();
	hud_hide();
	// Clear out or deactivate stuff from the old stage
	effects_clear();
	entities_clear(FILTER_ALL, 0);
	sprites_clear();
	player_reset_sprites(); // Reloads player/weapon/hud sprites
	stageID = id;
	stageTileset = stage_info[id].tileset;
	stage_load_blocks();
	camera_set_position(player.x, player.y);
	VDP_setPalette(PAL2, tileset_info[stageTileset].palette->data);
	stage_load_tileset();
	if(stageBackground != stage_info[id].background) {
		stageBackground = stage_info[id].background;
		VDP_clearPlan(BPLAN, true);
		if(stageBackground > 0) {
			stageBackgroundType = background_info[stageBackground].type;
			if(stageBackgroundType == 0) { // Static
				VDP_loadTileSet(background_info[stageBackground].tileset, TILE_BACKINDEX, true);
				stage_draw_background();
			} else if(stageBackgroundType == 1) { // Moon/Sky
				VDP_loadTileSet(background_info[stageBackground].tileset, TILE_BACKINDEX, true);
				for(u8 y = 0; y < 32; y++) backScrollTable[y] = 0;
				stage_draw_background2();
			//} else if(stageBackgroundType == 2) { // Solid Color
			//	VDP_setBackgroundColor(background_info[stageBackground].palette->data);
			}
		}
	}
	stage_load_tileflags();
	stage_draw_area(sub_to_block(camera.x) - pixel_to_block(SCREEN_HALF_W),
			sub_to_block(camera.y) - pixel_to_block(SCREEN_HALF_H), 21, 15);
	stage_load_entities();
	tsc_load_stage(id);
	hud_show();
	player_unlock_controls();
	VDP_setEnable(true);
	SYS_enableInts();
}

void stage_load_tileset() {
	VDP_setPalette(PAL3, stage_info[stageID].npcPalette->data);
	VDP_loadTileSet(tileset_info[stageTileset].tileset, TILE_USERINDEX, true);
	// Inject the breakable block sprite into the tileset
	const u8 *PXA = tileset_info[stageTileset].PXA;
	for(u16 i = 0; i < 160; i++) {
		if(PXA[i] == 0x43) {
			u32 addr1 = ((i * 2) / TS_WIDTH * TS_WIDTH * 2) + ((i * 2) % TS_WIDTH),
			addr2 = ((i * 2) / TS_WIDTH * TS_WIDTH * 2) + ((i * 2) % TS_WIDTH) + TS_WIDTH;
			VDP_loadTileData(&TS_Break.tiles[0], TILE_USERINDEX + addr1, 2, true);
			VDP_loadTileData(&TS_Break.tiles[2], TILE_USERINDEX + addr2, 2, true);
		}
	}
}

void stage_load_blocks() {
	const u8 *PXM = stage_info[stageID].PXM;
	stageWidth = PXM[4] + (PXM[5] << 8);
	stageHeight = PXM[6] + (PXM[7] << 8);
	PXM += 8;
	MEM_free(stageBlocks);
	stageBlocks = MEM_alloc(stageWidth * stageHeight);
	for(u32 i = 0; i < stageWidth * stageHeight; i++) {
		stageBlocks[i] = PXM[i];
	}
}

void stage_load_tileflags() {
	const u8 *PXA = tileset_info[stageTileset].PXA;
	s16 startx = sub_to_block(camera.x) - 16,
		starty = sub_to_block(camera.y) - 16,
		endx = startx + 32, endy = starty + 32;
	if(startx < 0) startx = 0;
	if(starty < 0) starty = 0;
	if(endx > stageWidth) endx = stageWidth;
	if(endy > stageHeight) endy = stageHeight;
	for(u16 y = starty; y < endy; y++) {
		for(u16 x = startx; x < endx; x++) {
			stageTileFlags[x%32][y%32] = PXA[stage_get_block(x,y)];
		}
	}
}

void stage_load_entities() {
	const u8 *PXE = stage_info[stageID].PXE;
	stageEntityCount = PXE[4];
	for(u8 i = 0; i < stageEntityCount; i++) {
		u16 x, y, id, event, type, flags;
		x = PXE[8 + i * 12] + (PXE[9 + i * 12]<<8);
		y = PXE[10 + i * 12] + (PXE[11 + i * 12]<<8);
		id = PXE[12 + i * 12] + (PXE[13 + i * 12]<<8);
		event = PXE[14 + i * 12] + (PXE[15 + i * 12]<<8);
		type = PXE[16 + i * 12] + (PXE[17 + i * 12]<<8);
		flags = PXE[18 + i * 12] + (PXE[19 + i * 12]<<8);
		if(id==0 && event==0 && type==0 && flags==0) continue;
		if((flags&NPC_DISABLEONFLAG) && system_get_flag(id)) continue;
		if((flags&NPC_ENABLEONFLAG) && !system_get_flag(id)) continue;
		entity_create(x, y, id, event, type, flags);
	}
}

bool stage_get_block_solid(u16 x, u16 y, bool checkNpcSolid) {
	u8 block = stage_get_block_type(x, y);
	if(block&BLOCK_SLOPE) return false;
	block &= 0xF;
	return block == 1 || block == 3 || (checkNpcSolid && block == 4);
}

void stage_replace_block(u16 bx, u16 by, u8 index) {
	//entity_create(bx, by, 0, 0, 4, 0);
	stageTileFlags[bx%32][by%32] = tileset_info[stageTileset].PXA[index];
	stageBlocks[by * stageWidth + bx] = index;
	stage_draw_area(bx, by, 1, 1);
	effect_create_smoke(1, block_to_pixel(bx) + 8, block_to_pixel(by) + 8);
}

// TODO: Use DMA here
void stage_update() {
	// Column
	if(morphingColumn != 0) {
		s16 x = sub_to_block(camera.x) + (morphingColumn*10),
			y = sub_to_block(camera.y);
		if(x >= 0 && x < stageWidth) stage_draw_column(x, y);
		morphingColumn = 0;
	}
	// Row
	if(morphingRow != 0) {
		s16 x = sub_to_block(camera.x),
			y = sub_to_block(camera.y) + (morphingRow*7);
		if(y >= 0 && y < stageHeight) stage_draw_row(x, y);
		morphingRow = 0;
	}
	s16 off[32];
	for(u8 i = 0; i < 32; i++) {
		off[i] = -sub_to_pixel(camera.x) + SCREEN_HALF_W;
	}
	// Foreground scrolling
	VDP_setHorizontalScrollTile(PLAN_A, 0, off, 32, true);
	//VDP_setHorizontalScroll(PLAN_A, -sub_to_pixel(camera.x) + SCREEN_HALF_W);
	VDP_setVerticalScroll(PLAN_A, sub_to_pixel(camera.y) - SCREEN_HALF_H);
	// Background Scrolling
	if(stageBackgroundType == 1) stage_update_back();
}

void stage_update_back() {
	for(u8 y = 0; y < 32; y++) {
		if(y < 12) backScrollTable[y] = 0;
		else if(y < 16) backScrollTable[y] += 1;
		else if(y < 20) backScrollTable[y] += 2;
		else backScrollTable[y] += 3;
	}
	VDP_setHorizontalScrollTile(PLAN_B, 0, backScrollTable, 32, true);
}

void stage_draw_column(s16 _x, s16 _y) {
	u16 attr[4], t, b, pal; u8 p;
	for(s16 y = _y-8; y < _y+8; y++) {
		if(y < 0) continue;
		if(y >= stageHeight) break;
		p = (stage_get_block_type(_x, y) & 0x40) > 0;
		pal = stage_get_block_type(_x, y) == 0x43 ? PAL1 : PAL2;
		t = block_to_tile(stage_get_block(_x, y));
		b = TILE_USERINDEX + (t / TS_WIDTH * TS_WIDTH * 2) + (t % TS_WIDTH);
		attr[0] = TILE_ATTR_FULL(pal, p, 0, 0, b);
		attr[1] = TILE_ATTR_FULL(pal, p, 0, 0, b+1);
		attr[2] = TILE_ATTR_FULL(pal, p, 0, 0, b+TS_WIDTH);
		attr[3] = TILE_ATTR_FULL(pal, p, 0, 0, b+TS_WIDTH+1);
		VDP_setTileMapDataRect(APLAN, attr, block_to_tile(_x)%64, block_to_tile(y)%32, 2, 2);
	}
}

void stage_draw_row(s16 _x, s16 _y) {
	u16 attr[4], t, b, pal; u8 p;
	for(s16 x = _x-11; x < _x+11; x++) {
		if(x < 0) continue;
		if(x >= stageWidth) break;
		p = (stage_get_block_type(x, _y) & 0x40) > 0;
		pal = stage_get_block_type(x, _y) == 0x43 ? PAL1 : PAL2;
		t = block_to_tile(stage_get_block(x, _y));
		b = TILE_USERINDEX + (t / TS_WIDTH * TS_WIDTH * 2) + (t % TS_WIDTH);
		attr[0] = TILE_ATTR_FULL(pal, p, 0, 0, b);
		attr[1] = TILE_ATTR_FULL(pal, p, 0, 0, b+1);
		attr[2] = TILE_ATTR_FULL(pal, p, 0, 0, b+TS_WIDTH);
		attr[3] = TILE_ATTR_FULL(pal, p, 0, 0, b+TS_WIDTH+1);
		VDP_setTileMapDataRect(APLAN, attr, block_to_tile(x)%64, block_to_tile(_y)%32, 2, 2);
	}
}

void stage_morph(s16 _x, s16 _y, s8 x_dir, s8 y_dir) {
	// Cache the tile flag data
	const u8 *PXA = tileset_info[stageTileset].PXA;
	if(x_dir != 0) {
		s16 x = _x + (x_dir*15);
		if(x >= 0 && x < stageWidth) {
			for(s16 y = _y-16; y < _y+16; y++) {
				if(y < 0) continue;
				if(y >= stageHeight) break;
				stageTileFlags[x%32][y%32] = PXA[stage_get_block(x,y)];
			}
		}
	}
	if(y_dir != 0) {
		s16 y = _y + (y_dir*15);
		if(y >= 0 && y < stageHeight) {
			for(s16 x = _x-16; x < _x+16; x++) {
				if(x < 0) continue;
				if(x >= stageWidth) break;
				stageTileFlags[x%32][y%32] = PXA[stage_get_block(x,y)];
			}
		}
	}
	// Draw part of the stage during vblank
	morphingColumn = x_dir;
	morphingRow = y_dir;
}

void stage_draw_area(u16 _x, u16 _y, u8 _w, u8 _h) {
	u16 t, b, xx, yy, pal; u8 p;
	for(u16 y = _y; y < _y + _h; y++) {
		for(u16 x = _x; x < _x + _w; x++) {
			p = (stage_get_block_type(x, y) & 0x40) > 0;
			pal = stage_get_block_type(x, y) == 0x43 ? PAL1 : PAL2;
			t = block_to_tile(stage_get_block(x, y));
			b = TILE_USERINDEX + (t / TS_WIDTH * TS_WIDTH * 2) + (t % TS_WIDTH);
			xx = block_to_tile(x) % 64;
			yy = block_to_tile(y) % 32;
			VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(pal, p, 0, 0, b), xx, yy);
			VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(pal, p, 0, 0, b+1), xx+1, yy);
			VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(pal, p, 0, 0, b+TS_WIDTH), xx, yy+1);
			VDP_setTileMapXY(APLAN, TILE_ATTR_FULL(pal, p, 0, 0, b+TS_WIDTH+1), xx+1, yy+1);
		}
	}
}

void stage_draw_background() {
	u8 w = background_info[stageBackground].width;
	u8 h = background_info[stageBackground].height;
	for(u8 y = 0; y < 32; y++) {
		for(u8 x = 0; x < 64; x++) {
			u16 b = TILE_BACKINDEX + (x%w) + ((y%h) * w);
			VDP_setTileMapXY(BPLAN, TILE_ATTR_FULL(PAL2, 0, 0, 0, b), x, y);
		}
	}
}

void stage_draw_background2() {
	for(u8 y = 0; y < 28; y++) {
		for(u16 x = 0; x < 64; x++) {
			u16 b = TILE_BACKINDEX;;
			if(y < 4 || (y >= 8 && y < 12)) { // Draw blank sky
				// Keep b how it is
			} else if(y < 8) { // Between 4-7, draw moon
				if(x >= 4 && x < 8) {
					b += 1 + ((y-4)*16) + (x-4);
				}
			} else if(y == 12) { // Top clouds
				b += 5 + (x%11);
			} else if(y < 16) { // Between top and mid
				b += 16;
			} else if(y == 16) { // Mid clouds
				b += 5 + 16 + (x%11);
			} else if(y < 20) {
				b += 32;
			} else if(y == 20) { // Bottom clouds
				b += 5 + 32 + (x%11);
			} else if(y == 21) { // Bottom clouds 2
				b += 5 + 48 + (x%11);
			} else {
				b += 48;
			}
			VDP_setTileMapXY(BPLAN, TILE_ATTR_FULL(PAL2, 0, 0, 0, b), x, y);
		}
	}
}

/*
void stage_cache_area(s16 _x, s16 _y, u8 _w, u8 _h) {
	const u8 *PXA = tileset_info[stageTileset].PXA;
	for(s16 y = _y; y < _y + _h; y++) {
		if(y < 0) continue;
		if(y >= stageHeight) break;
		for(s16 x = _x; x < _x + _w; x++) {
			if(x < 0) continue;
			if(x >= stageWidth) break;
			stageTileFlags[x%32][y%32] = PXA[stage_get_block(x,y)];
		}
	}
}
*/
/*
void stage_draw_area(u16 x, u16 y, u8 w, u8 h) {
	u16 index = 0, t, tile;
	u16 *b = MEM_alloc(block_to_tile(w) * block_to_tile(h));
	for(u16 sy = y; sy < y + h; sy++) {
		for(u16 sx = x; sx < x + w; sx++) {
			t = block_to_tile(stage_get_block(sx, sy));
			tile = TILE_USERINDEX + (t / TS_WIDTH * TS_WIDTH * 2) + (t % TS_WIDTH);
			b[index] = TILE_ATTR_FULL(PAL2, 0, 0, 0, tile);
			b[index + 1] = TILE_ATTR_FULL(PAL2, 0, 0, 0, tile + 1);
			b[index + block_to_tile(w)] =
					TILE_ATTR_FULL(PAL2, 0, 0, 0, tile + TS_WIDTH);
			b[index + block_to_tile(w) + 1] =
					TILE_ATTR_FULL(PAL2, 0, 0, 0, tile + TS_WIDTH + 1);
			index += 2;
		}
		index += block_to_tile(w);
	}
	VDP_setTileMapDataRect(APLAN, b,
			block_to_tile(x) % 64, block_to_tile(y) % 64,
			block_to_tile(w), block_to_tile(h));
	MEM_free(b);
}
*/
/*
if(stageBackgroundType == 1) {
	//stageBackgroundH++;
	VDP_setScrollingMode(HSCROLL_TILE, VSCROLL_PLANE);
	for(u8 y = 0; y < 16; y++) {
		backScrollTable[y] += 1 + (y>>2);
	}
	VDP_setHorizontalScrollTile(PLAN_B, 12, backScrollTable, 16, true);
	// 20 - bottom
	//s16 values1[2] = { value*3, value*3 };
	//VDP_setHorizontalScrollTile(PLAN_B, 20, values1, 2, true);
	// 16 - mid
	//s16 values2[4] = { value*2, value*2, value*2, value*2 };
	//VDP_setHorizontalScrollTile(PLAN_B, 16, values2, 4, true);
	// 12 - upper
	//s16 values3[4] = { value, value, value, value };
	//VDP_setHorizontalScrollTile(PLAN_B, 12, values3, 4, true);
	VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
}
*/
/*
	if(stageBackground != stage_info[id].background) {
		stageBackground = stage_info[id].background;
		VDP_clearPlan(BPLAN, true);
		if(stageBackground > 0) {
			stageBackgroundType = background_info[stageBackground].type;
			if(stageBackgroundType == 0) { // Static
				//VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
				//VDP_setPalette(PAL3, background_info[stageBackground].palette->data);
				VDP_loadTileSet(background_info[stageBackground].tileset, TILE_BACKINDEX, true);
				stage_draw_back();
			} //else if(stageBackgroundType == 1) { // Moon/Sky
			//	//VDP_setScrollingMode(HSCROLL_TILE, VSCROLL_PLANE);
			//	VDP_setPalette(PAL3, background_info[stageBackground].palette->data);
			//	VDP_loadTileSet(background_info[stageBackground].tileset, BACKGROUND_USERINDEX, true);
			//	stage_draw_back2();
			//} else if(stageBackgroundType == 2) { // Solid Color
			//	VDP_setBackgroundColor(background_info[stageBackground].palette->data);
			//}
		}
	}
	 */
