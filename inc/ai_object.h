#ifndef INC_AI_OBJECT_H_
#define INC_AI_OBJECT_H_

#include "common.h"
#include "entity.h"
#include "ai_common.h"

// 1 - Weapon Energy
void ai_update_energy(Entity *e);
// 18 - Door
void ai_activate_door(Entity *e);
// 211 - Spike
void ai_activate_spike();

#endif /* INC_AI_OBJECT_H_ */
