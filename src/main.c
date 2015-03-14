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
