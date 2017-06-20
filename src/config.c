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

// Settings like:
//
// Page 1 - Controls:
// Jump / Confirm - C
// Shoot / Cancel - B
// Switch (3btn) / FFwd - A
// Switch Right (6btn) - Z
// Switch Left (6btn) - Y
// Open Map (6btn) - X
// Open Menu - St
// 
// Page 2 - Gameplay pref:
// Fasr Fwd with A - ON
// Use Up to Interact - OFF
// Screen Shake in Hell - ON
// Bug pref:
// IFrame 0 on Pause - ON
// Sisters Skip - ON
// 
// Page 3 - Save Data:
// Clear Game Save
// Clear Nikumaru Counter
// Clear Everything
// 
// Bottom of each page except data:
// Apply
// Reset Default
// Return to Title

#define ANIM_SPEED	7
#define ANIM_FRAMES	4

enum { PAGE_CONTROL, PAGE_GAMEPLAY, PAGE_SAVEDATA };

enum { MI_LABEL, MI_INPUT, MI_TOGGLE, MI_ACTION };

typedef struct {
	uint8_t y;
	uint8_t type;
	char caption[37];
	uint8_t defaultVal;
} MenuItem;

const MenuItem menu[3][16] = {
	{
		{ 4,  MI_INPUT, "Jump / Confirm", 5 },
		{ 6,  MI_INPUT, "Shoot / Cancel", 4 },
		{ 8,  MI_INPUT, "Switch (3btn) / FFwd", 6 },
		{ 10, MI_INPUT, "Switch Right (6btn)", 8 },
		{ 12, MI_INPUT, "Switch Left (6btn)", 9 },
		{ 14, MI_INPUT, "Open Map (6btn)", 10 },
		{ 16, MI_INPUT, "Pause Menu", 7 },
	},
	{
		{ 4,  MI_TOGGLE, "Fast Fwd with A", TRUE },
		{ 6,  MI_TOGGLE, "Use Up to Interact", FALSE },
		{ 8,  MI_TOGGLE, "Screen Shake in Hell", TRUE },
		
		{ 11, MI_LABEL, "Reset Invincibility Frames in the", 0 },
		{ 12, MI_TOGGLE, "Pause Menu", TRUE },
	},
	{
		
	},
};

uint8_t set_page(uint8_t page) {
	uint8_t maxCursor = 0;
	
	VDP_setEnable(FALSE);
	VDP_clearPlan(PLAN_A, TRUE);
	
	switch(page) {
		case PAGE_CONTROL:
		maxCursor = 8;
		VDP_drawText("(1) Controller Config", 8, 2);
		VDP_drawText("WIP - Come back later", 4, 8);
		VDP_drawText("Press B", 12, 10);
		break;
		case PAGE_GAMEPLAY:
		maxCursor = 4;
		VDP_drawText("(2) Gameplay Config", 8, 2);
		
		break;
		case PAGE_SAVEDATA:
		maxCursor = 3;
		VDP_drawText("(3) Save Data", 8, 2);
		
		break;
	}
	
	// Apply / Reset / Return
	if(page != PAGE_SAVEDATA) {
		maxCursor += 3;
		
	}
	
	VDP_setEnable(TRUE);
	
	return maxCursor;
}

void config_main() {
	gamemode = GM_CONFIG;
	
	VDP_clearPlan(PLAN_B, TRUE);
	
	uint8_t sprFrame = 0;
	uint8_t sprTime = ANIM_SPEED;
	uint8_t page = PAGE_CONTROL;
	uint8_t waitButton = FALSE;
	uint8_t cursor = 0;
	uint8_t maxCursor = set_page(page);
	
	VDPSprite sprCursor = { 
		.attribut = TILE_ATTR_FULL(PAL1,0,0,1,TILE_SHEETINDEX),
		.size = SPRITE_SIZE(2,2)
	};
	
	while(!joy_pressed(BUTTON_B)) {
		input_update();
		if(waitButton) {
			// Show looking up frame
			sprite_index(sprCursor, TILE_SHEETINDEX+16);
		} else {
			if(joy_pressed(BUTTON_UP)) {
				if(cursor == 0) cursor = maxCursor - 1;
				else cursor--;
				sound_play(SND_MENU_MOVE, 0);
			} else if(joy_pressed(BUTTON_DOWN)) {
				if(cursor == maxCursor - 1) cursor = 0;
				else cursor++;
				sound_play(SND_MENU_MOVE, 0);
			}
			// Animate quote sprite
			if(--sprTime == 0) {
				sprTime = ANIM_SPEED;
				if(++sprFrame >= ANIM_FRAMES) sprFrame = 0;
				sprite_index(sprCursor, TILE_SHEETINDEX+sprFrame*4);
			}
		}
		
		// Draw quote sprite at cursor position
		sprite_pos(sprCursor, 16, (16+cursor*16)-4);
		sprite_add(sprCursor);
		
		ready = TRUE;
		vsync(); aftervsync();
	}
	
	SYS_hardReset(); // eh
}

