/*
 * An attempt to rewrite/port Cave Story's engine to an old console
 * Copyright (C) 2016 andwn
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
#include "common.h"
#include "resources.h"
#include "gamemode.h"
#include "input.h"
#include "system.h"

int main() {
	//system_verifychecksum();
    //VDP_init();
    SYS_disableInts();
    VDP_setPlanSize(64, 32);
    // Sprite list overlaps the bottom of the window, so move it
	VDP_setHScrollTableAddress(0xF800); // Default: 0xB800
	VDP_setSpriteListAddress(0xFC00); // Default: 0xBC00
    sound_init();
	input_init();
	SPR_init(80, 512, 512);
	SYS_enableInts();
    while(true) {
		u8 select = titlescreen_main();
		switch(select) {
			case 0:
			case 1:
				select = game_main(select);
				if(select > 0) credits_main(select);
				break;
			case 2: soundtest_main(); break;
			case 3: npctest_main(); break;
		}
    }
	return 0;
}
