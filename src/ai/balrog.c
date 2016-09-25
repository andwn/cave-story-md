#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "camera.h"

// Repurpose unused x_mark and y_mark variables
#define balrog_smoking		x_mark
#define balrog_smoketimer	y_mark

void ai_balrog(Entity *e) {
	u8 fall = TRUE;
	e->y_next = e->y + e->y_speed;
	e->x_next = e->x + e->x_speed;
	
	if(!(e->eflags & NPC_IGNORESOLID)) {
		if(!e->grounded) e->grounded = collide_stage_floor(e);
		else e->grounded = collide_stage_floor_grounded(e);
	}

	switch(e->state) {
		case 0:
		{
			e->eflags &= ~NPC_IGNORESOLID;
			e->x_speed = 0;
			e->balrog_smoking = FALSE;
		}
		/* no break */
		case 1:
		{
			e->frame = 0;
			RANDBLINK(e, 4, 200);
		}
		break;
		case 10:		// he jumps and flys away
		{
			e->x_speed = 0;
			e->frame = 2;
			e->timer = 0;
			e->state++;
		}
		/* no break */
		case 11:
		{
			if (++e->timer <= TIME(20)) break;
			e->frame = 3;
			e->state++;
			e->y_speed = SPEED(-0x800);
			e->eflags |= NPC_IGNORESOLID;
		}
		/* no break */
		case 12:
		{
			fall = FALSE;
			e->y_speed -= SPEED(0x10);
			if (e->y < 0) {
				e->state = STATE_DELETE;
				sound_play(SND_QUAKE, 5);
				camera_shake(30);
			}
		}
		break;
		// he looks shocked and shakes, then flys away
		// used when he is "hit by something"
		case 20:
		{
			e->state = 21;
			e->frame = 5;
			e->x_speed = 0;
			e->timer = e->timer2 = 0;
			//SmokeClouds(e, 4, 8, 8);
			sound_play(SND_BIG_CRASH, 5);
			e->balrog_smoking = 1;
		}
		/* no break */
		case 21:
		{
			e->timer2++;
			e->x += ((e->timer2 >> 1) & 1) ? (1<<9) : -(1<<9);
			if (++e->timer > TIME(100)) {
				e->state = 10;
			}
			e->y_speed += SPEED(0x20);
			LIMIT_Y(SPEED(0x5FF));
		}
		break;
		case 30:	// he smiles for a moment
		{
			e->frame = 6;
			e->timer = 0;
			e->state = 31;
		}
		/* no break */
		case 31:
		{
			if (++e->timer > TIME(100)) {
				e->state = 0;
			}
		}
		break;
		case 40: // Spell cast before transforming into Balfrog
		{
			e->state = 41;
			e->frame = 5;
		}
		break;
		case 42:
		{
			e->timer = 0;
			e->state = 43;
			// Spawn the Balfrog boss
			bossEntity = entity_create(e->x, e->y, 360 + BOSS_BALFROG, 0);
			bossEntity->event = 1000;
		}
		/* no break */
		case 43:
		{
			// flashing visibility
			// (transforming into Balfrog stage boss;
			//	our flashing is interlaced with his)
			e->timer++;
			e->hidden = (e->timer & 2) > 0;
		}
		break;
		case 50:	// he faces away
		{
			e->frame = 8;
			e->x_speed = 0;
		}
		break;
		case 60:	// he walks
		{
			e->state = 61;
			e->frame = 1;
		}
		/* no break */
		case 61:
		{
			MOVE_X(SPEED(0x200));
		}
		break;
		// he is teleported away (looking distressed)
		// this is when he is sent to Labyrinth at end of Sand Zone
		case 70:
		{
			e->x_speed = 0;
			e->timer = 0;
			e->frame = 7;
			e->state++;
		}
		/* no break */
		case 71:
		{
			fall = FALSE;
			if(++e->timer > 120) {
				e->state = STATE_DELETE;
				return;
			}
			e->hidden = (e->timer & 2) > 0;
		}
		break;
		case 80:	// hands up and shakes
		{
			e->frame = 5;
			e->state = 81;
		}
		/* no break */
		case 81:
		{
			fall = FALSE;
			if (++e->timer & 2) {
				e->x += (1 << 9);
			} else {
				e->x -= (1 << 9);
			}
		}
		break;
		// fly up and lift Curly & PNPC
		// (post-Ballos ending scene)
		case 100:
		{
			e->state = 101;
			e->timer = 0;
			e->frame = 2;	// prepare for jump;
		}
		/* no break */
		case 101:
		{
			if (++e->timer > TIME(20)) {
				e->state = 102;
				e->timer = 0;
				e->frame = 3;	// fly up;
				entities_clear_by_type(OBJ_NPC_PLAYER);
				entities_clear_by_type(OBJ_CURLY);
				// TODO: OBJ_BALROG_PASSENGER
				//CreateEntity(0, 0, OBJ_BALROG_PASSENGER, 0, 0, LEFT)->linkedobject = o;
				//CreateEntity(0, 0, OBJ_BALROG_PASSENGER, 0, 0, RIGHT)->linkedobject = o;
				e->y_speed = SPEED(-0x800);
				e->eflags |= NPC_IGNORESOLID;	// so can fly through ceiling
				fall = FALSE;
			}
		}
		break;
		case 102:	// flying up during escape seq
		{
			fall = FALSE;
			// bust through ceiling
			u16 y = sub_to_block(e->y + (4<<9));
			if (y < 35) {
				if (stage_get_block(sub_to_block(e->x), y) != 0) {
					// smoke needs to go at the bottom of z-order or you can't
					// see any of the characters through all the smoke.
					//map_ChangeTileWithSmoke(x, y, 0, 4, FALSE, lowestobject);
					//map_ChangeTileWithSmoke(x-1, y, 0, 4, FALSE, lowestobject);
					//map_ChangeTileWithSmoke(x+1, y, 0, 4, FALSE, lowestobject);
					camera_shake(10);
					sound_play(SND_MISSILE_HIT, 5);
				}
			}
			if (e->y + pixel_to_sub(e->hit_box.bottom) < -pixel_to_sub(20)) {
				camera_shake(30);
				e->state = STATE_DELETE;
				return;
			}
		}
		break;
		case 500:	// used during Balfrog death scene
		{
			fall = FALSE;
		}
		break;
	}

	e->x = e->x_next;
	e->y = e->y_next;

	if (e->balrog_smoking) {
		if (++e->balrog_smoketimer > 20 || !(random() % 16)) {
			//SmokeClouds(e, 1, 4, 4);
			e->balrog_smoketimer = 0;
		}
	}

	if (fall) {
		if (!e->grounded) e->y_speed += SPEED(0x20);
		LIMIT_Y(SPEED(0x5FF));
	}
}

