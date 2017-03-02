#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "vdp_ext.h"
#include "effect.h"
#include "camera.h"

enum STATES {
	// 0-9 are used by in-game scripts during intro
	// 500+ is triggered for defeat sequence
	
	STATE_BASE				= 10,		// fall; once on ground for a moment pick next attack
	
	STATE_JUMP				= 20,		// jumps at player
	STATE_JUMP_WITH_GP		= 30,		// taller jump and enable "ground pound"
	STATE_IN_AIR			= 40,		// in air
	STATE_IN_AIR_WITH_GP	= 50,		// in air and will "ground pound" if passes over player
	STATE_LANDED			= 60,		// landed from a jump
	
	STATE_RED_DASH			= 70,		// red dash; try to hit player with elbow
	STATE_MEGA_BATS			= 80,		// arms out & spawn mega bats
	
	STATE_TELEPORT			= 90,		// teleport away and reappear over players head
	
	STATE_DEFEATED			= 500,		// script constant
	STATE_DISSOLVE			= 510		// also scripted
};

#define DAMAGE_NORMAL		5
#define DAMAGE_RED_DASH		10

#define savedhp		id
#define angle		jump_time

// the Doctor repeats this series of attacks over and over,
// when he reaches the -1, it starts over at the beginning.
static const u16 attack_pattern[] = {
	STATE_JUMP_WITH_GP,
	STATE_TELEPORT,
	STATE_RED_DASH,
	STATE_JUMP,
	STATE_JUMP,
	STATE_RED_DASH,
	STATE_TELEPORT,
	STATE_MEGA_BATS,
	STATE_JUMP_WITH_GP,
	STATE_JUMP,
	STATE_JUMP,
	0xFFFF,
};

// Prototypes
static void run_red_drip(Entity *e);
static void do_redsplode(Entity *e);

