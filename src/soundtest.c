#include "common.h"

#include "audio.h"
#include "bank_data.h"
#include "dma.h"
#include "joy.h"
#include "resources.h"
#include "string.h"
#include "system.h"
#include "tables.h"
#include "vdp.h"
#include "xgm.h"

#include "gamemode.h"

#define DRAW_BYTE(b, x, y) { \
	char byte[4]; \
	sprintf(byte, "%02hX", b); \
	vdp_puts(VDP_PLAN_A, byte, x, y); \
}

enum { STOPPED, PLAYING, PAUSED };

// Music 0x00 - 0x7F, Sound Effects 0x80 - 0xFF
#define FIRST_SOUND 0x80

// Track # and name
void draw_track_info(uint8_t track) {
	DRAW_BYTE(track, 2, 6);
	vdp_text_clear(VDP_PLAN_A, 5, 6, 33);
	vdp_puts(VDP_PLAN_A, song_info[track].name, 5, 6);
}

void draw_status(uint8_t status) {
	switch(status) {
		case STOPPED: vdp_puts(VDP_PLAN_A, "Stopped", 2, 4); break;
		case PLAYING: vdp_puts(VDP_PLAN_A, "Playing", 2, 4); break;
		case PAUSED:  vdp_puts(VDP_PLAN_A, "Paused ", 2, 4); break;
	}
}

void soundtest_main() {
	gamemode = GM_SOUNDTEST;
	
	uint8_t track = 0;
	uint8_t status = STOPPED, oldstatus = STOPPED;

	disable_ints;
    z80_pause_fast();
	vdp_set_display(FALSE);
	
	vdp_sprites_clear();
	vdp_map_clear(VDP_PLAN_A);
	// Background picture
	vdp_tiles_load((uint32_t*) PAT_SndTest, 16, 208);
	uint16_t index = pal_mode ? 0 : 80 << 1;
	for(uint16_t y = 0; y < (pal_mode ? 30 : 28); y++) {
		dma_now(DmaVRAM, (uint32_t) &MAP_SndTest[index], VDP_PLAN_B + (y << 7), 40, 2);
		index += 40 << 1;
	}

	draw_status(status);
	vdp_puts(VDP_PLAN_A, "Sound Test", 15, 2);
	vdp_puts(VDP_PLAN_A, "Track: ", 2, 8);
	{
		char str[32];
		sprintf(str, "%s-Play %s-Stop %s-Quit",
		    btnName[cfg_btn_jump], btnName[cfg_btn_shoot], btnName[cfg_btn_pause]);
		vdp_puts(VDP_PLAN_A, str, 2, 15);
	}
	
	DRAW_BYTE(track, 10, 8);
	
	vdp_colors(0, PAL_Main.data, 16);
	vdp_colors(16, PAL_SndTest.data, 16);
	
	vdp_set_display(TRUE);
    z80_resume();
    enable_ints;

	song_stop();
	oldstate = ~0;
    while(TRUE) {
		// Switch between tracks
		// Skip the gap between the last song and first sfx
		if(joy_pressed(BUTTON_LEFT)) {
			if(track == 0) track = FIRST_SOUND + SOUND_COUNT - 1;
			else if(track == FIRST_SOUND) track = SONG_COUNT - 1;
			else track--;
			DRAW_BYTE(track, 10, 8);
		} else if(joy_pressed(BUTTON_RIGHT)) {
			if(track == SONG_COUNT - 1) track = FIRST_SOUND;
			else if(track == FIRST_SOUND + SOUND_COUNT - 1) track = 0;
			else track++;
			DRAW_BYTE(track, 10, 8);
		}
		if(joy_pressed(btn[cfg_btn_jump])) {
			// Play
			if(track < SONG_COUNT) {
				song_play(track);
				status = PLAYING;
				draw_track_info(track);
			} else if(track >= FIRST_SOUND && track < FIRST_SOUND + SOUND_COUNT){
				sound_play(track - FIRST_SOUND, 5);
			}
		} else if(joy_pressed(btn[cfg_btn_shoot])) {
			// Stop
			song_stop();
			status = STOPPED;
		}
		if(joy_pressed(btn[cfg_btn_pause])) {
			return;
		}
		if(status != oldstatus) {
			draw_status(status);
			oldstatus = status;
		}
		ready = TRUE;
		vdp_vsync();
		aftervsync();
    }
}
