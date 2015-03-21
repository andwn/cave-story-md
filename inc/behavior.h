#ifndef INC_BEHAVIOR_H_
#define INC_BEHAVIOR_H_

#include "common.h"
#include "entity.h"

// Entity behavior and AI
void ai_activate_null(Entity *e);
void ai_update_null(Entity *e);
//void ai_setstate_null(Entity *e, u16 state);

// 60 and 63 - Toroko / Toroko attack with stick
void ai_update_toroko(Entity *e);
//void ai_setstate_toroko(Entity *e, u16 state);

// 211 - Spike
void ai_activate_spike();

#endif /* INC_BEHAVIOR_H_ */
