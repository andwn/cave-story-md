#ifndef INC_SYSTEM_H_
#define INC_SYSTEM_H_

#include "common.h"

#define SRAM_UNCHECKED		0x00
#define SRAM_VALID_EMPTY	0x01
#define SRAM_VALID_SAVE		0x02
#define SRAM_INVALID		0xFF

#define FLAG_EVENTONDROWN		4000
#define FLAG_DISABLESAVE		4095

// Shows extra info on screen, fly through walls
extern bool debuggingEnabled;

// Normally after the player is hurt they are invincible on a short timer (~2 seconds).
// Cave Story has a bug where pausing resets this timer to 0, allowing a section
// in Grasstown to be skipped using the knockback of a bat twice in the same jump
// See player.c: player_unpause()
extern bool pauseCancelsIFrames;

// When true, player can skip through all the message text by holding the B button
extern bool holdBToSkipText;

extern bool checksumValid;

// Global flags are persisted to save data
void system_set_flag(u16 flag, bool value);
bool system_get_flag(u16 flag);
// Skip flags remain in memory until power off, or new game
// Used to skip boss cutscenes when retrying
void system_set_skip_flag(u16 flag, bool value);
bool system_get_skip_flag(u16 flag);

// Counts play time
void system_update();
// Returns play time
u8 system_get_frame();
// Draws play time
void system_drawtime(u16 x, u16 y);

// Initializes variables with defaults when starting a new game
void system_new();
// Initializes variables from SRAM when loading a game save
void system_load();
// Stores variables and game state into SRAM
void system_save();
// Validates whether any save data exists in SRAM
u8 system_checkdata();

void system_verifychecksum();

#endif // INC_SYSTEM_H_
