#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "effect.h"

/*
 * Curly [0117] -By Shmitz
0000	Stand around. Eyes blink.
0003	Walk forward
0004	Walk forward
0005	Poof defeat
0006	Defeat
0010	Walk towards main char until 1 away
0011	Walk forward no falling
0020	Back turned
0021	Look up
0030	Knocked a bit over
0031	Fall over
0032	Fall over
0070	Moonwalk
0071	Moonwalk
* */

void ai_curly_onUpdate(Entity *e) {
	if(!e->grounded) e->y_speed += GRAVITY_JUMP;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	// Don't test ceiling, only test sticking to ground while moving
	if(e->x_speed < 0) {
		collide_stage_leftwall(e);
	} else if(e->x_speed > 0) {
		collide_stage_rightwall(e);
	}
	if(e->grounded) {
		if(e->state != 11 && e->x_speed != 0) e->grounded = collide_stage_floor_grounded(e);
	} else if(e->y_speed > 0) {
		e->grounded = collide_stage_floor(e);
		if(e->grounded && e->state == 30) ENTITY_SET_STATE(e, 31, 0);
	}
	if(e->state == 10 && (e->direction ?
		(e->x > player.x - block_to_sub(2)) : (e->x < player.x + block_to_sub(2)))) {
		ENTITY_SET_STATE(e, 0, 0);
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_curly_onState(Entity *e) {
	switch(e->state) {
		case 0: // Stand
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 0);
		break;
		case 3:
		case 4: // Walk
		e->x_speed = pixel_to_sub(e->direction ? 1 : -1);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 1);
		break;
		case 5:
		effect_create_smoke(0, sub_to_block(e->x), sub_to_block(e->y));
		case 6: // Defeated
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 9);
		break;
		case 10: // Walk towards player until 1 block away
		FACE_PLAYER(e);
		e->x_speed = pixel_to_sub(e->direction ? 1 : -1);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 1);
		break;
		case 11: // Walk with no gravity
		e->x_speed = pixel_to_sub(e->direction ? 1 : -1);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 1);
		break;
		case 20: // Back turned
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 4);
		break;
		case 21: // Look up
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 2);
		break;
		case 30: // Knocked over
		SPR_SAFEANIM(e->sprite, 10);
		e->x_speed = -0x150 + 0x300 * e->direction;
		e->y_speed = -0x150;
		e->grounded = false;
		break;
		case 31:
		case 32: // Fell over
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 9);
		break;
		case 70:
		case 71: // Walk backwards
		e->x_speed = pixel_to_sub(e->direction ? 1 : -1);
		SPR_SAFEHFLIP(e->sprite, !e->direction);
		SPR_SAFEANIM(e->sprite, 1);
		break;
	}
}

// I'm copying from NXEngine again lol

#define CURLYB_FIGHT_START		10
#define CURLYB_WAIT				11
#define CURLYB_WALK_PLAYER		13
#define CURLYB_WALKING_PLAYER	14
#define CURLYB_CHARGE_GUN		20
#define CURLYB_FIRE_GUN			21
#define CURLYB_SHIELD			30

#ifdef PAL
#define CURLYB_WALK_ACCEL	0x40
#define CURLYB_WALK_SPEED	0x1FF
#else
#define CURLYB_WALK_ACCEL	0x36
#define CURLYB_WALK_SPEED	0x1B0
#endif

static void curlyboss_fire(Entity *o, u8 dir)
{
	Entity *shot = entity_create(0, 0, 0, 0, OBJ_CURLYBOSS_SHOT, 0, dir & 1);
	
	shot->attack = 6;
	//shot->sprite = SPR_SHOT_MGUN_L1;
	//shot->dir = dir & 1;
	//shot->shot.dir = dir;
	
	//effect(shot->CenterX(), shot->CenterY(), EFFECT_STARPOOF);
	
	switch(dir)
	{
		case 0:
			shot->x = o->x - pixel_to_sub(12);
			shot->y = o->y + pixel_to_sub(2);
			shot->x_speed = -4096;
		break;
		
		case 1:
			shot->x = o->x + pixel_to_sub(12);
			shot->y = o->y + pixel_to_sub(2);
			shot->x_speed = 4096;
		break;
		
		case 2:
			shot->x = o->x;
			shot->y = o->y - pixel_to_sub(10);
			shot->y_speed = -4096;
			SPR_SAFEANIM(shot->sprite, 1);
		break;
	}
	
	sound_play(SND_POLAR_STAR_L1_2, 4);
}

