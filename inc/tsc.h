#ifndef INC_TSC_H_
#define INC_TSC_H_

#include "common.h"

// This is the first event after hitting "New Game"
#define GAME_START_EVENT 200
// Events that get called when killed, drowned, out of bounds, etc
#define PLAYER_DEFEATED_EVENT 40
#define PLAYER_DROWN_EVENT 41
#define PLAYER_OOB_EVENT 42

u8 tscEventCount;

void tsc_init();

void tsc_load_stage(u8 id);

u8 tsc_update();

void tsc_call_event(u16 number);

bool tsc_running();

void tsc_unpause_debug();

#endif /* INC_TSC_H_ */
