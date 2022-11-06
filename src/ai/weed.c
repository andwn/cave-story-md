#include "ai_common.h"
#include "gamemode.h"

void onspawn_jelly(Entity *e) {
	e->timer = rand() & 31;
	e->x_mark = e->x;
	e->y_mark = e->y;
	e->flags &= ~NPC_SHOOTABLE;
	if(e->flags & NPC_OPTION2) e->dir = 1;
	MOVE_X(SPEED_8(0xFF));
}

void ai_jelly(Entity *e) {
	e->flags ^= NPC_SHOOTABLE;
	e->attack ^= 5;
	
	if(++e->animtime >= 12) {
		e->animtime = 0;
		if(++e->frame > 4) e->frame = 0;
	}
	switch(e->state) {
		case 0:
		{
			if(e->timer == 0) e->state = 10;
			else e->timer--;
		}
		break;
		case 10:
		{
			if(++e->timer > TIME_8(10)) {
				e->timer = 0;
				e->state = 11;
			}
		}
		break;
		case 11:
		{
			if(++e->timer == TIME_8(12)) {
				MOVE_X(SPEED_8(0xFF));
				e->y_speed -= SPEED_10(0x200);
			} else if(e->timer > TIME_8(16)) {
				e->state = 12;
				e->timer = 0;
			}
		}
		break;
		case 12:
		{
			e->timer++;
			if(e->y > e->y_mark && e->timer > TIME_8(10)) {
				e->timer = 0;
				e->state = 10;
			}
		}
		break;
	}
	e->dir = e->x < e->x_mark;
	if(e->y <= e->y_mark && e->y_speed < SPEED_10(0x1E0)) e->y_speed += SPEED_8(0x20);
	if(!e->attack) {
		if(e->x_speed > 0) {
			if((blk(e->x, -8, e->y, 0) & 0x41) == 0x41) e->x_speed = 0;
		} else if(e->x_speed < 0) {
			if((blk(e->x, 8, e->y, 0) & 0x41) == 0x41) e->x_speed = 0;
		}
		if(e->y_speed > 0) {
			if((blk(e->x, 0, e->y, 8) & 0x41) == 0x41) e->y_speed = -SPEED_10(0x200);
		} else if(e->y_speed < 0) {
			if((blk(e->x, 0, e->y, -8) & 0x41) == 0x41) e->y_speed = SPEED_8(0xFF);
		}
	}
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_kulala(Entity *e) {
	switch(e->state) {
		case 0:		// frozen/in stasis. waiting for player to shoot.
		{
			e->frame = 4;
			e->state = 1;
		}
		/* fallthrough */
		case 1:
		{
			if(e->damage_time) {
				camera_shake(30);
				e->state = 10;
				e->frame = 0;
				e->timer = 0;
			}
		}
		break;
		case 10:	// falling
		{
			e->flags &= ~NPC_INVINCIBLE;
			if(++e->timer > TIME_8(40)) {
				e->timer = 0;
				e->state++;
			}
		}
		break;
		case 11:	// animate thrust
		{
			if(++e->timer > TIME_8(5)) {
				e->timer = 0;
				if(++e->frame >= 3) e->state++;
			}
		}
		break;
		case 12:	// thrusting upwards
		{
			e->y_speed = -SPEED_10(0x155);
			if(++e->timer > TIME_8(20)) {
				e->state = 10;
				e->frame = 0;
				e->timer = 0;
			}
		}
		break;
		case 20:	// shot/freeze over/go invulnerable
		{
			e->frame = 4;
			e->x_speed >>= 1;
			e->y_speed += SPEED_8(0x20);
			if(!e->damage_time) {
				e->state = 10;
				e->frame = 0;
				e->timer = TIME_8(30);
			}
		}
		break;
	}
	
	if(e->damage_time) {
		// x_mark unused so use it as a second timer
		if(++e->x_mark > TIME(12)) {
			e->state = 20;
			e->frame = 4;
			e->flags |= NPC_INVINCIBLE;
		}
	} else {
		e->x_mark = 0;
	}
	
	if(e->state >= 10) {
		e->x_next = e->x + e->x_speed;
		e->y_next = e->y + e->y_speed;
		e->y_speed += SPEED(0x10);
		// Have to make an extra check because we are wide
		if(collide_stage_floor(e) || ((blk(e->x_next, 0, e->y_next, e->hit_box.bottom) & 0x41) == 0x41)) 
			e->y_speed = -SPEED(0x300);
		else if(e->y_speed < 0) {
			if(!collide_stage_ceiling(e)) {
				if(((blk(e->x_next, 0, e->y_next, -e->hit_box.top) & 0x41) == 0x41)) e->y_speed = 0x100;
			}
		}
		// Unused y_mark for third timer
		if(collide_stage_leftwall(e)) {
			e->y_mark = TIME(50);
			e->dir = 1;
		}
		if(collide_stage_rightwall(e)) {
			e->y_mark = TIME(50);
			e->dir = 0;
		}
		
		if(e->y_mark > 0) {
			e->y_mark--;
			ACCEL_X(SPEED(0x80));
		} else {
			e->y_mark = TIME(50);
			FACE_PLAYER(e);
		}
		LIMIT_X(SPEED(0x100));
		LIMIT_Y(SPEED(0x300));
		e->x = e->x_next;
		e->y = e->y_next;
	}
}

void ondeath_kulala(Entity *e) {
	//if(!system_get_flag(e->id)) {
		Entity *chest = entity_create(e->x, e->y, OBJ_CHEST_CLOSED, NPC_INTERACTIVE);
		chest->event = e->event;
	//}
	e->flags &= ~NPC_EVENTONDEATH;
	e->state = STATE_DESTROY;
}

void ai_mannan(Entity *e) {
	if(e->health >= 90) { // Alive
		if(e->state) { // Firing
			if(++e->timer > TIME(25)) {
				e->state = 0;
				e->timer = 0;
				e->frame = 0;
			}
		} else { // Waiting
			if(e->damage_time) {
				// Got hit, start firing
				e->state = 1;
				e->timer = 0;
				e->frame = 1;
				Entity *shot = entity_create(e->x, e->y + 0x400, OBJ_MANNAN_SHOT, 0);
				shot->dir = e->dir;
				shot->alwaysActive = TRUE;
			}
		}
	} else if(e->state < 3) { // Just got killed
		sound_play(e->deathSound, 5);
		effect_create_smoke(sub_to_pixel(e->x), sub_to_pixel(e->y));
		entity_drop_powerup(e);
		// Face sprite remains after defeated
		//e->eflags &= ~NPC_SHOOTABLE;
		e->flags &= ~NPC_SHOOTABLE;
		e->frame = 2;
		e->attack = 0;
		e->state = 3;
	}
}

void ai_mannanShot(Entity *e) {
	ACCEL_X(SPEED(0x20));
	if((e->timer & 7) == 1) {
		sound_play(SND_IRONH_SHOT_FLY, 2);
	}
	if(++e->timer > TIME(120)) e->state = STATE_DELETE;
	e->x += e->x_speed;
}

void ai_malco(Entity *e) {
	switch(e->state) {
		case 0:
			e->state = 1;
			e->frame = 0;
			
			// smushed into ground, used on re-entry to room
			// if player does not choose to pull him out
			// after Balrog fight
			if (e->dir) e->frame = 5;
		break;
		
		case 10:
			e->state = 11;
			e->timer = 0;
			e->timer = 0;
			effect_create_smoke(e->x >> CSF, e->y >> CSF);
			/* fallthrough */
		case 11:	// beeping and eyes flickering
			if (++e->timer < TIME(100)) {
				if (!(e->timer & 3)) {
					sound_play(SND_COMPUTER_BEEP, 5);
					e->frame ^= 1;
				}
			} else if (e->timer > TIME(150)) {
				e->timer = 0;
				e->state = 15;
			}
		break;
		
		case 15:		// shaking
			if (e->timer & 2) {
				e->x += (1<<CSF);
				sound_play(SND_DOOR, 5);
			} else {
				e->x -= (1<<CSF);
			}
			if (++e->timer > TIME(50)) e->state = 16;
		break;
		
		case 16:		// stand up
			e->state = 17;
			e->frame = 2;
			e->timer = 0;
			sound_play(SND_BLOCK_DESTROY, 5);
			effect_create_smoke(e->x >> CSF, e->y >> CSF);
			/* fallthrough */
		case 17:
			if (++e->timer > TIME(150)) {
				e->state = 18;
			}
		break;
		
		case 18:		// gawking/bobbing up and down
			e->state = 19;
			e->timer = 0;
			// go into gawk frame first time
			e->animtime = 3; e->frame = 9;
			/* fallthrough */
		case 19:
			if (++e->animtime > 3) {
				e->animtime = 0;
				if (++e->frame > 4) e->frame = 3;
				if (e->frame==3) sound_play(SND_DOOR, 5);
			} if (++e->timer > 100) {
				e->state = 20;
				sound_play(SND_DOOR, 5);
				effect_create_smoke(e->x >> CSF, e->y >> CSF);
			}
		break;
		
		case 20: e->frame = 4; break;
		
		case 21:	// got smushed!
			e->state = 22;
			e->frame = 5;
			sound_play(SND_ENEMY_HURT, 5);
		break;
		
		case 22:
		break;
		
		case 100:	// "baby" malcos waking up during credits
		{
			e->state = 101;
			e->frame = 3;
			e->animtime = 0;
		} /* fallthrough */
		case 101:
		{
			ANIMATE(e, 4, 3,4);
		}
		break;
		case 110:	// the one that blows up
		{
			effect_create_smoke(e->x >> CSF, e->y >> CSF);
			e->state = STATE_DELETE;
		}
		break;
	}
}

void ai_malcoBroken(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->frame = 6;
			RANDBLINK(e, 7, 200);
			if(gamemode == GM_CREDITS) {
				e->dir = 1;
			} else {
				FACE_PLAYER(e);
			}
		}
		break;
		case 10:	// set when pulled out of ground
		{
			sound_play(SND_BLOCK_DESTROY, 5);
			effect_create_smoke(e->x >> CSF, e->y >> CSF);
			e->state = 0;
		}
		break;
	}
}

