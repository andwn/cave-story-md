#include "ai_common.h"

enum Frame {
	STAND, GASP, DUCK, ARMSUP, BLINK, PAINED, SMILE, WORRY, 
	WALK1, WALK2, AWAY1, AWAY2, FLY1, FLY2
};

// "BalrogCommon" functions for grabbing/throwing the player
#define STATE_GRAB 100
// grab the player in preparation for running the toss_player_away animation.
static void balrog_grab_player(Entity *e) {
	controlsLocked = TRUE;
	player.hidden = TRUE;
	e->frame = AWAY1;	// face away
	e->state = STATE_GRAB;
}
// shake and toss the player away. call balrog_grab_player first.
// returns true when complete.
// used in boss battles in Shack and at end of Labyrinth.
static uint8_t balrog_toss_player_away(Entity *e) {
	// keep player locked in position while balrog has him
	if (e->state <= STATE_GRAB+1) {
		player.x = e->x;
		player.y = e->y;
		player.x_speed = player.y_speed = 0;
	}
	switch(e->state) {
		case STATE_GRAB:		// slowing to a stop
		{
			e->x_speed *= 4;
			e->x_speed /= 5;
			if (e->x_speed == 0) {
				e->state++;
				e->timer = 0;
			}
		}
		break;
		case STATE_GRAB+1:		// shaking with back turned
		{
			ANIMATE(e, 4, AWAY2,AWAY1);		// shake
			// after a moment toss player away
			if (++e->timer > TIME(100)) {
				controlsLocked = FALSE;
				player.hidden = FALSE;
				player.dir = e->dir ^ 1;
				player.x += player.dir ? pixel_to_sub(4) : -pixel_to_sub(4);
				player.x_speed = player.dir ? SPEED(0x5FF) : -SPEED(0x5FF);
				player.y -= pixel_to_sub(8);
				player.y_speed = -SPEED(0x200);
				sound_play(SND_FUNNY_EXPLODE, 5);
				e->dir = player.dir;
				e->state++;
				e->frame = ARMSUP;	// arms up
				e->timer = 0;
			}
		}
		break;
		case STATE_GRAB+2: return ++e->timer > TIME(50);
	}
	return FALSE;
}

// Repurpose unused x_mark and y_mark variables
#define balrog_smoking		x_mark
#define balrog_smoketimer	y_mark

