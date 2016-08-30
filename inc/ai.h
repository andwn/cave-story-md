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
 * This is called whenever a TSC script changes the state of an entity. It is also called
 * when a bullet causes the entity's health to reach zero, and sets STATE_DEFEATED.
 * * onHurt
 * Removed. Check if e->damage_time is nonzero instead.
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

// states to control the water-level object
#define WL_CALM				10	// calm and slow at set point
#define WL_CYCLE			20	// cycles between set point and top of screen
#define WL_DOWN				21	// in cycle--currently down
#define WL_UP				22	// in cycle--currently up
#define WL_STAY_UP			30	// goes to top of screen and doesn't come back down

#define WATER_TOP			0
#define WATER_DISABLE		255

/* Helper Macros */

#define SNAP_TO_GROUND(e); ({ \
	u16 bx = sub_to_block(e->x); \
	u16 by = sub_to_block(e->y + ((e->hit_box.bottom+1)<<CSF)); \
	if(stage_get_block_type(bx, by) != 0x41) { \
		e->y += 16 << CSF; \
	} else { \
		by = sub_to_block(e->y + ((e->hit_box.bottom-1)<<CSF)); \
		if(stage_get_block_type(bx, by) == 0x41) e->y -= 8 << CSF; \
	} \
})

#define FACE_PLAYER(e) ({ \
	if((e->direction && e->x < player.x) || (!e->direction && e->x > player.x)) { \
		e->direction ^= 1; \
		SPR_SAFEHFLIP(e->sprite, e->direction); \
	} \
})
#define TURN_AROUND(e) ({ \
	e->direction ^= 1; \
	SPR_SAFEHFLIP(e->sprite, e->direction); \
})

#define PLAYER_DIST_X(dist) (player.x > e->x - (dist) && player.x < e->x + (dist))
#define PLAYER_DIST_Y(dist) (player.y > e->y - (dist) && player.y < e->y + (dist))
#define PLAYER_DIST_X2(dist1, dist2) (player.x > e->x - (dist1) && player.x < e->x + (dist2))
#define PLAYER_DIST_Y2(dist1, dist2) (player.y > e->y - (dist1) && player.y < e->y + (dist2))

#define LIMIT_X(v) ({ \
	if(e->x_speed > (v)) e->x_speed = (v); \
	if(e->x_speed < -(v)) e->x_speed = -(v); \
})
#define LIMIT_Y(v) ({ \
	if(e->y_speed > (v)) e->y_speed = (v); \
	if(e->y_speed < -(v)) e->y_speed = -(v); \
})

#define MOVE_X(v) (e->x_speed = e->direction ? (v) : -(v))
#define ACCEL_X(v) (e->x_speed += e->direction ? (v) : -(v))

#define CURLY_TARGET_HERE(e) ({ \
	curly_target_time = 120; \
	curly_target_x = e->x; \
	curly_target_y = e->y; \
})

#define FIRE_ANGLED_SHOT(type, xx, yy, angle, speed) ({ \
	Entity *shot = entity_create(sub_to_block(xx), sub_to_block(yy), 0, 0, (type), 0, 0); \
	shot->x = (xx); \
	shot->y = (yy); \
	shot->x_speed = (sintab32[(angle)] >> 1) * ((speed) >> CSF); \
	shot->y_speed = (sintab32[((angle) + 256) % 1024] >> 1) * ((speed) >> CSF); \
})

#define SMOKE_AREA(x, y, w, h, count) ({ \
	for(u8 i = 0; i < (count); i++) { \
		effect_create_smoke(0, 	(x) + (random() % (w)), \
								(y) + (random() % (h))); \
	} \
})

/* Shared Variables */

Entity *water_entity;
u8 water_screenlevel;

u16 curly_target_time;
s32 curly_target_x, curly_target_y;

/* Utility functions - util.c */

void generic_npc_states(Entity *e);

/* Generic - basic.c */

