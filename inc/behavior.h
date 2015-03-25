#ifndef INC_BEHAVIOR_H_
#define INC_BEHAVIOR_H_

#include "common.h"
#include "entity.h"

// Entity behavior and AI
void ai_activate_stub(Entity *e);
void ai_update_stub(Entity *e);
void ai_setstate_stub(Entity *e, u16 state);

// 1 - Weapon Energy
void ai_update_energy(Entity *e);
// 18 - Door
void ai_activate_door(Entity *e);
// 60 and 63 - Toroko / Toroko attack with stick
void ai_update_toroko(Entity *e);
//void ai_setstate_toroko(Entity *e, u16 state);

// 211 - Spike
void ai_activate_spike();

#endif /* INC_BEHAVIOR_H_ */