void ai_balrog(Entity *e) {
	uint8_t fall = TRUE;
	e->y_next = e->y + e->y_speed;
	e->x_next = e->x + e->x_speed;
	
	if(!(e->flags & NPC_IGNORESOLID)) {
		if(!e->grounded) e->grounded = collide_stage_floor(e);
		else e->grounded = collide_stage_floor_grounded(e);
	}

	switch(e->state) {
		case 0:
		{
			e->flags &= ~NPC_IGNORESOLID;
			e->x_speed = 0;
			e->balrog_smoking = FALSE;
		}
		/* fallthrough */
		case 1:
		{
			e->frame = STAND;
			RANDBLINK(e, 4, 200);
		}
		break;
		case 10:		// he jumps and flys away
		{
			e->x_speed = 0;
			e->frame = DUCK;
			e->timer = 0;
			e->state++;
		}
		/* fallthrough */
		case 11:
		{
			if (++e->timer <= TIME_8(20)) break;
			e->frame = ARMSUP;
			e->state++;
			e->y_speed = -SPEED_12(0x800);
			e->flags |= NPC_IGNORESOLID;
		}
		/* fallthrough */
		case 12:
		{
			fall = FALSE;
			e->y_speed -= SPEED_8(0x10);
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
			e->frame = PAINED;
			e->x_speed = 0;
			e->timer = e->timer2 = 0;
			//SmokeClouds(e, 4, 8, 8);
			sound_play(SND_BIG_CRASH, 5);
			e->balrog_smoking = 1;
		}
		/* fallthrough */
		case 21:
		{
			e->timer2++;
			e->x += ((e->timer2 >> 1) & 1) ? (1<<9) : -(1<<9);
			if (++e->timer > TIME_8(120)) {
				e->state = 10;
			}
			e->y_speed += SPEED_8(0x20);
			LIMIT_Y(SPEED(0x5FF));
		}
		break;
		case 30:	// he smiles for a moment
		{
			e->frame = SMILE;
			e->timer = 0;
			e->state = 31;
		}
		/* fallthrough */
		case 31:
		{
			if (++e->timer > TIME_8(100)) {
				e->state = 0;
			}
		}
		break;
		case 40: // Spell cast before transforming into Balfrog
		{
			e->state = 41;
			e->frame = PAINED;
		}
		break;
		case 42:
		{
			e->timer = 0;
			e->state = 43;
			// Spawn the Balfrog boss
			bossEntity = entity_create(e->x, e->y, 360 + BOSS_BALFROG, 0);
			bossEntity->event = 1000;
			bossEntity->state = 20; // Blinking
		}
		/* fallthrough */
		case 43:
		{
			// flashing visibility
			// (transforming into Balfrog stage boss;
			//	our flashing is interlaced with his)
			e->timer++;
			e->hidden = e->timer & 2;
		}
		break;
		case 50:	// he faces away
		{
			ANIMATE(e, 16, AWAY1,AWAY2);
			e->x_speed = 0;
		}
		break;
		case 60:	// he walks
		case 61:
		{
			ANIMATE(e, 16, WALK1,STAND,WALK2,STAND);
			MOVE_X(SPEED(0x200));
		}
		break;
		// he is teleported away (looking distressed)
		// this is when he is sent to Labyrinth at end of Sand Zone
		case 70:
		{
			e->x_speed = 0;
			e->timer = 0;
			e->frame = WORRY;
			e->state++;
		}
		/* fallthrough */
		case 71:
		{
			fall = FALSE;
			if(++e->timer > 120) {
				e->state = STATE_DELETE;
				return;
			}
			e->hidden = e->timer & 2;
		}
		break;
		case 80:	// hands up and shakes
		{
			e->frame = PAINED;
			e->state = 81;
		}
		/* fallthrough */
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
			e->frame = DUCK;	// prepare for jump;
		}
		/* fallthrough */
		case 101:
		{
			if (++e->timer > TIME_8(20)) {
				e->state = 102;
				e->timer = 0;
				e->frame = ARMSUP;	// fly up;
				entities_clear_by_type(OBJ_NPC_PLAYER);
				entities_clear_by_type(OBJ_CURLY);
				Entity *curly = entity_create(e->x, e->y, OBJ_BALROG_PASSENGER, 0);
				Entity *quote = entity_create(e->x, e->y, OBJ_BALROG_PASSENGER, NPC_OPTION1);
				quote->linkedEntity = e;
				curly->linkedEntity = e;
				// Workaround because the <CNP after <FON seems to mess this up
				camera.target = e;
				e->y_speed = -SPEED_12(0x800);
				e->flags |= NPC_IGNORESOLID;	// so can fly through ceiling
				fall = FALSE;
			}
		}
		break;
		case 102:	// flying up during escape seq
		{
			fall = FALSE;
			if(e->y <= 0) {
				e->state = STATE_DELETE;
				return;
			}
			// bust through ceiling
			uint16_t x = sub_to_block(e->x);
			uint16_t y = sub_to_block(e->y);
			if(y < 35 && stage_get_block(x, y)) {
				stage_replace_block(x,   y, 0);
				stage_replace_block(x-1, y, 0);
				stage_replace_block(x+1, y, 0);
				camera_shake(10);
				sound_play(SND_MISSILE_HIT, 5);
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
		if (++e->balrog_smoketimer > 20 || !(rand() & 15)) {
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
			e->flags &= ~NPC_IGNORESOLID;
			e->state = 1;
			e->grounded = FALSE;
			e->frame = ARMSUP;	// falling;
		}
		/* fallthrough */
		case 1:
		{
			// since balrog often falls through the ceiling we must wait until he is 
			// free-falling before we start checking to see if he hit the floor
			if(((blk(e->x, 0, e->y, -16) | blk(e->x, 0, e->y, 0) | blk(e->x, 0, e->y, 16)) & 0x41) != 0x41) {
				e->state = 2;
			}
		}
		break;
		case 2:	// free-falling
		{
			if ((e->grounded = collide_stage_floor(e))) {
				e->frame = DUCK;
				e->state = 3;
				e->timer = 0;
				camera_shake(30);
			}
		}
		break;
		case 3:	// landed
		{
			if (++e->timer > TIME(20)) {
				e->state = 4;
				e->frame = STAND;
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
			e->y += pixel_to_sub(2);
			e->y_speed = -SPEED(0x100);
			camera_shake(30);
			e->state = 1;
			e->frame = ARMSUP;
			sound_play(SND_BLOCK_DESTROY, 5);
		}
		/* fallthrough */
		case 1:		// falling the short distance to ground
		{
			e->y_speed += SPEED(0x10);
			e->y_next = e->y + e->y_speed;
			e->x_next = e->x + e->x_speed;
			LIMIT_Y(SPEED(0x5FF));
			if (e->y_speed > 0 && (e->grounded = collide_stage_floor(e))) {
				e->state = 2;
				e->frame = DUCK;
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
				e->frame = STAND;
			}
		}
		break;
		// standing
		case 3: {
            e->state++;
            e->flags &= ~NPC_IGNORESOLID;
        } /* fallthrough */
		case 4: break;
	}
}

// 68 - Boss: Balrog (Mimiga Village)
void ai_balrogRunning(Entity *e) {
	enum {
		STATE_CHARGE = 10,
		STATE_JUMP = 20,
		STATE_SLOW_DOWN = 30
	};
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);
	if(e->y_speed < 0) {
		// Balrog is wide, check a third point in the middle of his head
		if(!collide_stage_ceiling(e)) {
			if(blk(e->x, 0, e->y, -e->hit_box.top) == 0x41) e->y_speed = 0;
		}
	}
	// try to catch player
	if (e->state == STATE_CHARGE+1 || e->state == STATE_JUMP) {
		if (e->timer > 8 && entity_overlapping(&player, e)) {
			if (!playerIFrames) player_inflict_damage(2);
			balrog_grab_player(e);
		}
	}
	switch(e->state) {
		case 0:
		{
			FACE_PLAYER(e);
			e->flags |= NPC_SHOOTABLE;
			e->frame = STAND;
			e->timer = 0;
			e->state = 1;
		} /* fallthrough */
		case 1:
		{
			if (++e->timer > TIME(30)) {
				e->state = STATE_CHARGE;
				e->timer2++;
			}
		}
		break;
		// running towards player
		case STATE_CHARGE:
		{
			e->state++;
			e->timer = 0;
			e->animtime = 0;
		} /* fallthrough */
		case STATE_CHARGE+1:
		{
			ACCEL_X(0x10);
			ANIMATE(e, 8, WALK1,STAND,WALK2,STAND);
			if (++e->timer > TIME(75) ||
				(!e->dir && collide_stage_leftwall(e)) ||
				(e->dir && collide_stage_rightwall(e)))
			{
				e->frame = STAND;
				e->state = STATE_SLOW_DOWN;
				break;
			}
			// can jump every 3rd time, but if he catches the player
			// before he gets a chance to he does NOT jump on the next charge.
			if (!(e->timer2 & 3)) {
				if (e->timer > TIME(25)) {	// initiate jump
					e->frame = ARMSUP;
					e->y_speed = -SPEED(0x400);
					e->grounded = FALSE;
					e->state = STATE_JUMP;
				}
			}
		}
		break;
		// jumping
		case STATE_JUMP:
		{
			if(e->grounded) {
				e->frame = DUCK;		// <-- Landed frame.
				camera_shake(30);
				e->state = STATE_SLOW_DOWN;
			}
		}
		break;
		// slowing down after charging or jumping
		case STATE_SLOW_DOWN:
		{
			e->x_speed *= 4;
			e->x_speed /= 5;
			if (e->x_speed == 0) e->state = 0;
		}
		break;
		// caught player
		case STATE_GRAB:
		case STATE_GRAB+1:
		case STATE_GRAB+2:
		{
			if (balrog_toss_player_away(e)) e->state = 0;
		}
		break;
		default: e->state = 0;
	}
	e->x = e->x_next;
	e->y = e->y_next;
	if(!e->grounded) e->y_speed += SPEED(0x20);
	LIMIT_X(SPEED(0x400));
	LIMIT_Y(SPEED(0x5FF));
}

void ondeath_balrogRunning(Entity *e) {
	e->x_speed = 0;
	e->flags &= ~NPC_SHOOTABLE;
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
				e->frame = STAND;
			}
		break;
		case SHOOT_PLAYER:
			e->timer++;
			FACE_PLAYER(e);
			if((e->timer & 15) == 15) {
				e->frame = GASP;
				// Fire shot
				Entity *shot = entity_create(e->x, e->y, OBJ_IGOR_SHOT, 0);
				shot->x_speed = e->dir ? 0x400 : -0x400;
				shot->y_speed = -0x100 + (rand() & 0x1FF);

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
				e->x_mark = e->x_speed; // Remember x speed
				e->y_speed = -0x600;
				e->frame = ARMSUP;
			}
		break;
		case JUMP_UP:		// jumping up
			if(e->y_speed > 0x200) {
				// Keep trying to move after hitting a wall
				if(e->x_speed == 0) e->x_speed = e->x_mark;
				if(e->health <= 60) {	
					// skip flying if low health
					e->state = JUMP_END;
				} else {
					e->state = FLYING;
					e->timer = 0;
					e->y_mark = e->y;
					e->frame = FLY1;
				}
			}
		break;
		case FLYING:
			ANIMATE(e, 8, FLY1,FLY2);
			e->timer++;
			if((e->timer & 15) == 0) {
				sound_play(SND_EXPLOSION2, 3);
			}
			if(e->timer >= 128) {
				e->state = JUMP_END;
				e->frame = ARMSUP;
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
					shot->x_speed = -0x400 + (rand() & 0x7FF);
					shot->y_speed = -0x400 + (rand() & 0x3FF);
				}
				e->state = LANDED;
				e->timer = 0;
				e->frame = STAND;
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
	e->flags &= ~NPC_SHOOTABLE;
	e->attack = 0;
	entities_clear_by_type(OBJ_BALROG_SHOT_BOUNCE);
	entities_clear_by_type(OBJ_IGOR_SHOT);
	tsc_call_event(e->event);
}

