#include "gamemode.h"

#include <genesis.h>
#include "resources.h"
#include "input.h"
#include "tables.h"
#include "audio.h"

void credits_main(u8 ending) {
	song_stop();
	SYS_disableInts();
	VDP_setEnable(FALSE);
	VDP_resetScreen();
	VDP_setPalette(PAL0, PAL_Main.data);
	//VDP_setPalette(PAL1, PAL_Credits.data);
	VDP_drawText("You won!!!", 4, 8);
	VDP_setEnable(TRUE);
	SYS_enableInts();
	song_play(0x21);
    while(TRUE) {
		input_update();
		if(joy_pressed(BUTTON_START) || joy_pressed(BUTTON_C)) {
			break;
		}
		VDP_waitVSync();
    }
    SYS_hardReset();
}
