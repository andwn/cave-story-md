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
	VDP_drawText("New Game", 15, 12);
	VDP_drawText("Continue", 15, 14);
	VDP_drawText("Sound Test", 15, 16);
	VDP_drawText("NPC Test", 15, 18);
	VDP_drawText("Version. 1.0.0.6", 12, 24);
	VDP_drawText("Mega Drive Version 0.1 2016.06", 4, 26);
	VDP_loadTileSet(&TS_Title, TILE_USERINDEX, true);
	VDP_fillTileMapRectInc(PLAN_B,
		TILE_ATTR_FULL(PAL0, 0, 0, 0, TILE_USERINDEX), 11, 3, 18, 4);
	VDP_fillTileMapRectInc(PLAN_B,
		TILE_ATTR_FULL(PAL0, 0, 0, 0, TILE_USERINDEX + 18 * 4), 11, 21, 18, 2);
	VDP_setEnable(true);
	SYS_enableInts();
	song_play(SONG_TITLE);
	u8 cursor = system_checkdata();
	while(!joy_pressed(BUTTON_C) && !joy_pressed(BUTTON_START)) {
		input_update();
		if(joy_pressed(BUTTON_UP)) {
			if(cursor == 0) cursor = OPTIONS - 1;
			else cursor--;
			sound_play(SOUND_CURSOR, 0);
		} else if(joy_pressed(BUTTON_DOWN)) {
			if(cursor == OPTIONS - 1) cursor = 0;
			else cursor++;
			sound_play(SOUND_CURSOR, 0);
		}
		// Draw quote sprite at cursor position
		SPR_setPosition(sprCursor, tile_to_pixel(13) - 4, tile_to_pixel(12 + cursor * 2) - 4);
		SPR_update();
		VDP_waitVSync();
	}
	// A + Start enables debug mode
	debuggingEnabled = (joystate&BUTTON_A) && joy_pressed(BUTTON_START);
	song_stop();
	sound_play(SOUND_CONFIRM, 0);
	return cursor;
}