void onspawn_powerf(Entity *e) {
	e->y += 8 << CSF;
}

void ai_powerf(Entity *e) {
	ANIMATE(e, 2, 0,1,2,3);
}

void ai_powers(Entity *e) {
	ANIMATE(e, 8, 0,1);
}

void onspawn_press(Entity *e) {
	// Press is 4 pixels too low
	e->y -= 4 << CSF;
}

void ai_press(Entity *e) {
	e->flags ^= NPC_SHOOTABLE;
	switch(e->state) {
		case 0:
			if((blk(e->x, 0, e->y, 14) & 0x41) != 0x41) {
				e->state = 10;
				e->timer = 0;
				e->frame = 1;
			}
		break;
		case 10:		// fall
			e->timer++;
			if(e->timer == TIME_8(5)) {
				e->frame = 2;
			}
			e->y_speed += SPEED_8(0x20);
			if(e->y_speed > SPEED_12(0x5FF)) e->y_speed = SPEED_12(0x5FF);
			e->y += e->y_speed;
			if(e->y + (12 << CSF) < player.y) {
				e->flags &= ~NPC_SOLID;
				e->attack = 127;
			} else {
				e->flags |= NPC_SOLID;
				e->attack = 0;
			}
			if((blk(e->x, 0, e->y, 13) & 0x41) == 0x41) {
				// Push out of the ground if necessary
				while((blk(e->x, 0, e->y, 12) & 0x41) == 0x41) {
					e->y -= 0x200;
				}
				//SmokeSide(o, 4, DOWN);
				camera_shake(10);
				e->y_speed = 0;
				e->state = 0;
				e->frame = 0;
				e->attack = 0;
				e->flags |= NPC_SOLID;
			}
		break;
	}
}

