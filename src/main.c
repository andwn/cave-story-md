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

void vsync() {
	vblank = 0;
	while(!vblank);
}

void aftervsync() {
	XGM_doVBlankProcess();
	XGM_set68KBUSProtection(TRUE);
	waitSubTick(10);
	DMA_flushQueue();
	XGM_set68KBUSProtection(FALSE);
	if(fading_cnt > 0) VDP_doStepFading(FALSE);
	
	dqueued = FALSE;
	if(ready) {
		if(inFade) { spr_num = 0; }
		else { sprites_send(); }
		ready = FALSE;
	}
	if(gamemode == GM_GAME) stage_update(); // Scrolling
	
	JOY_update();
}

int main() {
	puts("Hi Plum");
    sound_init();
	input_init();
    while(TRUE) {
		splash_main();
		uint8_t select = titlescreen_main();
		if(select == 2) {
			soundtest_main(); break;
		} else if(select == 3) {
			config_main(); break;
		} else {
			game_main(select);
			credits_main();
		}
    }
	return 0;
}