// Tons of NPCs are placed 1 block above where they are meant to appear
// This function pushes them down to the right spot
void oncreate_snap(Entity *e);
// NPC will face right if NPC_OPTION2 is set
void oncreate_op2flip(Entity *e);
// Combination of the above
void oncreate_snapflip(Entity *e);
// NPC's sprite will start at the second animation if NPC_OPTION2 is set
void oncreate_op2anim(Entity *e);
// Only push down for NPC_OPTION2, used for the door after rescuing Kazuma
void oncreate_op2snap(Entity *e);
// Blackboard needs to be pushed up, and changes frame on NPC_OPTION2
void oncreate_blackboard(Entity *e);
// Always active
void oncreate_persistent(Entity *e);
// Rotates the spikes so they are always sticking out of a solid area
void oncreate_spike(Entity *e);
// Apply gravity & collision, some NPC's need to fall when the floor disappears beneath them
// See save points and refills in Sand Zone, Labyrinth
void ai_grav_onUpdate(Entity *e);
// When the player enters a trigger's target area, an event will begin automatically
void ai_trigger_onUpdate(Entity *e);
// Generic enemy projectile
void ai_genericproj_onUpdate(Entity *e);
// Default onState just explodes on death
void ai_default_onState(Entity *e);
// Quote teleporting in
void ai_teleIn_onCreate(Entity *e);
void ai_teleIn_onUpdate(Entity *e);
// Quote teleporting out
void ai_teleOut_onCreate(Entity *e);
void ai_teleOut_onUpdate(Entity *e);
// TODO: Blue light that flashes while the teleporter is in use
void ai_teleLight_onCreate(Entity *e);
void ai_teleLight_onUpdate(Entity *e);
void ai_teleLight_onState(Entity *e);
// Quote NPC used in scenes where the actual player character is hidden
void ai_player_onUpdate(Entity *e);
void ai_player_onState(Entity *e);

/* Regular NPCs - regu.c */

void ai_jenka(Entity *e);

void ai_doctor(Entity *e);

void ai_toroko(Entity *e);
void ai_toroko_teleport_in(Entity *e);

void ai_sue(Entity *e);
void ai_sue_teleport_in(Entity *e);

void ai_kazuma(Entity *e);

void ai_king(Entity *e);

void ai_blue_robot(Entity *e);

void ai_kanpachi_fishing(Entity *e);

void ai_booster(Entity *e);
void ai_booster_falling(Entity *e);

void ai_npc_at_computer(Entity *e);

/* Balrog - balrog.c */

void ai_balfrog_onCreate(Entity *e);
void ai_balfrog_onUpdate(Entity *e);
void ai_balfrog_onState(Entity *e);

void oncreate_balrog(Entity *e);
void ai_balrog(Entity *e);
void ai_balrog_drop_in(Entity *e);
void ai_balrog_bust_in(Entity *e);

void ai_balrogRunning_onUpdate(Entity *e);
void ai_balrogRunning_onState(Entity *e);

void ai_balrogFlying_onUpdate(Entity *e);
void ai_balrogFlying_onState(Entity *e);
void ai_balrogShot_onUpdate(Entity *e);

void ai_balrog_boss_missiles(Entity *e);
void ai_balrog_missile(Entity *e);

/* Bats - bat.c */

void ai_batVertical_onCreate(Entity *e);
void ai_batVertical_onUpdate(Entity *e);

void ai_batHang_onCreate(Entity *e);
void ai_batHang_onUpdate(Entity *e);

void ai_batCircle_onUpdate(Entity *e);

/* Curly - curly.c */

void ai_curly(Entity *e);
void ai_curly_carried(Entity *e);

void ai_curlyBoss_onUpdate(Entity *e);
void ai_curlyBoss_onState(Entity *e);

void ai_curlyBossShot_onUpdate(Entity *e);

void ai_curly_ai(Entity *e);
void ai_cai_gun(Entity *e);
void ai_cai_watershield(Entity *e);

/* Egg Corridor - eggs.c */

void ai_basil_onCreate(Entity *e);
void ai_basil_onUpdate(Entity *e);

void ai_beetle_onUpdate(Entity *e);

void ai_beetleFollow_onCreate(Entity *e);
void ai_beetleFollow_onUpdate(Entity *e);

void ai_basu_onCreate(Entity *e);
void ai_basu_onUpdate(Entity *e);

void ai_behemoth_onUpdate(Entity *e);
void ai_behemoth_onState(Entity *e);

void ai_lift_onCreate(Entity *e);
void ai_lift_onUpdate(Entity *e);

void ai_terminal_onUpdate(Entity *e);

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

void oncreate_doorway(Entity *e);
void ai_doorway(Entity *e);

/* Items & Treasure - item.c */

void ai_energy_onCreate(Entity *e);
void ai_energy_onUpdate(Entity *e);

void ai_missile_onUpdate(Entity *e);

void ai_heart_onUpdate(Entity *e);

void ai_hiddenPowerup_onCreate(Entity *e);
void ai_hiddenPowerup_onUpdate(Entity *e);

/* Fans - fan.c */

void ai_fan_onCreate(Entity *e);
void ai_fan_onUpdate(Entity *e);

/* Minor Mimigas - mimiga.c */

