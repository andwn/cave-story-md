#include "common.h"

#include "audio.h"
#include "camera.h"
#include "dma.h"
#include "effect.h"
#include "entity.h"
#include "hud.h"
#include "joy.h"
#include "kanji.h"
#include "memory.h"
#include "npc.h"
#include "player.h"
#include "resources.h"
#include "sheet.h"
#include "stage.h"
#include "string.h"
#include "system.h"
#include "tables.h"
#include "tsc.h"
#include "vdp.h"
#include "weapon.h"
#include "window.h"

#include "gamemode.h"

#define ANIM_SPEED	7
#define ANIM_FRAMES	4
#define OPTIONS (SRAM_FILE_MAX + 2)

enum { CM_LOAD, CM_COPY, CM_PASTE, CM_DELETE, CM_CONFIRM };

static void draw_cursor_mode(uint8_t mode) {
	switch(mode) {
		case CM_LOAD:    vdp_puts(VDP_PLAN_A, " Load Save Data ", 12, 2); break;
		case CM_COPY:    vdp_puts(VDP_PLAN_A, " Copy Save Data ", 12, 2); break;
		case CM_PASTE:   vdp_puts(VDP_PLAN_A, "Paste Save Data ", 12, 2); break;
		case CM_DELETE:  vdp_puts(VDP_PLAN_A, "Delete Save Data", 12, 2); break;
		case CM_CONFIRM: vdp_puts(VDP_PLAN_A, " Are you sure?  ", 12, 2); break;
	}
}

static uint16_t GetNextChar(uint16_t stage, uint16_t index) {
	uint16_t chr = JStageName[(stage << 4) + index];
	if(chr >= 0xE0 && chr < 0xFF) {
		return (chr - 0xE0) * 0x60 + (JStageName[(stage << 4) + index + 1] - 0x20) + 0x100;
	} else {
		return chr;
	}
}

