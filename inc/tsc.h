#ifndef INC_TSC_H_
#define INC_TSC_H_

#include <genesis.h>

#define GAME_START_EVENT 200
//#define GAME_LOAD_EVENT 90 // "Playing Song" from save data is used instead
#define PLAYER_DEFEATED_EVENT 40
#define PLAYER_DROWN_EVENT 41
#define PLAYER_OOB_EVENT 42

u8 tscEventCount;

void tsc_init();

void tsc_load(u8 id);

u8 tsc_update();

void tsc_call_event(u16 number);

void tsc_unpause_debug();

#endif /* INC_TSC_H_ */
