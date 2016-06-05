#ifndef INC_AI_H_
#define INC_AI_H_

#include "common.h"
#include "entity.h"

/*
 * This module contains behavior or AI for entities specific to NPC type
 * There are 4 "methods" which may be indexed in the npc_info table for an NPC:
 * * onCreate
 * Called by entity_create() and entity_replace() after applying default values,
 * but before deciding to activate or deactivate (and therefore load the sprite).
 * Because of this SPR_XX functions will have to effect. Instead modify spriteAnim,
 * spriteFrame, spriteVFlip, and direction. The sprite, once created, will adjust accordingly
 * * onUpdate
 * Called for active entities only in entity_update(), after checking for deactivation, and
 * before combat. 
 * * onState
 * The macro ENTITY_SET_STATE will call this after setting a new state.
 * * onHurt
 * Called by entity_update() if the entity has taken damage from the player's weapon.
 * The sound effect and damage string are already managed in entity.c so do not do so
 * inside of onHurt. Additionally onHurt is NOT called when the entity is killed. Use
 * onState and check for STATE_DEFEATED to act upon death.
 */

// Special states
// Defeated is automatically set when health reaches zero. Many enemies do not immediately
// explode so this allows a custom onUpdate/onState method to show a death animation
#define STATE_DEFEATED 900
// Destroy signals the engine to make the entity explode and drop powerups
#define STATE_DESTROY 998
// Signal to delete, but don't explode
#define STATE_DELETE 999

/* Generic - basic.c */

void ai_pushdn_onCreate(Entity *e);
void ai_op2flip_onCreate(Entity *e);
void ai_op2frame_onCreate(Entity *e);
void ai_op2anim_onCreate(Entity *e);

void ai_blackboard_onCreate(Entity *e);

void ai_spike_onCreate(Entity *e);

void ai_grav_onUpdate(Entity *e);

void ai_default_onState(Entity *e);

void ai_trigger_onUpdate(Entity *e);

/* Balrog - balrog.c */

void ai_balfrog_onState(Entity *e);

void ai_balrog_onUpdate(Entity *e);
void ai_balrog_onState(Entity *e);

void ai_balrogRunning_onUpdate(Entity *e);
void ai_balrogRunning_onState(Entity *e);

/* Bats - bat.c */

void ai_batVertical_onCreate(Entity *e);
void ai_batVertical_onUpdate(Entity *e);

/* Egg Corridor - eggs.c */

void ai_basil_onCreate(Entity *e);
void ai_basil_onUpdate(Entity *e);

void ai_beetle_onUpdate(Entity *e);

void ai_beetleFollow_onUpdate(Entity *e);

void ai_behemoth_onUpdate(Entity *e);
void ai_behemoth_onState(Entity *e);

void ai_lift_onCreate(Entity *e);
void ai_lift_onUpdate(Entity *e);

/* Critters - critter.c */

void ai_critterHop_onUpdate(Entity *e);
void ai_critterHop_onState(Entity *e);

/* Doors - door.c */

void ai_door_onCreate(Entity *e);
void ai_door_onState(Entity *e);

void ai_theDoor_onUpdate(Entity *e);
void ai_theDoor_onHurt(Entity *e);

/* Items & Treasure - item.c */

void ai_energy_onCreate(Entity *e);
void ai_energy_onUpdate(Entity *e);

/* Fans - fan.c */

void ai_fan_onCreate(Entity *e);
void ai_fan_onUpdate(Entity *e);

/* Minor Mimigas - mimiga.c */

//void ai_kanpachi_onState(Entity *e);
void ai_jack_onUpdate(Entity *e);
//void ai_mahin_onState(Entity *e);

/* Kazuma - kazuma.c */

void ai_kazuComp_onUpdate(Entity *e);

/* King - king.c */

//void ai_king_onCreate(Entity *e);

/* Misery - misery.c */

void ai_miseryFloat_onUpdate(Entity *e);
void ai_miseryFloat_onState(Entity *e);

/* Toroko - toroko.c */

void ai_torokoAtk_onCreate(Entity *e);

void ai_toroko_onUpdate(Entity *e);
void ai_toroko_onState(Entity *e);

#endif /* INC_AI_H_ */
