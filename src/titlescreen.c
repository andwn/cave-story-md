#include "titlescreen.h"

#include <genesis.h>
#include "audio.h"
#include "sprite.h"
#include "input.h"
#include "system.h"
#include "common.h"
#include "resources.h"

#define OPTIONS 3

u8 titlescreen_main() {
	VDP_setEnable(false);
	VDP_resetScreen();
	VDP_setPalette(PAL0, PAL_Main.data);
	VDP_setHorizontalScroll(PLAN_A, 0);
	VDP_setVerticalScroll(PLAN_A, 0);
	sprites_init();
	u8 cursorSprite = sprite_create(&SPR_Quote, PAL0, true);
	sprite_set_position(cursorSprite, tile_to_pixel(12) - 4, tile_to_pixel(16) - 4);
	sprite_set_attr(cursorSprite, TILE_ATTR(PAL0, true, false, true));
	VDP_drawText("New Game", 14, 16);
	VDP_drawText("Continue", 14, 18);
	VDP_drawText("Sound Test", 14, 20);
	VDP_drawText(" ", 2, 25);
	VDP_drawText("2015.03", 31, 25);
	VDP_setEnable(true);
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
		sprite_set_position(cursorSprite,
				tile_to_pixel(12) - 4, tile_to_pixel(16 + cursor * 2) - 4);
		sprites_update();
		VDP_waitVSync();
	}
	sound_play(SOUND_CONFIRM, 0);
	return cursor;
}
