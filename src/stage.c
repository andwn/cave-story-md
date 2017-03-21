#include "common.h"

#include "ai.h"
#include "camera.h"
#include "dma.h"
#include "effect.h"
#include "entity.h"
#include "hud.h"
#include "input.h"
#include "joy.h"
#include "memory.h"
#include "npc.h"
#include "player.h"
#include "resources.h"
#include "sheet.h"
#include "sprite.h"
#include "system.h"
#include "tables.h"
#include "timer.h"
#include "tools.h"
#include "tsc.h"
#include "vdp.h"
#include "vdp_bg.h"
#include "vdp_pal.h"
#include "vdp_tile.h"
#include "vdp_ext.h"
#include "xgm.h"

#include "stage.h"

// Could fit under the Oside map (192 tile gap)
#define TILE_MOONINDEX (TILE_TSINDEX + 32*8)

// Another tile gap, fits under both Almond and Cave
#define TILE_WATERINDEX (TILE_TSINDEX + 384)

// Index of background in db/back.c and the effect type
uint8_t stageBackground = 0xFF;

int16_t backScrollTable[32];
uint8_t *stageBlocks = NULL;
uint16_t *stageTable = NULL;

void stage_load_tileset();
void stage_load_blocks();
void stage_load_entities();

void stage_draw_block(uint16_t x, uint16_t y);
void stage_draw_screen();
void stage_draw_background();
void stage_draw_moonback();

void stage_load(uint16_t id) {
	VDP_setPaletteColor(15, 0x000); // Hide the white crap on the screen
	VDP_setEnable(FALSE);
	
	// Prevents an issue where a column of the previous map would get drawn over the new one
	DMA_flushQueue();
	
	input_update(); // Prevent menu from opening after loading save
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
	sprites_clear();
	water_entity = NULL;
	// Load the tileset
	if(stageTileset != stage_info[id].tileset) {
		stageTileset = stage_info[id].tileset;
		
		XGM_doVBlankProcess();
		XGM_set68KBUSProtection(TRUE);
		waitSubTick(10);
		
		stage_load_tileset();
		
		XGM_set68KBUSProtection(FALSE);
	}
	// Load sprite sheets
	XGM_doVBlankProcess();
	XGM_set68KBUSProtection(TRUE);
	waitSubTick(10);
	
	sheets_load_stage(id, FALSE, TRUE);
	
	XGM_set68KBUSProtection(FALSE);
	// Stage palette and shared NPC palette
	if(stageID == 0x30) {
		VDP_setCachedPalette(PAL2, PAL_RiverAlt.data); // For Waterway green background
	} else {
		VDP_setCachedPalette(PAL2, tileset_info[stageTileset].palette->data);
	}
	VDP_setCachedPalette(PAL3, stage_info[id].npcPalette->data);
	// Load backgrounds
	if(background_info[stage_info[id].background].type == 4 || 
			stageBackground != stage_info[id].background) {
		stageBackground = stage_info[id].background;
		stageBackgroundType = background_info[stageBackground].type;
		
		XGM_doVBlankProcess();
		XGM_set68KBUSProtection(TRUE);
		waitSubTick(10);
		
		VDP_setBackgroundColor(0); // Color index 0 for everything except fog
		if(stageBackgroundType == 0) { // Tiled image
			VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
			VDP_loadTileSet(background_info[stageBackground].tileset, TILE_BACKINDEX, TRUE);
			stage_draw_background();
		} else if(stageBackgroundType == 1) { // Moon
			VDP_setScrollingMode(HSCROLL_TILE, VSCROLL_PLANE);
			stage_draw_moonback();
		} else if(stageBackgroundType == 2) { // Solid Color
			VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
			VDP_clearPlan(PLAN_B, TRUE);
		} else if(stageBackgroundType == 3) { // Tiled image, auto scroll
			VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
			VDP_loadTileSet(background_info[stageBackground].tileset, TILE_BACKINDEX, TRUE);
			stage_draw_background();
		} else if(stageBackgroundType == 4) { // Almond Water
			VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
			VDP_clearPlan(PLAN_B, TRUE);
			VDP_loadTileSet(&BG_Water, TILE_WATERINDEX, TRUE);
		} else if(stageBackgroundType == 5) { // Fog
			VDP_setScrollingMode(HSCROLL_TILE, VSCROLL_PLANE);
			// Use background color from tileset
			VDP_setBackgroundColor(32);
			stage_draw_moonback();
		}
		XGM_set68KBUSProtection(FALSE);
	}
	// Load stage into RAM
	XGM_doVBlankProcess();
	XGM_set68KBUSProtection(TRUE);
	waitSubTick(10);
	
	stage_load_blocks();
	
	XGM_set68KBUSProtection(FALSE);
	
	camera_set_position(player.x, player.y - (stageBackgroundType == 3 ? 8<<CSF : 0));
	camera.target = &player;
	camera.x_offset = 0;
	camera.y_offset = 0;
	
	XGM_doVBlankProcess();
	XGM_set68KBUSProtection(TRUE);
	waitSubTick(10);
	
	stage_draw_screen();
	
	XGM_set68KBUSProtection(FALSE);
	
	XGM_doVBlankProcess();
	XGM_set68KBUSProtection(TRUE);
	waitSubTick(10);
	
	stage_load_entities();
	
	XGM_set68KBUSProtection(FALSE);
	
	XGM_doVBlankProcess();
	XGM_set68KBUSProtection(TRUE);
	waitSubTick(10);
	
	if(stageBackgroundType == 3) {
		bossEntity = entity_create(0, 0, 360 + BOSS_IRONHEAD, 0);
	} else if(stageBackgroundType == 4) {
		backScrollTable[0] = (SCREEN_HEIGHT >> 3) + 1;
	}
	if(stageID == 68) { // Black Space
		bossEntity = entity_create(0, 0, 360 + BOSS_UNDEADCORE, 0);
	}
	tsc_load_stage(id);
	
	XGM_set68KBUSProtection(FALSE);
	
	VDP_setEnable(TRUE);
	VDP_setPaletteColor(15, 0xEEE); // Restore white color for text
}

