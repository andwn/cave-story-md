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

void print_version(void) {
    vdp_nputs(VDP_PLANE_A, Version,  5, 23, VER_LEN);
	vdp_nputs(VDP_PLANE_A, Date + 9, 28, 23, DATE_LEN);
	vdp_nputs(VDP_PLANE_A, Homepage + 8, 5, 25, PAGE_LEN);
}

uint8_t titlescreen_main() {
	gamemode = GM_TITLE;
	
	uint8_t cheatEntry[2] = { 0, 0 }, cheatEnable[2] = { FALSE, FALSE };
	uint8_t cursor = 0;
	uint32_t besttime = 0xFFFFFFFF;
	uint8_t tsong = SONG_TITLE;
	const SpriteDefinition *tsprite = &SPR_Quote;
	
	vdp_colors(0, PAL_FadeOut, 64);
	vdp_set_display(FALSE);
	vdp_map_clear(VDP_PLANE_A);
	vdp_map_clear(VDP_PLANE_B);
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
	SHEET_LOAD(tsprite, 5, 4, TILE_SHEETINDEX+32, 1, 0,1, 0,0, 0,2, 0,0, 0,3);
	Sprite sprCursor = {
		.attr = TILE_ATTR(tpal,0,0,1,TILE_SHEETINDEX+32),
		.size = SPRITE_SIZE(2,2)
	};
	uint8_t sprFrame = 0, sprTime = ANIM_SPEED;
	
    print_version();

	// Title & CS version
    vdp_tiles_load_uftc(*TS_TITLE, 16, 0, 72);
	vdp_map_fill_rect(VDP_PLANE_B, TILE_ATTR(PAL0, 0, 0, 0, 16), 11, 3, 18, 4, 1);
	
#ifdef SHOW_MYCOM_LOGO
	const uint16_t MENUY = 15;
	vdp_tiles_load_uftc(UFTC_MyCOM, 16+72, 0, 72);
	vdp_map_fill_rect(VDP_PLANE_B, TILE_ATTR(PAL0, 0, 0, 0, 16+72), 11, 8, 18, 4, 1);
#else
	const uint16_t MENUY = 12;
	//vdp_map_fill_rect(VDP_PLANE_B, TILE_ATTR(PAL0, 0, 0, 0, 16 + 18 * 4), 11, 23, 18, 2, 1);
#endif

	// Menu and version text
	vdp_puts(VDP_PLANE_A, "Start Game", 15, MENUY);
	vdp_puts(VDP_PLANE_A, "Sound Test", 15, MENUY+4);
	vdp_puts(VDP_PLANE_A, "Config", 15, MENUY+6);

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
			sprite_index(&sprCursor, TILE_SHEETINDEX+32+sprFrame*4);
		}
		// Draw quote sprite at cursor position
		sprite_pos(&sprCursor, 13*8-4, (MENUY*8+cursor*16)-4);
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
		static const char *levelStr[] = {
			" First Cave",
			" Mimiga Village",
			" Egg Corridor",
			" Grasstown",
			" Malco",
			" Balfrog",
			" Sand Zone",
			" Omega",
			" Storehouse",
			" Labyrinth",
			" Monster X",
			" Labyrinth M",
			" Core",
			" Waterway",
			" Egg Corridor 2",
			" Outer Wall",
			" Plantation",
			" Last Cave",
			" Last Cave 2",
			" Balcony",
			" Sacred Ground",
			" Seal Chamber",
		};
		uint16_t tx = 11, ty = 1;
		if(pal_mode) ty++;
		vdp_puts(VDP_PLANE_A, "= Welcome to Warp Zone =", 7, ty);
		ty += 2;
		for(uint8_t i = 0; i < SAVES; i++) {
			if(cursor == i) vdp_font_pal(PAL0);
			else vdp_font_pal(PAL1);
			vdp_puts(VDP_PLANE_A, levelStr[i], tx, ty + i);
		}

        joystate_old = ~0;
		while(!joy_pressed(btn[cfg_btn_jump]) && !joy_pressed(btn[cfg_btn_pause])) {
			if(joy_pressed(JOY_UP)) {
				vdp_font_pal(PAL1);
				vdp_puts(VDP_PLANE_A, levelStr[cursor], tx, ty + cursor);
				if(cursor == 0) cursor = SAVES - 1;
				else cursor--;
				//if(cursor == 8 || cursor == 16) cursor--;
				vdp_font_pal(PAL0);
				vdp_puts(VDP_PLANE_A, levelStr[cursor], tx, ty + cursor);
				sound_play(SND_MENU_MOVE, 0);
			} else if(joy_pressed(JOY_DOWN)) {
				vdp_font_pal(PAL1);
				vdp_puts(VDP_PLANE_A, levelStr[cursor], tx, ty + cursor);
				if(cursor == SAVES - 1) cursor = 0;
				else cursor++;
				//if(cursor == 8 || cursor == 16) cursor++;
				vdp_font_pal(PAL0);
				vdp_puts(VDP_PLANE_A, levelStr[cursor], tx, ty + cursor);
				sound_play(SND_MENU_MOVE, 0);
			}
			// Animate quote sprite
			if(--sprTime == 0) {
				sprTime = ANIM_SPEED;
				if(++sprFrame >= ANIM_FRAMES) sprFrame = 0;
				sprite_index(&sprCursor, TILE_SHEETINDEX+32+sprFrame*4);
			}
			// Draw quote sprite at cursor position
			sprite_pos(&sprCursor, (tx-2)*8-4, (ty*8+cursor*8)-5);
		    vdp_sprite_add(&sprCursor);
			
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
