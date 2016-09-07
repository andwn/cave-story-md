#include "gamemode.h"

#include <genesis.h>
#include "audio.h"
#include "input.h"
#include "system.h"
#include "common.h"
#include "resources.h"
#include "sprite.h"
#include "sheet.h"

#define OPTIONS		3
#define ANIM_SPEED	7
#define ANIM_FRAMES	4

u8 titlescreen_main() {
	SYS_disableInts();
	VDP_setEnable(0);
	VDP_resetScreen(); // This brings back the default SGDK font with transparency
	// No special scrolling for title screen
	VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
	VDP_setHorizontalScroll(PLAN_A, 0);
	VDP_setVerticalScroll(PLAN_A, 0);
	// Main palette
	VDP_setPalette(PAL0, PAL_Main.data);
	VDP_setPaletteColor(PAL0, 0x0444); // Gray background
	VDP_setPaletteColor(PAL0 + 8, 0x08CE); // Yellow text
	// Load quote sprite
	SHEET_LOAD(&SPR_Quote, 4, 4, TILE_SHEETINDEX, 1, 1,0, 1,1, 1,2, 1,3);
	VDPSprite sprCursor = { 
		.attribut = TILE_ATTR_FULL(PAL0,0,0,1,TILE_SHEETINDEX),
		.size = SPRITE_SIZE(2,2)
	};
	u8 sprFrame = 0, sprTime = ANIM_SPEED;
	// Menu and version text
	VDP_drawText("New Game", 15, 12);
	VDP_drawText("Sound Test", 15, 16);
	VDP_drawText("Mega Drive Version 0.2.1 2016.08", 4, 26);
	VDP_loadTileSet(&TS_Title, TILE_USERINDEX, true);
	VDP_fillTileMapRectInc(PLAN_B,
			TILE_ATTR_FULL(PAL0,0,0,0,TILE_USERINDEX), 11, 3, 18, 4);
	VDP_fillTileMapRectInc(PLAN_B,
			TILE_ATTR_FULL(PAL0,0,0,0,TILE_USERINDEX + 18*4), 11, 23, 18, 2);
	// Check save data, only enable continue if save data exists
	u8 cursor = 0;
	u8 sram_state = system_checkdata();
	if(sram_state == SRAM_VALID_SAVE) {
		VDP_drawText("Continue", 15, 14);
		cursor = 1;
	}
	VDP_setEnable(true);
	SYS_enableInts();
	song_play(SONG_TITLE);
	while(!joy_pressed(BUTTON_C) && !joy_pressed(BUTTON_START)) {
		input_update();
		if(joy_pressed(BUTTON_UP)) {
			if(cursor == 0) cursor = OPTIONS - 1;
			else cursor--;
			// Skip over "continue" if no save data
			if(sram_state != SRAM_VALID_SAVE && cursor == 1) cursor--;
			sound_play(SND_MENU_MOVE, 0);
		} else if(joy_pressed(BUTTON_DOWN)) {
			if(cursor == OPTIONS - 1) cursor = 0;
			else cursor++;
			// Skip over "continue" if no save data
			if(sram_state != SRAM_VALID_SAVE && cursor == 1) cursor++;
			sound_play(SND_MENU_MOVE, 0);
		}
		// Animate quote sprite
		if(--sprTime == 0) {
			sprTime = ANIM_SPEED;
			if(++sprFrame >= ANIM_FRAMES) sprFrame = 0;
			sprCursor.attribut = TILE_ATTR_FULL(PAL0,0,0,1,TILE_SHEETINDEX+sprFrame*4);
		}
		// Draw quote sprite at cursor position
		sprite_pos(sprCursor, 13*8-4, (12*8+cursor*16)-4);
		SYS_disableInts();
		DMA_doDma(DMA_VRAM, (u32)(&sprCursor), VDP_SPRITE_TABLE, 4, 2);
		SYS_enableInts();
		VDP_waitVSync();
	}
	// A + Start enables debug mode
	debuggingEnabled = (joystate&BUTTON_A) && joy_pressed(BUTTON_START);
	song_stop();
	sound_play(SND_MENU_SELECT, 0);
	//VDP_waitVSync();
	return cursor;
}
