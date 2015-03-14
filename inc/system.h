#ifndef INC_SYSTEM_H_
#define INC_SYSTEM_H_

#include "common.h"

extern bool debuggingEnabled;

// Global flags are persisted to save data
void system_set_flag(u16 flag, bool value);
bool system_get_flag(u16 flag);
// Skip flags remain in memory until power off, or new game
// Used to skip boss cutscenes when retrying
void system_set_skip_flag(u16 flag, bool value);
bool system_get_skip_flag(u16 flag);

// Mostly play time
void system_update();
void system_drawtime(u16 x, u16 y);

// Save data
void system_new();
void system_load();
void system_save();
u8 system_checkdata();

#endif // INC_SYSTEM_H_
