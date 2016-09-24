#include "stage.h"

#include "entity.h"
#include "resources.h"
#include "tables.h"
#include "camera.h"
#include "tsc.h"
#include "player.h"
#include "input.h"
#include "system.h"
#include "effect.h"
#include "hud.h"
#include "vdp_ext.h"
#include "ai.h"
#include "sheet.h"

// Index of background in db/back.c and the effect type
u8 stageBackground = 0xFF, stageBackgroundType;

s16 backScrollTable[32];
u8 *stageBlocks = NULL;
u16 *stageTable = NULL;

void stage_load_tileset();
void stage_load_blocks();
void stage_load_entities();

void stage_draw_area(u16 _x, u16 _y, u8 _w, u8 _h);
void stage_draw_screen();
void stage_draw_background();
void stage_draw_moonback();

void stage_load(u16 id) {
	u8 vdpEnabled = VDP_getEnable();
	if(vdpEnabled) {
		SYS_disableInts();
		VDP_setEnable(FALSE); // Turn the screen off, speeds up writes to VRAM
	}
	stageID = id;
	// Clear out or deactivate stuff from the old stage
	effects_clear();
	entities_clear();
	if(stageBlocks) {
		MEM_free(stageBlocks);
		stageBlocks = NULL;
	}
	if(stageTable) {
		MEM_free(stageTable);
		stageTable = NULL;
	}
	water_screenlevel = WATER_DISABLE;
	water_entity = NULL;
	// Load the tileset
	if(stageTileset != stage_info[id].tileset) {
		stageTileset = stage_info[id].tileset;
		stage_load_tileset();
	}
	// Load sprite sheets
	sheets_load_stage(id);
	// Stage palette and shared NPC palette
	VDP_setCachedPalette(PAL2, tileset_info[stageTileset].palette->data);
	VDP_setCachedPalette(PAL3, stage_info[id].npcPalette->data);
	if(stageBackground != stage_info[id].background) {
		stageBackground = stage_info[id].background;
		stageBackgroundType = background_info[stageBackground].type;
		if(stageBackgroundType == 0) { // Tiled image
			VDP_loadTileSet(background_info[stageBackground].tileset, TILE_BACKINDEX, TRUE);
			stage_draw_background();
		} else if(stageBackgroundType == 1) { // Moon/Sky
			VDP_loadTileSet(background_info[stageBackground].tileset, TILE_BACKINDEX, TRUE);
			for(u8 y = 0; y < 32; y++) backScrollTable[y] = 0;
			stage_draw_moonback();
		} else if(stageBackgroundType == 2) { // Solid Color
			VDP_clearPlan(PLAN_B, TRUE);
		} else if(stageBackgroundType == 3) { // Tiled image, auto scroll
			VDP_loadTileSet(background_info[stageBackground].tileset, TILE_BACKINDEX, TRUE);
			stage_draw_background();
		}
	}
	// Load stage into RAM and draw it around camera position
	stage_load_blocks();
	camera_set_position(player.x, player.y);
	stage_draw_screen();
	stage_load_entities();
	tsc_load_stage(id);
	//hud_create();
	if(vdpEnabled) {
		VDP_setEnable(TRUE);
		SYS_enableInts();
	}
}

void stage_load_tileset() {
	if(!VDP_loadTileSet(tileset_info[stageTileset].tileset, TILE_TSINDEX, TRUE)) {
		puts("Not enough memory to decompress tileset!");
	}
	// Inject the breakable block sprite into the tileset
	const u8 *PXA = tileset_info[stageTileset].PXA;
	for(u16 i = 0; i < 160; i++) {
		if(PXA[i] == 0x43) {
			u32 addr1 = ((i * 2) / TS_WIDTH * TS_WIDTH * 2) + ((i * 2) % TS_WIDTH),
			addr2 = ((i * 2) / TS_WIDTH * TS_WIDTH * 2) + ((i * 2) % TS_WIDTH) + TS_WIDTH;
			VDP_loadTileData(TS_Break.tiles, TILE_TSINDEX + addr1, 2, TRUE);
			VDP_loadTileData(TS_Break.tiles + 16, TILE_TSINDEX + addr2, 2, TRUE);
		}
	}
}