void stage_load_tileset() {
	VDP_loadTileSet(tileset_info[stageTileset].tileset, TILE_TSINDEX, TRUE);
	// Inject the breakable block sprite into the tileset
	const uint8_t *PXA = tileset_info[stageTileset].PXA;
	for(uint16_t i = 0; i < 160; i++) {
		if(PXA[i] == 0x43) {
			uint32_t addr1 = ((i * 2) / TS_WIDTH * TS_WIDTH * 2) + ((i * 2) % TS_WIDTH),
			addr2 = ((i * 2) / TS_WIDTH * TS_WIDTH * 2) + ((i * 2) % TS_WIDTH) + TS_WIDTH;
			VDP_loadTileData(TS_Break.tiles, TILE_TSINDEX + addr1, 2, TRUE);
			VDP_loadTileData(TS_Break.tiles + 16, TILE_TSINDEX + addr2, 2, TRUE);
		}
	}
}

void stage_load_blocks() {
	const uint8_t *PXM = stage_info[stageID].PXM;
	stageWidth = PXM[4] + (PXM[5] << 8);
	stageHeight = PXM[6] + (PXM[7] << 8);
	PXM += 8;
	stageBlocks = MEM_alloc(stageWidth * stageHeight);
	memcpy(stageBlocks, PXM, stageWidth * stageHeight);
	// Multiplication table for stage rows
	stageTable = MEM_alloc(stageHeight * 2);
	uint16_t blockTotal = 0;
	for(uint16_t y = 0; y < stageHeight; y++) {
		stageTable[y] = blockTotal;
		blockTotal += stageWidth;
	}
}

void stage_load_entities() {
	const uint8_t *PXE = stage_info[stageID].PXE;
	// PXE[4] is the number of entities to load. It's word length but never more than 255
	for(uint8_t i = 0; i < PXE[4]; i++) {
		uint16_t x, y, id, event, type, flags;
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
		// I'll probably need this code block again in the future.
		// When an NPC is assigned the improper number of sprites for their metasprite
		// loading it will crash BlastEm and possibly hardware too. This steps through
		// each entity as it is loaded so the problematic NPC can be found
		/*
		{
			
			//VDP_setEnable(TRUE);
			VDP_setPaletteColor(15, 0xEEE);
			char str[2][36];
			sprintf(str[0], "X:%hu Y:%hu I:%hu", x, y, id);
			sprintf(str[1], "E:%hu T:%hu F:%hu", event, type, flags);
			VDP_drawText(str[0], 2, 2);
			VDP_drawText(str[1], 2, 4);
			
			input_update();
			while(!joy_pressed(BUTTON_C)) {
				input_update();
				vsync();
				aftervsync();
				VDP_setHorizontalScroll(PLAN_A, 0);
				VDP_setVerticalScroll(PLAN_A, 0);
			}
			//
			//VDP_setEnable(FALSE);
		}
		*/
		Entity *e = entity_create((x<<CSF)*16 + (8<<CSF), 
								  (y<<CSF)*16 + (8<<CSF), type, flags);
		e->id = id;
		e->event = event;
	}
}