void ai_balrog_drop_in(Entity *e) {
	if(!e->grounded) e->y_speed += SPEED(0x20);
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;

	switch(e->state) {
		case 0:
		{
			e->eflags &= ~NPC_IGNORESOLID;
			e->state = 1;
			e->grounded = FALSE;
			e->frame = 3;	// falling;
		}
		/* no break */
		case 1:
		{
			// since balrog often falls through the ceiling we must wait until he is 
			// free-falling before we start checking to see if he hit the floor
			u16 x = sub_to_block(e->x), y = sub_to_block(e->y);
			if(((stage_get_block_type(x, y-1) | stage_get_block_type(x, y+1)) & 0x41) != 0x41) {
				e->state = 2;
			}
		}
		break;
		case 2:	// free-falling
		{
			if ((e->grounded = collide_stage_floor(e))) {
				e->frame = 2;
				e->state = 3;
				e->timer = 0;

				//SmokeSide(o, 4, DOWN);
				camera_shake(30);
			}
		}
		break;
		case 3:	// landed
		{
			if (++e->timer > TIME(20)) {
				e->state = 4;
				e->frame = 0;
			}
		}
		break;
	}

	e->x = e->x_next;
	e->y = e->y_next;
}

// Balrog busting in the door of the Shack.
// he exists like this for only a moment, then the script
// changes him to a standard OBJ_BALROG.
void ai_balrog_bust_in(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->y += (2 << CSF);
			e->y_speed = SPEED(-0x100);
			camera_shake(30);
			e->state = 1;
			e->frame = 3;
		}
		/* no break */
		case 1:		// falling the short distance to ground
		{
			e->y_speed += SPEED(0x10);
			e->y_next = e->y + e->y_speed;
			e->x_next = e->x + e->x_speed;
			LIMIT_Y(SPEED(0x5FF));
			if (e->y_speed > 0 && (e->grounded = collide_stage_floor(e))) {
				e->state = 2;
				e->frame = 2;
				e->timer = 0;
				camera_shake(30);
			}
			e->x = e->x_next;
			e->y = e->y_next;
		}
		break;
		// landing animation
		case 2:
		{
			if (++e->timer > 16) {
				e->state = 3;
				e->frame = 0;
			}
		}
		break;
		// standing
		case 3:
		case 4: break;
	}
}

// 68 - Boss: Balrog (Mimiga Village)
void ai_balrogRunning(Entity *e) {
	
}

void ondeath_balrogRunning(Entity *e) {
	e->x_speed = 0;
	e->eflags &= ~NPC_SHOOTABLE;
	e->attack = 0;
	tsc_call_event(e->event);
}

