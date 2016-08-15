#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "effect.h"
#include "camera.h"
#include "system.h"

#ifdef PAL
#define BLOCK_SOUND_INTERVAL	10
#define BLOCK_TRAVEL_ACCEL		0x20
#define BLOCK_TRAVEL_SPEED		0x200
#else
#define BLOCK_SOUND_INTERVAL	12
#define BLOCK_TRAVEL_ACCEL		0x1B
#define BLOCK_TRAVEL_SPEED		0x1B0
#endif

void ai_block_onCreate(Entity *e) {
	e->x += pixel_to_sub(8);
	e->y += pixel_to_sub(8);
	e->hit_box = (bounding_box) { 16, 16, 16, 16 };
	e->display_box = (bounding_box) { 16, 16, 16, 16 };
	e->eflags |= NPC_SOLID;
	e->eflags |= NPC_IGNORE44;
	e->enableSlopes = false;
	e->attack = 0;
	e->state = (e->eflags & NPC_OPTION2) ? 20 : 10;
}

void ai_blockh_onUpdate(Entity *e) {
	switch(e->state) {
		case 10:
		if((player.x > e->x && player.x - e->x < 0x3200) ||
			(player.x < e->x && e->x - player.x < 0x32000)) {
			if(PLAYER_DIST_Y(0x3200)) {
				e->state = 30;
				e->state_time = 0;
			}
		}
		break;
		case 20:
		if((player.x > e->x && player.x - e->x < 0x32000) ||
			(player.x < e->x && e->x - player.x < 0x3200)) {
			if(PLAYER_DIST_Y(0x3200)) {
				e->state = 30;
				e->state_time = 0;
			}
		}
		break;
		case 30:
		{
			u16 dir = e->eflags & NPC_OPTION2;
			e->x_speed += dir ? BLOCK_TRAVEL_ACCEL : -BLOCK_TRAVEL_ACCEL;
			if(e->x_speed > BLOCK_TRAVEL_SPEED) e->x_speed = BLOCK_TRAVEL_SPEED;
			if(e->x_speed < -BLOCK_TRAVEL_SPEED) e->x_speed = -BLOCK_TRAVEL_SPEED;
			e->x_next = e->x + e->x_speed;
			// hit edge
			if(stage_get_block_type(
					sub_to_block(e->x_next + 0x3200), sub_to_block(e->y) == 0x41 ||
				stage_get_block_type(
					sub_to_block(e->x_next - 0x3200), sub_to_block(e->y) == 0x41))) {
				camera_shake(10);
				e->x_speed = 0;
				e->eflags ^= NPC_OPTION2;
				e->state = dir ? 10 : 20;
			} else {
				e->x = e->x_next;
				if((++e->state_time % BLOCK_SOUND_INTERVAL) == 6) {
					sound_play(SND_BLOCK_MOVE, 2);
				}
			}
		}
		break;
	}
}

void ai_blockv_onUpdate(Entity *e) {
	switch(e->state) {
		case 10:
		if((player.y > e->y && player.y - e->y < 0x3200) ||
			(player.y < e->y && e->y - player.y < 0x32000)) {
			if(PLAYER_DIST_X(0x3200)) {
				e->state = 30;
				e->state_time = 0;
			}
		}
		break;
		case 20:
		if((player.y > e->y && player.y - e->y < 0x32000) ||
			(player.y < e->y && e->y - player.y < 0x3200)) {
			if(PLAYER_DIST_X(0x3200)) {
				e->state = 30;
				e->state_time = 0;
			}
		}
		break;
		case 30:
		e->y_speed += e->direction ? BLOCK_TRAVEL_ACCEL : -BLOCK_TRAVEL_ACCEL;
		if(e->y_speed > BLOCK_TRAVEL_SPEED) e->y_speed = BLOCK_TRAVEL_SPEED;
		if(e->y_speed < -BLOCK_TRAVEL_SPEED) e->y_speed = -BLOCK_TRAVEL_SPEED;
		e->y_next = e->y + e->y_speed;
		// hit edge
		if((e->direction && collide_stage_floor(e)) || 
			(!e->direction && collide_stage_ceiling(e))) {
			
			camera_shake(10);
			
			e->y_speed = 0;
			e->direction ^= 1;
			e->state = (e->direction) ? 20 : 10;
		}
		e->y = e->y_next;
		if((++e->state_time % BLOCK_SOUND_INTERVAL) == 6) {
			sound_play(SND_BLOCK_MOVE, 2);
		}
		break;
	}
}

