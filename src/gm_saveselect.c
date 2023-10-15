#include "common.h"

#include "audio.h"
#include "res/tiles.h"
#include "res/local.h"
#include "camera.h"
#include "md/dma.h"
#include "effect.h"
#include "entity.h"
#include "hud.h"
#include "md/joy.h"
#include "cjk.h"
#include "math.h"
#include "md/stdlib.h"
#include "npc.h"
#include "player.h"
#include "resources.h"
#include "sheet.h"
#include "stage.h"
#include "system.h"
#include "tables.h"
#include "tsc.h"
#include "md/sys.h"
#include "md/vdp.h"
#include "weapon.h"
#include "window.h"
#include "md/xgm.h"

#include "gamemode.h"

#define ANIM_SPEED	7
#define ANIM_FRAMES	4
#define OPTIONS (SRAM_FILE_MAX + 2)

enum { CM_LOAD, CM_COPY, CM_PASTE, CM_DELETE, CM_CONFIRM };

static void draw_cursor_mode(uint8_t mode) {
	switch(mode) {
		case CM_LOAD:    vdp_puts(VDP_PLANE_A, " Load Save Data ", 12, 2); break;
		case CM_COPY:    vdp_puts(VDP_PLANE_A, " Copy Save Data ", 12, 2); break;
		case CM_PASTE:   vdp_puts(VDP_PLANE_A, "Paste Save Data ", 12, 2); break;
		case CM_DELETE:  vdp_puts(VDP_PLANE_A, "Delete Save Data", 12, 2); break;
		case CM_CONFIRM: vdp_puts(VDP_PLANE_A, " Are you sure?  ", 12, 2); break;
	}
}

static uint16_t GetNextChar(uint16_t stage, uint16_t index) {
	const uint8_t *name = ((const uint8_t*)STAGE_NAMES) + (stage << 4);
	uint16_t chr = name[index];
	if(chr >= 0xE0 && chr < 0xFF) {
		return (chr - 0xE0) * 0x60 + (name[index + 1] - 0x20) + 0x100;
	} else {
		return chr;
	}
}

