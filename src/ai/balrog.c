#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "camera.h"

// 12 - Balrog (Cutscene)
void ai_balrog_onCreate(Entity *e) {
	e->alwaysActive = true;
}

void ai_balrog_onUpdate(Entity *e) {
	if(e->state_time > 0) {
		e->state_time--;
		if(e->state_time == 0) {
			switch(e->state) {
			case 20:
			case 21: ENTITY_SET_STATE(e, 10, 0); break;
			case 30: ENTITY_SET_STATE(e, 0, 0); break;
			default: break;
			}
		}
	}
	if(!e->grounded) e->y_speed += GRAVITY_JUMP;
	e->y_next = e->y + e->y_speed;
	e->x_next = e->x + e->x_speed;
	if(e->state != 10 && e->y > block_to_sub(6)) entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_balrog_onState(Entity *e) {
	switch(e->state) {
	case 0: // Standing around
		SPR_SAFEANIM(e->sprite, 0);
		break;
	case 10: // Going up!
	case 11:
		SPR_SAFEANIM(e->sprite, 3);
		e->y_speed = pixel_to_sub(-2);
		break;
	case 20: // Smoking, going up!
	case 21:
		SPR_SAFEANIM(e->sprite, 5);
		e->state_time = 160;
		break;
	case 30: // Smile
		SPR_SAFEANIM(e->sprite, 6);
		e->state_time = 90;
		break;
	case 42: // Balfrog
		entity_create_boss(sub_to_block(e->x), sub_to_block(e->y), BOSS_BALFROG, 1000);
		break;
	case 70: // Vanish
	case 71:
		ENTITY_SET_STATE(e, STATE_DELETE, 0);
		break;
	}
}

// 68 - Boss: Balrog (Mimiga Village)
void ai_balrogRunning_onUpdate(Entity *e) {
	if(e->state_time > 0) e->state_time--;
	switch(e->state) {
	case 0:
	case 2:
	case 4:
		if(e->x_speed > 0) e->x_speed -= 0x10;
		else if(e->x_speed < 0) e->x_speed += 0x10;
		if(abs(e->x_speed) < 0x10) {
			e->x_speed = 0;
			SPR_setAnim(e->sprite, 0);
		}
		if(e->state_time == 0) ENTITY_SET_STATE(e, e->state + 1, 0);
		break;
	case 1:
	case 3:
	case 5:
		e->x_speed -= 0x10 - (0x20 * e->direction);
		if(e->state_time % 16 == 14) sound_play(SND_THUD, 3);
		if(e->state_time == 0 || (abs(e->x - player.x) < block_to_sub(2)))
			ENTITY_SET_STATE(e, e->state + 1, 0);
		break;
	case 6:
		if(e->grounded) {
			e->x_speed >>= 1;
			camera_shake(30);
			ENTITY_SET_STATE(e, 0, 0);
		} else e->y_speed += GRAVITY_JUMP;
		break;
	case 7: // Grabbed player
		player.x = e->x;
		player.y = e->y;
		if(!e->grounded) e->y_speed += GRAVITY_JUMP;
		if(e->state_time == 0) ENTITY_SET_STATE(e, 8, 0);
		break;
	case 8:
		if(e->state_time == 0) ENTITY_SET_STATE(e, 0, 0);
		break;
	default:
		break;
	}
	// Grab/throw player
	if(e->state < 7 && !player_invincible() && entity_overlapping(&player, e)) {
		ENTITY_SET_STATE(e, 7, 0);
	}
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_balrogRunning_onState(Entity *e) {
	if(e->state == STATE_DEFEATED) {
		tsc_call_event(e->event); // Boss defeated event
		return;
	}
	switch(e->state) {
	case 0: // Stand still
	case 2:
	case 4:
		e->state_time = 160;
		break;
	case 1: // Run towards player
	case 3: // Run towards player (2)
	case 5: // Run towards player and jump
		e->direction = e->x < player.x;
		SPR_SAFEANIM(e->sprite, 1);
		SPR_setHFlip(e->sprite, e->direction);
		e->state_time = 120;
		break;
	case 6: // Jumping
		e->grounded = false;
		e->y_speed = pixel_to_sub(-2);
		SPR_SAFEANIM(e->sprite, 3);
		e->state_time = 160;
		break;
	case 7: // Grab player
		player_lock_controls();
		player.x = e->x;
		player.y = e->y;
		player.x_speed = 0;
		player.y_speed = 0;
		player.grounded = true;
		e->x_speed = 0;
		SPR_SAFEANIM(e->sprite, 8);
		e->state_time = 120;
		break;
	case 8: // Throw player
		player_unlock_controls();
		SPR_SAFEANIM(e->sprite, 3);
		if(player_inflict_damage(1)) break;
		player.y_speed = pixel_to_sub(-1);
		player.x_speed = pixel_to_sub(2) - (pixel_to_sub(4) * e->direction);
		e->state_time = 60;
		break;
	default:
		break;
	}
}

void ai_balrogFlying_onUpdate(Entity *e) {
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
			SPR_SAFEHFLIP(e->sprite, e->direction);
			if(++e->state_time > 12) {
				e->state = SHOOT_PLAYER;
				e->state_time = 0;
				SPR_SAFEANIM(e->sprite, 2);
			}
		break;
		case SHOOT_PLAYER:
			e->state_time++;
			FACE_PLAYER(e);
			SPR_SAFEHFLIP(e->sprite, e->direction);
			if((e->state_time % 16) == 15) {
				SPR_SAFEANIM(e->sprite, 2);
				// Fire shot
				Entity *shot = entity_create(sub_to_block(e->x), sub_to_block(e->y),
					0, 0, OBJ_IGOR_SHOT, 0, 0);
				shot->x_speed = e->direction ? 0x400 : -0x400;
				shot->y_speed = -0x100 + (random() % 0x200);

				sound_play(SND_EM_FIRE, 5);
				if(e->state_time > 32) {	// 3 shots
					e->state = JUMP_BEGIN;
					e->state_time = 0;
				}
			}
		break;
		case JUMP_BEGIN:	// begin jump
			e->state_time++;
			FACE_PLAYER(e);
			SPR_SAFEHFLIP(e->sprite, e->direction);
			if(e->state_time > 3) {
				e->state = JUMP_UP;
				e->state_time = 0;
				e->x_speed = (player.x - e->x) / 128;
				e->y_speed = -0x600;
				SPR_SAFEANIM(e->sprite, 3);
			}
		break;
		case JUMP_UP:		// jumping up
			if(e->y_speed > 0x200) {
				if(e->health <= 60) {	
					// skip flying if low health
					e->state = JUMP_END;
				} else {
					e->state = FLYING;
					e->state_time = 0;
					e->y_mark = e->y;
					SPR_SAFEANIM(e->sprite, 9);
				}
			}
		break;
		case FLYING:
			e->state_time++;
			if(e->state_time % 16 == 0) {
				sound_play(SND_EXPLOSION2, 3);
			}
			if(e->state_time >= 128) {
				e->state = JUMP_END;
				SPR_SAFEANIM(e->sprite, 3);
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
					Entity *shot = entity_create(sub_to_block(e->x), sub_to_block(e->y), 
						0, 0, OBJ_BALROG_SHOT_BOUNCE, 0, 0);
					shot->x_speed = -0x400 + (random() % 0x800);
					shot->y_speed = -0x400 + (random() % 0x400);
				}
				e->state = LANDED;
				e->state_time = 0;
				SPR_SAFEANIM(e->sprite, 0);
			}
		break;
		case LANDED:
			if(++e->state_time > 3) {
				e->state = SHOOT_PLAYER;
				e->state_time = 0;
			}
		break;
	}
	entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_balrogFlying_onState(Entity *e) {
	if(e->state == STATE_DEFEATED) {
		e->x_speed = 0;
		e->eflags &= ~NPC_SHOOTABLE;
		e->attack = 0;
		entities_clear_by_type(OBJ_BALROG_SHOT_BOUNCE);
		entities_clear_by_type(OBJ_IGOR_SHOT);
		tsc_call_event(e->event);
	}
}

#ifdef PAL
#define BALROG_SHOT_SPEED	0x400
#define BALROG_SHOT_TIME	250
#define BALROG_SHOT_GRAVITY	42
#else
#define BALROG_SHOT_SPEED	0x355
#define BALROG_SHOT_TIME	300
#define BALROG_SHOT_GRAVITY	35
#endif

void ai_balrogShot_onUpdate(Entity *e) {
	e->y_speed += BALROG_SHOT_GRAVITY;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(e->y_speed > 0 && collide_stage_floor(e)) {
		e->y_speed = -BALROG_SHOT_SPEED;
	}
	if((e->x_speed > 0 && collide_stage_rightwall(e)) ||
		(e->x_speed < 0 && collide_stage_leftwall(e)) ||
		++e->state_time > BALROG_SHOT_TIME) {
		e->state = STATE_DELETE;
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

#define STATE_CHARGE			10
#define STATE_JUMP_FIRE			20
#define STATE_PAUSE				30
#define STATE_CAUGHT_PLAYER		40

void ai_balrog_boss_missiles(Entity *e)
{
	// try to catch player
	switch(e->state)
	{
		case STATE_CHARGE+1:
		case STATE_JUMP_FIRE+1:
		{
			if (PLAYER_DIST_X(12<<9) && PLAYER_DIST_Y2(12<<9, 8<<9))
			{
				//balrog_grab_player(e);
				//hurtplayer(5);
				e->state = STATE_CAUGHT_PLAYER;
			}
		}
		break;
	}
	
	// main state engine
	switch(e->state)
	{
		case 0:
		{
			//e->nxflags |= NXFLAG_FOLLOW_SLOPE;
			FACE_PLAYER(e);
			
			e->state = 1;
			//e->frame = 0;
			e->state_time = 0;
		}
		case 1:
		{
			if (++e->state_time > 30)
			{
				e->state = STATE_CHARGE;
				e->state_time2 ^= 1;	// affects how we react if we miss the player
			}
		}
		break;
		
		// charge the player
		case STATE_CHARGE:
		{
			e->state_time = 0;
			//e->frame = 9;
			//e->animtimer = 0;
			e->state++;
		}
		case STATE_CHARGE+1:
		{
			e->x_speed += e->direction ? 0x20 : -0x20;
			//walking_animation(o);
			
			// stuck against the wall?
			if ((e->direction == 0 && collide_stage_leftwall(e)) || \
				(e->direction == 1 && collide_stage_rightwall(e)))
			{
				if (++e->x_mark > 5)
					e->state = STATE_JUMP_FIRE;
			}
			else
			{
				// Use x_mark instead of timer3
				e->x_mark = 0;
			}
			
			// he behaves differently after every other time he pauses
			if (e->state_time2)
			{
				if (++e->state_time > 75)
				{
					//e->frame = 0;
					e->state = STATE_PAUSE;
				}
			}
			else
			{
				if (++e->state_time > 24)
					e->state = STATE_JUMP_FIRE;
			}
		}
		break;
		
		// jump and fire missiles
		case STATE_JUMP_FIRE:
		{
			e->state++;
			e->state_time = 0;
			//e->frame = 3;
			e->y_speed = -0x5ff;
		}
		case STATE_JUMP_FIRE+1:
		{
			FACE_PLAYER(e);
			
			// fire missiles
			if (++e->state_time < 30)
			{
				if ((e->state_time % 6) == 1)
				{
					sound_play(SND_EM_FIRE, 5);
					
					//Entity *shot = SpawnEntityAtActionPoint(o, OBJ_BALROG_MISSILE);
					//shot->dir = e->dir;
					//shot->xinertia = 0x100;
				}
			}
			
			// landed?
			if (e->y_speed >= 0 && (e->grounded = collide_stage_floor(e)))
			{
				//e->frame = 2;
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
	
	e->y_speed += 0x20;
	LIMIT_X(0x300);
	LIMIT_Y(0x5ff);
}

void ai_balrog_missile(Entity *e)
{
	if ((e->direction == 1 && collide_stage_rightwall(e)) || \
		(e->direction == 0 && collide_stage_leftwall(e)))
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
		if (e->direction == 0) e->x_speed = -e->x_speed;
		
		e->y_speed = random(-2, 0) << 9;
		
		e->state = 1;
	}
	
	e->x_speed += e->direction ? 0x20 : -0x20;
	
	//if ((++e->state_time2 % 4) == 1)
	//{
	//	effect(e->CenterX() - e->x_speed, e->CenterY(), EFFECT_SMOKETRAIL_SLOW);
	//}
	
	// heat-seeking at start, then level out straight
	if (e->state_time2 < 50)
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
