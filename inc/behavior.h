#ifndef INC_AI_COMMON_H_
#define INC_AI_COMMON_H_

#include "common.h"
#include "entity.h"

#define STATE_DEFEATED 900

// Applies proper behavior functions to newly created entity
// Also handles special initialization cases per NPC type
void ai_setup(Entity *e);

// Entity behavior and AI
void ai_activate_base(Entity *e);
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

/* Anywhere Objects */

// 1 - Weapon Energy
void ai_update_energy(Entity *e);
// 18 - Door
void ai_activate_door(Entity *e);
// 46 - Trigger
void ai_update_trigger(Entity *e);
// Fans
void ai_activate_fan(Entity *e);
void ai_update_fan(Entity *e);
// 211 - Spike
void ai_activate_spike();

/* Mimiga Village NPCs */

// 60 and 63 - Toroko / Toroko attack with stick
void ai_update_toroko(Entity *e);
//void ai_setstate_toroko(Entity *e, u16 state);

/* Balrog */

// 12 - Balrog (Cutscene)
void ai_update_balrog_scene(Entity *e);
bool ai_setstate_balrog_scene(Entity *e, u16 state);
// 19 - Balrog (Busts In)
// 9 - Balrog (Drops In)
// 68 - Boss: Balrog (Mimiga Village)
void ai_update_balrog_boss1(Entity *e);
bool ai_setstate_balrog_boss1(Entity *e, u16 state);
// 10 - Boss: Balrog (Grasstown)
// B2 - Balfrog
bool ai_setstate_balfrog(Entity *e, u16 state);
// Balrog (Missiles)

/* Misery */

// 67 - Misery (float)
void ai_update_misery_float(Entity *e);
bool ai_setstate_misery_float(Entity *e, u16 state);
// TODO: Standing, boss, projectiles

/* Egg Corridor NPCs */



/* Grasstown NPCs */



#endif /* INC_AI_COMMON_H_ */