void ai_boulder_onUpdate(Entity *e)
{
	switch(e->state)
	{
		// shaking
		case 10:
		{
			e->state = 11;
			e->state_time = 0;
			e->x_mark = e->x;
		}
		case 11:
		{
			if ((++e->state_time % 3) != 0)
				e->x = e->x_mark + (1 << 9);
			else
				e->x = e->x_mark;
		}
		break;
		
		// thrown away by Balrog
		case 20:
		{
			e->y_speed = -0x400;
			e->x_speed = 0x100;
			sound_play(SND_FUNNY_EXPLODE, 5);
			
			e->state = 21;
			e->state_time = 0;
		}
		case 21:
		{
			e->y_speed += 0x10;
			
			if (collide_stage_floor(e) && e->y_speed >= 0)
			{
				sound_play(SND_EXPLOSION1, 5);
				camera_shake(40);
				
				e->x_speed = 0;
				e->y_speed = 0;
				e->state = 0;
			}
		}
		break;
	}
}

#define GAUDI_HP 15
#define GAUDI_FLYING_HP 15
#define GAUDI_ARMORED_HP 15

void ai_gaudiDying_onUpdate(Entity *e)
{
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state)
	{
		case 0:		// just died (initilizing)
		{
			e->eflags &= ~(NPC_SHOOTABLE | NPC_IGNORESOLID | NPC_SHOWDAMAGE);
			e->attack = 0;
			
			//e->sprite = SPR_GAUDI;
			//e->frame = 9;
			SPR_SAFEANIM(e->sprite, 7);
			
			e->y_speed = -0x200;
			MOVE_X(-0x100);
			sound_play(SND_ENEMY_HURT_SMALL, 5);
			
			e->state = 1;
		}
		break;
		
		case 1:		// flying backwards through air
		{
			if (e->y_speed >= 0 && (e->grounded = collide_stage_floor(e)))
			{
				//e->frame = 10;
				SPR_SAFEANIM(e->sprite, 8);
				e->state = 2;
				e->state_time = 0;
			}
		}
		break;
		
		case 2:		// landed, shake
		{
			e->x_speed *= 8;
			e->x_speed /= 9;
			SPR_SAFEANIM(e->sprite, (e->state_time % 8) > 3 ? 9 : 8);
			
			if (++e->state_time > 50)
			{	// this deletes Entity while generating smoke effects and boom
				e->state = STATE_DESTROY;
			}
		}
		break;
	}
	
	e->x = e->x_next;
	e->y = e->y_next;
	
	if(!e->grounded) e->y_speed += 0x20;
	LIMIT_Y(0x5ff);
}

void ai_gaudi_onUpdate(Entity *e)
{
	if (e->health <= (1000 - GAUDI_HP))
	{
		SPR_SAFERELEASE(e->sprite);
		e->type = OBJ_GAUDI_DYING;
		entity_default(e, OBJ_GAUDI_DYING, 0);
		entity_sprite_create(e);
		ai_gaudiDying_onUpdate(e);
		return;
	}
	
	//if (!PLAYER_NEAR_ENOUGH())
	//	return;
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	switch(e->state)
	{
		case 0:
		{
			// Gaudi's in shop
			if (e->eflags & NPC_INTERACTIVE)
			{
				e->attack = 0;
				e->eflags &= ~NPC_SHOOTABLE;
			}
			
			e->x_speed = 0;
			e->state = 1;
		}
		case 1:
		{
			SPR_SAFEANIM(e->sprite, 1);
			//randblink(o, 1, 20, 100);
			
			if (!(random() % 100)) {
				if (random() & 1) {
					e->direction ^= 1;
				} else {
					e->state = 10;
				}
			}
		}
		break;
		
		case 10:		// walking
		{
			e->state = 11;
			e->state_time = (random() % 75) + 25;		// how long to walk for
			
			SPR_SAFEANIM(e->sprite, 1);
		}
		case 11:
		{
			// time to stop walking?
			if (--e->state_time <= 0)
				e->state = 0;
				
			MOVE_X(0x200);
			
			// try to jump over any walls we come to
			if ((e->x_speed < 0 && collide_stage_leftwall(e)) || \
				(e->x_speed > 0 && collide_stage_rightwall(e)))
			{
				e->y_speed = -0x5ff;
				//e->frame = 2;
				e->state = 20;
				e->state_time = 0;
				
				//if (!player)	// no sound during ending cutscene
					sound_play(SND_ENEMY_JUMP, 5);
			}
		}
		break;
		
		case 20:		// jumping
		{
			// landed?
			if (e->y_speed >= 0 && (e->grounded = collide_stage_floor(e)))
			{
				e->x_speed = 0;
				e->state = 21;
				//e->frame = 12;
				e->state_time = 0;
				
				//if (!player->inputs_locked)	// no sound during ending cutscene
					sound_play(SND_THUD, 5);
			}
			
			// count how long we've been touching the wall
			// we're trying to jump over..if it's not working
			// go the other way.
			if ((e->direction == 0 && collide_stage_leftwall(e)) || \
				(e->direction == 1 && collide_stage_rightwall(e))) {
				if (++e->state_time > 10) {
					e->state_time = 0;
					e->direction ^= 1;
				}
			} else {
				e->state_time = 0;
			}
			
			MOVE_X(0x100);
		}
		break;
		
		case 21:	// landed from jump
		{
			if (++e->state_time > 10)
				e->state = 0;
		}
		break;
	}
	
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);
	
	e->x = e->x_next;
	e->y = e->y_next;
	
	if(!e->grounded) e->y_speed += 0x40;
	LIMIT_Y(0x5ff);
}

