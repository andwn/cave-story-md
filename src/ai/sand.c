#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "effect.h"
#include "camera.h"

void ai_omega_onCreate(Entity *e) {
	e->direction = 1;
	e->health = 300;
	e->attack = 5;
	e->hurtSound = 52;
	e->deathSound = 72;
	e->deathSmoke = 3;
	e->hit_box = (bounding_box) { 32, 24, 32, 32 };
	e->display_box = (bounding_box) { 40, 32, 40, 32 };
	//SYS_die("Hi I am create");
}

void ai_omega_onUpdate(Entity *e) {
	if(e->state == 20) {
		if(++e->state_time > 120) {
			ENTITY_SET_STATE(e, 21, 0);
		}
	} else {
		
	}
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_omega_onState(Entity *e) {
	if(e->state == STATE_DEFEATED) {
		tsc_call_event(e->event); // Boss defeated event
		e->state = STATE_DESTROY;
		bossEntity = NULL;
	}
	switch(e->state) {
		case 20:
		e->y_speed = -0x0A0;
		break;
		case 21:
		e->y_speed = 0;
		break;
	}
}

void ai_sunstone_onCreate(Entity *e) {
	e->x += pixel_to_sub(8);
	e->y += pixel_to_sub(8);
}

void ai_puppy_onCreate(Entity *e) {
	e->eflags |= NPC_INTERACTIVE; // Yeah..
}

void ai_puppy_onUpdate(Entity *e) {
	
}

void ai_puppyCarry_onCreate(Entity *e) {
	e->alwaysActive = true;
	// One's all you can manage. One's all you can manage. One's all you can manage.
	e->eflags &= ~NPC_INTERACTIVE;
	e->nflags &= ~NPC_INTERACTIVE;
}

void ai_puppyCarry_onUpdate(Entity *e) {
	if(player.direction != e->direction) {
		e->direction ^= 1;
		SPR_SAFEHFLIP(e->sprite, e->direction);
	}
	e->x = player.x + pixel_to_sub(e->direction ? -4 : 4);
	e->y = player.y - pixel_to_sub(5);
}

void ai_jenka_onCreate(Entity *e) {
	if(e->type == OBJ_JENKA_COLLAPSED) {
		e->spriteAnim = 2;
	}
}

// Shoutouts to Noxious because I keep stealing his AI code

void ai_polish_onUpdate(Entity *e) {
	#define POLISH_ACCEL	0x20
	#define POLISH_SPEED	0x200
	#define POLISH_BOUNCE	0x100
	
	#define POLISH_CCW_LEFT		1
	#define POLISH_CCW_UP		2
	#define POLISH_CCW_RIGHT	3
	#define POLISH_CCW_DOWN		4
	
	#define POLISH_CW_LEFT		5
	#define POLISH_CW_UP		6
	#define POLISH_CW_RIGHT		7
	#define POLISH_CW_DOWN		8
	
	// Split after 20 damage
	if(e->health <= 100) {
		entity_create(sub_to_block(e->x), sub_to_block(e->y), 
				0, 0, OBJ_POLISHBABY, 0, 0)->x -= 0x1600;
		entity_create(sub_to_block(e->x), sub_to_block(e->y), 
				0, 0, OBJ_POLISHBABY, 0, 1)->x += 0x1600;
		e->state = STATE_DELETE;
		effect_create_smoke(0, sub_to_pixel(e->x), sub_to_pixel(e->y));
		sound_play(e->deathSound, 5);
		return;
	}
	
	switch(e->state) {
		case 0:		// initilization
			if(e->eflags & NPC_OPTION2) {
				e->state = POLISH_CW_RIGHT;
			} else {
				e->state = POLISH_CCW_LEFT;
			}
		break;
		
		// -------------- Traveling around counter-clockwise --------------
		
		case POLISH_CCW_LEFT:	// traveling left on ceiling
			e->y_speed -= POLISH_ACCEL;
			if (e->y_speed < 0 && collide_stage_ceiling(e))
			{
				e->y_speed = POLISH_BOUNCE;
				e->x_speed -= POLISH_BOUNCE;
			}
			
			if (collide_stage_leftwall(e)) e->state = POLISH_CCW_DOWN;
		break;
		
		case POLISH_CCW_UP:	// traveling up right wall
		{
			e->x_speed += POLISH_ACCEL;
			if (e->x_speed > 0 && collide_stage_rightwall(e))
			{
				e->x_speed = -POLISH_BOUNCE;
				e->y_speed -= POLISH_BOUNCE;
			}
			
			if (collide_stage_ceiling(e)) e->state = POLISH_CCW_LEFT;
		}
		break;
		
		case POLISH_CCW_RIGHT:	// traveling right on floor
		{
			e->y_speed += POLISH_ACCEL;
			if (e->y_speed > 0 && collide_stage_floor(e))
			{
				e->y_speed = -POLISH_BOUNCE;
				e->x_speed += POLISH_BOUNCE;
			}
			
			if (collide_stage_rightwall(e)) e->state = POLISH_CCW_UP;
		}
		break;
		
		case POLISH_CCW_DOWN:	// traveling down left wall
		{
			e->x_speed -= POLISH_ACCEL;
			if (e->x_speed < 0 && collide_stage_leftwall(e))
			{
				e->x_speed = POLISH_BOUNCE;
				e->y_speed += POLISH_BOUNCE;
			}
			
			if (collide_stage_floor(e)) e->state = POLISH_CCW_RIGHT;
		}
		break;
		
		// -------------- Traveling around clockwise --------------
		
		case POLISH_CW_LEFT:		// traveling left on floor
		{
			e->y_speed += POLISH_ACCEL;
			if (e->y_speed > 0 && collide_stage_floor(e))
			{
				e->y_speed = -POLISH_BOUNCE;
				e->x_speed -= POLISH_BOUNCE;
			}
			
			if (collide_stage_leftwall(e)) e->state = POLISH_CW_UP;
		}
		break;
		
		case POLISH_CW_UP:		// traveling up left wall
		{
			e->x_speed -= POLISH_ACCEL;
			if (e->x_speed < 0 && collide_stage_leftwall(e))
			{
				e->x_speed = POLISH_BOUNCE;
				e->y_speed -= POLISH_BOUNCE;
			}
			
			if (collide_stage_ceiling(e)) e->state = POLISH_CW_RIGHT;
		}
		break;
		
		case POLISH_CW_RIGHT:		// traveling right on ceiling
		{
			e->y_speed -= POLISH_ACCEL;
			if (e->y_speed < 0 && collide_stage_ceiling(e))
			{
				e->y_speed = POLISH_BOUNCE;
				e->x_speed += POLISH_BOUNCE;
			}
			
			if (collide_stage_rightwall(e)) e->state = POLISH_CW_DOWN;
		}
		break;
		
		case POLISH_CW_DOWN:		// traveling down right wall
		{
			e->x_speed += POLISH_ACCEL;
			if (e->x_speed > 0 && collide_stage_rightwall(e))
			{
				e->x_speed = -POLISH_BOUNCE;
				e->y_speed += POLISH_BOUNCE;
			}
			
			if (collide_stage_floor(e)) e->state = POLISH_CW_LEFT;
		}
		break;
	}
	
	LIMIT_X(POLISH_SPEED);
	LIMIT_Y(POLISH_SPEED);
	
	e->direction = 1;
}

void ai_polishBaby_onUpdate(Entity *e) {
	if(!e->state) {
		e->state = 1;
		if(random() & 1) {
			e->x_speed = (random() % 0x100) + 0x100;
		} else {
			e->x_speed = (random() % 0x100) - 0x300;
		}
		if(random() & 1) {
			e->y_speed = (random() % 0x100) + 0x100;
		} else {
			e->y_speed = (random() % 0x100) - 0x300;
		}
	}
	// Collide functions set speed to 0. Remember using mark vars
	e->x_mark = e->x_speed;
	e->y_mark = e->y_speed;
	if (e->x_speed > 0 && collide_stage_rightwall(e)) e->x_speed = -e->x_mark;
	if (e->x_speed < 0 && collide_stage_leftwall(e)) e->x_speed = -e->x_mark;
	if (e->y_speed > 0 && collide_stage_floor(e)) e->y_speed = -e->y_mark;
	if (e->y_speed < 0 && collide_stage_ceiling(e)) e->y_speed = -e->y_mark;
}

void ai_sandcroc_onUpdate(Entity *e) {
	int pbottom, crocbottom;
	switch(e->state) {
		case 0:
			e->state = 1;
			e->state_time = 0;
			e->y_mark = e->y;
			e->eflags &= ~(NPC_SOLID | NPC_SHOOTABLE | NPC_INVINCIBLE | NPC_IGNORESOLID);
			e->nflags &= ~(NPC_SOLID | NPC_SHOOTABLE | NPC_INVINCIBLE | NPC_IGNORESOLID);
		case 1:
			// track player invisibly underground
			e->x_speed = (e->x < player.x) ? 0x400:-0x400;
			
			if(PLAYER_DIST_X(pixel_to_sub(19))) {
				// check if bottoms of player and croc are near
				if(player.y < e->y && sub_to_pixel(player.y) + player.hit_box.bottom + 12 >
					sub_to_pixel(e->y) + e->hit_box.bottom) {
					// attack!!
					e->x_speed = 0;
					e->state = 2;
					e->state_time = 0;
					sound_play(SND_JAWS, 5);
					SPR_SAFEANIM(e->sprite, 1);
				}
			}
		break;
		
		case 2:		// attacking
			e->state_time++;
			if(e->state_time == 12) {
				e->attack = (e->type == OBJ_SANDCROC_OSIDE) ? 15 : 10;
			} else if(e->state_time==16) {
				e->eflags |= NPC_SHOOTABLE;
				e->eflags |= NPC_SOLID;
				e->attack = 0;
				e->state = 3;
				e->state_time = 0;
				SPR_SAFEANIM(e->sprite, 2);
			}
		break;
		
		case 3:
			e->state_time++;
			if(e->damage_time) {
				e->state = 4;
				e->state_time = 0;
				e->y_speed = 0;
				e->damage_time += 25;		// delay floattext until after we're underground
			}
		break;
		
		case 4:		// retreat
			e->y += 0x280;
			
			if (++e->state_time == 30) {
				e->eflags &= ~(NPC_SOLID | NPC_SHOOTABLE);
				e->state = 5;
				e->state_time = 0;
			}
		break;
		
		case 5:
			SPR_SAFEANIM(e->sprite, 0);
			e->y = e->y_mark;
			
			if(e->state_time < 100) {
				e->state_time++;
				// have to wait before moving: till floattext goes away
				// else they can see us jump
				if(e->state_time==98) {
					e->x_speed = player.x - e->x;
				} else {
					e->x_speed = 0;
				}
			} else {
				e->state = 0;
			}
		break;
	}
	LIMIT_Y(0x100);
	
	// these guys (from oside) don't track
	if(e->type == OBJ_SANDCROC_OSIDE) e->x_speed = 0;
}


void ai_sunstone_onUpdate(Entity *e) {
	switch(e->state) {
		case 0:
			e->eflags |= NPC_IGNORESOLID;
			e->state = 1;
		break;
		
		case 10:	// triggered to move by hvtrigger script
			// Don't flip the sprite
			SPR_SAFEHFLIP(e->sprite, 0);
			e->state_time = 0;
			e->state++;
		case 11:
			if(e->direction) e->x += 0x80; else e->x -= 0x80;
			
			if((e->state_time & 7) == 0) sound_play(SND_QUAKE, 5);
			e->state_time++;
				
			camera_shake(20);
		break;
	}
}

void ai_armadillo_onUpdate(Entity *e) {
	switch(e->state) {
		case 0:
			FACE_PLAYER(e);
			e->state = 1;
		case 1:
			if(collide_stage_leftwall(e) && !e->direction) e->direction = 1;
			if(collide_stage_rightwall(e) && e->direction) e->direction = 0;
			MOVE_X(0x100);
		break;
	}
	
	e->y_speed += 0x40;
	LIMIT_Y(0x5ff);
}
