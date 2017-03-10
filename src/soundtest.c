#include "common.h"

#include "audio.h"
#include "dma.h"
#include "input.h"
#include "joy.h"
#include "resources.h"
#include "sprite.h"
#include "system.h"
#include "tables.h"
#include "vdp.h"
#include "vdp_bg.h"
#include "vdp_pal.h"
#include "vdp_tile.h"
#include "vdp_ext.h"

#include "gamemode.h"

enum { STOPPED, PLAYING, PAUSED };

// Music 0x00 - 0x7F, Sound Effects 0x80 - 0xFF
#define FIRST_SOUND 0x80

// Track # and name
void draw_track_info(uint8_t track) {
	VDP_drawByte(track, 2, 6);
	VDP_clearText(5, 6, 33);
	VDP_drawText(song_info[track].name, 5, 6);
}

void draw_status(uint8_t status) {
	switch(status) {
		case STOPPED: VDP_drawText("Stopped", 2, 4); break;
		case PLAYING: VDP_drawText("Playing", 2, 4); break;
		case PAUSED:  VDP_drawText("Paused ", 2, 4); break;
	}
}

void soundtest_main() {
	uint8_t track = 0;
	uint8_t status = STOPPED, oldstatus = STOPPED;
	
	VDP_setEnable(FALSE);
	
	sprites_clear();
	VDP_clearPlan(PLAN_A, TRUE);
	// Background picture
	if(!VDP_loadTileSet(&TS_SndTest, TILE_USERINDEX, TRUE)) {
		SYS_die("Not enough memory to unpack tileset.");
	}
	VDP_fillTileMapRectInc(PLAN_B,
			TILE_ATTR_FULL(PAL1, 0, 0, 0, TILE_USERINDEX), 0, 0, 40, 28);
	draw_status(status);
	VDP_drawText("Sound Test", 15, 2);
	VDP_drawText("Track: ", 2, 8);
	VDP_drawText("C-Play B-Stop", 2, 14);
	VDP_drawText("A-Pause Start-Quit", 2, 16);
	VDP_drawByte(track, 10, 8);
	
	VDP_setPalette(PAL0, PAL_Main.data);
	VDP_setPalette(PAL1, PAL_SndTest.data);
	
	VDP_setEnable(TRUE);
	
    while(TRUE) {
		input_update();
		// Switch between tracks
		// Skip the gap between the last song and first sfx
		if(joy_pressed(BUTTON_LEFT)) {
			if(track == 0) track = FIRST_SOUND + SOUND_COUNT - 1;
			else if(track == FIRST_SOUND) track = SONG_COUNT - 1;
			else track--;
			VDP_drawByte(track, 10, 8);
		} else if(joy_pressed(BUTTON_RIGHT)) {
			if(track == SONG_COUNT - 1) track = FIRST_SOUND;
			else if(track == FIRST_SOUND + SOUND_COUNT - 1) track = 0;
			else track++;
			VDP_drawByte(track, 10, 8);
		}
		if(joy_pressed(BUTTON_C)) {
			// Play
			if(track < SONG_COUNT) {
				song_play(track);
				status = PLAYING;
				draw_track_info(track);
			} else if(track >= FIRST_SOUND && track < FIRST_SOUND + SOUND_COUNT){
				sound_play(track - FIRST_SOUND, 0);
			}
		} else if(joy_pressed(BUTTON_B)) {
			// Stop
			song_stop();
			status = STOPPED;
		} else if(joy_pressed(BUTTON_A)) {
			// Pause
			if(status == PLAYING) {
				song_stop();
				status = PAUSED;
			} else if(status == PAUSED) {
				song_resume();
				status = PLAYING;
			}
		}
		if(joy_pressed(BUTTON_START)) {
			song_stop();
			oldstate |= BUTTON_START;
			break;
		}
		if(status != oldstatus) {
			draw_status(status);
			oldstatus = status;
		}
		ready = TRUE;
		vsync();
		aftervsync();
    }
    SYS_hardReset(); // eh
}
