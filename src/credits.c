#include "common.h"

#include "ai.h"
#include "audio.h"
#include "dma.h"
#include "input.h"
#include "joy.h"
#include "resources.h"
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

enum CreditCmd { 
	TEXT, ICON, WAIT, MOVE, SONG, SONG_FADE, FLAG_JUMP, JUMP, LABEL, PALETTE, END 
};

void credits_main() {
	gamemode = GM_CREDITS;
	
	uint16_t pc = 0;
	uint16_t textX = 0, textY = 0;
	
	uint16_t waitTime = 0;
	uint16_t backScroll = 0;
	//uint16_t illScroll = 0;
	
#ifndef PAL
	uint8_t skipScroll = FALSE;
#endif
	
	//song_stop();
	sprites_clear();
	vsync(); aftervsync(); // Make sure nothing in DMA queue and music completely stops
	
	VDP_setEnable(FALSE);
	// Clear planes, reset palettes
	VDP_clearPlan(PLAN_A, TRUE);
	VDP_clearPlan(PLAN_B, TRUE);
	VDP_setPalette(PAL0, PAL_Main.data);
	VDP_setPalette(PAL1, PAL_Sym.data);
	VDP_setPalette(PAL3, PAL_Regu.data);
	// Reset background and scrolling
	VDP_setPaletteColor(0, 0x200); // Dark blue background
	VDP_setBackgroundColor(0);
	stageBackground = stageBackgroundType = 0;
	VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
	VDP_setHorizontalScroll(PLAN_B, 0);
	VDP_setVerticalScroll(PLAN_B, 0);
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
		if(waitTime) {
			waitTime--;
		} else while(!waitTime) {
			switch(credits_info[pc].cmd) {
				case TEXT:
					VDP_drawTextBG(PLAN_B, credits_info[pc].text.string, textX, textY & 31);
					break;
				case ICON: /* TODO */ break;
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
					if(!system_get_flag(credits_info[pc].fjump.flag))
						break;
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
		backScroll++;
#ifndef PAL
		// Slow the scrolling down slightly for NTSC
		if((backScroll % 6) == 0 && !skipScroll) {
			backScroll--;
			skipScroll = TRUE;
		} else {
			skipScroll = FALSE;
		}
#endif
		if((backScroll & 15) == 0) {
			textY++;
			VDP_clearTextBG(PLAN_B, 0, textY & 31, 40);
		}
		VDP_setVerticalScroll(PLAN_B, backScroll >> 1);
		vsync(); aftervsync();
    }
}
