/*
 * An attempt to rewrite/port Cave Story's engine to an old console
 * Copyright (C) 2015 aderosier
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
 *
 * The above applies to the code only and not any of the art assets.
 * See credits.txt for detailed information
 */

#include <genesis.h>

#include "audio.h"
#include "sprite.h"
#include "common.h"
#include "resources.h"
#include "titlescreen.h"
#include "soundtest.h"
#include "game.h"
#include "input.h"

int main() {
    VDP_init();
    VDP_setPlanSize(64, 32);
    // Sprite list overlaps the bottom of the window, so move it up
	VDP_setSpriteListAddress(0xB600); // Default: 0xBC00
	VDP_setHScrollTableAddress(0xF800);
	sprites_init();
    sound_init();
	input_init();
    while(true) {
		u8 select = titlescreen_main();
		switch(select) {
			case 0: game_main(false); break;
			case 1: game_main(true); break;
			case 2: soundtest_main(); break;
		}
    }
	return 0;
}
