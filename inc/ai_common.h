#ifndef INC_AI_COMMON_H_
#define INC_AI_COMMON_H_

#include "common.h"
#include "entity.h"

#define STATE_DEFEATED 900

// Entity behavior and AI
void ai_activate_base(Entity *e);
void ai_update_base(Entity *e);
bool ai_setstate_base(Entity *e, u16 state);

// Bats
void ai_update_bat_vert(Entity *e);
void ai_update_bat_circle(Entity *e);

// Critters
void ai_update_critter_hop(Entity *e);
bool ai_setstate_critter_hop(Entity *e, u16 state);

// Door Enemy
void ai_update_door_enemy(Entity *e);
void ai_hurt_door_enemy(Entity *e);

#endif /* INC_AI_COMMON_H_ */
