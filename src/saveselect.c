#include "common.h"

#include "ai.h"
#include "audio.h"
#include "camera.h"
#include "dma.h"
#include "effect.h"
#include "entity.h"
#include "hud.h"
#include "input.h"
#include "joy.h"
#include "npc.h"
#include "player.h"
#include "resources.h"
#include "sheet.h"
#include "sprite.h"
#include "stage.h"
#include "string.h"
#include "system.h"
#include "tables.h"
#include "tsc.h"
#include "vdp.h"
#include "vdp_bg.h"
#include "vdp_pal.h"
#include "vdp_tile.h"
#include "vdp_ext.h"
#include "weapon.h"
#include "window.h"

#include "gamemode.h"

#define ANIM_SPEED	7
#define ANIM_FRAMES	4
#define OPTIONS (SRAM_FILE_MAX + 2)

enum { CM_LOAD, CM_COPY, CM_PASTE, CM_DELETE };

static void draw_cursor_mode(uint8_t mode) {
	switch(mode) {
		case CM_LOAD:   VDP_drawText(" Load Save Data ", 12, 2); break;
		case CM_COPY:   VDP_drawText(" Copy Save Data ", 12, 2); break;
		case CM_PASTE:  VDP_drawText("Paste Save Data ", 12, 2); break;
		case CM_DELETE: VDP_drawText("Delete Save Data", 12, 2); break;
	}
}

static uint8_t refresh_file(uint8_t index) {
	uint16_t y = 4 + index * 5;
	SaveEntry file;
	
	system_peekdata(index, &file);
	VDP_clearText(6, y, 16); // Erase any previous stage name text
	if(file.used) {
		VDP_drawText(stage_info[file.stage_id].name, 6, y);
	} else {
		VDP_drawText("New Game", 6, y);
	}
	return file.used;
}

const struct {
	int16_t x, y;
} cursor_pos[OPTIONS] = {
	{  4*8,  5*8 +  0*8 },
	{  4*8,  5*8 +  5*8 },
	{  4*8,  5*8 + 10*8 },
	{  4*8,  5*8 + 15*8 },
	{  4*8,  5*8 + 20*8 },
	{ 14*8,  5*8 + 20*8 },
};

uint8_t saveselect_main() {
	gamemode = GM_SAVESEL;
	
	uint8_t cursor = 0;
	uint8_t cursorMode = CM_LOAD;
	uint8_t fileToCopy = 0;
	uint8_t sprFrame = 0, sprTime = ANIM_SPEED;
	uint8_t file_used[SRAM_FILE_MAX];
	
	VDP_setEnable(FALSE);
	// Keep stuff from the title screen
	VDP_clearPlan(PLAN_A, TRUE);
	VDP_clearPlan(PLAN_B, TRUE);
	sprites_clear();
	VDPSprite sprCursor = { 
		.attribut = TILE_ATTR_FULL(PAL1,0,0,1,TILE_SHEETINDEX),
		.size = SPRITE_SIZE(2,2)
	};
	
	draw_cursor_mode(cursorMode);
	for(uint16_t i = 0; i < SRAM_FILE_MAX; i++) file_used[i] = refresh_file(i);
	VDP_drawText("Copy", 6, 25);
	VDP_drawText("Delete", 16, 25);
	
	VDP_setEnable(TRUE);
	
	oldstate = ~0;
	while(TRUE) {
		input_update();
		if(joy_pressed(BUTTON_C) || joy_pressed(BUTTON_START)) { // Confirm action
			if(cursor < SRAM_FILE_MAX) {
				switch(cursorMode) {
					case CM_LOAD: { // Load/New Game on file over cursor
						song_stop();
						sound_play(SND_MENU_SELECT, 0);
						sram_file = cursor;
						return file_used[cursor];
					}
					case CM_COPY: { // Source of copy
						sound_play(SND_MENU_MOVE, 0);
						fileToCopy = cursor;
						cursorMode = CM_PASTE;
						break;
					}
					case CM_PASTE: { // Destination of copy
						sound_play(SND_MENU_MOVE, 0);
						if(fileToCopy != cursor) {
							system_copy(fileToCopy, cursor);
							file_used[cursor] = refresh_file(cursor);
						}
						cursorMode = CM_LOAD;
						break;
					}
					case CM_DELETE: { // Clear file
						sound_play(SND_PLAYER_DIE, 0);
						system_delete(cursor);
						file_used[cursor] = refresh_file(cursor);
						cursorMode = CM_LOAD;
						break;
					}
				}
			} else if(cursor == SRAM_FILE_MAX) {
				sound_play(SND_MENU_MOVE, 0);
				cursorMode = CM_COPY;
			} else if(cursor == SRAM_FILE_MAX + 1) {
				sound_play(SND_MENU_MOVE, 0);
				cursorMode = CM_DELETE;
			}
			draw_cursor_mode(cursorMode);
		} else if(joy_pressed(BUTTON_B)) { // Cancel copy/delete
			cursorMode = CM_LOAD;
			draw_cursor_mode(cursorMode);
		}
		if(joy_pressed(BUTTON_UP)) {
			if(cursor == 0) cursor = OPTIONS - 1;
			else cursor--;
			sound_play(SND_MENU_MOVE, 0);
		} else if(joy_pressed(BUTTON_DOWN)) {
			if(cursor == OPTIONS - 1) cursor = 0;
			else cursor++;
			sound_play(SND_MENU_MOVE, 0);
		}
		// Animate quote sprite
		if(--sprTime == 0) {
			sprTime = ANIM_SPEED;
			if(++sprFrame >= ANIM_FRAMES) sprFrame = 0;
			sprite_index(sprCursor, TILE_SHEETINDEX+sprFrame*4);
		}
		if(cursorMode == CM_PASTE && (sprTime & 1)) {
			// Blink quote between source and destination of a copy
			sprite_pos(sprCursor, cursor_pos[fileToCopy].x, cursor_pos[fileToCopy].y);
		} else {
			// Draw quote sprite at cursor position
			sprite_pos(sprCursor, cursor_pos[cursor].x, cursor_pos[cursor].y);
		}
		sprite_add(sprCursor);
		
		ready = TRUE;
		vsync(); aftervsync();
	}
	return file_used[cursor];
}
