#include "gamemode.h"

#include <genesis.h>
#include "resources.h"
#include "input.h"
#include "tables.h"
#include "audio.h"
#include "vdp_ext.h"
#include "sprite.h"

enum { STOPPED, PLAYING, PAUSED };

// Music 0x00 - 0x7F, Sound Effects 0x80 - 0xFF
#define FIRST_SOUND 0x80

// Track # and name
void draw_track_info(u8 track) {
	VDP_drawByte(track, 2, 6);
	VDP_clearText(5, 6, 33);
	VDP_drawText(song_info[track].name, 5, 6);
}

void draw_status(u8 status) {
	switch(status) {
		case STOPPED: VDP_drawText("Stopped", 2, 4); break;
		case PLAYING: VDP_drawText("Playing", 2, 4); break;
		case PAUSED:  VDP_drawText("Paused ", 2, 4); break;
	}
}

void soundtest_main() {
	u8 track = 0;
	u8 status = STOPPED, oldstatus = STOPPED;
	song_stop();
	SYS_disableInts();
	VDP_setEnable(false);
	//Kill all sprites
	spr_num = 1;
	sprites[0] = (VDPSprite) {};
	sprites_send();
	VDP_clearPlan(PLAN_A, true);
	VDP_setPalette(PAL0, PAL_Main.data);
	VDP_setPalette(PAL1, PAL_SndTest.data);
	// Background picture
	if(!VDP_loadTileSet(&TS_SndTest, TILE_USERINDEX, true)) {
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
	VDP_setEnable(true);
	SYS_enableInts();
    while(true) {
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
		VDP_waitVSync();
    }
}