// Replaces a block with another (for <CMP, <SMP, and breakable blocks)
void stage_replace_block(uint16_t bx, uint16_t by, uint8_t index) {
	stageBlocks[stageTable[by] + bx] = index;
	int16_t cx = sub_to_block(camera.x), cy = sub_to_block(camera.y);
	if(cx - 16 > bx || cx + 16 < bx || cy - 8 > by || cy + 8 < by) return;
	// Only redraw if change was made onscreen
	stage_draw_block(bx, by);
}

// Stage vblank drawing routine
void stage_update() {
	// Background Scrolling
	// Type 2 is not included here, that's blank backgrounds which are not scrolled
	if(stageBackgroundType == 0) {
		VDP_setHorizontalScroll(PLAN_A, -sub_to_pixel(camera.x) + SCREEN_HALF_W);
		VDP_setVerticalScroll(PLAN_A, sub_to_pixel(camera.y) - SCREEN_HALF_H);
		VDP_setHorizontalScroll(PLAN_B, -sub_to_pixel(camera.x) / 4 + SCREEN_HALF_W);
		VDP_setVerticalScroll(PLAN_B, sub_to_pixel(camera.y) / 4 - SCREEN_HALF_H);
	} else if(stageBackgroundType == 1 || stageBackgroundType == 5) {
		// PLAN_A Tile scroll
		int16_t off[32];
		off[0] = -sub_to_pixel(camera.x) + SCREEN_HALF_W;
		for(uint8_t i = 1; i < 32; i++) {
			off[i] = off[0];
		}
		VDP_setHorizontalScrollTile(PLAN_A, 0, off, 32, TRUE);
		VDP_setVerticalScroll(PLAN_A, sub_to_pixel(camera.y) - SCREEN_HALF_H);
		// Moon background has different spots scrolling horizontally at different speeds
		backScrollTimer--;
#ifdef PAL
		uint8_t y = 28;
		for(;y >= 22; --y) backScrollTable[y] = backScrollTimer << 1;
		for(;y >= 18; --y) backScrollTable[y] = backScrollTimer;
		for(;y >= 15; --y) backScrollTable[y] = backScrollTimer >> 1;
		for(;y >= 11; --y) backScrollTable[y] = backScrollTimer >> 2;
		VDP_setHorizontalScrollTile(PLAN_B, 0, backScrollTable, 32, TRUE);
		VDP_setVerticalScroll(PLAN_B, -8);
#else
		uint8_t y = 27;
		for(;y >= 21; --y) backScrollTable[y] = backScrollTimer << 1;
		for(;y >= 17; --y) backScrollTable[y] = backScrollTimer;
		for(;y >= 14; --y) backScrollTable[y] = backScrollTimer >> 1;
		for(;y >= 10; --y) backScrollTable[y] = backScrollTimer >> 2;
		VDP_setHorizontalScrollTile(PLAN_B, 0, backScrollTable, 32, TRUE);
		VDP_setVerticalScroll(PLAN_B, 0);
#endif
	} else if(stageBackgroundType == 3) {
		// Lock camera at specific spot
		camera.target = NULL;
		// Ironhead boss background auto scrolls leftward
		backScrollTable[0] -= 2;
		VDP_setHorizontalScroll(PLAN_A, -sub_to_pixel(camera.x) + SCREEN_HALF_W);
		VDP_setVerticalScroll(PLAN_A, sub_to_pixel(camera.y) - SCREEN_HALF_H);
		VDP_setHorizontalScroll(PLAN_B, backScrollTable[0]);
	} else if(stageBackgroundType == 4) {
		static const int16_t rowc = SCREEN_HEIGHT >> 3;
		static const int16_t rowgap = 31 - rowc;
		// Water surface relative to top of screen
		int16_t scroll = (water_entity->y >> CSF) - ((camera.y >> CSF) - SCREEN_HALF_H);
		int16_t row = scroll >> 3;
		int16_t oldrow = backScrollTable[0];
		while(row < oldrow) { // Water is rising (Y decreasing)
			oldrow--;
			uint8_t rowup = 31 - ((oldrow + rowgap) & 31);// Row that will be updated
			if(oldrow > rowc) { // Below Screen
				uint16_t mapBuffer[64] = {};
				DMA_doDma(DMA_VRAM, (uint32_t)mapBuffer, VDP_PLAN_B + (rowup << 7), 64, 2);
			} else { // On screen or above
				uint16_t mapBuffer[64];
				for(uint16_t x = 0; x < 64; x++) {
					mapBuffer[x] = TILE_ATTR_FULL(PAL0,1,0,0,
							TILE_WATERINDEX + (oldrow == rowc ? x&3 : 4 + (random()&15)));
				}
				DMA_doDma(DMA_VRAM, (uint32_t)mapBuffer, VDP_PLAN_B + (rowup << 7), 64, 2);
			}
		}
		while(row > oldrow) { // Water is lowering (Y increasing)
			oldrow++;
			uint8_t rowup = 31 - (oldrow & 31); // Row that will be updated
			if(oldrow <= 0) { // Above screen
				uint16_t mapBuffer[64];
				for(uint16_t x = 0; x < 64; x++) {
					mapBuffer[x] = TILE_ATTR_FULL(PAL0,1,0,0,
							TILE_WATERINDEX + (oldrow == 0 ? x&3 : 4 + (random()&15)));
				}
				DMA_doDma(DMA_VRAM, (uint32_t)mapBuffer, VDP_PLAN_B + (rowup << 7), 64, 2);
			} else { // On screen or below
				uint16_t mapBuffer[64] = {};
				DMA_doDma(DMA_VRAM, (uint32_t)mapBuffer, VDP_PLAN_B + (rowup << 7), 64, 2);
			}
		}
		
		VDP_setHorizontalScroll(PLAN_B, -sub_to_pixel(camera.x) + SCREEN_HALF_W - backScrollTimer);
		VDP_setVerticalScroll(PLAN_B, -scroll);
		VDP_setHorizontalScroll(PLAN_A, -sub_to_pixel(camera.x) + SCREEN_HALF_W);
		VDP_setVerticalScroll(PLAN_A, sub_to_pixel(camera.y) - SCREEN_HALF_H);
		backScrollTable[0] = row;
	} else {
		// Only scroll foreground
		VDP_setHorizontalScroll(PLAN_A, -sub_to_pixel(camera.x) + SCREEN_HALF_W);
		VDP_setVerticalScroll(PLAN_A, sub_to_pixel(camera.y) - SCREEN_HALF_H);
	}
}