static uint8_t refresh_file(uint8_t index) {
	uint16_t y = 4 + index * 5;
	SaveEntry file;
	
	system_peekdata(index, &file);
    //disable_ints();
    //z80_pause_fast();
	vdp_text_clear(VDP_PLANE_A, 6, y, 16); // Erase any previous stage name text
	if(cfg_language >= LANG_JA && cfg_language <= LANG_KO) {
        vdp_text_clear(VDP_PLANE_A, 6, y + 1, 16); // And a second line underneath
    }
	if(file.used) {
		// Map name
		if(cfg_language >= LANG_JA && cfg_language <= LANG_KO) {
			uint16_t x = 6;
			uint16_t name_index = 0;
			for(uint16_t pos = 0; name_index < 16; pos++) {
				uint16_t c = GetNextChar(file.stage_id, name_index++);
				if(c == 0) break; // End of string
				if(c > 0xFF) name_index++;
                cjk_draw(VDP_PLANE_A, c, x, y, 0, 1);
				x += 1 + (pos & 1);
			}
            cjk_newline();
		} else {
			vdp_puts(VDP_PLANE_A, stage_info[file.stage_id].name, 6, y);
		}
		// Play time
		char timeStr[4] = "00";
		//sprintf(timeStr, "%02hu:%02hu:%02hu", file.hour, file.minute, file.second);
        timeStr[0] = '0' + div10[file.hour];
        timeStr[1] = '0' + mod10[file.hour];
        vdp_puts(VDP_PLANE_A, timeStr, 26, y);
        vdp_puts(VDP_PLANE_A, ":", 28, y);
        timeStr[0] = '0' + div10[file.minute];
        timeStr[1] = '0' + mod10[file.minute];
        vdp_puts(VDP_PLANE_A, timeStr, 29, y);
        vdp_puts(VDP_PLANE_A, ":", 31, y);
        timeStr[0] = '0' + div10[file.second];
        timeStr[1] = '0' + mod10[file.second];
        vdp_puts(VDP_PLANE_A, timeStr, 32, y);
		// Health bar
		{
			uint32_t tileData[8][8];
			int16_t fillHP = 40 * file.health / max(file.max_health, 1);
			for(uint8_t i = 0; i < 5; i++) {
				int16_t addrHP = min(fillHP*8, 7*8);
				if(addrHP < 0) addrHP = 0;
				memcpy(tileData[i+3], &TS_HudBar[addrHP], 32);
				fillHP -= 8;
			}
			memcpy(tileData[0], &SPR_TILES(&SPR_Hud2, 0, 0)[3*8], 32);
			uint8_t digit = div10[file.health];
			if(digit) {
				memcpy(tileData[1], &TS_Numbers[(digit)*8], 32);
			} else {
				memcpy(tileData[1], BlankData, 32);
			}
			memcpy(tileData[2], &TS_Numbers[mod10[file.health]*8], 32);
			uint16_t tile = TILE_SHEETINDEX + index*8;
			dma_now(DmaVRAM, (uint32_t)tileData[0], tile*32, 16*8, 2);
			for(int i = 0; i < 8; i++) {
				vdp_map_xy(VDP_PLANE_A, TILE_ATTR(PAL0, 0, 0, 0, tile + i), 6 + i, y + 2);
			}
		}
		// Weapon list
		for(uint16_t i = 0; i < 5; i++) {
			if(!file.weapon[i]) continue;
			// X tile pos and VRAM index to put the ArmsImage tiles
			uint16_t x = 24 + i*2;
			uint16_t tile = TILE_FACEINDEX - 40 + index*20 + i*4;
			vdp_tiles_load(SPR_TILES(&SPR_ArmsImage, 0, file.weapon[i]), tile, 4);
			// 4 mappings for ArmsImage icon
			vdp_map_xy(VDP_PLANE_A, TILE_ATTR(PAL0, 0, 0, 0, tile), x, y + 2);
			vdp_map_xy(VDP_PLANE_A, TILE_ATTR(PAL0, 0, 0, 0, tile + 2), x + 1, y + 2);
			vdp_map_xy(VDP_PLANE_A, TILE_ATTR(PAL0, 0, 0, 0, tile + 1), x, y + 3);
			vdp_map_xy(VDP_PLANE_A, TILE_ATTR(PAL0, 0, 0, 0, tile + 3), x + 1, y + 3);
		}
	} else {
		if(cfg_language == LANG_JA) {
			cjk_draw(VDP_PLANE_A, 0x100 + 584, 6, y, 0, 1); // 新
			cjk_draw(VDP_PLANE_A, 0x100 + 61, 7, y, 0, 1); // し
			cjk_draw(VDP_PLANE_A, 0x100 + 42, 9, y, 0, 1); // い
            cjk_newline();
        } else if(cfg_language == LANG_ZH) {
            cjk_draw(VDP_PLANE_A, 0x100 + 1172, 6, y, 0, 1); // 新
            cjk_draw(VDP_PLANE_A, 0x100 + 267, 7, y, 0, 1); // 的
            cjk_newline();
        } else if(cfg_language == LANG_KO) {
            cjk_draw(VDP_PLANE_A, 0x100 + 384, 6, y, 0, 1); // 새
            cjk_draw(VDP_PLANE_A, 0x100 + 250, 7, y, 0, 1); // 로
            cjk_draw(VDP_PLANE_A, 0x100 + 516, 9, y, 0, 1); // 운
            cjk_newline();
        } else {
            vdp_puts(VDP_PLANE_A, "New Game", 6, y);
        }
		vdp_text_clear(VDP_PLANE_A, 26, y, 10);
		vdp_text_clear(VDP_PLANE_A, 6, y + 2, 8);
		vdp_text_clear(VDP_PLANE_A, 24, y + 2, 12);
		vdp_text_clear(VDP_PLANE_A, 24, y + 3, 12);
	}
    //z80_resume();
    //enable_ints();
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
	vdp_map_clear(VDP_PLANE_A);
	vdp_map_clear(VDP_PLANE_B);
	vdp_sprites_clear();
	Sprite sprCursor = {
		.attr = TILE_ATTR(tpal,0,0,1,TILE_SHEETINDEX+32),
		.size = SPRITE_SIZE(2,2)
	};
	
	draw_cursor_mode(cursorMode);

    cjk_reset(CJK_TITLE);
	for(uint16_t i = 0; i < SRAM_FILE_MAX; i++) {
        file_used[i] = refresh_file(i);
    }
	vdp_puts(VDP_PLANE_A, "Copy", 6, 25);
	vdp_puts(VDP_PLANE_A, "Delete", 16, 25);
	
	vdp_set_display(TRUE);

    joystate_old = ~0;
	while(TRUE) {
		if(joy_pressed(btn[cfg_btn_jump]) || joy_pressed(btn[cfg_btn_pause])) { // Confirm action
			if(cursor < SRAM_FILE_MAX) {
				switch(cursorMode) {
					case CM_LOAD: { // Load/New Game on file over cursor
						sound_play(SND_MENU_SELECT, 0);
						sram_file = cursor;
						goto SelectDone;
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
                            if(cfg_language >= LANG_JA && cfg_language <= LANG_KO) {
                                cjk_reset(CJK_TITLE);
                                for (uint16_t i = 0; i < SRAM_FILE_MAX; i++) {
                                    file_used[i] = refresh_file(i);
                                }
                            } else {
                                file_used[cursor] = refresh_file(cursor);
                            }
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
                        if(cfg_language >= LANG_JA && cfg_language <= LANG_KO) {
                            cjk_reset(CJK_TITLE);
                            for (uint16_t i = 0; i < SRAM_FILE_MAX; i++) {
                                file_used[i] = refresh_file(i);
                            }
                        } else {
                            file_used[cursor] = refresh_file(cursor);
                        }
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
		} else if(joy_pressed(btn[cfg_btn_shoot])) { // Cancel copy/delete
			if(cursorMode != CM_LOAD) {
				cursorMode = CM_LOAD;
				draw_cursor_mode(cursorMode);
			} else {
				cursor = 0xFF;
				goto SelectDone;
			}
		}
		if(joy_pressed(JOY_UP) || joy_pressed(JOY_LEFT)) {
			if(cursor == 0) cursor = OPTIONS - 1;
			else cursor--;
			sound_play(SND_MENU_MOVE, 0);
		} else if(joy_pressed(JOY_DOWN) || joy_pressed(JOY_RIGHT)) {
			if(cursor == OPTIONS - 1) cursor = 0;
			else cursor++;
			sound_play(SND_MENU_MOVE, 0);
		}
		// Animate quote sprite
		if(--sprTime == 0) {
			sprTime = ANIM_SPEED;
			if(++sprFrame >= ANIM_FRAMES) sprFrame = 0;
			sprite_index(&sprCursor, TILE_SHEETINDEX+32+sprFrame*4);
		}
		if(cursorMode == CM_PASTE && (sprTime & 1)) {
			// Blink quote between source and destination of a copy
			sprite_pos(&sprCursor, cursor_pos[fileToCopy].x, cursor_pos[fileToCopy].y);
		} else {
			// Draw quote sprite at cursor position
			sprite_pos(&sprCursor, cursor_pos[cursor].x, cursor_pos[cursor].y);
		}
	    vdp_sprite_add(&sprCursor);
		
		ready = TRUE;
        sys_wait_vblank(); aftervsync();
	}
SelectDone:
	vdp_map_clear(VDP_PLANE_A);
	vdp_sprites_clear();
	ready = TRUE;
    sys_wait_vblank(); aftervsync();
	return file_used[cursor];
}
