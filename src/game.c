#include "game.h"

#include <genesis.h>
#include "resources.h"
#include "input.h"
#include "system.h"
#include "player.h"
#include "stage.h"
#include "camera.h"
#include "tables.h"
#include "tsc.h"
#include "vdp_ext.h"
#include "effect.h"
#include "sprite.h"
#include "hud.h"

u32 playerProf, entityProf;

void debug_update() {
	// Change Map
	if(joy_pressed(BUTTON_Y)) {
		u8 newstage = (stageID + 1) % STAGE_COUNT;
		if(newstage == 0) newstage = 1; // Softlocking is bad
		stage_load(newstage);
		player.x = block_to_sub(11);
		player.y = block_to_sub(7);
	} else if(joy_pressed(BUTTON_X)) {
		u8 newstage = (stageID - 1) % STAGE_COUNT;
		if(newstage == 0) newstage = STAGE_COUNT - 1;
		stage_load(newstage);
		player.x = block_to_sub(11);
		player.y = block_to_sub(7);
	}
}

void draw_pause() {
	if(debuggingEnabled) {
		char numstr[4][8];
		char fullstr[4][20] = { "Entity: ", "Events: ", "Active: ", "FreeMem:" };
		uintToStr(stageEntityCount, numstr[0], 3);
		uintToStr(tscEventCount, numstr[1], 3);
		uintToStr(entities_count(), numstr[2], 3);
		uintToStr(MEM_getFree(), numstr[3], 6);
		//uintToStr(sizeof(void*), numstr[3], 6);
		for(u8 i = 0; i < 4; i++) strcat(fullstr[i], numstr[i]);
		VDP_setWindowPos(0, 243); // Show window plane
		VDP_drawTextWindow(fullstr[0], 2, 25);
		VDP_drawTextWindow(fullstr[1], 2, 26);
		VDP_drawTextWindow(fullstr[2], 22, 25);
		VDP_drawTextWindow(fullstr[3], 22, 26);
	}
}

void erase_pause() {
	if(debuggingEnabled) {
		VDP_setWindowPos(0, 251); // Hide window plane
		tsc_unpause_debug(); // Brings message window back if open
	}
}

bool update_pause() {
	if(joy_pressed(BUTTON_START)) {
		erase_pause();
		return false;
	}
	return true;
}

void game_reset(bool load) {
	effects_clear();
	sprites_init();
	camera_init();
	tsc_init();
	if(load) {
		system_load();
	} else {
		system_new();
		tsc_call_event(GAME_START_EVENT);
	}
	// Load up the main palette
	VDP_setPalette(PAL0, PAL_Main.data);
	VDP_setPalette(PAL1, PAL_Sym.data);
	VDP_setPalette(PAL3, PAL_Regu.data);
}

void vblank() {
	stage_update();
	hud_update_vblank();
	u8 str[10];
	uintToStr(playerProf, str, 8);
	VDP_drawTextWindow(str, 1, 27);
	uintToStr(entityProf, str, 8);
	VDP_drawTextWindow(str, 10, 27);
}

void game_main(bool load) {
	VDP_loadTileSet(&TS_MsgFont, TILE_FONTINDEX, true);
	SYS_setVIntCallback(vblank);
	VDP_setScrollingMode(HSCROLL_TILE, VSCROLL_PLANE);
	game_reset(load);
	VDP_setWindowPos(0, 251);
	bool paused = false, can_pause = true;
	while(true) {
		input_update();
		if(paused) {
			paused = update_pause();
		} else {
			if(can_pause && joy_pressed(BUTTON_START)) {
				draw_pause();
				paused = true;
			} else {
				if(debuggingEnabled) debug_update();
				camera_update();
				playerProf = getSubTick();
				player_update();
				playerProf = getSubTick() - playerProf;
				hud_update();
				entityProf = getSubTick();
				entities_update();
				entityProf = getSubTick() - entityProf;
				u8 rtn = tsc_update();
				if(rtn > 0) {
					if(rtn == 1) {
						SYS_reset();
					} else if(rtn == 2) {
						game_reset(true);
						continue;
					} else if(rtn == 3) {
						game_reset(false);
						continue;
					}
				}
				effects_update();
				sprites_update();
				system_update();
			}
		}
		VDP_waitVSync();
	}
	SYS_setVIntCallback(NULL);
}
