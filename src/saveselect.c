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
#define OPTIONS 4

uint8_t saveselect_main() {
	gamemode = GM_SAVESEL;
	
	uint8_t cursor = 0;
	uint8_t sprFrame = 0, sprTime = ANIM_SPEED;
	SaveEntry file[OPTIONS] = {};
	
	VDP_setEnable(FALSE);
	// Keep stuff from the title screen
	VDP_clearPlan(PLAN_A, TRUE);
	VDP_clearPlan(PLAN_B, TRUE);
	sprites_clear();
	VDPSprite sprCursor = { 
		.attribut = TILE_ATTR_FULL(PAL1,0,0,1,TILE_SHEETINDEX),
		.size = SPRITE_SIZE(2,2)
	};
	
	for(uint16_t i = 0; i < OPTIONS; i++) {
		uint16_t y = 4 + i * 5;
		system_peekdata(i, &file[i]);
		if(file[i].used) {
			VDP_drawText(stage_info[file[i].stage_id].name, 6, y);
		} else {
			VDP_drawText("New Game", 6, y);
		}
	}
	
	VDP_setEnable(TRUE);
	
	oldstate = ~0;
	while(!joy_pressed(btn[cfg_btn_jump]) && !joy_pressed(btn[cfg_btn_pause])) {
		input_update();
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
		// Draw quote sprite at cursor position
		sprite_pos(sprCursor, 4*8, 6*8 + cursor*5*8);
		sprite_add(sprCursor);
		
		ready = TRUE;
		vsync(); aftervsync();
	}
	song_stop();
	sound_play(SND_MENU_SELECT, 0);
	sram_file = cursor;
	return file[cursor].used ? 1 : 0;
}
