/*
 * A "demake" of Cave Story for the Sega Mega Drive
 * Copyright (C) 2017 Andy Grind
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "common.h"

#include "audio.h"
#include "dma.h"
#include "effect.h"
#include "gamemode.h"
#include "input.h"
#include "joy.h"
#include "memory.h"
#include "resources.h"
#include "sprite.h"
#include "stage.h"
#include "system.h"
#include "tools.h"
#include "tsc.h"
#include "vdp.h"
#include "vdp_pal.h"
#include "vdp_tile.h"
#include "xgm.h"
#include "z80_ctrl.h"

extern const uint16_t time_tab_ntsc[0x100];
extern const uint16_t speed_tab_ntsc[0x100];

void vsync() {
	vblank = 0;
	while(!vblank);
	vblank = 0;
}

void aftervsync() {
	XGM_doVBlankProcess();
	XGM_set68KBUSProtection(TRUE);
	waitSubTick(10);
	
	DMA_flushQueue();
	
	if(fading_cnt > 0) VDP_doStepFading(FALSE);
	
	dqueued = FALSE;
	if(ready) {
		if(inFade) { spr_num = 0; }
		else { sprites_send(); }
		ready = FALSE;
	}
	if(gamemode == GM_GAME) stage_update(); // Scrolling
	
	XGM_set68KBUSProtection(FALSE);
	
	JOY_update();
}

int main() {
	// initiate random number generator
    setRandomSeed(0xC427);
    // enable interrupts
    __asm__("move #0x2500,%sr");
    // init part
    MEM_init();
    VDP_init();
    DMA_init(0, 0);
    //PSG_init();
    JOY_init();
    // reseting z80 also reset the ym2612
    Z80_init();
    // Initialize time and speed tables (framerate adjusted)
    if(pal_mode) {
		for(uint16_t i = 0; i < 0x100; i++) {
			time_tab[i] = i;
			speed_tab[i] = i;
		}
	} else {
		for(uint16_t i = 0; i < 0x100; i++) {
			time_tab[i] = time_tab_ntsc[i];
			speed_tab[i] = speed_tab_ntsc[i];
		}
	}
    // let's the fun go on !
    
	puts("Hi June");
    sound_init();
	input_init();
	
	// Error Tests
	//__asm__("move.w (1),%d0"); // Address Error
	//__asm__("illegal"); // Illegal Instruction
	//__asm__("divu #0,%d0"); // Zero Divide

	splash_main();
	intro_main();
    while(TRUE) {
		uint8_t select = titlescreen_main();
		if(select == 0) {
			select = saveselect_main();
			if(select >= 4) continue;
			game_main(select);
			credits_main();
		} else if(select == 2) {
			soundtest_main();
		} else if(select == 3) {
			config_main();
		} else {
			game_main(select);
			credits_main();
		}
    }
	return 0;
}
