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

#define ENTITY_ONCREATE(e) ({                                                                  \
	if(npc_info[e->type].onCreate != NULL) npc_info[e->type].onCreate(e);                      \
})
#define ENTITY_ONUPDATE(e) ({                                                                  \
	if(npc_info[e->type].onUpdate != NULL) npc_info[e->type].onUpdate(e);                      \
})
#define ENTITY_ONSTATE(e) ({                                                                   \
	if(npc_info[e->type].onState != NULL) npc_info[e->type].onState(e);                        \
})
#define ENTITY_ONHURT(e) ({                                                                    \
	if(npc_info[e->type].onHurt != NULL) npc_info[e->type].onHurt(e);                          \
})

#define ENTITY_SET_STATE(e, s, t) ({                                                           \
	e->state = s;                                                                              \
	e->state_time = t;                                                                         \
	ENTITY_ONSTATE(e);                                                                         \
})         

typedef void (*EntityMethod)(Entity*);

// Special states
// Defeated is automatically set when health reaches zero. Many enemies do not immediately
// explode so this allows a custom onUpdate/onState method to show a death animation
#define STATE_DEFEATED 900
// Destroy signals the engine to make the entity explode and drop powerups
#define STATE_DESTROY 998
// Signal to delete, but don't explode
#define STATE_DELETE 999

/* Helper Macros */

#define FACE_PLAYER(e) (e->direction = e->x > player.x ? 0 : 1)

/* Generic - basic.c */

// Tons of NPCs are placed 1 block above where they are meant to appear
// This function pushes them down to the right spot
void ai_pushdn_onCreate(Entity *e);
// NPC will face right if NPC_OPTION2 is set
void ai_op2flip_onCreate(Entity *e);
// NPC's sprite will start at the second frame if NPC_OPTION2 is set
void ai_op2frame_onCreate(Entity *e);
// NPC's sprite will start at the second animation if NPC_OPTION2 is set
void ai_op2anim_onCreate(Entity *e);
// Blackboard needs to be pushed up, and changes frame on NPC_OPTION2
void ai_blackboard_onCreate(Entity *e);
// Rotates the spikes so they are always sticking out of a solid area
void ai_spike_onCreate(Entity *e);
// Apply gravity & collision, some NPC's need to fall when the floor disappears beneath them
// See save points and refills in Sand Zone, Labyrinth
void ai_grav_onUpdate(Entity *e);
// When the player enters a trigger's target area, an event will begin automatically
void ai_trigger_onUpdate(Entity *e);
// Generic enemy projectile
void ai_genericproj_onUpdate(Entity *e);
// Default onState just explodes on death
void ai_default_onState(Entity *e);

void ai_teleIn_onCreate(Entity *e);
void ai_teleIn_onUpdate(Entity *e);

void ai_teleOut_onCreate(Entity *e);
void ai_teleOut_onUpdate(Entity *e);

void ai_player_onUpdate(Entity *e);
void ai_player_onState(Entity *e);

/* Balrog - balrog.c */

void ai_balfrog_onCreate(Entity *e);
void ai_balfrog_onUpdate(Entity *e);
void ai_balfrog_onState(Entity *e);
void ai_balfrog_onHurt(Entity *e);

void ai_balrog_onUpdate(Entity *e);
void ai_balrog_onState(Entity *e);

void ai_balrogRunning_onUpdate(Entity *e);
void ai_balrogRunning_onState(Entity *e);

/* Bats - bat.c */

void ai_batVertical_onCreate(Entity *e);
void ai_batVertical_onUpdate(Entity *e);

void ai_batHang_onCreate(Entity *e);
void ai_batHang_onUpdate(Entity *e);

/* Curly - curly.c */

void ai_curly_onUpdate(Entity *e);
void ai_curly_onState(Entity *e);

/* Egg Corridor - eggs.c */

void ai_basil_onCreate(Entity *e);
void ai_basil_onUpdate(Entity *e);

void ai_beetle_onUpdate(Entity *e);

void ai_beetleFollow_onCreate(Entity *e);
void ai_beetleFollow_onUpdate(Entity *e);

void ai_behemoth_onUpdate(Entity *e);
void ai_behemoth_onState(Entity *e);

void ai_lift_onCreate(Entity *e);
void ai_lift_onUpdate(Entity *e);

/* Igor - igor.c */

void ai_igor_onCreate(Entity *e);
void ai_igor_onUpdate(Entity *e);
void ai_igor_onState(Entity *e);

void ai_igorscene_onUpdate(Entity *e);
void ai_igorscene_onState(Entity *e);

void ai_igordead_onUpdate(Entity *e);

/* Critters - critter.c */

void ai_critter_onUpdate(Entity *e);
void ai_critter_onState(Entity *e);
void ai_critter_onHurt(Entity *e);

/* Doors - door.c */

void ai_door_onCreate(Entity *e);
void ai_door_onUpdate(Entity *e);

void ai_theDoor_onUpdate(Entity *e);
void ai_theDoor_onHurt(Entity *e);

/* Items & Treasure - item.c */

void ai_energy_onCreate(Entity *e);
void ai_energy_onUpdate(Entity *e);
void ai_missile_onUpdate(Entity *e);
void ai_heart_onUpdate(Entity *e);

/* Fans - fan.c */

void ai_fan_onCreate(Entity *e);
void ai_fan_onUpdate(Entity *e);

/* Minor Mimigas - mimiga.c */

void ai_flower_onCreate(Entity *e);
//void ai_kanpachi_onState(Entity *e);
void ai_jack_onUpdate(Entity *e);
void ai_jack_onState(Entity *e);
//void ai_mahin_onState(Entity *e);
void ai_santa_onUpdate(Entity* e);
void ai_santa_onState(Entity *e);
void ai_chaco_onUpdate(Entity* e);
void ai_chaco_onState(Entity *e);

/* Sue - sue.c */

void ai_sue_onUpdate(Entity *e);
void ai_sue_onState(Entity *e);

/* Kazuma - kazuma.c */

void ai_kazuComp_onUpdate(Entity *e);

/* King - king.c */

void ai_king_onUpdate(Entity *e);
void ai_king_onState(Entity *e);

/* Misery - misery.c */

void ai_miseryFloat_onUpdate(Entity *e);
void ai_miseryFloat_onState(Entity *e);

/* Toroko - toroko.c */

void ai_torokoAtk_onCreate(Entity *e);

void ai_toroko_onUpdate(Entity *e);
void ai_toroko_onState(Entity *e);

/* Cemetery Enemies - cemetery.c */

void ai_pignon_onUpdate(Entity *e);
void ai_pignon_onHurt(Entity *e);

void ai_gkeeper_onUpdate(Entity *e);

/* Grasstown - weed.c */

void ai_jelly_onCreate(Entity *e);
void ai_jelly_onUpdate(Entity *e);
void ai_jelly_onHurt(Entity *e);

void ai_mannan_onUpdate(Entity *e);
void ai_mannan_onState(Entity *e);
void ai_mannan_onHurt(Entity *e);

void ai_mannanShot_onUpdate(Entity *e);

void ai_malco_onUpdate(Entity *e);
void ai_malco_onState(Entity *e);

void ai_malcoBroken_onCreate(Entity *e);
void ai_malcoBroken_onState(Entity *e);

/* Sand Zone - sand.c */

void ai_sunstone_onCreate(Entity *e);
void ai_sunstone_onUpdate(Entity *e);
void ai_sunstone_onState(Entity *e);

#endif /* INC_AI_H_ */
