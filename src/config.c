#include "common.h"

#include "audio.h"
#include "camera.h"
#include "dma.h"
#include "effect.h"
#include "entity.h"
#include "hud.h"
#include "joy.h"
#include "kanji.h"
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

#define MAX_OPTIONS	12

enum { PAGE_CONTROL, PAGE_GAMEPLAY, PAGE_SAVEDATA, NUM_PAGES };

enum { MI_LABEL, MI_INPUT, MI_LANG, MI_TOGGLE, MI_ACTION, MI_RETURN, MI_MODE };

// Forward & array for actions, implementations farther down
void act_default(uint8_t page);
void act_apply(uint8_t page);
void act_title(uint8_t page);
void act_resetcounter(uint8_t page);
void act_format(uint8_t page);
ActionFunc action[5] = {
	&act_default, &act_apply, &act_title, &act_resetcounter, &act_format
};

typedef struct {
	uint8_t y;
	uint8_t jstr_index;
	uint16_t jtile_index;
	uint8_t type;
	char caption[36];
	uint8_t *valptr;
} MenuItem;

#define AKI (0x6000 >> 5)
#define WKI (0xB000 >> 5)
#define BKI (0xD000 >> 5)

const MenuItem menu[NUM_PAGES][MAX_OPTIONS] = {
	{
		{ 4,  1,  AKI,    MI_INPUT, "Jump / Confirm", &cfg_btn_jump },
		{ 6,  2,  AKI+60, MI_INPUT, "Shoot / Cancel", &cfg_btn_shoot },
		{ 8,  3,  AKI+120,MI_INPUT, "Switch (3btn) / FFwd", &cfg_btn_ffwd },
		{ 10, 4,  AKI+180,MI_INPUT, "Switch Right (6btn)", &cfg_btn_rswap },
		{ 12, 5,  AKI+240,MI_INPUT, "Switch Left (6btn)", &cfg_btn_lswap },
		{ 14, 6,  AKI+300,MI_INPUT, "Open Map (6btn)", &cfg_btn_map },
		{ 16, 7,  AKI+360,MI_INPUT, "Pause Menu", &cfg_btn_pause },
		
		{ 19, 8,  AKI+420,MI_MODE,  "Force Button Mode", &cfg_force_btn },

		{ 23, 19, BKI,    MI_ACTION, "Apply", (uint8_t*)1 },
		{ 25, 20, BKI+40, MI_ACTION, "Reset to Default", (uint8_t*)0 },
	},{
		{ 4,  9,  AKI,    MI_LANG,   "Language", &cfg_language },
		{ 7,  10, AKI+60, MI_TOGGLE, "Enable Fast Forward", &cfg_ffwd },
		{ 9,  11, AKI+120,MI_TOGGLE, "Use Up to Interact", &cfg_updoor },
		{ 11, 12, AKI+180,MI_TOGGLE, "Screen Shake in Hell", &cfg_hellquake },
		{ 13, 13, AKI+240,MI_TOGGLE, "Vulnerable After Pause", &cfg_iframebug },
		{ 15, 14, AKI+300,MI_TOGGLE, "Message Blip Sound", &cfg_msg_blip },
		{ 17, 15, AKI+360,MI_TOGGLE, "Mute BGM", &cfg_music_mute },
		{ 19, 16, AKI+420,MI_TOGGLE, "Mute SFX", &cfg_sfx_mute },

		{ 23, 19, BKI,    MI_ACTION, "Apply", (uint8_t*)1 },
		{ 25, 20, BKI+40, MI_ACTION, "Reset to Default", (uint8_t*)0 },
	},{
		{ 4,  17, AKI,    MI_ACTION, "Erase Counter", (uint8_t*)3 },
		{ 6,  18, AKI+60, MI_ACTION, "Erase All Save Data (!)", (uint8_t*)4 },
	},
};

const char boolstr[2][4] = { "OFF", "ON " };
const char modestr[3][6] = { "OFF ", "3BTN", "6BTN" };

const uint16_t jboolstr[2][2] = { 
	{ 0x100+80, 0x100+61 },  // なし
	{ 0x100+40, 0x100+110 }, // ある
};
const uint16_t jmodestr[3][2] = { 
	{ 0x100+80, 0x100+61 },  // なし
	{ '3', 'B' }, { '6', 'B' },
};

