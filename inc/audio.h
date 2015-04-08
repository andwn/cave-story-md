#ifndef INC_AUDIO_H_
#define INC_AUDIO_H_

#include "common.h"

// IDs for hard coded sounds
// The IDs are the index of sound_info in tables.c
#define SOUND_CURSOR 0x01
#define SOUND_PROMPT 0x05
#define SOUND_CONFIRM 0x12
#define SOUND_HURT 0x10
#define SOUND_DIE 0x11
#define SOUND_HEADBONK 0x03 // Bump ceiling
#define SOUND_THUD 0x17 // Landing
#define SOUND_GETEXP 0x0E
#define SOUND_LEVELUP 0x1B
#define SOUND_REFILL 0x14
#define SOUND_EXPBOUNCE 0x2D
#define SOUND_BREAK 0x0C
#define SOUND_HOP 0x1E
// IDs for title screen songs
// Indexes of song_info
#define SONG_TITLE 0x18
#define SONG_TOROKO 0x28
#define SONG_KING 0x29

// Initialize sound system
void sound_init();
// Play sound by ID, priority is 0-15, 15 is most important
void sound_play(u8 id, u8 priority);
// Play a song by ID
void song_play(u8 id);
// Stops music if playing, same as song_play(0)
void song_stop();
// Resume previous song that was playing
// TSC calls this after fanfare tracks
void song_resume();

u8 song_get_playing();

#endif /* INC_AUDIO_H_ */
