#include "common.h"

#include "md/joy.h"
#include "md/sys.h"
#include "md/vdp.h"

#include "audio.h"
#include "camera.h"
#include "effect.h"
#include "entity.h"
#include "hud.h"
#include "npc.h"
#include "player.h"
#include "resources.h"
#include "sheet.h"
#include "stage.h"
#include "system.h"
#include "tables.h"
#include "tsc.h"
#include "weapon.h"
#include "window.h"
#include "cjk.h"

#include "res/local.h"
#include "res/pal.h"
#include "res/tiles.h"

#include "gamemode.h"

#define OPTIONS		4
#define SAVES		22
#define ANIM_SPEED	7
#define ANIM_FRAMES	4

static const uint16_t cheat[2][10] = {
	{JOY_UP, JOY_DOWN, JOY_LEFT, JOY_RIGHT, NULL },
	{JOY_UP, JOY_UP,   JOY_DOWN, JOY_DOWN, JOY_LEFT, JOY_RIGHT, JOY_LEFT, JOY_RIGHT, NULL },
};

uint8_t tpal;

#define DATE_LEN	7
#define VER_LEN		10
#define PAGE_LEN	32
extern const char Date[];
extern const char Version[];
extern const char Homepage[];

static uint16_t ls_cursor_x(uint16_t cur) {
	return cur >= 11 ? 4+18 : 4;
}

static uint16_t ls_cursor_y(uint16_t cur) {
	return (pal_mode ? 5 : 4) + (cur % 11) * 2;
}

void print_version(void) {
    vdp_nputs(VDP_PLANE_A, Version,  5, 23, VER_LEN);
	vdp_nputs(VDP_PLANE_A, Date + 9, 28, 23, DATE_LEN);
	vdp_nputs(VDP_PLANE_A, Homepage + 8, 5, 25, PAGE_LEN);
}