static uint16_t GetNextChar(uint16_t option, uint16_t index) {
	uint16_t chr = JConfigText[(option - 1) * 24 + index];
	if(chr >= 0xE0 && chr < 0xFF) {
		return (chr - 0xE0) * 0x60 + (JConfigText[(option - 1) * 24 + index + 1] - 0x20) + 0x100;
	} else {
		return chr;
	}
}

static void DrawJStr(uint16_t x, uint16_t y, uint16_t tile_index, uint16_t item_index) {
	uint16_t str_index = 0;
	while(str_index < 24) {
		uint16_t c = GetNextChar(item_index, str_index++);
		if(c == 0) break; // End of string
		if(c > 0xFF) str_index++;
		kanji_draw(VDP_PLAN_A, tile_index, c, x, y, 0, 1);
		x += 2;
		tile_index += 4;
	}
}

void draw_menuitem(const MenuItem *item) {
	vdp_text_clear(VDP_PLAN_A, 2, item->y, 36);
	vdp_text_clear(VDP_PLAN_A, 2, item->y+1, 36);
	if(cfg_language && item->jstr_index) {
		DrawJStr(4, item->y, item->jtile_index, item->jstr_index);
	} else {
		vdp_puts(VDP_PLAN_A, item->caption, 4, item->y);
	}
	switch(item->type) {
		case MI_LABEL: break; // Nothing
		case MI_INPUT:
		if(cfg_language) {
			uint16_t tile_index = item->jtile_index + 40;
			uint16_t c1 = btnName[*item->valptr][0];
			uint16_t c2 = btnName[*item->valptr][1];
			if(!c2) c2 = ' ';
			kanji_draw(VDP_PLAN_A, tile_index,   c1, 30, item->y, 0, TRUE);
			kanji_draw(VDP_PLAN_A, tile_index+4, c2, 32, item->y, 0, TRUE);
		} else {
			vdp_puts(VDP_PLAN_A, btnName[*item->valptr], 30, item->y);
		}
		break;
		case MI_LANG:
		if(*item->valptr) {
			uint16_t tile_index = item->jtile_index + 40;
			kanji_draw(VDP_PLAN_A, tile_index,   0x100+794, 30, item->y, 0, TRUE); // 日
			kanji_draw(VDP_PLAN_A, tile_index+4, 0x100+909, 32, item->y, 0, TRUE); // 本
			kanji_draw(VDP_PLAN_A, tile_index+8, 0x100+417, 34, item->y, 0, TRUE); // 語
		} else {
			vdp_puts(VDP_PLAN_A, "English", 30, item->y);
			vdp_text_clear(VDP_PLAN_A, 30, item->y + 1, 6); // Hide kanji
		}
		break;
		case MI_TOGGLE:
		if(cfg_language) {
			uint16_t tile_index = item->jtile_index + 40;
			kanji_draw(VDP_PLAN_A, tile_index,   jboolstr[*item->valptr][0], 32, item->y, 0, TRUE);
			kanji_draw(VDP_PLAN_A, tile_index+4, jboolstr[*item->valptr][1], 34, item->y, 0, TRUE);
		} else {
			vdp_puts(VDP_PLAN_A, boolstr[*item->valptr], 30, item->y);
		}
		break;
		case MI_ACTION: break;
		case MI_MODE:
		if(cfg_language) {
			uint16_t tile_index = item->jtile_index + 40;
			kanji_draw(VDP_PLAN_A, tile_index,   jmodestr[*item->valptr][0], 30, item->y, 0, TRUE);
			kanji_draw(VDP_PLAN_A, tile_index+4, jmodestr[*item->valptr][1], 32, item->y, 0, TRUE);
		} else {
			vdp_puts(VDP_PLAN_A, modestr[*item->valptr], 30, item->y);
		}
		break;
	}
}