void onspawn_frog(Entity *e) {
	// Balfrog sets OPTION1
	if(e->flags & NPC_OPTION1) {
		e->alwaysActive = TRUE;
		e->dir = rand() & 1;
		e->flags |= NPC_IGNORESOLID;
		e->state = 3;
		e->frame = 2;
	} else {
		e->grounded = TRUE;
		e->flags &= ~NPC_IGNORESOLID;
		e->state = 1;
	}
	e->flags &= ~NPC_SHOOTABLE;
}

void ai_frog(Entity *e) {
	if(!e->grounded) e->y_speed += SPEED_8(0x80);
	LIMIT_Y(SPEED_12(0x5FF));

	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	e->flags ^= NPC_SHOOTABLE;

	switch(e->state) {
		case 0:
		{
			e->timer = 0;
			e->x_speed = 0;
			e->y_speed = 0;
		}
		/* fallthrough */
		case 1:		// standing
		case 2:
		{
			e->frame = 0;
			RANDBLINK(e, 1, 100);
			e->timer++;
		}
		break;
		case 3:		// falling out of ceiling during balfrog fight
		{
			if(++e->timer > TIME_8(40)) {
				e->flags &= ~NPC_IGNORESOLID;
				if((e->grounded = collide_stage_floor(e))) {
					e->state = 1;
					e->frame = 0;
					e->timer = 0;
				}
			}
		}
		break;
		case 10:	// jumping
		case 11:
		{
			if (e->x_speed < 0 && collide_stage_leftwall(e)) {
				TURN_AROUND(e);
				MOVE_X(abs(e->x_speed));
			}
			if (e->x_speed > 0 && collide_stage_rightwall(e)) {
				TURN_AROUND(e);
				MOVE_X(abs(e->x_speed));
			}
			if (e->y_speed >= 0 && (e->grounded = collide_stage_floor(e))) {
				e->state = 0;
				e->frame = 0;
				e->timer = 0;
			}
		}
		break;
	}
	// rand jumping, and jump when shot
	if ((e->state < 3) && e->timer > TIME_8(15)) {
		uint8_t dojump = FALSE;
		if(e->damage_time) {
			dojump = TRUE;
		} else if(PLAYER_DIST_X(e, 0x14000) && PLAYER_DIST_Y(e, 0x8000)) {
			if((rand() & 31) == 0) {
				dojump = TRUE;
			}
		}
		if (dojump) {
			FACE_PLAYER(e);
			e->state = 10;
			e->frame = 2;
			e->y_speed = -SPEED_12(0x5ff);
			e->grounded = FALSE;

			// no jumping sound in cutscenes at ending
			if (!controlsLocked)
				sound_play(SND_ENEMY_JUMP, 3);

			MOVE_X(SPEED_10(0x200));
		}
	}

	e->x = e->x_next;
	e->y = e->y_next;
}