void ai_muscle_doctor(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	u8 blockl = e->x_speed < 0 && collide_stage_leftwall(e);
	u8 blockr = e->x_speed > 0 && collide_stage_rightwall(e);
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);
	
	switch(e->state) {
		// INIT
		
		case 0:
		{
			// put ourselves at correct position over the other doctor
			e->dir = (crystal_xmark < player.x);
			
			e->x = crystal_xmark + ((e->dir) ? (6<<CSF) : -(6<<CSF));
			e->y = crystal_ymark;
			
			// make sure we're front of other doctor
			//e->BringToFront();
			// make sure front crystal is in front of us
			//crystal_tofront = TRUE;
			
			// because we moved our x/y directly
			//e->UpdateBlockStates(ALLDIRMASK);
			e->state = 1;
		}
		case 1:		// appearing/transforming
		{
			//e->y_speed += SPEED(0x80);
			
			e->timer++;
			e->frame = (e->timer & 2) ? 0 : 3;
		}
		break;
		// standing idle & panting
		// this state doesn't seem to ever be used;
		// AFAIK can only be triggered by modifying the script.
		case 5:
		{
			e->frame = 1;
			e->animtime = 0;
			e->state = 6;
		}
		case 6:
		{
			//e->y_speed += SPEED(0x80);
			ANIMATE(e, 30, 1,2);
		}
		break;
		
		case 7: 	// prepare-to-fight pause
		{
			e->state = 8;
			e->timer = 0;
			e->frame = 3;
		}
		case 8:
		{
			//e->y_speed += SPEED(0x40);
			
			if (++e->timer > TIME(40)) e->state = STATE_BASE;
		}
		break;
		
		// BASE
		
		case STATE_BASE:
		{
			e->eflags |= NPC_SHOOTABLE;
			e->eflags &= ~NPC_INVINCIBLE;
			
			e->x_speed = 0;
			e->frame = 1;
			
			e->timer = 0;
			e->animtime = 0;
			
			e->savedhp = e->health;
			e->state++;
		}
		case STATE_BASE+1:
		{
			e->y_speed += SPEED(0x80);
			FACE_PLAYER(e);
			
			// select frame
			if (e->frame == 6) { // arms out
				// must have done "redsplode" attack; leave alone for the duration
			} else if (!e->grounded) {	// falling
				e->frame = 4;
			} else {	// panting animation
				ANIMATE(e, 12, 1,2);
				
				// "redsplode" attack if possible
				if ((e->savedhp - e->health) > 20) {
					if (PLAYER_DIST_X(48<<CSF) && player.grounded)
						do_redsplode(e);
				}
			}
			
			// after a moment select next attack in the loop
			if (++e->timer > 30 || (e->savedhp - e->health) > 20) {
				e->state = attack_pattern[e->timer2];
				e->timer = 0;
				
				// move to next state
				if (attack_pattern[++e->timer2] == 0xFFFF) e->timer2 = 0;
			}
		}
		break;
		
		// RUN & JUMP
		
		// jump at player
		case STATE_JUMP:
		{
			e->frame = 3;
			FACE_PLAYER(e);
			
			if (++e->timer > TIME(20)) {
				e->grounded = FALSE;
				e->state = STATE_IN_AIR;
				e->frame = 4;
				e->animtime = 0;
				
				e->y_speed = -SPEED(0x600);
				MOVE_X(SPEED(0x400));
			}
		}
		break;
		
		// slight taller jump with no delay, and can "ground pound"
		case STATE_JUMP_WITH_GP:
		{
			FACE_PLAYER(e);
			e->grounded = FALSE;
			e->state = STATE_IN_AIR_WITH_GP;
			e->frame = 4;
			e->animtime = 0;
			
			e->y_speed = -SPEED(0x800);
			MOVE_X(SPEED(0x400));
		}
		break;
		
		case STATE_IN_AIR:			// in air (normal)
		case STATE_IN_AIR_WITH_GP:	// in air; can "ground pound" if we pass over player
		{
			ANIMATE(e, 1, 4, 5);
			e->y_speed += SPEED(0x40);
			
			if (e->state == STATE_IN_AIR_WITH_GP) {
				if (PLAYER_DIST_X(8<<CSF) && player.y >= e->y_next) {
					e->x_speed = 0;
					e->y_speed = SPEED(0x5ff);
					e->state = STATE_IN_AIR;
				}
			} else {
				FACE_PLAYER(e);
			}
			
			if (e->grounded) e->state = STATE_LANDED;
		}
		break;
		
		// landed from jump
		case STATE_LANDED:
		{
			e->frame = 3;
			camera_shake(10);
			
			e->timer = 0;
			e->state++;
		}
		case STATE_LANDED+1:
		{
			//e->y_speed += 0x80;
			
			e->x_speed += (e->x_speed << 2) + (e->x_speed << 1);
			e->x_speed >>= 3;
			
			if (++e->timer > TIME(10)) e->state = STATE_BASE;
		}
		break;
		
		// DASH
		
		case STATE_RED_DASH:
		{
			e->frame = 3;	// crouch down; look mean
			e->eflags |= (NPC_SOLID | NPC_INVINCIBLE);
			
			e->timer = 0;
			e->state++;
		}
		case STATE_RED_DASH+1:
		{
			if (++e->timer > TIME(20)) {
				e->frame = 7;		// elbow-out dash frame
				e->timer = 0;
				e->state++;
				
				sound_play(SND_FUNNY_EXPLODE, 5);
				MOVE_X(SPEED(0x5ff));
				
				e->attack = DAMAGE_RED_DASH;
				e->eflags |= NPC_FRONTATKONLY;
				e->y_speed = 0;	// he does not fall during dash
			}
		}
		break;
		// doing red dash
		case STATE_RED_DASH+2:
		{
			e->y_speed = 0;	// he does not fall during dash
			// flash red
			e->timer++;
			e->frame = (e->timer & 2) ? 7 : 8;
			
			// time to stop?
			if (blockl || blockr || e->timer > TIME(30)) {
				if (e->timer > TIME(30))		// stopped because timeout
					e->state++;
				else					// stopped because hit a wall
					e->state = STATE_JUMP;
				
				e->eflags &= ~(NPC_SOLID | NPC_FRONTATKONLY | NPC_INVINCIBLE);
				e->attack = DAMAGE_NORMAL;
				e->timer = 0;
			}
		}
		break;
		// dash ending due to timeout
		case STATE_RED_DASH+3:
		{
			e->grounded = FALSE;
			//e->y_speed += 0x80;
			e->frame = 3;
			
			e->x_speed += (e->x_speed << 2) + (e->x_speed << 1);
			e->x_speed >>= 3;
			
			if (++e->timer > TIME(10)) e->state = STATE_BASE;
		}
		break;
		
		// MEGA BATS
		
		case STATE_MEGA_BATS:
		{
			e->state++;
			e->timer = 0;
		}
		case STATE_MEGA_BATS+1:
		{
			e->frame = 6;
			e->timer++;
			
			if (e->timer > TIME(20) && (e->timer & 7) == 1) {
				Entity *bat = entity_create(e->x + (8<<CSF),
										   e->y - (4<<CSF), OBJ_DOCTOR_BAT, 0);
				
				bat->health <<= 1;
				bat->x_speed = 0x400 + (random() & 0x3FF);
				bat->y_speed = -0x200 + (random() & 0x3FF);
				bat->dir = e->dir;
				
				if (!e->dir) {
					bat->x -= (16 << CSF);
					bat->x_speed = -bat->x_speed;
				}
				
				sound_play(SND_EM_FIRE, 3);
			}
			
			if (e->timer > TIME(90)) e->state = STATE_BASE;
		}
		break;
		
		// TELEPORT
		
		case STATE_TELEPORT:
		{
			e->eflags &= ~NPC_SHOOTABLE;
			e->attack = 0;
			
			e->state++;
			//dr_tp_out_init(o);
		}
		case STATE_TELEPORT+1:
		{
			//if (dr_tp_out(o))
			//{
				e->state++;
				e->timer = 0;
				
				// mark the location above player's head where we'll reappear
				e->x_mark = player.x;
				e->y_mark = player.y - (32<<CSF);
				
				// don't be fooled into going off bounds of map
				#define TP_X_MIN	((4 * 16) << CSF)
				#define TP_X_MAX	((36 * 16) << CSF)
				#define TP_Y_MIN	((4 * 16) << CSF)
				
				if (e->x_mark < TP_X_MIN) e->x_mark = TP_X_MIN;
				if (e->x_mark > TP_X_MAX) e->x_mark = TP_X_MAX;
				if (e->y_mark < TP_Y_MIN) e->y_mark = TP_Y_MIN;
			//}
		}
		break;
		// invisible...waiting to reappear
		case STATE_TELEPORT+2:
		{
			if (++e->timer > 40) {
				e->x_next = e->x_mark;
				e->y_next = e->y_mark;
				e->frame = 4;
				
				FACE_PLAYER(e);
				
				e->state++;
			}
		}
		break;
		// reappear
		case STATE_TELEPORT+3:
		{
			//dr_tp_in_init(o);
			e->y_speed = 0;
			e->state++;
		}
		case STATE_TELEPORT+4:
		{
			//if (dr_tp_in(o))
			//{
				e->eflags |= NPC_SHOOTABLE;
				e->attack = DAMAGE_NORMAL;
				
				e->grounded = FALSE;
				e->x_speed = 0;
				e->y_speed = -SPEED(0x200);
				e->state = STATE_IN_AIR;
			//}
		}
		break;
		
		// DEFEAT
		
		// Defeated!
		case STATE_DEFEATED:
		{
			entities_clear_by_type(OBJ_DOCTOR_BAT);
			e->eflags &= ~NPC_SHOOTABLE;
			e->grounded = FALSE;
			e->attack = 0;
			e->frame = 4;
			e->x_speed = 0;
			e->state++;
		}
		case STATE_DEFEATED+1:		// wait till we hit ground
		{
			//e->y_speed += SPEED(0x20);
			
			if (e->grounded) {
				e->state++;
				e->timer = 0;
				e->x_mark = e->x_next;
				FACE_PLAYER(e);
			}
		}
		break;
		// shaking (script tells us when to stop)
		case STATE_DEFEATED+2:
		{
			e->frame = 9;
			e->timer++;
			
			e->x_next = e->x_mark;
			if (!(e->timer & 2)) e->x_next += (1 << CSF);
		}
		break;
		// dissolve into red energy
		// we already did this once before; think Pooh Black.
		case STATE_DISSOLVE:
		{
			e->frame = 9;
			e->x_next = e->x_mark;
			
			//e->ResetClip();
			//e->clip_enable = TRUE;
			
			e->state++;
			e->timer = 0;
		}
		case STATE_DISSOLVE+1:
		{
			e->timer++;
			
			// shaking
			e->x_next = e->x_mark;
			if (!(e->timer & 2)) e->x_next += (1 << CSF);
			
			camera_shake(2);
			
			// sound
			if ((e->timer & 7) == 3)
				sound_play(SND_FUNNY_EXPLODE, 5);
			
			// move energy spawn point
			if (++e->timer2 >= 8) {
				e->timer2 = 0;
				
				//e->clipy1++;
				//if (e->clipy1 >= sprites[e->sprite].h)
					e->hidden = TRUE;
			}
			
			// spawn copious amount of energy
			//for(int i=0;i<3;i++) {
				//int x, y;
				
				s32 x = e->x_next + ((-16 + (random() & 31)) << CSF);
				//y = e->y + (e->clipy1 << CSF);
				
				Entity *drip = entity_create(x, e->y, OBJ_RED_ENERGY, 0);
				
				drip->x_speed = -0x200 + (random() & 0x3FF);
				drip->y_speed = -(random() & 0x3FF);
				drip->angle = A_DOWN;
				// otherwise during the last few frames they'll get stuck in the floor
				// (they still delete themselves once they hit the floor, just are
				// able to come up out of it then back down during last few moments).
				drip->eflags |= NPC_IGNORESOLID;
			//}
			
			// he doesn't take up the entire height of the sprite,
			// so we stop a little bit early.
			//if (e->clipy1 >= 44)
			//{
				e->hidden = TRUE;
				e->frame = 0;
				e->state++;
			//}
		}
		break;
		// script: crystal up and away
		case 520:
		{
			crystal_ymark = -(32 << CSF);
		}
		break;
	}
	
	if (e->state < STATE_DISSOLVE) run_red_drip(e);
	
	// set crystal follow position
	// still set it on first 2 DEFEATED states (falling to ground)
	// but not after that (alert: this seems pretty damn bug prone, fixme)
	if (e->state >= STATE_BASE && e->state <= STATE_DEFEATED+1) {
		if (e->hidden)	// in middle of teleport: after tp out, before tp in
		{
			crystal_xmark = e->x_mark;
			crystal_ymark = e->y_mark;
		} else {
			crystal_xmark = e->x_next;
			crystal_ymark = e->y_next;
		}
	}
	
	if (!e->grounded) e->y_speed += SPEED(0x80);
	if (e->y_speed > SPEED(0x5ff)) e->y_speed = SPEED(0x5ff);
	
	e->x = e->x_next;
	e->y = e->y_next;
}