void ai_flower_onCreate(Entity *e);
void ai_jack(Entity *e);
void ai_santa(Entity* e);
void ai_chaco(Entity* e);

/* Misery - misery.c */

void ai_misery_float(Entity *e);
void ai_misery_bubble(Entity *e);

/* Toroko - toroko.c */

void ai_torokoAtk_onCreate(Entity *e);

void ai_toroko_onUpdate(Entity *e);
void ai_toroko_onState(Entity *e);

void ai_torokoBoss_onCreate(Entity *e);
void ai_torokoBoss_onUpdate(Entity *e);
void ai_torokoBoss_onState(Entity *e);
void ai_torokoBlock_onUpdate(Entity *e);
void ai_torokoFlower_onUpdate(Entity *e);

/* Cemetery Enemies - cemetery.c */

void ai_pignon_onUpdate(Entity *e);
void ai_pignon_onHurt(Entity *e);

void ai_gkeeper_onCreate(Entity *e);
void ai_gkeeper_onUpdate(Entity *e);

/* Grasstown - weed.c */

//void ai_jelly_onCreate(Entity *e);
void ai_jelly_onUpdate(Entity *e);
//void ai_jelly_onHurt(Entity *e);

void ai_kulala_onUpdate(Entity *e);

void ai_mannan_onUpdate(Entity *e);
void ai_mannan_onState(Entity *e);
void ai_mannan_onHurt(Entity *e);

void ai_mannanShot_onUpdate(Entity *e);

void ai_malco_onUpdate(Entity *e);
void ai_malco_onState(Entity *e);

void ai_malcoBroken_onCreate(Entity *e);
void ai_malcoBroken_onState(Entity *e);

void ai_powerc_onCreate(Entity *e);

void ai_press_onUpdate(Entity *e);

void ai_frog_onUpdate(Entity *e);

void ai_hey_onUpdate(Entity *e);

void ai_motorbike_onUpdate(Entity *e);

/* Sand Zone - sand.c */

void ai_omega_onCreate(Entity *e);
void ai_omega_onUpdate(Entity *e);
void ai_omega_onState(Entity *e);

void ai_sunstone_onCreate(Entity *e);
void ai_sunstone_onUpdate(Entity *e);
void ai_sunstone_onState(Entity *e);

void ai_puppy_onCreate(Entity *e);
void ai_puppy_onUpdate(Entity *e);

void ai_puppyCarry_onCreate(Entity *e);
void ai_puppyCarry_onUpdate(Entity *e);

void ai_polish_onUpdate(Entity *e);
void ai_baby_onUpdate(Entity *e);

void ai_sandcroc_onUpdate(Entity *e);

void ai_skullhead_onUpdate(Entity *e);

void ai_crow_onUpdate(Entity *e);

void ai_curlys_mimigas(Entity *e);

void ai_armadillo_onUpdate(Entity *e);

void ai_jenka_onCreate(Entity *e);

/* Labyrinth - maze.c */

void ai_block_onCreate(Entity *e);
void ai_blockh_onUpdate(Entity *e);
void ai_blockv_onUpdate(Entity *e);
void ai_boulder_onUpdate(Entity *e);

void ai_gaudiDying_onUpdate(Entity *e);
void ai_gaudi_onUpdate(Entity *e);
void ai_gaudiFlying_onUpdate(Entity *e);
void ai_gaudiArmored_onUpdate(Entity *e);
void ai_gaudiArmoredShot_onUpdate(Entity *e);

void ai_pooh_black(Entity *e);
void ai_pooh_black_bubble(Entity *e);
void ai_pooh_black_dying(Entity *e);

void ai_firewhirr(Entity *e);
void ai_firewhirr_shot(Entity *e);
void ai_gaudi_egg(Entity *e);
void ai_fuzz_core(Entity *e);
void ai_fuzz(Entity *e);
void ai_buyobuyo_base(Entity *e);
void ai_buyobuyo(Entity *e);

/* Core Room - almond.c */

void ai_waterlevel(Entity *e);
void ai_shutter(Entity *e);
void ai_shutter_stuck(Entity *e);
void ai_almond_robot(Entity *e);

/* Core - core.c */

void oncreate_core(Entity *e);
void ai_core(Entity *e);
void ondeath_core(Entity *e);
void ai_core_front(Entity *e);
void ai_core_back(Entity *e);
void ai_minicore(Entity *e);
void ai_minicore_shot(Entity *e);
void ai_core_ghostie(Entity *e);
void ai_core_blast(Entity *e);

#endif /* INC_AI_H_ */
