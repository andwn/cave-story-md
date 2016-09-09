#ifndef INC_AI_H_
#define INC_AI_H_

#include "common.h"
#include "entity.h"

/*
 * This module contains behavior or AI for entities specific to NPC type
 * There are 3 "methods" which may be indexed in the npc_info table for an NPC
 * * onCreate: when first created or replaced
 * * onFrame: each frame while it is active
 * * onDeath: when it's killed
 */

#define ENTITY_ONSPAWN(e) { if(npc_info[e->type].onSpawn) npc_info[e->type].onSpawn(e); }
#define ENTITY_ONFRAME(e) { if(npc_info[e->type].onFrame) npc_info[e->type].onFrame(e); }
#define ENTITY_ONDEATH(e) { if(npc_info[e->type].onDeath) npc_info[e->type].onDeath(e); }

typedef void (*EntityMethod)(Entity*);

// Special states
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

#define SNAP_TO_GROUND(e); ({                                                                  \
	u16 bx = sub_to_block(e->x);                                                               \
	u16 by = sub_to_block(e->y + ((e->hit_box.bottom+1)<<CSF));                                \
	if(stage_get_block_type(bx, by) != 0x41) {                                                 \
		e->y += 16 << CSF;                                                                     \
	} else {                                                                                   \
		by = sub_to_block(e->y + ((e->hit_box.bottom-1)<<CSF));                                \
		if(stage_get_block_type(bx, by) == 0x41) e->y -= 8 << CSF;                             \
	}                                                                                          \
})

#define FACE_PLAYER(e) (e->dir = e->x < player.x)
#define TURN_AROUND(e) (e->dir ^= 1)

#define PLAYER_DIST_X(dist) (player.x > e->x - (dist) && player.x < e->x + (dist))
#define PLAYER_DIST_Y(dist) (player.y > e->y - (dist) && player.y < e->y + (dist))
#define PLAYER_DIST_X2(dist1, dist2) (player.x > e->x - (dist1) && player.x < e->x + (dist2))
#define PLAYER_DIST_Y2(dist1, dist2) (player.y > e->y - (dist1) && player.y < e->y + (dist2))

#define LIMIT_X(v) ({                                                                          \
	if(e->x_speed > (v)) e->x_speed = (v);                                                     \
	if(e->x_speed < -(v)) e->x_speed = -(v);                                                   \
})
#define LIMIT_Y(v) ({                                                                          \
	if(e->y_speed > (v)) e->y_speed = (v);                                                     \
	if(e->y_speed < -(v)) e->y_speed = -(v);                                                   \
})

#define MOVE_X(v) (e->x_speed = e->dir ? (v) : -(v))
#define ACCEL_X(v) (e->x_speed += e->dir ? (v) : -(v))

#define CURLY_TARGET_HERE(e) ({                                                                \
	curly_target_time = 120;                                                                   \
	curly_target_x = e->x;                                                                     \
	curly_target_y = e->y;                                                                     \
})

#define FIRE_ANGLED_SHOT(type, xx, yy, angle, speed) ({                                        \
	Entity *shot = entity_create(xx, yy, (type), 0);                                           \
	shot->x_speed = (sintab32[(angle)] >> 1) * ((speed) >> CSF);                               \
	shot->y_speed = (sintab32[((angle) + 256) % 1024] >> 1) * ((speed) >> CSF);                \
})

#define SMOKE_AREA(x, y, w, h, count) ({                                                       \
	for(u8 i = 0; i < (count); i++) {                                                          \
		effect_create_smoke((x) + (random() % (w)),                                            \
							(y) + (random() % (h)));                                           \
	}                                                                                          \
})

#define ANIMATE(e, spd, ...) {                                                                 \
	const u8 anim[] = { __VA_ARGS__ };                                                         \
	(e)->animtime++;                                                                           \
	if((e)->animtime > spd * sizeof(anim)) (e)->animtime = 0;                                  \
	if((e)->animtime % sizeof(anim) == 0) (e)->frame = anim[(e)->animtime / spd];              \
}

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
void onspawn_snap(Entity *e);
// NPC will face right if NPC_OPTION2 is set
void onspawn_op2flip(Entity *e);
// Combination of the above
void onspawn_snapflip(Entity *e);
// NPC's sprite will start at the second animation if NPC_OPTION2 is set
void onspawn_op2anim(Entity *e);
// Only push down for NPC_OPTION2, used for the door after rescuing Kazuma
void onspawn_op2snap(Entity *e);
// Blackboard needs to be pushed up, and changes frame on NPC_OPTION2
void onspawn_blackboard(Entity *e);
// Always active
void onspawn_persistent(Entity *e);
// Rotates the spikes so they are always sticking out of a solid area
void onspawn_spike(Entity *e);
// Apply gravity & collision, some NPC's need to fall when the floor disappears beneath them
// See save points and refills in Sand Zone, Labyrinth
void ai_grav(Entity *e);
// When the player enters a trigger's target area, an event will begin automatically
void ai_trigger(Entity *e);
// Generic enemy projectile
void ai_genericproj(Entity *e);
// Default onState just explodes on death
void ondeath_default(Entity *e);
// Quote teleporting in
void onspawn_teleIn(Entity *e);
void ai_teleIn(Entity *e);
// Quote teleporting out
void onspawn_teleOut(Entity *e);
void ai_teleOut(Entity *e);
// TODO: Blue light that flashes while the teleporter is in use
void onspawn_teleLight(Entity *e);
void ai_teleLight(Entity *e);
void ondeath_teleLight(Entity *e);
// Quote NPC used in scenes where the actual player character is hidden
void ai_player(Entity *e);
void ondeath_player(Entity *e);

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
void onspawn_balfrog(Entity *e);
void ai_balfrog(Entity *e);
void ondeath_balfrog(Entity *e);
void onspawn_balrog(Entity *e);
void ai_balrog(Entity *e);
void ai_balrog_drop_in(Entity *e);
void ai_balrog_bust_in(Entity *e);
void ai_balrogRunning(Entity *e);
void ondeath_balrogRunning(Entity *e);
void ai_balrogFlying(Entity *e);
void ondeath_balrogFlying(Entity *e);
void ai_balrogShot(Entity *e);
void ai_balrog_boss_missiles(Entity *e);
void ai_balrog_missile(Entity *e);