void stage_load_blocks() {
	const u8 *PXM = stage_info[stageID].PXM;
	stageWidth = PXM[4] + (PXM[5] << 8);
	stageHeight = PXM[6] + (PXM[7] << 8);
	PXM += 8;
	stageBlocks = MEM_alloc(stageWidth * stageHeight);
	memcpy(stageBlocks, PXM, stageWidth * stageHeight);
	// Multiplication table for stage rows
	stageTable = MEM_alloc(stageHeight * 2);
	u16 blockTotal = 0;
	for(u16 y = 0; y < stageHeight; y++) {
		stageTable[y] = blockTotal;
		blockTotal += stageWidth;
	}
}

void stage_load_entities() {
	const u8 *PXE = stage_info[stageID].PXE;
	// PXE[4] is the number of entities to load. It's word length but never more than 255
	for(u8 i = 0; i < PXE[4]; i++) {
		u16 x, y, id, event, type, flags;
		// Like all of cave story's data files PXEs are little endian
		x     = PXE[8  + i * 12] + (PXE[9  + i * 12]<<8);
		y     = PXE[10 + i * 12] + (PXE[11 + i * 12]<<8);
		id    = PXE[12 + i * 12] + (PXE[13 + i * 12]<<8);
		event = PXE[14 + i * 12] + (PXE[15 + i * 12]<<8);
		type  = PXE[16 + i * 12] + (PXE[17 + i * 12]<<8);
		flags = PXE[18 + i * 12] + (PXE[19 + i * 12]<<8);
		// There are some unused entities that have all these values as 0, as well as
		// entities that should only exist when specific flags are on/off
		// Loading these would be a waste of memory, just skip them
		if(!id && !event && !type && !flags) continue;
		if((flags&NPC_DISABLEONFLAG) && system_get_flag(id)) continue;
		if((flags&NPC_ENABLEONFLAG) && !system_get_flag(id)) continue;
		// Special case to not load save points if SRAM is not found
		if(type == OBJ_SAVE_POINT && system_get_flag(FLAG_DISABLESAVE)) continue;
		Entity *e = entity_create((x<<CSF)*16 + (8<<CSF), 
								  (y<<CSF)*16 + (8<<CSF), type, flags);
		e->id = id;
		e->event = event;
	}
}

// Replaces a block with another (for <CMP, <SMP, and breakable blocks)
void stage_replace_block(u16 bx, u16 by, u8 index) {
	stageBlocks[stageTable[by] + bx] = index;
	stage_draw_area(bx, by, 1, 1);
}

// Stage vblank drawing routine
void stage_update() {
	SYS_disableInts();
	// Background Scrolling
	// Type 2 is not included here, that's blank backgrounds which are not scrolled
	if(stageBackgroundType == 0) {
		VDP_setHorizontalScroll(PLAN_A, -sub_to_pixel(camera.x) + SCREEN_HALF_W);
		VDP_setVerticalScroll(PLAN_A, sub_to_pixel(camera.y) - SCREEN_HALF_H);
		VDP_setHorizontalScroll(PLAN_B, -sub_to_pixel(camera.x) / 4 + SCREEN_HALF_W);
		VDP_setVerticalScroll(PLAN_B, sub_to_pixel(camera.y) / 4 - SCREEN_HALF_H);
	} else if(stageBackgroundType == 1) {
		// PLAN_A Tile scroll
		s16 off[32];
		off[0] = -sub_to_pixel(camera.x) + SCREEN_HALF_W;
		for(u8 i = 1; i < 32; i++) {
			off[i] = off[0];
		}
		VDP_setHorizontalScrollTile(PLAN_A, 0, off, 32, TRUE);
		VDP_setVerticalScroll(PLAN_A, sub_to_pixel(camera.y) - SCREEN_HALF_H);
		// Moon background has different spots scrolling horizontally at different speeds
		for(u8 y = 0; y < 32; y++) {
			if(y < 12) backScrollTable[y] = 0;
			else if(y < 16) backScrollTable[y] += 1;
			else if(y < 20) backScrollTable[y] += 2;
			else backScrollTable[y] += 3;
		}
		VDP_setHorizontalScrollTile(PLAN_B, 0, backScrollTable, 32, TRUE);
	} else if(stageBackgroundType == 3) {
		// Ironhead boss background auto scrolls leftward
		backScrollTable[0] -= 2;
		VDP_setHorizontalScroll(PLAN_A, -sub_to_pixel(camera.x) + SCREEN_HALF_W);
		VDP_setVerticalScroll(PLAN_A, sub_to_pixel(camera.y) - SCREEN_HALF_H);
		VDP_setHorizontalScroll(PLAN_B, backScrollTable[0]);
	} else {
		// Only scroll foreground
		VDP_setHorizontalScroll(PLAN_A, -sub_to_pixel(camera.x) + SCREEN_HALF_W);
		VDP_setVerticalScroll(PLAN_A, sub_to_pixel(camera.y) - SCREEN_HALF_H);
	}
	SYS_enableInts();
}