void stage_draw_screen() {
	uint16_t maprow[64];
	int16_t y = sub_to_tile(camera.y) - 16;
	for(uint8_t i = 32; i--; ) {
		if(y >= 0 && y < stageHeight * 2) {
			int16_t x = sub_to_tile(camera.x) - 32;
			for(uint8_t j = 64; j--; ) {
				if(x >= stageWidth * 2) break;
				if(x >= 0) {
					uint8_t b = stage_get_block(x/2, y/2);
					uint16_t t = (b%16) * 2 + (b/16) * 64;
					uint8_t ta = stage_get_block_type(x/2, y/2);
					maprow[x%64] = TILE_ATTR_FULL(ta == 0x43 ? PAL1 : PAL2, (ta&0x40) > 0, 
							0, 0, TILE_TSINDEX + t + (x&1) + ((y&1)*32));
				}
				x++;
			}
			DMA_doDma(DMA_VRAM, (uint32_t)maprow, VDP_PLAN_A + (y%32)*0x80, 64, 2);
		}
		y++;
	}
}

// Draws just one block
void stage_draw_block(uint16_t x, uint16_t y) {
	if(x >= stageWidth || y >= stageHeight) return;
	uint16_t t, b, xx, yy; uint8_t p;
	p = (stage_get_block_type(x, y) & 0x40) > 0;
	t = block_to_tile(stage_get_block(x, y));
	b = TILE_TSINDEX + (t / TS_WIDTH * TS_WIDTH * 2) + (t % TS_WIDTH);
	xx = block_to_tile(x) % 64;
	yy = block_to_tile(y) % 32;
	
	VDP_setTileMapXY(PLAN_A, TILE_ATTR_FULL(2, p, 0, 0, b), xx, yy);
	VDP_setTileMapXY(PLAN_A, TILE_ATTR_FULL(2, p, 0, 0, b+1), xx+1, yy);
	VDP_setTileMapXY(PLAN_A, TILE_ATTR_FULL(2, p, 0, 0, b+TS_WIDTH), xx, yy+1);
	VDP_setTileMapXY(PLAN_A, TILE_ATTR_FULL(2, p, 0, 0, b+TS_WIDTH+1), xx+1, yy+1);
	
}

