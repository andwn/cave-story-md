#include "common.h"

#include "audio.h"
#include "input.h"
#include "joy.h"
#include "resources.h"
#include "system.h"
#include "tables.h"
#include "vdp.h"
#include "vdp_bg.h"
#include "vdp_pal.h"

#include "gamemode.h"

void credits_main() {
	song_stop();
	
	VDP_resetScreen();
	VDP_setPalette(PAL0, PAL_Main.data);
	//VDP_setPalette(PAL1, PAL_Credits.data);
	VDP_drawText("You won!!!", 4, 8);
	
	song_play(0x21);
    while(TRUE) {
		input_update();
		if(joy_pressed(BUTTON_START) || joy_pressed(BUTTON_C)) {
			break;
		}
		vsync();
		aftervblank();
    }
    SYS_hardReset();
}