void ai_curlyBoss_onUpdate(Entity *o) 
{

	//o->y_next = o->y + o->y_speed;
	switch(o->state) 
	{
		case CURLYB_FIGHT_START:
		{
			//o->health = 12;
			o->state = CURLYB_WAIT;
			o->state_time = (random() % 50) + 50;
			SPR_SAFEANIM(o->sprite, 0);
			o->direction = (o->x <= player.x);
			SPR_SAFEHFLIP(o->sprite, o->direction);
			o->eflags |= NPC_SHOOTABLE;
			o->eflags &= ~NPC_INVINCIBLE;
			o->nflags &= ~NPC_INVINCIBLE;
		}
		case CURLYB_WAIT:
			if (o->state_time) o->state_time--;
			else
			{
				o->state = CURLYB_WALK_PLAYER;
			}
		break;
		
		
		case CURLYB_WALK_PLAYER:
			o->state = CURLYB_WALKING_PLAYER;
			SPR_SAFEANIM(o->sprite, 1);
			o->state_time = (random() % 50) + 50;
			o->direction = (o->x <= player.x);
			SPR_SAFEHFLIP(o->sprite, o->direction);
		case CURLYB_WALKING_PLAYER:
			o->x_speed += o->direction ? CURLYB_WALK_ACCEL : -CURLYB_WALK_ACCEL;
			
			if (o->state_time) o->state_time--;
			else
			{
				o->eflags |= NPC_SHOOTABLE;
				o->state = CURLYB_CHARGE_GUN;
				o->state_time = 0;
				sound_play(SND_CHARGE_GUN, 5);
			}
		break;
		
		
		case CURLYB_CHARGE_GUN:
			o->direction = (o->x <= player.x);
			
			o->x_speed -= o->x_speed >> 3;
			
			SPR_SAFEANIM(o->sprite, 0);
			if (++o->state_time > 50)
			{
				o->state = CURLYB_FIRE_GUN;
				//o->frame = 0;
				o->x_speed = 0;
				o->state_time = 0;
			}
		break;
		
		case CURLYB_FIRE_GUN:
			o->state_time++;
			
			if (!(o->state_time & 3))
			{	// time to fire
				
				// check if player is trying to jump over
				if (abs(o->x - player.x) < pixel_to_sub(32) && player.y + pixel_to_sub(10) < o->y)
				{	// shoot up instead
					SPR_SAFEANIM(o->sprite, 2);
					curlyboss_fire(o, 2);
				}
				else
				{
					SPR_SAFEANIM(o->sprite, 0);
					curlyboss_fire(o, o->direction);
				}
			}
			
			if (o->state_time > 30) o->state = CURLYB_FIGHT_START;
		break;
		
		case CURLYB_SHIELD:
			o->x_speed = 0;
			//if (++o->frame > 8) o->frame = 7;
			if (++o->state_time > 30)
			{
				//o->frame = 0;
				o->state = CURLYB_FIGHT_START;
			}
		break;
	}


	if (o->state > CURLYB_FIGHT_START && o->state < CURLYB_SHIELD)
	{
		// curly activates her shield anytime a missile's explosion goes off,
		// even if it's nowhere near her at all
		if(bullet_missile_is_exploding())
		{
			o->state_time = 0;
			o->state = CURLYB_SHIELD;
			SPR_SAFEANIM(o->sprite, 3);
			o->eflags &= ~NPC_SHOOTABLE;
			o->eflags |= NPC_INVINCIBLE;
			o->x_speed = 0;
		}
	}
	
	if (o->x_speed > CURLYB_WALK_SPEED) o->x_speed = CURLYB_WALK_SPEED;
	if (o->x_speed < -CURLYB_WALK_SPEED) o->x_speed = -CURLYB_WALK_SPEED;

	o->x_next = o->x + o->x_speed;

	collide_stage_leftwall(o);
	collide_stage_rightwall(o);

	o->x = o->x_next;
}

void ai_curlyBoss_onState(Entity *e) {
	if(e->state == STATE_DEFEATED) {
		SPR_SAFERELEASE(e->sprite);
		entity_default(e, OBJ_CURLY, 0);
		entity_sprite_create(e);
		e->x -= 8;
		e->eflags &= ~NPC_SHOOTABLE;
		e->state = 0;
		entities_clear_by_type(OBJ_CURLYBOSS_SHOT);
		tsc_call_event(e->event);
	}
}

void ai_curlyBossShot_onUpdate(Entity *o)
{
	o->x_next = o->x + o->x_speed;
	o->y_next = o->y + o->y_speed;
	if(collide_stage_leftwall(o) || collide_stage_rightwall(o) || collide_stage_ceiling(o)) {
		o->state = STATE_DELETE;
	} else if(!player_invincible() && entity_overlapping(o, &player)) {
		player_inflict_damage(o->attack);
		o->state = STATE_DELETE;
	} else {
		o->x = o->x_next;
		o->y = o->y_next;
	}
}