void ai_balrogShot(Entity *e) {
	ANIMATE(e, 8, 0,1);
	if(!e->state) {
		e->flags |= NPC_SHOOTABLE | NPC_SHOWDAMAGE;
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

void ai_balrog_boss_msl(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(e->x_speed < 0) collide_stage_leftwall(e);
	if(e->x_speed > 0) collide_stage_rightwall(e);
	if(e->y_speed < 0) collide_stage_ceiling(e);
	if(e->grounded) e->grounded = collide_stage_floor_grounded(e);
	else if(!e->grounded) e->grounded = collide_stage_floor(e);
	// try to catch player
	switch(e->state) {
		case STATE_CHARGE+1:
		case STATE_JUMP_FIRE+1:
		{
			if (PLAYER_DIST_X(e, 12<<CSF) && PLAYER_DIST_Y2(e, 12<<CSF, 8<<CSF)) {
				balrog_grab_player(e);
				if(!playerIFrames) player_inflict_damage(5);
				//e->state = STATE_CAUGHT_PLAYER;
			}
		}
		break;
	}
	// main state engine
	switch(e->state) {
		case 0:
		{
			FACE_PLAYER(e);
			e->grounded = TRUE;
			e->state = 1;
			e->frame = STAND;
			e->timer = 0;
		}
		/* fallthrough */
		case 1:
		{
			if (++e->timer > TIME_8(30)) {
				e->state = STATE_CHARGE;
				e->timer2 ^= 1;	// affects how we react if we miss the player
			}
		}
		break;
		// charge the player
		case STATE_CHARGE:
		{
			e->timer = 0;
			e->animtime = 0;
			e->state++;
		}
		/* fallthrough */
		case STATE_CHARGE+1:
		{
			ANIMATE(e, 16, WALK1,STAND,WALK2,STAND);
			e->x_speed += e->dir ? SPEED_8(0x20) : -SPEED_8(0x20);
			//walking_animation(o);
			
			// stuck against the wall?
			//if((!e->dir && collide_stage_leftwall(e)) || 
			//	(e->dir && collide_stage_rightwall(e))) {
			//	if (++e->x_mark > 5) e->state = STATE_JUMP_FIRE;
			//} else {
				// Use x_mark instead of timer3
			//	e->x_mark = 0;
			//}
			// he behaves differently after every other time he pauses
			if (e->timer2) {
				if (++e->timer > TIME_8(75)) {
					e->frame = STAND;
					e->state = STATE_PAUSE;
				}
			} else {
				if (++e->timer > TIME_8(24)) e->state = STATE_JUMP_FIRE;
			}
		}
		break;
		
		// jump and fire missiles
		case STATE_JUMP_FIRE:
		{
			e->state++;
			e->grounded = FALSE;
			e->timer = 0;
			e->frame = ARMSUP;
			e->y_speed = -SPEED_12(0x5ff);
		}
		/* fallthrough */
		case STATE_JUMP_FIRE+1:
		{
			FACE_PLAYER(e);
			// fire missiles
			if (++e->timer < TIME_8(30)) {
				if ((e->timer & 7) == 1) {
					sound_play(SND_EM_FIRE, 5);
					Entity *shot = entity_create(e->x, e->y, OBJ_BALROG_MISSILE, 0);
					shot->dir = e->dir;
					shot->hit_box = (bounding_box) {{ 6,6,6,6 }};
					shot->display_box = (bounding_box) {{ 8,8,8,8 }};
					//shot->x_speed = SPEED_8(0xFF);
					//if(shot->dir) shot->x_speed = -shot->x_speed;
				}
			}
			// landed?
			if (e->grounded) {
				e->frame = DUCK;
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
		case STATE_GRAB:	// caught player
		case STATE_GRAB+1:
		case STATE_GRAB+2: 
		{
			if (balrog_toss_player_away(e)) e->state = 0; 
		}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
	if(!e->grounded) e->y_speed += SPEED_8(0x20);
	LIMIT_X(SPEED_10(0x300));
	LIMIT_Y(SPEED_12(0x5ff));
}

void ai_balrog_missile(Entity *e) {
	if ((e->dir && blk(e->x, 6, e->y, 0) == 0x41) ||
		(!e->dir && blk(e->x, -6, e->y, 0) == 0x41)) {
		SMOKE_AREA((e->x >> CSF) - 16, (e->y >> CSF) - 16, 32, 32, 3);
		//effect(e->CenterX(), e->CenterY(), EFFECT_BOOMFLASH);
		sound_play(SND_MISSILE_HIT, 5);
		
		e->state = STATE_DELETE;
		return;
	}
	
	if (e->state == 0) {
		// recoil in oppisite direction
		e->x_speed = SPEED_10(rand() & 0x1FF);
		if(e->dir) e->x_speed = -e->x_speed;
		
		e->y_speed = -SPEED_10(rand() & 0x1FF);
		e->state = 1;
	}
	
	e->x_speed += e->dir ? SPEED_8(0x20) : -SPEED_8(0x20);
	
	//if ((++e->timer2 & 3) == 1)
	//{
	//	effect(e->CenterX() - e->x_speed, e->CenterY(), EFFECT_SMOKETRAIL_SLOW);
	//}
	
	// heat-seeking at start, then level out straight
	if (e->timer2 < TIME_8(50)) {
		if (e->y < player.y) e->y_speed += SPEED_8(0x20);
		else e->y_speed -= SPEED_8(0x20);
	} else {
		e->y_speed = 0;
	}
	
	// flash
	//e->frame ^= 1;
	
	if (e->x_speed < -SPEED_12(0x400)) e->x_speed = -SPEED_12(0x600);
	if (e->x_speed >  SPEED_12(0x400)) e->x_speed =  SPEED_12(0x600);

	e->x += e->x_speed;
	e->y += e->y_speed;
}