uint8_t titlescreen_main(void) {
	gamemode = GM_TITLE;
	
	uint8_t cheatEntry[2] = { 0, 0 }, cheatEnable[2] = { FALSE, FALSE };
	uint8_t cursor = 0;
	uint32_t besttime = 0xFFFFFFFF;
	uint8_t tsong = SONG_TITLE;
	const SpriteDef *tsprite = &SPR_Quote;
	
	vdp_colors(0, PAL_FadeOut, 64);
	vdp_set_display(FALSE);
	vdp_map_clear(VDP_PLANE_A);
	vdp_map_clear(VDP_PLANE_B);
	vdp_hscroll(VDP_PLANE_A, 0);
	vdp_hscroll(VDP_PLANE_B, 0);
	vdp_vscroll(VDP_PLANE_A, 0);
	vdp_vscroll(VDP_PLANE_B, 0);
	vdp_sprites_clear();
	// Check save data, only enable continue if save data exists
	if(system_checkdata() != SRAM_INVALID) {
		besttime = system_load_counter(); // 290.rec data
		system_load_config();
	}
	// Change character & song based on 290.rec value
	tpal = PAL1;
	if(besttime <= 3*3000) {
		tsprite = &SPR_Sue;
		tsong = 2; // Safety
	} else if(besttime <= 4*3000) {
		tsprite = &SPR_King;
		tpal = PAL3;
		tsong = 41; // White Stone Wall
	} else if(besttime <= 5*3000) {
		tsprite = &SPR_Toroko;
		tpal = PAL3;
		tsong = 40; // Toroko's Theme
	} else if(besttime <= 6*3000) {
		tsprite = &SPR_Curly;
		tpal = PAL3;
		tsong = 36; // Running Hell
	}
	// Load quote sprite
	SHEET_LOAD(tsprite, 5, 4, TILE_PROMPTINDEX, 1, 1, 0, 2, 0, 3);
	Sprite sprCursor = {
		.attr = TILE_ATTR(tpal,0,0,1,TILE_PROMPTINDEX),
		.size = SPRITE_SIZE(2,2)
	};
	uint8_t sprFrame = 0, sprTime = ANIM_SPEED;
	
    print_version();

	// Title & CS version
    vdp_tiles_load_uftc(*TS_TITLE, TILE_BACKINDEX, 0, 18*5);
	vdp_map_fill_rect(VDP_PLANE_B, TILE_ATTR(PAL0, 0, 0, 0, TILE_BACKINDEX), 11, 3, 18, 5, 1);
	
	const uint16_t MENUY = 12;

	// Menu and version text
	cjk_reset(CJK_TITLE);
	const int16_t cur_yoff = (cfg_language >= LANG_JA && cfg_language < LANG_RU) ? 0 : -4;
	const uint8_t *txt = (const uint8_t *)MENU_STR;
	loc_vdp_nputs(VDP_PLANE_A, &txt[MENU_TITLE_START_GAME*32], 15, MENUY, 16);
	loc_vdp_nputs(VDP_PLANE_A, &txt[MENU_TITLE_SOUND_TEST*32], 15, MENUY+4, 16);
	loc_vdp_nputs(VDP_PLANE_A, &txt[MENU_TITLE_CONFIG*32], 15, MENUY+6, 16);

	// Set palettes last
	vdp_colors(0, PAL_Main, 16);
	vdp_colors(16, PAL_Main, 16);
	vdp_color(PAL0, 0x444); // Gray background
	vdp_color(PAL0 + 8, 0x8CE); // Yellow text
	// PAL_Regu, for King and Toroko
	vdp_colors(48, PAL_Regu, 16);
	
	vdp_set_display(TRUE);

	song_play(tsong);
    joystate_old = ~0;
	while(!joy_pressed(btn[cfg_btn_jump]) && !joy_pressed(btn[cfg_btn_pause])) {
		for(uint8_t i = 0; i < 2; i++) {
			if(!cheatEnable[i]) {
				if(joy_pressed(cheat[i][cheatEntry[i]])) {
					cheatEntry[i]++;
					if(cheat[i][cheatEntry[i]] == NULL) {
						cheatEnable[i] = TRUE;
						sound_play(SND_COMPUTER_BEEP, 5);
					}
				} else if(cheatEntry[i]) {
					if((joystate & (~cheat[i][cheatEntry[i]-1])) &&
						!joy_pressed(cheat[i][cheatEntry[i]-1])) cheatEntry[i] = 0;
				}
			}
		}
		if(joy_pressed(JOY_UP)) {
			if(cursor == 0) cursor = OPTIONS - 1;
			else cursor--;
			// Skip over "continue" if no save data
			if(/*sram_state != SRAM_VALID_SAVE &&*/ cursor == 1) cursor--;
			sound_play(SND_MENU_MOVE, 0);
		} else if(joy_pressed(JOY_DOWN)) {
			if(cursor == OPTIONS - 1) cursor = 0;
			else cursor++;
			// Skip over "continue" if no save data
			if(/*sram_state != SRAM_VALID_SAVE &&*/ cursor == 1) cursor++;
			sound_play(SND_MENU_MOVE, 0);
		}
		// Animate quote sprite
		if(--sprTime == 0) {
			sprTime = ANIM_SPEED;
			if(++sprFrame >= ANIM_FRAMES) sprFrame = 0;
			sprite_index(&sprCursor, TILE_PROMPTINDEX+sprFrame*4);
		}
		// Draw quote sprite at cursor position
		sprite_pos(&sprCursor, 13*8-4, (MENUY*8+cursor*16) + cur_yoff);
	    vdp_sprite_add(&sprCursor);

		if(besttime > 0 && besttime < 300000) system_draw_counter();
		
		ready = TRUE;
        sys_wait_vblank(); aftervsync();
	}
	if(cheatEnable[0] && (joystate & JOY_A) && joy_pressed(btn[cfg_btn_pause])) {
		cursor = 0;
		
		vdp_map_clear(VDP_PLANE_A);
		vdp_color(0, 0x222); // Darken background colors
		vdp_color(8, 0x468);
		vdp_color(12, 0x666);
		vdp_color(16 + 15, 0xAAA);
		static const uint16_t stageIDs[SAVES] = {
			STAGE_FIRST_CAVE, //" First Cave",
			STAGE_MIMIGA_VILLAGE, //" Mimiga Village",
			STAGE_EGG_CORRIDOR, //" Egg Corridor",
			STAGE_GRASSTOWN, //" Grasstown",
			STAGE_POWER_ROOM, //" Malco",
			STAGE_GRASSTOWN_GUM, //" Balfrog",
			STAGE_SAND_ZONE_BAR, //" Sand Zone",
			STAGE_SAND_ZONE, //" Omega",
			STAGE_SAND_ZONE_STOREHOUSE, //" Storehouse",
			STAGE_LABYRINTH_I, //" Labyrinth",
			STAGE_LABYRINTH_W, //" Monster X",

			STAGE_LABYRINTH_M, //" Labyrinth M",
			STAGE_CORE, //" Core",
			STAGE_WATERWAY, //" Waterway",
			STAGE_EGG_CORRIDOR_2, //" Egg Corridor 2",
			STAGE_OUTER_WALL, //" Outer Wall",
			STAGE_PLANTATION, //" Plantation",
			STAGE_LAST_CAVE_1, //" Last Cave",
			STAGE_LAST_CAVE_2, //" Last Cave 2",
			STAGE_BALCONY, //" Balcony",
			STAGE_HELL_B1, //" Sacred Ground",
			STAGE_SEAL_CHAMBER, //" Seal Chamber",
		};

		loc_vdp_nputs(VDP_PLANE_A, &txt[MENU_TITLE_STAGE_SELECT*32], 4, pal_mode ? 2 : 1, 32);
		const uint8_t *names = (const uint8_t *)STAGE_NAMES;
		for(uint16_t i = 0; i < SAVES; i++) {
			if(cursor == i) vdp_font_pal(PAL0);
			else vdp_font_pal(PAL1);
			uint16_t tx = ls_cursor_x(i);
			uint16_t ty = ls_cursor_y(i);
			loc_vdp_nputs(VDP_PLANE_A, &names[stageIDs[i]*32], tx, ty, 16);
		}

        joystate_old = ~0;
		while(!joy_pressed(btn[cfg_btn_jump]) && !joy_pressed(btn[cfg_btn_pause])) {
			if(joy_pressed(JOY_UP)) {
				uint16_t tx = ls_cursor_x(cursor);
				uint16_t ty = ls_cursor_y(cursor);
				vdp_font_pal(PAL1);
				if(!(cfg_language >= LANG_JA && cfg_language < LANG_RU)) {
					loc_vdp_nputs(VDP_PLANE_A, &names[stageIDs[cursor]*32], tx, ty, 16);
				}
				if(cursor == 0) {
					cursor = 11-1;
				} else if(cursor == 11) {
					cursor = 22-1;
				} else {
					cursor--;
				}
				ty = ls_cursor_y(cursor);
				vdp_font_pal(PAL0);
				if(!(cfg_language >= LANG_JA && cfg_language < LANG_RU)) {
					loc_vdp_nputs(VDP_PLANE_A, &names[stageIDs[cursor]*32], tx, ty, 16);
				}
				sound_play(SND_MENU_MOVE, 0);
			} else if(joy_pressed(JOY_DOWN)) {
				uint16_t tx = ls_cursor_x(cursor);
				uint16_t ty = ls_cursor_y(cursor);
				vdp_font_pal(PAL1);
				if(!(cfg_language >= LANG_JA && cfg_language < LANG_RU)) {
					loc_vdp_nputs(VDP_PLANE_A, &names[stageIDs[cursor]*32], tx, ty, 16);
				}
				if(cursor == 11-1) {
					cursor = 0;
				} else if(cursor == 22-1) {
					cursor = 11;
				} else {
					cursor++;
				}
				ty = ls_cursor_y(cursor);
				vdp_font_pal(PAL0);
				if(!(cfg_language >= LANG_JA && cfg_language < LANG_RU)) {
					loc_vdp_nputs(VDP_PLANE_A, &names[stageIDs[cursor]*32], tx, ty, 16);
				}
				sound_play(SND_MENU_MOVE, 0);
			} else if(joy_pressed(JOY_LEFT) || joy_pressed(JOY_RIGHT)) {
				uint16_t tx = ls_cursor_x(cursor);
				uint16_t ty = ls_cursor_y(cursor);
				vdp_font_pal(PAL1);
				if(!(cfg_language >= LANG_JA && cfg_language < LANG_RU)) {
					loc_vdp_nputs(VDP_PLANE_A, &names[stageIDs[cursor]*32], tx, ty, 16);
				}
				if(cursor >= 11) cursor -= 11;
				else cursor += 11;
				tx = ls_cursor_x(cursor);
				vdp_font_pal(PAL0);
				if(!(cfg_language >= LANG_JA && cfg_language < LANG_RU)) {
					loc_vdp_nputs(VDP_PLANE_A, &names[stageIDs[cursor]*32], tx, ty, 16);
				}
				sound_play(SND_MENU_MOVE, 0);
			}
			// Animate quote sprite
			if(--sprTime == 0) {
				sprTime = ANIM_SPEED;
				if(++sprFrame >= ANIM_FRAMES) sprFrame = 0;
				sprite_index(&sprCursor, TILE_PROMPTINDEX+sprFrame*4);
			}
			// Draw quote sprite at cursor position
			{
				uint16_t tx = ls_cursor_x(cursor);
				uint16_t ty = ls_cursor_y(cursor);
				sprite_pos(&sprCursor, (tx-2)*8-4, (ty*8) + cur_yoff);
		    	vdp_sprite_add(&sprCursor);
			}
			
			ready = TRUE;
            sys_wait_vblank(); aftervsync();
		}
		vdp_font_pal(PAL0);
		cursor += 4;
	}
	if(cheatEnable[1]) iSuckAtThisGameSHIT = TRUE;
	//if(cursor > 3) {
	//	song_stop();
	//}
	sound_play(SND_MENU_SELECT, 0);
	return cursor;
}