void press_menuitem(const MenuItem *item, uint8_t page, VDPSprite *sprCursor) {
	//uint8_t sprFrame = 0;
	//uint8_t sprTime = ANIM_SPEED;
	
	switch(item->type) {
		case MI_LABEL: return; // Nothing
		case MI_LANG:
		case MI_TOGGLE:
		sound_play(SND_MENU_SELECT, 5);
		*item->valptr ^= 1;
		break;
		case MI_INPUT: {
			sound_play(SND_MENU_SELECT, 5);
			uint8_t released = FALSE;
			vdp_puts(VDP_PLAN_A, "Press..", 30, item->y);
			while(TRUE) {
				if(!(joystate & btn[cfg_btn_jump])) released = TRUE;
				if(joy_pressed(BUTTON_BTN)) {
					// Just in case player never releases confirm before hitting something else
					if(released) break;
					if(!(joystate & btn[cfg_btn_jump])) break;
				}
				// Animate quote sprite
				//if(--sprTime == 0) {
				//	sprTime = ANIM_SPEED;
				//	if(++sprFrame >= ANIM_FRAMES) sprFrame = 0;
				//	sprite_index((*sprCursor), TILE_SHEETINDEX+32+sprFrame*4);
				//}
				sprite_index((*sprCursor), TILE_SHEETINDEX+32+16);
			vdp_sprite_add(sprCursor);
				ready = TRUE;
				vdp_vsync(); aftervsync();
			}
			sound_play(SND_MENU_SELECT, 5);
			switch(joystate & BUTTON_BTN) {
				case BUTTON_A: *item->valptr = 6; break;
				case BUTTON_B: *item->valptr = 4; break;
				case BUTTON_C: *item->valptr = 5; break;
				case BUTTON_X: *item->valptr = 10; break;
				case BUTTON_Y: *item->valptr = 9; break;
				case BUTTON_Z: *item->valptr = 8; break;
				case BUTTON_START: *item->valptr = 7; break;
				case BUTTON_MODE: *item->valptr = 11; break;
			}
		}
		break;
		case MI_ACTION: {
			sound_play(SND_MENU_SELECT, 5);
			action[(uint32_t)item->valptr](page);
		}
		break;
		case MI_MODE: {
			sound_play(SND_MENU_SELECT, 5);
			(*item->valptr) += 1;
			if((*item->valptr) > 2) (*item->valptr) = 0;
		}
	}
	draw_menuitem(item);
}

uint8_t set_page(uint8_t page) {
	uint8_t numItems = 0;
	
	vdp_set_display(FALSE);
	vdp_map_clear(VDP_PLAN_A);
	
	switch(page) {
		case PAGE_CONTROL:
		if(cfg_language) {
			DrawJStr(8, 1, WKI, 21);
		} else {
			vdp_puts(VDP_PLAN_A, "(1) Controller Config", 8, 1);
		}
		break;
		case PAGE_GAMEPLAY:
		if(cfg_language) {
			DrawJStr(8, 1, WKI, 22);
		} else {
			vdp_puts(VDP_PLAN_A, "(2) Gameplay Config", 8, 1);
		}
		break;
		case PAGE_SAVEDATA:
		if(cfg_language) {
			DrawJStr(8, 1, WKI, 23);
		} else {
			vdp_puts(VDP_PLAN_A, "(3) Save Data", 8, 1);
		}
		break;
	}
	for(uint8_t i = 0; i < MAX_OPTIONS; i++) {
		if(menu[page][i].caption[0]) {
			draw_menuitem(&menu[page][i]);
			numItems++;
		}
	}

	vdp_set_display(TRUE);
	
	return numItems;
}

extern uint8_t tpal;

