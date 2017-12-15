#include "common.h"

#include "ai.h"
#include "audio.h"
#include "camera.h"
#include "dma.h"
#include "entity.h"
#include "input.h"
#include "joy.h"
#include "resources.h"
#include "sheet.h"
#include "sprite.h"
#include "stage.h"
#include "system.h"
#include "tables.h"
#include "tsc.h"
#include "vdp.h"
#include "vdp_bg.h"
#include "vdp_pal.h"
#include "vdp_tile.h"

#include "gamemode.h"

#define TILE_ICONINDEX	(tiloc_index + 224)

enum CreditCmd { 
	TEXT, ICON, WAIT, MOVE, SONG, SONG_FADE, FLAG_JUMP, JUMP, LABEL, PALETTE, END 
};

static int8_t illScrolling = 0;

void credits_main() {
	gamemode = GM_CREDITS;
	
	VDPSprite icon[16] = {};
	
	uint16_t pc = 0;
	uint16_t textX = 0, textY = 0;
	
	uint16_t waitTime = 0;
	uint16_t backScroll = 0;
	uint16_t illScroll = 0;
	
#ifndef PAL
	uint8_t skipScroll = FALSE;
#endif
	
	inFade = FALSE;
	ready = TRUE;
	sprites_clear();
	vsync(); aftervsync(); // Make sure nothing in DMA queue and music completely stops
	
	VDP_setEnable(FALSE);
	// Clear planes, reset palettes
	VDP_clearPlan(PLAN_A, TRUE);
	VDP_clearPlan(PLAN_B, TRUE);
	VDP_setPalette(PAL0, PAL_Main.data);
	VDP_setPalette(PAL1, PAL_Sym.data);
	VDP_setPalette(PAL3, PAL_Regu.data);
	// Stick camera to upper right
	camera.target = NULL;
	camera.x = SCREEN_HALF_W << CSF;
	camera.y = SCREEN_HALF_H << CSF;
	camera.x_offset = camera.y_offset = 0;
	camera.x_shifted = camera.y_shifted = 0;
	camera_xmin = camera_ymin = 0;
	// Reset background and scrolling
	VDP_setPaletteColor(0, 0x200); // Dark blue background
	VDP_setBackgroundColor(0);
	stageBackground = stageBackgroundType = 0;
	VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
	VDP_setHorizontalScroll(PLAN_B, 0);
	VDP_setVerticalScroll(PLAN_B, 0);
	VDP_setHorizontalScroll(PLAN_A, 0);
	VDP_setVerticalScroll(PLAN_A, 0);
	// Text on background plane, priority 1
	VDP_loadFont(&TS_SysFont, TRUE);
	VDP_setTextPlan(PLAN_B);
	VDP_setTextPriority(1);
	// Disable joypads, load credits TSC
	joystate = oldstate = 0;
	tsc_load_stage(ID_CREDITS); // credits.tsb
	tsc_call_event(100);
	
	textY = (SCREEN_HEIGHT >> 3) + (backScroll >> 3) + 1;
	
	VDP_setEnable(TRUE);
	
    while(TRUE) {
		tsc_update();
		if(waitTime) waitTime--;
		while(!waitTime) {
			switch(credits_info[pc].cmd) {
				case TEXT:
					VDP_drawTextBG(PLAN_B, credits_info[pc].text.string, textX, textY & 31);
					break;
				case ICON:
					for(uint8_t i = 0; i < 16; i++) {
						if(icon[i].size) continue;
						icon[i] = (VDPSprite) {
							.x = textX * 8 - 22 + 128,
							.y = SCREEN_HEIGHT - 6 + 128,
							.size = SPRITE_SIZE(3, 3),
							.attribut = TILE_ATTR_FULL(credits_info[pc].icon.pal,
									1,0,0,TILE_ICONINDEX + i * 9)
						};
						TILES_QUEUE(SPR_TILES(&SPR_Casts, 
								credits_info[pc].icon.id, 0), TILE_ICONINDEX + i * 9, 9);
						break;
					}
					break;
				case WAIT:
					waitTime = credits_info[pc].wait.ticks;
					break;
				case MOVE:
					textX = credits_info[pc].move.pos >> 3;
					break;
				case SONG:
					song_play(credits_info[pc].song.id);
					break;
				case SONG_FADE:
					song_stop(); // TODO: Figure out how to fade
					break;
				case FLAG_JUMP:
					if(!system_get_flag(credits_info[pc].fjump.flag)) break;
					/* fallthrough */
				case JUMP:
				{
					uint16_t label = credits_info[pc].jump.label;
					do {
						pc++;
					} while(credits_info[pc].cmd != LABEL || credits_info[pc].label.value != label);
				}
				break;
				case LABEL: /* Ignore */ break;
				case PALETTE: /* Unused */ break;
				// The End
				case END: while(TRUE) { vsync(); aftervsync(); } break;
			}
			pc++;
		}
		
		entities_update();
		entities_draw();
		
		backScroll++;
#ifndef PAL
		// Slow the scrolling down slightly for NTSC
		if((backScroll % 6) == 0 && !skipScroll) {
			backScroll--;
			waitTime++;
			skipScroll = TRUE;
		} else {
			skipScroll = FALSE;
		}
#endif
		if((backScroll & 15) == 0) {
			textY++;
			VDP_clearTextBG(PLAN_B, 0, textY & 31, 40);
		}
		// Scrolling for illustrations
		illScroll += illScrolling;
		if(illScroll <= 0 || illScroll >= 160) illScrolling = 0;
		// Icon sprites
		for(uint8_t i = 0; i < 16; i++) {
			if(!icon[i].size) continue;
			if((backScroll & 1) == 0) {
				if(--icon[i].y < -22 + 128) icon[i].size = 0;
			}
			sprite_add(icon[i]);
		}
		
		VDP_setVerticalScroll(PLAN_B, backScroll >> 1);
		VDP_setHorizontalScroll(PLAN_A, illScroll);
		ready = TRUE;
		vsync(); aftervsync();
    }
}

void credits_show_image(uint16_t id) {
	if(id > 19) return;
	if(illustration_info[id].tileset == NULL) return; // Can't draw null tileset
	VDP_setEnable(FALSE);
	VDP_setPalette(PAL2, illustration_info[id].palette->data);
	VDP_loadTileSet(illustration_info[id].tileset, TILE_TSINDEX, TRUE);
#ifdef PAL
	VDP_fillTileMapRectInc(PLAN_A, 
			TILE_ATTR_FULL(PAL2,0,0,0,TILE_TSINDEX), 44, 0, 20, 30);
#else
	VDP_fillTileMapRectInc(PLAN_A, 
			TILE_ATTR_FULL(PAL2,0,0,0,TILE_TSINDEX+20), 44, 0, 20, 28);
#endif
	VDP_setEnable(TRUE);
	illScrolling = 8;
}

void credits_clear_image() {
	illScrolling = -8;
}
