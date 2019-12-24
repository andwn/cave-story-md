#include "common.h"

#include "audio.h"
#include "dma.h"
#include "effect.h"
#include "error.h"
#include "gamemode.h"
#include "joy.h"
#include "memory.h"
#include "resources.h"
#include "stage.h"
#include "system.h"
#include "tools.h"
#include "tsc.h"
#include "vdp.h"
#include "xgm.h"

extern const uint16_t time_tab_ntsc[0x100];
extern const uint16_t speed_tab_ntsc[0x100];

void aftervsync() {
	//xgm_vblank();
	DMA_flushQueue();
	vdp_fade_step();
	dqueued = FALSE;
	if(ready) {
		if(inFade) vdp_sprites_clear();
		vdp_sprites_update();
		ready = FALSE;
	}
	if(gamemode == GM_GAME) stage_update(); // Scrolling
	joy_update();
}

int main() {
    setRandomSeed(0xC427); // initiate random number generator
    mem_init();
    vdp_init();
    DMA_init(0, 0);
    xgm_init();
    __asm__("move #0x2500,%sr"); // enable interrupts
	joy_init();
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
    
    sound_init();
	
	// Error Tests
	//__asm__("move.w (1),%d0"); // Address Error
	//__asm__("illegal"); // Illegal Instruction
	//__asm__("divu #0,%d0"); // Zero Divide
	//error_oom(); // Out of memory
	//error_other("Test test!\nTest!"); // Fatal message

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
