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

#define OPTIONS		4
#define SAVES		22
#define ANIM_SPEED	7
#define ANIM_FRAMES	4

static const uint16_t cheat[2][10] = {
	{ BUTTON_UP, BUTTON_DOWN, BUTTON_LEFT, BUTTON_RIGHT, NULL },
	{ BUTTON_UP, BUTTON_UP, BUTTON_DOWN, BUTTON_DOWN, 
	  BUTTON_LEFT, BUTTON_RIGHT, BUTTON_LEFT, BUTTON_RIGHT, NULL },
};

uint8_t titlescreen_main() {
	gamemode = GM_TITLE;
	
	uint8_t cheatEntry[2] = { 0, 0 }, cheatEnable[2] = { FALSE, FALSE };
	uint8_t cursor = 0;
	uint32_t besttime = 0xFFFFFFFF;
	uint8_t tsong = SONG_TITLE;
	const SpriteDefinition *tsprite = &SPR_Quote;
	
	VDP_setEnable(FALSE);
	
	sprites_clear();
	// Check save data, only enable continue if save data exists
	uint8_t sram_state = system_checkdata();
	if(sram_state == SRAM_VALID_SAVE) {
	//	VDP_drawText("Continue", 15, 14);
	//	cursor = 1;
		besttime = system_load_counter(); // 290.rec data
	}
	if(sram_state != SRAM_INVALID) system_load_config();
	// Change character & song based on 290.rec value
	if(besttime <= 3*3000) {
		tsprite = &SPR_Sue;
		tsong = 2; // Safety
	} else if(besttime <= 4*3000) {
		tsprite = &SPR_King;
		tsong = 41; // White Stone Wall
	} else if(besttime <= 5*3000) {
		tsprite = &SPR_Toroko;
		tsong = 40; // Toroko's Theme
	} else if(besttime <= 6*3000) {
		tsprite = &SPR_Curly;
		tsong = 36; // Running Hell
	}
	// Load quote sprite
	SHEET_LOAD(tsprite, 5, 4, TILE_SHEETINDEX+32, 1, 0,1, 0,0, 0,2, 0,0, 0,3);
	VDPSprite sprCursor = { 
		.attribut = TILE_ATTR_FULL(PAL1,0,0,1,TILE_SHEETINDEX+32),
		.size = SPRITE_SIZE(2,2)
	};
	uint8_t sprFrame = 0, sprTime = ANIM_SPEED;
	// Menu and version text
	VDP_drawText("Start Game", 15, 12);
	VDP_drawText("Sound Test", 15, 16);
	VDP_drawText("Config", 15, 18);
	// Debug
	{
		char vstr[40];
		sprintf(vstr, "Test Build - %s", __DATE__);
		VDP_drawText(vstr, 4, 26);
	}
	// Release
	//VDP_drawText("Mega Drive Version 0.4.1 2017.09", 4, 26);
	VDP_loadTileSet(cfg_language ? &TS_J_Title : &TS_Title, TILE_USERINDEX, TRUE);
	VDP_fillTileMapRectInc(PLAN_B,
			TILE_ATTR_FULL(PAL0,0,0,0,TILE_USERINDEX), 11, 3, 18, 4);
	VDP_fillTileMapRectInc(PLAN_B,
			TILE_ATTR_FULL(PAL0,0,0,0,TILE_USERINDEX + 18*4), 11, 23, 18, 2);
	
	// Set palettes last
	VDP_setPalette(PAL0, PAL_Main.data);
	VDP_setPalette(PAL1, PAL_Main.data);
	VDP_setPaletteColor(PAL0, 0x444); // Gray background
	VDP_setPaletteColor(PAL0 + 8, 0x8CE); // Yellow text
	// PAL_Regu, for King and Toroko
	VDP_setPalette(PAL3, PAL_Regu.data);
	
	VDP_setEnable(TRUE);

	song_play(tsong);
	while(!joy_pressed(btn[cfg_btn_jump]) && !joy_pressed(btn[cfg_btn_pause])) {
		input_update();
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
		if(joy_pressed(BUTTON_UP)) {
			if(cursor == 0) cursor = OPTIONS - 1;
			else cursor--;
			// Skip over "continue" if no save data
			if(/*sram_state != SRAM_VALID_SAVE &&*/ cursor == 1) cursor--;
			sound_play(SND_MENU_MOVE, 0);
		} else if(joy_pressed(BUTTON_DOWN)) {
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
			sprite_index(sprCursor, TILE_SHEETINDEX+32+sprFrame*4);
		}
		// Draw quote sprite at cursor position
		sprite_pos(sprCursor, 13*8-4, (12*8+cursor*16)-4);
		sprite_add(sprCursor);
		
		ready = TRUE;
		vsync(); aftervsync();
	}
	if(cheatEnable[0] && (joystate&BUTTON_A) && joy_pressed(btn[cfg_btn_pause])) {
		cursor = 0;
		input_update();
		
		VDP_clearPlan(PLAN_A, TRUE);
		VDP_setPaletteColor(0, 0x222); // Darken background colors
		VDP_setPaletteColor(8, 0x468);
		VDP_setPaletteColor(12, 0x666);
		VDP_setPaletteColor(16 + 15, 0xAAA);
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
		if(IS_PALSYSTEM) ty++;
		VDP_drawText("= Welcome to Warp Zone =", 7, ty);
		ty += 2;
		for(uint8_t i = 0; i < SAVES; i++) {
			if(cursor == i) VDP_setTextPalette(PAL0);
			else VDP_setTextPalette(PAL1);
			VDP_drawText(levelStr[i], tx, ty+i);
		}
		
		while(!joy_pressed(btn[cfg_btn_jump]) && !joy_pressed(btn[cfg_btn_pause])) {
			input_update();
			if(joy_pressed(BUTTON_UP)) {
				VDP_setTextPalette(PAL1);
				VDP_drawText(levelStr[cursor], tx, ty + cursor);
				if(cursor == 0) cursor = SAVES - 1;
				else cursor--;
				//if(cursor == 8 || cursor == 16) cursor--;
				VDP_setTextPalette(PAL0);
				VDP_drawText(levelStr[cursor], tx, ty + cursor);
				sound_play(SND_MENU_MOVE, 0);
			} else if(joy_pressed(BUTTON_DOWN)) {
				VDP_setTextPalette(PAL1);
				VDP_drawText(levelStr[cursor], tx, ty + cursor);
				if(cursor == SAVES - 1) cursor = 0;
				else cursor++;
				//if(cursor == 8 || cursor == 16) cursor++;
				VDP_setTextPalette(PAL0);
				VDP_drawText(levelStr[cursor], tx, ty + cursor);
				sound_play(SND_MENU_MOVE, 0);
			}
			// Animate quote sprite
			if(--sprTime == 0) {
				sprTime = ANIM_SPEED;
				if(++sprFrame >= ANIM_FRAMES) sprFrame = 0;
				sprite_index(sprCursor, TILE_SHEETINDEX+32+sprFrame*4);
			}
			// Draw quote sprite at cursor position
			sprite_pos(sprCursor, (tx-2)*8-4, (ty*8+cursor*8)-5);
			sprite_add(sprCursor);
			
			ready = TRUE;
			vsync(); aftervsync();
		}
		VDP_setTextPalette(PAL0);
		cursor += 4;
	}
	if(cheatEnable[1]) iSuckAtThisGameSHIT = TRUE;
	//if(cursor > 3) {
	//	song_stop();
	//}
	sound_play(SND_MENU_SELECT, 0);
	return cursor;
}