// Fills PLAN_B with a tiled background
void stage_draw_background() {
	uint8_t w = background_info[stageBackground].width;
	uint8_t h = background_info[stageBackground].height;
	uint16_t pal = background_info[stageBackground].palette;
	for(uint8_t y = 0; y < 32; y += h) {
		for(uint8_t x = 0; x < 64; x += w) {
			VDP_fillTileMapRectInc(PLAN_B, TILE_ATTR_FULL(pal,0,0,0,TILE_BACKINDEX), x, y, w, h);
		}
	}
	
}

void stage_draw_moonback() {
	uint16_t mapBuffer[64];
	const uint32_t *topTiles, *btmTiles;
	const uint8_t *topMap, *btmMap;
	if(stageBackgroundType == 1) {
		// Moon
		topTiles = (uint32_t*)MoonTopTiles;
		btmTiles = (uint32_t*)MoonBtmTiles;
		topMap = MoonTopMap;
		btmMap = MoonBtmMap;
	} else {
		// Fog
		topTiles = (uint32_t*)FogTopTiles;
		btmTiles = (uint32_t*)FogBtmTiles;
		topMap = FogTopMap;
		btmMap = FogBtmMap;
	}
	// Load the top section in the designated background area
	VDP_loadTileData(topTiles, TILE_BACKINDEX, 12, TRUE);
	// Load the clouds under the map, it just fits
	VDP_loadTileData(btmTiles, TILE_MOONINDEX, 188, TRUE);
	for(uint8_t y = 0; y < 32; y++) backScrollTable[y] = 0;
	// Top part
	uint16_t cursor = 0;
	for(uint16_t y = 0; y < 10; y++) {
		for(uint16_t x = 0; x < 40; x++) {
			mapBuffer[x] = TILE_ATTR_FULL(PAL2,0,0,0,
					TILE_BACKINDEX + (topMap[cursor]<<8) + topMap[cursor+1]);
			cursor += 2;
		}
		VDP_setTileMapDataRect(PLAN_B, mapBuffer, 0, y, 40, 1);
	}
	// Bottom part
	cursor = 0;
	for(uint16_t y = 10; y < 28; y++) {
		for(uint16_t x = 0; x < 32; x++) {
			mapBuffer[x] = mapBuffer[x+32] = TILE_ATTR_FULL(PAL2,0,0,0,
					TILE_MOONINDEX + (btmMap[cursor]<<8) + btmMap[cursor+1]);
			cursor += 2;
		}
		VDP_setTileMapDataRect(PLAN_B, mapBuffer, 0, y, 64, 1);
	}
	// For 240 mode
	if(IS_PALSYSTEM) {
		// Shift the vscroll down 8 pixels
		VDP_setVerticalScroll(PLAN_B, -8);
		// Duplicate top line in row 31 (-1)
		cursor = 0;
		for(uint16_t x = 0; x < 40; x++) {
			mapBuffer[x] = TILE_ATTR_FULL(PAL2,0,0,0,
					TILE_BACKINDEX + (topMap[cursor]<<8) + topMap[cursor+1]);
			cursor += 2;
		}
		VDP_setTileMapDataRect(PLAN_B, mapBuffer, 0, 31, 40, 1);
		// Duplicate bottom row in row 28
		cursor = 32*17*2;
		for(uint16_t x = 0; x < 32; x++) {
			mapBuffer[x] = mapBuffer[x+32] = TILE_ATTR_FULL(PAL2,0,0,0,
					TILE_MOONINDEX + (btmMap[cursor]<<8) + btmMap[cursor+1]);
			cursor += 2;
		}
		VDP_setTileMapDataRect(PLAN_B, mapBuffer, 0, 28, 64, 1);
	}
	backScrollTimer = 0;
}