void config_main() {
	gamemode = GM_CONFIG;
	
	vdp_map_clear(VDP_PLAN_B);
	
	uint8_t sprFrame = 0;
	uint8_t sprTime = ANIM_SPEED;
	uint8_t page = PAGE_CONTROL;
	//uint8_t waitButton = FALSE;
	uint8_t cursor = 0;
	uint8_t numItems = set_page(page);
	
	VDPSprite sprCursor = { 
		.attr = TILE_ATTR(tpal,0,0,1,TILE_SHEETINDEX+32),
		.size = SPRITE_SIZE(2,2)
	};
	
	set_page(page);
	oldstate = ~0;
	while(TRUE) {
		//if(waitButton) {
			// Show looking up frame
		//	sprite_index(sprCursor, TILE_SHEETINDEX+32+16);
		//} else {
		if(joy_pressed(BUTTON_UP)) {
			do {
				if(cursor == 0) cursor = numItems - 1;
				else cursor--;
			} while(menu[page][cursor].type == MI_LABEL);
			sound_play(SND_MENU_MOVE, 0);
		} else if(joy_pressed(BUTTON_DOWN)) {
			do {
				if(cursor == numItems - 1) cursor = 0;
				else cursor++;
			} while(menu[page][cursor].type == MI_LABEL);
			sound_play(SND_MENU_MOVE, 0);
		} else if(joy_pressed(BUTTON_LEFT)) {
			if(--page >= NUM_PAGES) page = NUM_PAGES - 1;
			cursor = 0;
			set_page(page);
			sound_play(SND_MENU_MOVE, 0);
		} else if(joy_pressed(BUTTON_RIGHT)) {
			if(++page >= NUM_PAGES) page = 0;
			cursor = 0;
			set_page(page);
			sound_play(SND_MENU_MOVE, 0);
		} else if(joy_pressed(btn[cfg_btn_jump])) {
			if(menu[page][cursor].type == MI_RETURN) break;
			press_menuitem(&menu[page][cursor], page, &sprCursor);
		} else if(joy_pressed(btn[cfg_btn_shoot])) {
			break;
		}
		// Animate quote sprite
		if(--sprTime == 0) {
			sprTime = ANIM_SPEED;
			if(++sprFrame >= ANIM_FRAMES) sprFrame = 0;
			sprite_index(sprCursor, TILE_SHEETINDEX+32+sprFrame*4);
		}
		//}
		
		// Draw quote sprite at cursor position
		sprite_pos(sprCursor, 16, (menu[page][cursor].y << 3) - 4);
		vdp_sprite_add(&sprCursor);
		
		ready = TRUE;
		vdp_vsync(); aftervsync();
	}
	
	system_load_config();
}

void act_default(uint8_t page) {
	if(page == 0) {
		cfg_btn_jump = 5;
		cfg_btn_shoot = 4;
		cfg_btn_ffwd = 6;
		cfg_btn_rswap = 8;
		cfg_btn_lswap = 9;
		cfg_btn_map = 10;
		cfg_btn_pause = 7;
		cfg_force_btn = 0;
	} else if(page == 1) {
		cfg_language = 0;
		cfg_ffwd = TRUE;
		cfg_updoor = FALSE;
		cfg_hellquake = TRUE;
		cfg_iframebug = TRUE;
		cfg_msg_blip = TRUE;
		cfg_music_mute = TRUE;
		cfg_sfx_mute = TRUE;
	}
	set_page(page);
}

void act_apply(uint8_t page) {
	(void)(page);
	system_save_config();
}

void act_title(uint8_t page) {
	(void)(page);
	SYS_hardReset();
}

void act_resetcounter(uint8_t page) {
	(void)(page);
	system_save_counter(0xFFFFFFFF);
}

void act_format(uint8_t page) {
	(void)(page);
	uint8_t starts = 0;
	uint16_t timer = 0;
	vdp_puts(VDP_PLAN_A, "Are you sure?", 13, 12);
	vdp_puts(VDP_PLAN_A, "Press Start three times", 8, 14);
	song_stop();
	while(!joy_pressed(btn[cfg_btn_shoot])) {
		if(joy_pressed(BUTTON_START) && ++starts >= 3) {
			vdp_init();
			system_format_sram();
			sound_play(0xE5-0x80, 15);
			uint16_t timer = TIME_8(150);
			while(--timer) {
				vdp_vsync(); aftervsync();
			}
			SYS_hardReset();
		} else {
			timer++;
			switch(starts) {
				case 0: if((timer & 3) == 0) sound_play(0xBB-0x80, 5); break;
				case 1: if((timer & 3) == 0) sound_play(0xBC-0x80, 5); break;
				case 2: if((timer & 3) == 0) sound_play(0xBD-0x80, 5); break;
			}
		}
		vdp_vsync(); aftervsync();
	}
	vdp_text_clear(VDP_PLAN_A, 13, 12, 13);
	vdp_text_clear(VDP_PLAN_A, 8, 14, 23);
	song_resume();
}