void ai_balrogFlying(Entity *e) {
	enum {
		WAIT_BEGIN = 0,
		SHOOT_PLAYER,
		JUMP_BEGIN,
		JUMP_UP,
		FLYING,
		JUMP_END,
		LANDED
	};
	// Physics stuff
	if(e->state != FLYING) e->y_speed += 0x33;
	if(e->y_speed > 0x5FF) e->y_speed = 0x5FF;
	else if(e->y_speed < -0x5FF) e->y_speed = -0x5FF;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	switch(e->state) {
		case WAIT_BEGIN:	// wait at start of battle
			FACE_PLAYER(e);
			if(++e->timer > 12) {
				e->state = SHOOT_PLAYER;
				e->timer = 0;
				e->frame = 2;
			}
		break;
		case SHOOT_PLAYER:
			e->timer++;
			FACE_PLAYER(e);
			if((e->timer % 16) == 15) {
				e->frame = 2;
				// Fire shot
				Entity *shot = entity_create(e->x, e->y, OBJ_IGOR_SHOT, 0);
				shot->x_speed = e->dir ? 0x400 : -0x400;
				shot->y_speed = -0x100 + (random() % 0x200);

				sound_play(SND_EM_FIRE, 5);
				if(e->timer > 32) {	// 3 shots
					e->state = JUMP_BEGIN;
					e->timer = 0;
				}
			}
		break;
		case JUMP_BEGIN:	// begin jump
			e->timer++;
			FACE_PLAYER(e);
			if(e->timer > 3) {
				e->state = JUMP_UP;
				e->timer = 0;
				e->x_speed = (player.x - e->x) / 128;
				e->y_speed = -0x600;
				e->frame = 3;
			}
		break;
		case JUMP_UP:		// jumping up
			if(e->y_speed > 0x200) {
				if(e->health <= 60) {	
					// skip flying if low health
					e->state = JUMP_END;
				} else {
					e->state = FLYING;
					e->timer = 0;
					e->y_mark = e->y;
					e->frame = 9;
				}
			}
		break;
		case FLYING:
			e->timer++;
			if(e->timer % 16 == 0) {
				sound_play(SND_EXPLOSION2, 3);
			}
			if(e->timer >= 128) {
				e->state = JUMP_END;
				e->frame = 3;
			}
			e->y_speed += (e->y >= e->y_mark) ? -0x40 : 0x40;
			if(e->y_speed > 0x200) e->y_speed = 0x200;
			else if(e->y_speed < -0x200) e->y_speed = -0x200;
		break;
		case JUMP_END:		// coming down from jump
			if(e->y + pixel_to_sub(16) < player.y) {
				e->attack = 10;
			} else {
				e->attack = 0;
			}
			e->grounded = collide_stage_floor(e);
			if(e->grounded) {
				e->x_speed = 0;
				e->attack = 0;
				sound_play(SND_FUNNY_EXPLODE, 5);
				camera_shake(30);
				for(int i=0;i<4;i++) {
					Entity *shot = entity_create(e->x, e->y, OBJ_BALROG_SHOT_BOUNCE, 0);
					shot->x_speed = -0x400 + (random() % 0x800);
					shot->y_speed = -0x400 + (random() % 0x400);
				}
				e->state = LANDED;
				e->timer = 0;
				e->frame = 0;
			}
		break;
		case LANDED:
			if(++e->timer > 3) {
				e->state = SHOOT_PLAYER;
				e->timer = 0;
			}
		break;
	}
	entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

void ondeath_balrogFlying(Entity *e) {
	e->x_speed = 0;
	e->eflags &= ~NPC_SHOOTABLE;
	e->attack = 0;
	entities_clear_by_type(OBJ_BALROG_SHOT_BOUNCE);
	entities_clear_by_type(OBJ_IGOR_SHOT);
	tsc_call_event(e->event);
}

void ai_balrogShot(Entity *e) {
	if(!e->state) {
		e->eflags |= NPC_SHOOTABLE | NPC_SHOWDAMAGE;
		e->health = 1000;
		e->state++;
	}
	if(e->damage_time) {
		e->state = STATE_DELETE;
		return;
	}
	e->y_speed += SPEED(42);
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(e->y_speed > 0 && collide_stage_floor(e)) {
		e->y_speed = -SPEED(0x400);
	}
	if((e->x_speed > 0 && collide_stage_rightwall(e)) ||
		(e->x_speed < 0 && collide_stage_leftwall(e)) ||
		++e->timer > TIME(250)) {
		e->state = STATE_DELETE;
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

#define STATE_CHARGE			10
#define STATE_JUMP_FIRE			20
#define STATE_PAUSE				30
#define STATE_CAUGHT_PLAYER		40

void ai_balrog_boss_missiles(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	// try to catch player
	switch(e->state) {
		case STATE_CHARGE+1:
		case STATE_JUMP_FIRE+1:
		{
			if (PLAYER_DIST_X(12<<CSF) && PLAYER_DIST_Y2(12<<CSF, 8<<CSF)) {
				//balrog_grab_player(e);
				//hurtplayer(5);
				e->state = STATE_CAUGHT_PLAYER;
			}
		}
		break;
	}
	// main state engine
	switch(e->state) {
		case 0:
		{
			FACE_PLAYER(e);
			e->state = 1;
			e->frame = 0;
			e->timer = 0;
		}
		/* no break */
		case 1:
		{
			if (++e->timer > TIME(30)) {
				e->state = STATE_CHARGE;
				e->timer2 ^= 1;	// affects how we react if we miss the player
			}
		}
		break;
		// charge the player
		case STATE_CHARGE:
		{
			e->timer = 0;
			e->frame = 9;
			e->animtime = 0;
			e->state++;
		}
		/* no break */
		case STATE_CHARGE+1:
		{
			e->x_speed += e->dir ? SPEED(0x20) : -SPEED(0x20);
			//walking_animation(o);
			
			// stuck against the wall?
			if((!e->dir && collide_stage_leftwall(e)) || 
				(e->dir && collide_stage_rightwall(e))) {
				if (++e->x_mark > 5) e->state = STATE_JUMP_FIRE;
			} else {
				// Use x_mark instead of timer3
				e->x_mark = 0;
			}
			// he behaves differently after every other time he pauses
			if (e->timer2) {
				if (++e->timer > TIME(75)) {
					e->frame = 0;;
					e->state = STATE_PAUSE;
				}
			} else {
				if (++e->timer > TIME(24)) e->state = STATE_JUMP_FIRE;
			}
		}
		break;
		
		// jump and fire missiles
		case STATE_JUMP_FIRE:
		{
			e->state++;
			e->timer = 0;
			e->frame = 3;;
			e->y_speed = -SPEED(0x5ff);
		}
		/* no break */
		case STATE_JUMP_FIRE+1:
		{
			FACE_PLAYER(e);
			// fire missiles
			if (++e->timer < 30) {
				if ((e->timer % 6) == 1) {
					sound_play(SND_EM_FIRE, 5);
					//Entity *shot = SpawnEntityAtActionPoint(o, OBJ_BALROG_MISSILE);
					//shot->dir = e->dir;
					//shot->xinertia = 0x100;
				}
			}
			// landed?
			if (e->y_speed >= 0 && (e->grounded = collide_stage_floor(e))){
				e->frame = 2;
				e->state = STATE_PAUSE;
				camera_shake(30);
			}
		}
		break;
		// stop for a moment
		case STATE_PAUSE:
		{
			e->x_speed *= 4;
			e->x_speed /= 5;
			if (e->x_speed != 0) break;
			e->state = 0;
		}
		break;
		case STATE_CAUGHT_PLAYER:	// caught player
		{
			//if (balrog_toss_player_away(o))
				e->state = 0;
		}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
	if(!e->grounded) e->y_speed += 0x20;
	LIMIT_X(0x300);
	LIMIT_Y(0x5ff);
}

void ai_balrog_missile(Entity *e) {
	if ((e->dir == 1 && collide_stage_rightwall(e)) || \
		(e->dir == 0 && collide_stage_leftwall(e)))
	{
		//SmokeClouds(o, 3, 0, 0);
		//effect(e->CenterX(), e->CenterY(), EFFECT_BOOMFLASH);
		sound_play(SND_MISSILE_HIT, 5);
		
		e->state = STATE_DELETE;
		return;
	}
	
	if (e->state == 0)
	{
		// recoil in oppisite direction
		e->x_speed = random(-2, -1) << 9;
		if (e->dir == 0) e->x_speed = -e->x_speed;
		
		e->y_speed = random(-2, 0) << 9;
		
		e->state = 1;
	}
	
	e->x_speed += e->dir ? 0x20 : -0x20;
	
	//if ((++e->timer2 % 4) == 1)
	//{
	//	effect(e->CenterX() - e->x_speed, e->CenterY(), EFFECT_SMOKETRAIL_SLOW);
	//}
	
	// heat-seeking at start, then level out straight
	if (e->timer2 < 50)
	{
		if (e->y < player.y)
			e->y_speed += 0x20;
		else
			e->y_speed -= 0x20;
	}
	else
	{
		e->y_speed = 0;
	}
	
	// flash
	//e->frame ^= 1;
	
	if (e->x_speed < -0x400)
		e->x_speed = -0x600;
	
	if (e->x_speed > 0x400)
		e->x_speed = 0x600;
}