void stage_draw_screen() {
	u16 maprow[64];
	s16 y = sub_to_tile(camera.y) - 15;
	for(u8 i = 32; i--; ) {
		if(y >= 0 && y < stageHeight * 2) {
			s16 x = sub_to_tile(camera.x) - 31;
			for(u8 j = 64; j--; ) {
				if(x >= stageWidth * 2) break;
				if(x >= 0) {
					u8 b = stage_get_block(x/2, y/2);
					u16 t = (b%16) * 2 + (b/16) * 64;
					u8 ta = stage_get_block_type(x/2, y/2);
					maprow[x%64] = TILE_ATTR_FULL(ta == 0x43 ? PAL1 : PAL2, (ta&0x40) > 0, 
							0, 0, TILE_TSINDEX + t + (x&1) + ((y&1)*32));
				}
				x++;
			}
			DMA_doDma(DMA_VRAM, (u32)maprow, VDP_getAPlanAddress() + (y%32)*64*2, 64, 2);
		}
		y++;
	}
}

// This draws an arbitrary rectangular area of 16x16 tiles
// It's used on stage load to draw the full screen area
void stage_draw_area(u16 _x, u16 _y, u8 _w, u8 _h) {
	if(_x > stageWidth) _x = 0;
	if(_y > stageHeight) _y = 0;
	u16 t, b, xx, yy, pal; u8 p;
	for(u16 y = _y; y < _y + _h; y++) {
		for(u16 x = _x; x < _x + _w; x++) {
			p = (stage_get_block_type(x, y) & 0x40) > 0;
			pal = stage_get_block_type(x, y) == 0x43 ? PAL1 : PAL2;
			t = block_to_tile(stage_get_block(x, y));
			b = TILE_TSINDEX + (t / TS_WIDTH * TS_WIDTH * 2) + (t % TS_WIDTH);
			xx = block_to_tile(x) % 64;
			yy = block_to_tile(y) % 32;
			VDP_setTileMapXY(PLAN_A, TILE_ATTR_FULL(pal, p, 0, 0, b), xx, yy);
			VDP_setTileMapXY(PLAN_A, TILE_ATTR_FULL(pal, p, 0, 0, b+1), xx+1, yy);
			VDP_setTileMapXY(PLAN_A, TILE_ATTR_FULL(pal, p, 0, 0, b+TS_WIDTH), xx, yy+1);
			VDP_setTileMapXY(PLAN_A, TILE_ATTR_FULL(pal, p, 0, 0, b+TS_WIDTH+1), xx+1, yy+1);
		}
	}
}

// Fills PLAN_B with a tiled background
void stage_draw_background() {
	u8 w = background_info[stageBackground].width;
	u8 h = background_info[stageBackground].height;
	u16 pal = background_info[stageBackground].palette;
	for(u8 y = 0; y < 32; y++) {
		for(u8 x = 0; x < 64; x++) {
			u16 b = TILE_BACKINDEX + (x%w) + ((y%h) * w);
			VDP_setTileMapXY(PLAN_B, TILE_ATTR_FULL(pal, 0, 0, 0, b), x, y);
		}
	}
}

// TODO: Replace this routine with a generated tileset/mapping
void stage_draw_moonback() {
	
}
