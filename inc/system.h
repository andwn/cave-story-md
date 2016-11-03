#ifndef INC_SYSTEM_H_
#define INC_SYSTEM_H_

#include "common.h"

#define SRAM_UNCHECKED		0x00
#define SRAM_VALID_EMPTY	0x01
#define SRAM_VALID_SAVE		0x02
#define SRAM_INVALID		0xFF

#define FLAG_EVENTONDROWN		4000
#define FLAG_DISABLESAVE		4095

extern u8 debuggingEnabled;

// Global flags are persisted to save data
void system_set_flag(u16 flag, u8 value);
u8 system_get_flag(u16 flag);
// Skip flags remain in memory until power off, or new game
// Used to skip boss cutscenes when retrying
void system_set_skip_flag(u16 flag, u8 value);
u8 system_get_skip_flag(u16 flag);

// Increments play time
void system_update();

// Initializes variables with defaults when starting a new game
void system_new();
// Initializes variables from SRAM when loading a game save
void system_load();
// Stores variables and game state into SRAM
void system_save();
// Validates whether any save data exists in SRAM
u8 system_checkdata();

// Counter stuff
void system_start_counter();
u32 system_counter_ticks();
void system_counter_draw();
u32 system_load_counter();
void system_save_counter(u32 ticks);

#endif // INC_SYSTEM_H_