/*
void c------------------------------() {}
*/

void ai_gaudiFlying_onUpdate(Entity *e)
{
	if (e->health <= (1000 - GAUDI_FLYING_HP))
	{
		if (e->direction == 0)
			e->x -= (2 << 9);
		else
			e->x += (2 << 9);
		
		SPR_SAFERELEASE(e->sprite);
		e->type =OBJ_GAUDI_DYING;
		entity_default(e, OBJ_GAUDI_DYING, 0);
		entity_sprite_create(e);
		ai_gaudiDying_onUpdate(e);
		return;
	}
	
	//if (!PLAYER_NEAR_ENOUGH())
	//	return;
	
	switch(e->state)
	{
		case 0:
		{
			//vector_from_angle(random(0, 255), (1 << 9), &e->x_speed, &e->y_speed);
			u16 angle = random() % 1024;
			e->x_speed = sintab32[angle] >> 1;
			e->y_speed = sintab32[(angle + 256) % 1024] >> 1;
			e->x_mark = e->x + (e->x_speed * 8);
			e->y_mark = e->y + (e->y_speed * 8);
			e->state = 1;
			e->state_time2 = 120;
			SPR_SAFEANIM(e->sprite, 10);
		}
		case 1:
		{
			e->state_time = (random() % 80) - 70;
			e->state = 2;
		}
		case 2:
		{
			if (!--e->state_time)
			{
				SPR_SAFEANIM(e->sprite, 11);
				e->state = 3;
				//e->frame |= 0x02;	// switch us into using flashing purple animation
			}
		}
		break;
		
		case 3:		// preparing to fire
		{
			e->state_time++;
			
			if (++e->state_time > 30)
			{
				//EmFireAngledShot(o, OBJ_GAUDI_FLYING_SHOT, 6, 0x500);
				sound_play(SND_EM_FIRE, 5);
				
				e->state = 1;
				SPR_SAFEANIM(e->sprite, 10);
				//e->frame &= 1;		// stop flashing purple
			}
		}
	}
	
	//e->frame ^= 1;		// animate wings
	FACE_PLAYER(e);
	SPR_SAFEHFLIP(e->sprite, e->direction);
	
	// sinusoidal circling pattern
	e->x_speed += (e->x > e->x_mark) ? -0x10 : 0x10;
	e->y_speed += (e->y > e->y_mark) ? -0x10 : 0x10;
	LIMIT_X(0x200);
	LIMIT_Y(0x200);
	e->x += e->x_speed;
	e->y += e->y_speed;
}

/*
void c------------------------------() {}
*/

