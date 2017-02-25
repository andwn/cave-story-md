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

#include <genesis.h>

#include "audio.h"
#include "common.h"
#include "gamemode.h"
#include "input.h"
#include "resources.h"
#include "system.h"

int main() {
	puts("Hi Plum");
    SYS_disableInts();
    VDP_setScreenHeight240(); // Only has any effect on PAL
    VDP_setPlanSize(64, 32);
    // Sprite list overlaps the bottom of the window, so move it
	VDP_setHScrollTableAddress(0xF800); // Default: 0xB800
	VDP_setSpriteListAddress(0xFC00); // Default: 0xBC00
    sound_init();
	input_init();
	VDP_loadTileSet(&TS_SysFont, TILE_FONTINDEX, TRUE);
	SYS_enableInts();
    while(TRUE) {
		splash_main();
		u8 select = titlescreen_main();
		if(select != 2) {
			game_main(select);
			credits_main();
		} else {
			soundtest_main(); break;
		}
    }
	return 0;
}
