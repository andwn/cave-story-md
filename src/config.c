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
#include "kanji.h"
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
#include "z80_ctrl.h"

#include "gamemode.h"

#define ANIM_SPEED	7
#define ANIM_FRAMES	4

#define VAL_DEFAULT 0xFF

enum { PAGE_CONTROL, PAGE_GAMEPLAY, PAGE_SAVEDATA };

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
	uint8_t type;
	char caption[36];
	uint8_t *valptr;
} MenuItem;

const MenuItem menu[3][12] = {
	{
		{ 4,  MI_INPUT, "Jump / Confirm", &cfg_btn_jump },
		{ 6,  MI_INPUT, "Shoot / Cancel", &cfg_btn_shoot },
		{ 8,  MI_INPUT, "Switch (3btn) / FFwd", &cfg_btn_ffwd },
		{ 10, MI_INPUT, "Switch Right (6btn)", &cfg_btn_rswap },
		{ 12, MI_INPUT, "Switch Left (6btn)", &cfg_btn_lswap },
		{ 14, MI_INPUT, "Open Map (6btn)", &cfg_btn_map },
		{ 16, MI_INPUT, "Pause Menu", &cfg_btn_pause },
		
		{ 19, MI_MODE, "Force Button Mode", &cfg_force_btn },

		{ 22, MI_ACTION, "Apply", (uint8_t*)1 },
		{ 24, MI_ACTION, "Reset to Default", (uint8_t*)0 },
	},{
		{ 4,  MI_LANG,   "Language", &cfg_language },
		
		{ 8,  MI_TOGGLE, "Enable Fast Forward", &cfg_ffwd },
		{ 10, MI_TOGGLE, "Use Up to Interact", &cfg_updoor },
		{ 12, MI_TOGGLE, "Screen Shake in Hell", &cfg_hellquake },
		{ 14, MI_TOGGLE, "Vulnerable After Pause", &cfg_iframebug },
		{ 16, MI_TOGGLE, "Message Blip Sound", &cfg_msg_blip },

		{ 22, MI_ACTION, "Apply", (uint8_t*)1 },
		{ 24, MI_ACTION, "Reset to Default", (uint8_t*)0 },
	},{
		{ 4,  MI_ACTION, "Erase Counter", (uint8_t*)3 },
		{ 6,  MI_ACTION, "Erase All Save Data (!)", (uint8_t*)4 },
	},
};

const char boolstr[2][4] = { "OFF", "ON " };
const char modestr[3][6] = { "OFF ", "3BTN", "6BTN" };

void draw_menuitem(const MenuItem *item) {
	VDP_clearText(2, item->y, 36);
	VDP_drawText(item->caption, 4, item->y);
	switch(item->type) {
		case MI_LABEL: break; // Nothing
		case MI_INPUT:
		VDP_drawText(btnName[*item->valptr], 30, item->y);
		break;
		case MI_LANG:
		if(*item->valptr) {
			kanji_draw(PLAN_A, TILE_BACKINDEX,   0x100+794, 30, item->y, 0); // 日
			kanji_draw(PLAN_A, TILE_BACKINDEX+4, 0x100+909, 32, item->y, 0); // 本
			kanji_draw(PLAN_A, TILE_BACKINDEX+8, 0x100+417, 34, item->y, 0); // 語
		} else {
			VDP_drawText("English", 30, item->y);
			VDP_clearText(30, item->y + 1, 6); // Hide kanji
		}
		break;
		case MI_TOGGLE:
		VDP_drawText(boolstr[*item->valptr], 30, item->y);
		break;
		case MI_ACTION: break;
		case MI_MODE:
		VDP_drawText(modestr[*item->valptr], 30, item->y);
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
			VDP_drawText("Press..", 30, item->y);
			while(TRUE) {
				if(!(joystate & btn[cfg_btn_jump])) released = TRUE;
				input_update();
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
				sprite_add((*sprCursor));
				ready = TRUE;
				vsync(); aftervsync();
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
	uint8_t maxCursor = 0;
	
	VDP_setEnable(FALSE);
	VDP_clearPlan(PLAN_A, TRUE);
	
	switch(page) {
		case PAGE_CONTROL:
		maxCursor = 10;
		VDP_drawText("(1) Controller Config", 8, 2);
		for(uint8_t i = 0; i < maxCursor; i++) {
			draw_menuitem(&menu[page][i]);
		}
		break;
		case PAGE_GAMEPLAY:
		maxCursor = 9;
		VDP_drawText("(2) Gameplay Config", 8, 2);
		for(uint8_t i = 0; i < maxCursor; i++) {
			draw_menuitem(&menu[page][i]);
		}
		break;
		case PAGE_SAVEDATA:
		maxCursor = 3;
		VDP_drawText("(3) Save Data", 8, 2);
		for(uint8_t i = 0; i < maxCursor; i++) {
			draw_menuitem(&menu[page][i]);
		}
		break;
	}
	
	VDP_setEnable(TRUE);
	
	return maxCursor;
}

extern uint8_t tpal;

void config_main() {
	gamemode = GM_CONFIG;
	
	VDP_clearPlan(PLAN_B, TRUE);
	
	uint8_t sprFrame = 0;
	uint8_t sprTime = ANIM_SPEED;
	uint8_t page = PAGE_CONTROL;
	//uint8_t waitButton = FALSE;
	uint8_t cursor = 0;
	uint8_t maxCursor = set_page(page);
	
	VDPSprite sprCursor = { 
		.attribut = TILE_ATTR_FULL(tpal,0,0,1,TILE_SHEETINDEX+32),
		.size = SPRITE_SIZE(2,2)
	};
	
	set_page(page);
	oldstate = ~0;
	while(TRUE) {
		input_update();
		//if(waitButton) {
			// Show looking up frame
		//	sprite_index(sprCursor, TILE_SHEETINDEX+32+16);
		//} else {
		if(joy_pressed(BUTTON_UP)) {
			do {
				if(cursor == 0) cursor = maxCursor - 1;
				else cursor--;
			} while(menu[page][cursor].type == MI_LABEL);
			sound_play(SND_MENU_MOVE, 0);
		} else if(joy_pressed(BUTTON_DOWN)) {
			do {
				if(cursor == maxCursor - 1) cursor = 0;
				else cursor++;
			} while(menu[page][cursor].type == MI_LABEL);
			sound_play(SND_MENU_MOVE, 0);
		} else if(joy_pressed(BUTTON_LEFT)) {
			if(page == 0) page = 2;
			else page--;
			cursor = 0;
			set_page(page);
			sound_play(SND_MENU_MOVE, 0);
		} else if(joy_pressed(BUTTON_RIGHT)) {
			if(page == 2) page = 0;
			else page++;
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
		sprite_add(sprCursor);
		
		ready = TRUE;
		vsync(); aftervsync();
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
	VDP_drawText("Are you sure?", 13, 12);
	VDP_drawText("Press start three times", 8, 14);
	song_stop();
	while(!joy_pressed(btn[cfg_btn_shoot])) {
		input_update();
		if(joy_pressed(BUTTON_START) && ++starts >= 3) {
			VDP_init();
			system_format_sram();
			sound_play(0xE5-0x80, 15);
			uint16_t timer = TIME_8(150);
			while(--timer) {
				vsync(); aftervsync();
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
		vsync(); aftervsync();
	}
	VDP_clearText(13, 12, 13);
	VDP_clearText(8, 14, 23);
	song_resume();
}