void onspawn_hey(Entity *e) {
	SNAP_TO_GROUND(e);
	e->display_box.top = 16;
}

void ai_hey(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->timer = 0;
		}
		break;
		case 1:
		{
			e->hidden = ++e->timer > TIME_8(50);
		}
		break;
	}
}

void ai_motorbike(Entity *e) {
	switch(e->state) {
		case 0:		// parked
		break;
		case 10:	// kazuma and booster mounted
		{
			e->alwaysActive = TRUE;
			e->frame = 1;
			e->state++;
		}
		break;
		case 20:	// kazuma and booster start the engine
		{
			e->state = 21;
			e->timer = 1;
			e->x_mark = e->x;
			e->y_mark = e->y;
		}
		/* fallthrough */
		case 21:
		{
			e->x = e->x_mark + 0x200 - (rand() & 0x3FF);
			e->y = e->y_mark + 0x200 - (rand() & 0x3FF);
			if(++e->timer > TIME_8(30)) {
				e->state = 30;
			}
		}
		break;
		case 30:	// kazuma and booster take off
		{
			e->state = 31;
			e->timer = 1;
			e->x_speed = -SPEED_12(0x800);
			e->y_mark = e->y;
			sound_play(SND_MISSILE_HIT, 5);
		}
		/* fallthrough */
		case 31:
		{
			e->x_speed += SPEED_8(0x20);
			e->timer++;
			e->y = e->y_mark + 0x200 - (rand() & 0x3FF);
			if (e->timer > TIME_8(10))  e->dir = 1;
			if (e->timer > TIME_8(200)) e->state = 40;
		}
		break;
		
		case 40:		// flying away (fast out-of-control)
		{
			e->state = 41;
			e->timer = 2;
			e->dir = 0;
			e->y -= pixel_to_sub(48);		// move up...
			e->x_speed = -SPEED_12(0xFFF);		// ...and fly fast
		}
		/* fallthrough */
		case 41:
		{
			e->timer += 2;	// makes exhaust sound go faster
			if(e->timer > TIME_10(1000)) {
				e->state = STATE_DELETE;
			}
		}
		break;
	}
	if(e->state >= 20 && (e->timer & 3) == 0) {
		sound_play(SND_FIREBALL, 5);
		// make exhaust puffs, and make them go out horizontal
		// instead of straight up as this effect usually does
		//Caret *puff = effect(o->ActionPointX(), o->ActionPointY(), EFFECT_SMOKETRAIL_SLOW);
		//puff->yinertia = 0;
		//puff->xinertia = (o->dir == LEFT) ? 0x280 : -0x280;
	}
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_ravil(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	if(e->x_speed < 0) collide_stage_leftwall(e);
	if(e->x_speed > 0) collide_stage_rightwall(e);
	if(!e->grounded) {
		e->grounded = collide_stage_floor(e);
		if(e->grounded) sound_play(SND_THUD, 5);
	} else {
		e->grounded = collide_stage_floor_grounded(e);
	}
	
	switch(e->state) {
		case 0:
		{	// Don't push the player into walls
			//e->eflags &= ~(NPC_SOLID | NPC_SPECIALSOLID);
			e->flags &= ~(NPC_SOLID | NPC_SPECIALSOLID);
			e->x_speed = 0;
			e->state = 1;
			e->timer = 0;
			e->timer2 = 0;
		} /* fallthrough */
		case 1:
		{	// waiting-- attack once player gets too close or shoots us
			if (++e->timer > 40) e->state = 2;
		}
		break;
		case 2:
		{
			if(e->damage_time || (PLAYER_DIST_X(e, pixel_to_sub(96)) && PLAYER_DIST_Y2(e, pixel_to_sub(96), 32<<CSF))) {
				e->state = 10;
				e->timer = 0;
			}
		}
		break;
		case 10:
		{	// woken up-- preparing to attack
			FACE_PLAYER(e);
			e->frame = 1;
			
			if (++e->timer > 20) {
				e->timer = 0;
				e->state = 20;
			}
		}
		break;
		case 20:
		{	// hop, hop, lunge...
			e->attack = 0;
			e->x_speed = 0;
			
			ANIMATE(e, 4, 1,2,3);
			if (e->frame >= 3) {
				FACE_PLAYER(e);
				MOVE_X(SPEED(0x200));
				
				if (++e->timer2 >= 3) { // lunge/bite
					e->timer2 = 0;
					
					sound_play(SND_JAWS, 5);
					e->frame = 4;
					e->attack = 5;
					e->x_speed <<= 1;
				} else {
					sound_play(SND_ENEMY_JUMP, 5);
				}
				
				e->state = 21;
				e->y_speed = -SPEED(0x400);
				e->grounded = FALSE;
			}
		}
		break;
		case 21:	// doing jump or lunge
		{
			if (e->grounded) {
				e->state = 20;
				e->frame = 1;
				e->animtime = 0;
				e->attack = 0;
				
				// if player too far away return to wait state
				if (!PLAYER_DIST_X(e, pixel_to_sub(144)) || !PLAYER_DIST_Y2(e, pixel_to_sub(144), pixel_to_sub(72))) {
					e->state = 0;
				}
			}
		}
		break;
		case 30:		// jumping out of fireplace (set by script)
		{
			effect_create_smoke(e->x >> CSF, e->y >> CSF);
			e->frame = 0;
			e->state = 0;
		}
		break;
		case 50:		// killed (as boss, in Grasstown Hut) (set by script)
		{
			sound_play(SND_ENEMY_HURT, 5);
			e->frame = 4;
			e->attack = 0;
			e->flags &= ~(NPC_SHOOTABLE | NPC_SOLID);
			//e->eflags &= ~(NPC_SHOOTABLE | NPC_SOLID);
			e->state = 51;
			e->y_speed = -SPEED(0x200);
		} /* fallthrough */
		case 51:
		{
			if (e->grounded) {
				e->frame = 5;
				e->x_speed = 0;
				e->state = 52;	// falls slower
			}
		}
		break;
	}
	
	if (e->state == 52)
		e->y_speed += SPEED(0x20);
	else
		e->y_speed += SPEED(0x40);
	
	LIMIT_Y(SPEED(0x5ff));
	e->x = e->x_next;
	e->y = e->y_next;
}

// Collapse in Grasstown Hut, explode in Balcony
void ondeath_ravil(Entity *e) {
	if(stageID == 30) tsc_call_event(e->event);
	else e->state = STATE_DESTROY;
}