/* Bats - bat.c */
void onspawn_batVertical(Entity *e);
void ai_batVertical(Entity *e);
void onspawn_batHang(Entity *e);
void ai_batHang(Entity *e);
void ai_batCircle(Entity *e);

/* Curly - curly.c */
void ai_curly(Entity *e);
void ai_curly_carried(Entity *e);
void ai_curlyBoss(Entity *e);
void ondeath_curlyBoss(Entity *e);
void ai_curlyBossShot(Entity *e);
void ai_curly_ai(Entity *e);
void ai_cai_gun(Entity *e);
void ai_cai_watershield(Entity *e);

/* Egg Corridor - eggs.c */
void onspawn_basil(Entity *e);
void ai_basil(Entity *e);
void ai_beetle(Entity *e);
void onspawn_beetleFollow(Entity *e);
void ai_beetleFollow(Entity *e);
void onspawn_basu(Entity *e);
void ai_basu(Entity *e);
void ai_behemoth(Entity *e);
void onspawn_lift(Entity *e);
void ai_lift(Entity *e);
void ai_terminal(Entity *e);

/* Igor - igor.c */
void onspawn_igor(Entity *e);
void ai_igor(Entity *e);
void ondeath_igor(Entity *e);
void ai_igorscene(Entity *e);
void ondeath_igorscene(Entity *e);
void ai_igordead(Entity *e);

/* Critters - critter.c */
void ai_critter(Entity *e);

/* Doors - door.c */
void onspawn_door(Entity *e);
void ai_door(Entity *e);
void ai_theDoor(Entity *e);
void ai_theDoor_onHurt(Entity *e);
void onspawn_doorway(Entity *e);
void ai_doorway(Entity *e);

/* Items & Treasure - item.c */
void onspawn_energy(Entity *e);
void ai_energy(Entity *e);
void ai_missile(Entity *e);
void ai_heart(Entity *e);
void onspawn_hiddenPowerup(Entity *e);
void ai_hiddenPowerup(Entity *e);

/* Fans - fan.c */
void onspawn_fan(Entity *e);
void ai_fan(Entity *e);

/* Minor Mimigas - mimiga.c */
void onspawn_flower(Entity *e);
void ai_jack(Entity *e);
void ai_santa(Entity* e);
void ai_chaco(Entity* e);

/* Misery - misery.c */
void ai_misery_float(Entity *e);
void ai_misery_bubble(Entity *e);

/* Toroko - toroko.c */
void onspawn_torokoAtk(Entity *e);
void ai_torokoAtk(Entity *e);
void ondeath_toroko(Entity *e);
void onspawn_torokoBoss(Entity *e);
void ai_torokoBoss(Entity *e);
void ondeath_torokoBoss(Entity *e);
void ai_torokoBlock(Entity *e);
void ai_torokoFlower(Entity *e);

/* Cemetery Enemies - cemetery.c */
void ai_pignon(Entity *e);
void onspawn_gkeeper(Entity *e);
void ai_gkeeper(Entity *e);

/* Grasstown - weed.c */
void ai_jelly(Entity *e);
void ai_kulala(Entity *e);
void ai_mannan(Entity *e);
void ondeath_mannan(Entity *e);
void ai_mannanShot(Entity *e);
void ai_malco(Entity *e);
void ondeath_malco(Entity *e);
void onspawn_malcoBroken(Entity *e);
void ai_malcoBroken(Entity *e);
void onspawn_powerc(Entity *e);
void ai_press(Entity *e);
void ai_frog(Entity *e);
void ai_hey(Entity *e);
void ai_motorbike(Entity *e);

/* Sand Zone - sand.c */
void onspawn_omega(Entity *e);
void ai_omega(Entity *e);
void ondeath_omega(Entity *e);
void onspawn_sunstone(Entity *e);
void ai_sunstone(Entity *e);
void ondeath_sunstone(Entity *e);
void onspawn_puppy(Entity *e);
void ai_puppy(Entity *e);
void onspawn_puppyCarry(Entity *e);
void ai_puppyCarry(Entity *e);
void ai_polish(Entity *e);
void ai_baby(Entity *e);
void ai_sandcroc(Entity *e);
void ai_skullhead(Entity *e);
void ai_crow(Entity *e);
void ai_curlys_mimigas(Entity *e);
void ai_armadillo(Entity *e);
void onspawn_jenka(Entity *e);

/* Labyrinth - maze.c */
void onspawn_block(Entity *e);
void ai_blockh(Entity *e);
void ai_blockv(Entity *e);
void ai_boulder(Entity *e);
void ai_gaudiDying(Entity *e);
void ai_gaudi(Entity *e);
void ai_gaudiFlying(Entity *e);
void ai_gaudiArmored(Entity *e);
void ai_gaudiArmoredShot(Entity *e);
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
void onspawn_core(Entity *e);
void ai_core(Entity *e);
void ondeath_core(Entity *e);
void ai_core_front(Entity *e);
void ai_core_back(Entity *e);
void ai_minicore(Entity *e);
void ai_minicore_shot(Entity *e);
void ai_core_ghostie(Entity *e);
void ai_core_blast(Entity *e);

#endif /* INC_AI_H_ */
