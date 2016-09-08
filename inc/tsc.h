#ifndef INC_TSC_H_
#define INC_TSC_H_

#include "common.h"

// This is the first event after hitting "New Game"
#define GAME_START_EVENT 200
// Events that get called when killed, drowned, out of bounds, etc
#define PLAYER_DEFEATED_EVENT 40
#define PLAYER_DROWN_EVENT 41
#define PLAYER_OOB_EVENT 42

#define ALMOND_DROWN_FLAG	4000
#define ALMOND_DROWN_EVENT	1100

// Number of events loaded by tsc_load(), for debugging
u8 tscEventCount;

u8 tscState;

// As teleporter locations are made available to the player, this list is populated
// with which event is called when selecting a particular area.
// This is in the header so that system_save() can write the values to SRAM
u16 teleportEvent[8];

// True while boss health is being displayed in the corner
bool showingBossHealth;

// Initialize default values to avoid strange glitches
// Also loads persistent head.tsc and arms.tsc events
void tsc_init();
// Called by stage_load(), fills up the event list with pointers to each event
void tsc_load_stage(u8 id);
// Per frame update, will advance the script or wait for something based on whats happening
u8 tsc_update();
// This will begin executing a scripted event if it exists
void tsc_call_event(u16 number);
// Returns true if a script is being executed
bool tscState;

void tsc_unpause_debug();

void tsc_update_boss_health();

#endif /* INC_TSC_H_ */