// this is a lesser-seen attack in which he pushes you away amongst
// a shower of red sparks. To trigger it, immediately after he lands
// you must walk directly up to him and deal more than 20 damage.
static void do_redsplode(Entity *e) {
	// arms out full
	e->frame = 6;
	FACE_PLAYER(e);
	
	player.y_speed = -SPEED(0x400);
	player.x_speed = (e->x_next > player.x) ? -SPEED(0x5ff) : SPEED(0x5ff);
	
	player_inflict_damage(5);
	camera_shake(10);
	
	// big shower of red energy
	for(int i=0;i<32;i++) {
		int x = e->x_next + ((-16 + (random() & 31)) << CSF);
		int y = e->y_next + ((-16 + (random() & 31)) << CSF);
		
		Entity *spark = entity_create(x, y, OBJ_RED_ENERGY, 0);
		
		spark->x_speed = -0x3FF + (random() & 0x7FF);
		spark->y_speed = -0x3FF + (random() & 0x7FF);
		spark->angle = A_DOWN;
	}
}

// the red energy that oozes off of him during most of the battle
static void run_red_drip(Entity *e) {
	if ((random() & 7) == 2) {
		int x = e->x_next + ((-16 + (random() & 31)) << CSF);
		int y = e->y_next + ((-8 + (random() % 12)) << CSF);
		
		Entity *drip = entity_create(x, y, OBJ_RED_ENERGY, 0);
		drip->x_speed = e->x_speed;
		drip->angle = A_DOWN;
	}
}

void ai_doctor_bat(Entity *e) {
	ANIMATE(e, 2, 0,1,2);
	
	if ((blk(e->x, -8, e->y, 0) == 0x41) || (blk(e->x, 8, e->y, 0) == 0x41)) {
		e->dir ^= 1;
		e->x_speed = -e->x_speed;
	}
	else if ((blk(e->x, 0, e->y, -8) == 0x41) || (blk(e->x, 0, e->y, 8) == 0x41)) {
		e->y_speed = -e->y_speed;
	}
	
	e->x += e->x_speed;
	e->y += e->y_speed;
}
