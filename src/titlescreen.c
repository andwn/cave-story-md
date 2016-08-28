#include "gamemode.h"

#include <genesis.h>
#include "audio.h"
#include "input.h"
#include "system.h"
#include "common.h"
#include "resources.h"

#define OPTIONS 4

u8 titlescreen_main() {
	SYS_disableInts();
	VDP_setEnable(false);
	SPR_reset();
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
	Sprite *sprCursor = SPR_addSprite(&SPR_Quote, 0, 0, TILE_ATTR(PAL0, 1, 0, 1));
	SPR_setAnim(sprCursor, 1);
	// Menu and version text
	if(!checksumValid) {
		VDP_drawText("WARNING: Bad Checksum!", 8, 9);
	}
	VDP_drawText("New Game", 15, 12);
	VDP_drawText("Sound Test", 15, 16);
	VDP_drawText("NPC Test", 15, 18);
	VDP_drawText("Mega Drive Version 0.2 2016.08", 4, 26);
	VDP_loadTileSet(&TS_Title, TILE_USERINDEX, true);
	VDP_fillTileMapRectInc(PLAN_B,
		TILE_ATTR_FULL(PAL0, 0, 0, 0, TILE_USERINDEX), 11, 3, 18, 4);
	VDP_fillTileMapRectInc(PLAN_B,
		TILE_ATTR_FULL(PAL0, 0, 0, 0, TILE_USERINDEX + 18 * 4), 11, 23, 18, 2);
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
		// Draw quote sprite at cursor position
		SPR_setPosition(sprCursor, tile_to_pixel(13) - 4, tile_to_pixel(12 + cursor * 2) - 4);
		SPR_update();
		VDP_waitVSync();
	}
	// A + Start enables debug mode
	debuggingEnabled = (joystate&BUTTON_A) && joy_pressed(BUTTON_START);
	song_stop();
	sound_play(SND_MENU_SELECT, 0);
	VDP_waitVSync();
	return cursor;
}