static uint8_t refresh_file(uint8_t index) {
	uint16_t y = 4 + index * 5;
	SaveEntry file;
	
	system_peekdata(index, &file);
	vdp_text_clear(VDP_PLAN_A, 6, y, 16); // Erase any previous stage name text
	if(cfg_language) vdp_text_clear(VDP_PLAN_A, 6, y+1, 16); // And a second line underneath
	if(file.used) {
		// Map name
		if(cfg_language) {
			uint16_t x = 6;
			uint16_t tile_index = (0xB000 >> 5) + (index << 5);
			uint16_t name_index = 0;
			while(name_index < 16) {
				uint16_t c = GetNextChar(file.stage_id, name_index++);
				if(c == 0) break; // End of string
				if(c > 0xFF) name_index++;
				kanji_draw(VDP_PLAN_A, tile_index, c, x, y, 0, 1);
				x += 2;
				tile_index += 4;
			}
		} else {
			vdp_puts(VDP_PLAN_A, stage_info[file.stage_id].name, 6, y);
		}
		// Play time
		char timeStr[10] = {};
		sprintf(timeStr, "%02hu:%02hu:%02hu", file.hour, file.minute, file.second);
		vdp_puts(VDP_PLAN_A, timeStr, 26, y);
		// Health bar
		{
			uint32_t tileData[8][8];
			int16_t fillHP = 40 * file.health / max(file.max_health, 1);
			for(uint8_t i = 0; i < 5; i++) {
				int16_t addrHP = min(fillHP*8, 7*8);
				if(addrHP < 0) addrHP = 0;
				memcpy(tileData[i+3], &TS_HudBar.tiles[addrHP], 32);
				fillHP -= 8;
			}
			memcpy(tileData[0], &SPR_TILES(&SPR_Hud2, 0, 0)[3*8], 32);
			uint8_t digit = div10[file.health];
			if(digit) {
				memcpy(tileData[1], &TS_Numbers.tiles[(digit)*8], 32);
			} else {
				memcpy(tileData[1], TILE_BLANK, 32);
			}
			memcpy(tileData[2], &TS_Numbers.tiles[mod10[file.health]*8], 32);
			uint16_t tile = TILE_HUDINDEX + index*8;
			DMA_doDma(DMA_VRAM, (uint32_t)tileData[0], tile*32, 16*8, 2);
			for(int i = 0; i < 8; i++) {
				vdp_map_xy(VDP_PLAN_A, TILE_ATTR(PAL0,0,0,0,tile+i), 6+i, y+2);
			}
		}
		// Weapon list
		for(uint16_t i = 0; i < 5; i++) {
			if(!file.weapon[i]) continue;
			// X tile pos and VRAM index to put the ArmsImage tiles
			uint16_t x = 24 + i*2;
			uint16_t tile = TILE_FACEINDEX + index*20 + i*4;
			vdp_tiles_load_from_rom(SPR_TILES(&SPR_ArmsImage, 0, file.weapon[i]), tile, 4);
			// 4 mappings for ArmsImage icon
			vdp_map_xy(VDP_PLAN_A, TILE_ATTR(PAL0,0,0,0,tile),   x,   y+2);
			vdp_map_xy(VDP_PLAN_A, TILE_ATTR(PAL0,0,0,0,tile+2), x+1, y+2);
			vdp_map_xy(VDP_PLAN_A, TILE_ATTR(PAL0,0,0,0,tile+1), x,   y+3);
			vdp_map_xy(VDP_PLAN_A, TILE_ATTR(PAL0,0,0,0,tile+3), x+1, y+3);
		}
	} else {
		if(cfg_language) {
			uint16_t tile_index = (0xB000 >> 5) + (index << 5);
			kanji_draw(VDP_PLAN_A, tile_index,   0x100+584, 6, y, 0, 1); // 新
			kanji_draw(VDP_PLAN_A, tile_index+4, 0x100+61,  8, y, 0, 1); // し
			kanji_draw(VDP_PLAN_A, tile_index+8, 0x100+42, 10, y, 0, 1); // い
		} else {
			vdp_puts(VDP_PLAN_A, "New Game", 6, y);
		}
		vdp_text_clear(VDP_PLAN_A, 26, y, 10);
		vdp_text_clear(VDP_PLAN_A, 6, y+2, 8);
		vdp_text_clear(VDP_PLAN_A, 24, y+2, 12);
		vdp_text_clear(VDP_PLAN_A, 24, y+3, 12);
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

extern uint8_t tpal;

uint8_t saveselect_main() {
	gamemode = GM_SAVESEL;
	
	uint8_t cursor = 0;
	uint8_t cursorMode = CM_LOAD;
	uint8_t fileToCopy = 0;
	uint8_t sprFrame = 0, sprTime = ANIM_SPEED;
	uint8_t file_used[SRAM_FILE_MAX];
	
	vdp_set_display(FALSE);
	// Keep stuff from the title screen
	vdp_map_clear(VDP_PLAN_A);
	vdp_map_clear(VDP_PLAN_B);
	vdp_sprites_clear();
	VDPSprite sprCursor = { 
		.attr = TILE_ATTR(tpal,0,0,1,TILE_SHEETINDEX+32),
		.size = SPRITE_SIZE(2,2)
	};
	
	draw_cursor_mode(cursorMode);
	for(uint16_t i = 0; i < SRAM_FILE_MAX; i++) file_used[i] = refresh_file(i);
	vdp_puts(VDP_PLAN_A, "Copy", 6, 25);
	vdp_puts(VDP_PLAN_A, "Delete", 16, 25);
	
	vdp_set_display(TRUE);
	
	oldstate = ~0;
	while(TRUE) {
		if(joy_pressed(BUTTON_C) || joy_pressed(BUTTON_START)) { // Confirm action
			if(cursor < SRAM_FILE_MAX) {
				switch(cursorMode) {
					case CM_LOAD: { // Load/New Game on file over cursor
						//song_stop();
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
						sound_play(SND_MENU_PROMPT, 0);
						cursorMode = CM_CONFIRM;
						break;
					}
					case CM_CONFIRM: { // Clear file
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
			if(cursorMode != CM_LOAD) {
				cursorMode = CM_LOAD;
				draw_cursor_mode(cursorMode);
			} else {
				return 0xFF;
			}
		}
		if(joy_pressed(BUTTON_UP) || joy_pressed(BUTTON_LEFT)) {
			if(cursor == 0) cursor = OPTIONS - 1;
			else cursor--;
			sound_play(SND_MENU_MOVE, 0);
		} else if(joy_pressed(BUTTON_DOWN) || joy_pressed(BUTTON_RIGHT)) {
			if(cursor == OPTIONS - 1) cursor = 0;
			else cursor++;
			sound_play(SND_MENU_MOVE, 0);
		}
		// Animate quote sprite
		if(--sprTime == 0) {
			sprTime = ANIM_SPEED;
			if(++sprFrame >= ANIM_FRAMES) sprFrame = 0;
			sprite_index(sprCursor, TILE_SHEETINDEX+32+sprFrame*4);
		}
		if(cursorMode == CM_PASTE && (sprTime & 1)) {
			// Blink quote between source and destination of a copy
			sprite_pos(sprCursor, cursor_pos[fileToCopy].x, cursor_pos[fileToCopy].y);
		} else {
			// Draw quote sprite at cursor position
			sprite_pos(sprCursor, cursor_pos[cursor].x, cursor_pos[cursor].y);
		}
	vdp_sprite_add(&sprCursor);
		
		ready = TRUE;
		vdp_vsync(); aftervsync();
	}
	return file_used[cursor];
}
