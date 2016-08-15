#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "camera.h"

void ai_torokoAtk_onCreate(Entity *e) {
	e->y -= block_to_sub(1);
	e->x_speed = 0x300; // 1.5px
	e->state = 3; // Running back and forth
	e->spriteAnim = 2;
}

void ai_toroko_onUpdate(Entity *e) {
	switch(e->state) {
	case 0: // Stand still
		SPR_SAFEANIM(e->sprite, 0);
		break;
	case 3: // Run back and forth
	case 4:
		if(e->attack == 0) {
			SPR_SAFEANIM(e->sprite, 1);
		} else {
			SPR_SAFEANIM(e->sprite, 2);
			Bullet *b = bullet_colliding(e);
			if(b != NULL) {
				sound_play(e->hurtSound, 10); // Squeak
				e->attack = 0; // Don't hurt the player anymore
				e->eflags |= NPC_INTERACTIVE; // Enable interaction
				e->state = 10; // Change animation to falling on ground
				e->y_speed = pixel_to_sub(-1);
				e->x_speed /= 2;
				e->grounded = false;
				SPR_SAFEANIM(e->sprite, 3);
				b->ttl = 0;
				SPR_SAFERELEASE(b->sprite);
			}
		}
		// Switch direction in specific range
		if((e->x_speed > 0 && e->x > block_to_sub(15)) || 
			(e->x_speed < 0 && e->x < block_to_sub(10))) {
			e->direction = !e->direction;
			e->x_speed = -0x300 + 0x600 * e->direction;
			SPR_SAFEHFLIP(e->sprite, e->direction);
		}
		break;
	case 6: // Jump then run
		if(e->grounded && abs(e->x_speed) < 0x300) {
			e->x_speed = -0x300 + 0x600 * e->direction;
			e->state = 7; // Toroko stops after hitting a wall so don't keep doing this
		}
		break;
	case 7:
		break;
	case 8: // Jump in place (don't run after)
		break;
	case 10: // Falling down
		if(e->grounded) {
			e->x_speed = 0;
			e->state = 11;
			SPR_SAFEANIM(e->sprite, 4);
		}
		break;
	case 11: // After falling on ground
		e->direction = 0;
		break;
	default:
		break;
	}
	if(!e->grounded) e->y_speed += GRAVITY_JUMP;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_toroko_onState(Entity *e) {
	switch(e->state) {
	case 6: // Jump
		SPR_SAFEANIM(e->sprite, 1);
		e->y_speed = -0x150;
		e->x_speed = -0x150 + 0x300 * e->direction;
		e->grounded = false;
		break;
	case 8:
		e->y_speed = 0x150;
		e->grounded = false;
		break;
	default:
		break;
	}
}

void ai_torokoBoss_onCreate(Entity *e) {
	e->spriteAnim = 10;
}

void ai_torokoBoss_onUpdate(Entity *o) {
	Entity *block = entity_find_by_type(OBJ_TOROKO_BLOCK);

#define SPAWNBLOCK	\
{					\
	block = entity_create(0, 0, 0, 0, OBJ_TOROKO_BLOCK, 0, o->direction);	\
	block->x = o->x + (o->direction ? 0x100 : -0x100); \
	block->y = o->y - 0x200; \
	block->eflags &= ~NPC_INVINCIBLE;		\
	block->nflags &= ~NPC_INVINCIBLE;		\
}
#define THROWBLOCK	\
{			\
	block->x += pixel_to_sub(16) * (o->direction ? 1 : -1);	\
	block->y += pixel_to_sub(9);	\
	block->eflags |= NPC_INVINCIBLE;		\
	block->x_speed = o->direction ? 0x600 : -0x600;		\
	block->y_speed = (o->y - player.y) - abs(o->x - player.x) / 4; \
	if(block->y_speed > 0x300) block->y_speed = 0x300; \
	sound_play(SND_EM_FIRE, 5);		\
}

#define HOLDBRICKTIME		30

	o->x_next = o->x + o->x_speed;
	o->y_next = o->y + o->y_speed;

	switch(o->state)
	{
		case 0:
			o->y_next -= 0x400;
			o->grounded = true;
			o->state = 1;
			//o->frame = 9;
			o->eflags &= ~(NPC_INTERACTIVE | NPC_SHOOTABLE | NPC_IGNORESOLID);
		case 1:		// wait a sec before morphing
			if (++o->state_time > 50)
			{
				o->state_time = 0;
				o->state = 2;
				SPR_SAFEANIM(o->sprite, 11);
				//o->frame = 8;
			}
		break;
		
		case 2:		// morph into big toroko
			//ANIMATE(0, 9, 10);
			SPR_SAFEANIM(o->sprite, (o->state_time & 1) ? 11 : 7);
			if (++o->state_time > 50)
			{
				o->state = 3;
				o->state_time = 0;
				o->eflags |= NPC_SHOOTABLE;
			}
		break;
		
		case 3:		// rest a moment, then jump
			SPR_SAFEANIM(o->sprite, 0);
			//o->frame = 1;
			if (++o->state_time > 5)
			{
				o->state = 10;
				o->eflags |= NPC_SHOOTABLE;
			}
		break;
		
		case 10:	// wait a moment then ATTACK!!
			o->state = 11;
			SPR_SAFEANIM(o->sprite, 0);
			//o->frame = 0;
			//o->animtimer = 0;
			o->state_time = (random() % 110) + 20;
			o->x_speed = 0;
		case 11:
			FACE_PLAYER(o);
			SPR_SAFEHFLIP(o->sprite, o->direction);
			//ANIMATE(4, 0, 1);
			///FIXME:: I think Toroko does not like fireball either?
			//if (o->frame==0 && (sound_is_playing(SND_MISSILE_HIT) /*|| sound_is_playing(0)*/))
			//{
			//	o->state = 20;
			//}
			
			if (!o->state_time)
			{
				o->state = (random() & 1) ? 20 : 50;
			}
			else o->state_time--;
		break;
		
		case 20:	// init for a jump
			o->state = 21;
			SPR_SAFEANIM(o->sprite, 1);
			//o->frame = 2;
			o->state_time = 0;
		case 21:	// preparing to jump
			if (++o->state_time > 10)
			{
				o->state = 22;
				o->state_time = 0;
				SPR_SAFEANIM(o->sprite, 2);
				//o->frame = 3;
				o->grounded = false;
				o->y_speed = -0x5ff;
				o->x_speed = o->direction ? 0x200 : -0x200;
			}
		break;
		case 22:	// jumping up
			if (++o->state_time > 10) { 
				o->state = 23; 
				o->state_time = 0; 
				SPR_SAFEANIM(o->sprite, 5);
				//o->frame = 6; 
				SPAWNBLOCK; 
			}
		break;
		case 23:	// preparing to throw block
			if (++o->state_time > HOLDBRICKTIME) { 
				o->state = 24; 
				o->state_time = 0; 
				SPR_SAFEANIM(o->sprite, 6);
				//o->frame = 7; 
				THROWBLOCK; 
			}
			FACE_PLAYER(o);
		break;
		case 24:	// threw block
			if (++o->state_time > 3) { 
				o->state = 25; 
				SPR_SAFEANIM(o->sprite, 2);
				//o->frame = 3; 
			}
		break;
		case 25:	// coming back down
			if ((o->grounded = collide_stage_floor(o)))
			{
				o->state = 26;
				o->state_time = 0;
				SPR_SAFEANIM(o->sprite, 1);
				//o->frame = 2;
				sound_play(SND_QUAKE, 5);
				camera_shake(20);
			}
		break;
		case 26:	// landed
			o->x_speed *= 8;
			o->x_speed /= 9;
			if (++o->state_time > 20)
			{
				o->state = 10;
				SPR_SAFEANIM(o->sprite, 0);
				//o->frame = 0;
			}
		break;
		
		case 50:	// throw a block (standing on ground)
			o->state = 51;
			o->state_time = 0;
			SPR_SAFEANIM(o->sprite, 3);
			//o->frame = 4;
			SPAWNBLOCK;
		case 51:
			if (++o->state_time > HOLDBRICKTIME)
			{
				o->state = 52;
				o->state_time = 0;
				SPR_SAFEANIM(o->sprite, 4);
				//o->frame = 5;
				THROWBLOCK;
			}
			FACE_PLAYER(o);
		break;
		case 52:
			if (++o->state_time > 3) { 
				o->state = 10; 
				SPR_SAFEANIM(o->sprite, 0);
				//o->frame = 0; 
			}
		break;
		
		case 100:		// defeated (set by Script On Death)
			//o->frame = 3;
			SPR_SAFEANIM(o->sprite, 2);
			o->state = 101;
			o->eflags &= ~NPC_SHOOTABLE;
			//SmokeClouds(o, 8, 8, 8);
		case 101:		// wait till fall to ground
			if ((o->grounded = collide_stage_floor(o)))
			{
				o->state = 102;
				o->state_time = 0;
				//o->frame = 2;
				SPR_SAFEANIM(o->sprite, 1);
				sound_play(SND_QUAKE, 5);
				camera_shake(20);
			}
		break;
		case 102:		// crouching & smoking
			o->x_speed *= 8; 
			o->x_speed /= 9;
			if (++o->state_time > 50) { 
				o->state = 103; 
				o->state_time = 0; 
				SPR_SAFEANIM(o->sprite, 7);
				//o->frame = 10; 
			}
		break;
		case 103:		// looks pained
			if (++o->state_time > 50) { 
				o->state = 104; 
				o->state_time = 0; 
				SPR_SAFEANIM(o->sprite, 8);
				//o->frame = 9; 
			}
		break;
		case 104:		// morphing back into normal toroko
			SPR_SAFEANIM(o->sprite, (o->state_time & 1) ? 11 : 7);
			//o->frame = (o->frame==9) ? 10:9;
			if (++o->state_time > 100) { 
				o->state = 105; 
				o->state_time = 0; 
				SPR_SAFEANIM(o->sprite, 11);
				//o->frame = 9; 
			}
		break;
		case 105:		// back to normal
			if (++o->state_time > 50) { 
				o->state = 106; 
				o->state_time = 0;
				//o->animtimer = 0; 
				SPR_SAFEANIM(o->sprite, 8);
				//o->frame = 11; 
			}
		break;
		case 106:		// red goes out of her eyes, she falls down
			if (++o->state_time > 50) {
				SPR_SAFEANIM(o->sprite, 9);
			}
			//if (++o->animtimer > 50)
			//{	// collapse
			//	o->animtimer = 0;
			//	if (++o->frame > 12) o->frame = 12;
			//}
		break;
		
		case 140:		// disappear
			o->state = 141;
			o->state_time = 0;
			//o->frame = 12;
			sound_play(SND_TELEPORT, 5);
		case 141:
			SPR_SAFEVISIBILITY(o->sprite, o->state_time & 1);
			//o->invisible ^= 1;
			if (++o->state_time > 100)
			{
				// normal SmokeClouds() doesn't spawn in the right place because the sprite
				// is still sized for big toroko, so I made an Action Point for this frame.
				//SmokePuff(o->ActionPointX(), o->ActionPointY());
				//SmokePuff(o->ActionPointX(), o->ActionPointY());
				//SmokePuff(o->ActionPointX(), o->ActionPointY());
				//SmokePuff(o->ActionPointX(), o->ActionPointY());
				o->state = STATE_DELETE;
			}
		break;
	}
	
	SPR_SAFEANIM(o->sprite, o->direction);

	//if(!o->grounded) o->grounded = collide_stage_floor(o);
	//else o->grounded = collide_stage_floor_grounded(o);

	o->x = o->x_next;
	o->y = o->y_next;

	//if (o->state > 100 && o->state <= 105)
	//{
	//	if ((o->timer % 9)==1)
	//	{
	//		SmokeClouds(o, 1, 8, 8);
	//	}
	//}
	
	if(!o->grounded) o->y_speed += 0x20;
	//LIMITY(0x5ff);
}

void ai_torokoBoss_onState(Entity *e) {
	if(e->state == STATE_DEFEATED) {
		e->state = STATE_DELETE;
		e->eflags &= ~NPC_SHOOTABLE;
		e->nflags &= ~NPC_SHOOTABLE;
		tsc_call_event(e->event);
	}
}

// the blocks Frenzied Toroko throws
void ai_torokoBlock_onUpdate(Entity *o)
{
	o->x_next = o->x + o->x_speed;
	o->y_next = o->y + o->y_speed;
	if(collide_stage_leftwall(o) || collide_stage_rightwall(o) || collide_stage_floor(o)) {
		sound_play(SND_BLOCK_DESTROY, 5);
		SPR_SAFERELEASE(o->sprite);
		entity_default(o, OBJ_TOROKO_FLOWER, 0);
		//o->frame = 0;
		entity_sprite_create(o);
		o->state = 20;
		o->x_speed = 0;
		o->eflags &= ~NPC_INVINCIBLE;
		o->eflags |= NPC_SHOOTABLE;
		//o->state = STATE_DELETE;
	} else if(entity_overlapping(o, &player)) {
		if(!player_invincible()) player_inflict_damage(o->attack);
		sound_play(SND_BLOCK_DESTROY, 5);
		o->state = STATE_DELETE;
	} else {
		o->x = o->x_next;
		o->y = o->y_next;
	}
}

void ai_torokoFlower_onUpdate(Entity *o)
{
	switch(o->state)
	{
		case 10:
			o->state = 11;
			o->state_time = 0;
		case 11:
			if (++o->state_time > 30)
			{
				o->state = 12;
				o->state_time = 0;
				//o->frame = 1;
				//o->animtimer = 0;
			}
		break;
		case 12:
			//ANIMATE_FWD(8);
			if (++o->state_time > 30)
			{
				o->state = 20;
				o->y_speed = -0x200;
				o->x_speed = (o->x > player.x) ? -0x200 : 0x200;
			}
		break;
		
		case 20:	// falling/jumping
			//if (o->yinertia > -0x80) o->frame = 4; else o->frame = 3;
			if ((o->grounded = collide_stage_floor(o)))
			{
				//o->frame = 2;
				o->state = 21;
				o->state_time = o->x_speed = 0;
				sound_play(SND_THUD, 3);
			}
		break;
		case 21:
			if (++o->state_time > 10) { 
				o->state = 10; 
				//o->frame = 0; 
			}
		break;
	}
	
	o->y_speed += 0x40;
	//LIMITY(0x5ff);
}