void ai_gaudiArmored_onUpdate(Entity *e)
{
	if (e->health <= (1000 - GAUDI_ARMORED_HP))
	{
		SPR_SAFERELEASE(e->sprite);
		e->type =OBJ_GAUDI_DYING;
		entity_default(e, OBJ_GAUDI_DYING, 0);
		entity_sprite_create(e);
		ai_gaudiDying_onUpdate(e);
		return;
	}
	
	//if (!PLAYER_NEAR_ENOUGH())
	//	return;
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	FACE_PLAYER(e);
	SPR_SAFEHFLIP(e->sprite, e->direction);
	
	//debug("%d", e->state);
	//debugVline(e->x, 0, 0, 255);
	//debugVline(e->x_mark, 0, 255, 0);
	
	switch(e->state)
	{
		case 0:
		{
			SPR_SAFEANIM(e->sprite, 0);
			e->x_mark = e->x;
			e->state = 1;
		}
		case 1:
		{
			e->x_speed = 0;
			
			if (++e->state_time >= 5)
			{
				if (PLAYER_DIST_X(192 << 9) && PLAYER_DIST_Y(160 << 9))
				{	// begin hopping
					e->state = 10;
					e->state_time = 0;
					SPR_SAFEANIM(e->sprite, 1);
				}
			}
		}
		break;
		
		case 10:	// on ground inbetween hops
		{
			if (++e->state_time > 3)
			{
				sound_play(SND_ENEMY_JUMP, 5);
				SPR_SAFEANIM(e->sprite, 2);
				e->state_time = 0;
				
				if (++e->state_time2 < 3)
				{	// hopping back and forth
					e->state = 20;
					e->y_speed = -0x200;
					e->x_speed = (e->x < e->x_mark) ? 0x200 : -0x200;
				}
				else
				{	// big jump and attack
					e->state = 30;
					e->y_speed = -0x600;
					e->x_speed = (e->x < e->x_mark) ? 0x80 : -0x80;
					
					e->state_time2 = 0;
				}
			}
		}
		break;
		
		case 20:	// jumping (small hop)
		{
			// landed?
			if (e->y_speed >= 0 && (e->grounded = collide_stage_floor(e)))
			{
				// drop sub-pixel precision
				// (required to maintain stability of back-and-forth sequence).
				e->y >>= 9; e->y <<= 9;
				
				sound_play(SND_THUD, 5);
				e->state = 40;
				SPR_SAFEANIM(e->sprite, 1);
				e->state_time = 0;
			}
		}
		break;
		
		case 30:	// jumping (big jump + attack)
		{
			e->state_time++;
			
			// throw attacks at player
			if (e->state_time == 30 || e->state_time == 40)
			{
				//EmFireAngledShot(o, OBJ_GAUDI_ARMORED_SHOT, 6, 0x600);
				sound_play(SND_EM_FIRE, 5);
				
				SPR_SAFEANIM(e->sprite, 3);
				//e->CurlyTargetHere();
			}
			
			// stop throwing animation
			if (e->state_time == 35 || e->state_time == 45)
				SPR_SAFEANIM(e->sprite, 2);
			
			if (collide_stage_floor(e) && e->y_speed > 0)
			{
				sound_play(SND_THUD, 5);
				e->state = 40;
				SPR_SAFEANIM(e->sprite, 1);
				e->state_time = 0;
			}
		}
		break;
		
		case 40:		// landed
		{
			e->x_speed *= 7;
			e->x_speed /= 8;
			
			if (++e->state_time >= 2)
			{
				//stat("dtt= %d", abs(e->x_mark - e->x)>>CSF);
				SPR_SAFEANIM(e->sprite, 0);
				e->x_speed = 0;
				
				e->state = 1;
				e->state_time = 0;
			}
		}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
	
	if(!e->grounded) e->y_speed += 0x33;
	LIMIT_Y(0x5ff);
}


void ai_gaudiArmoredShot_onUpdate(Entity *e)
{
	//ANIMATE(0, 0, 2);
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state)
	{
		case 0:
		{
			bool bounced = false;
			if (e->x_speed <= 0 && collide_stage_leftwall(e)) { 
				e->x_speed = 0x200; 
				bounced = true; 
			}
			if (e->x_speed >= 0 && collide_stage_rightwall(e)) { 
				e->x_speed = -0x200; 
				bounced = true; 
			}
			if (e->y_speed >= 0 && collide_stage_floor(e)) { 
				e->y_speed = -0x200; 
				bounced = true; 
			}
			if (e->y_speed <= 0 && collide_stage_ceiling(e)) { 
				e->y_speed = 0x200; 
				bounced = true; 
			}
			
			if (bounced) {
				e->state = 1;
				sound_play(SND_TINK, 5);
			}
		}
		break;
		
		case 1:
		{
			e->y_speed += 0x40;
			LIMIT_Y(0x5ff);
			
			if (e->y_speed >= 0 && collide_stage_floor(e)) {
				e->state = STATE_DELETE;
				return;
			}
		}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
}
