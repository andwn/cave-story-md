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
	e->y -= 8 << CSF;
}

void ai_torokoBoss_onUpdate(Entity *e) {
	Entity *block = e->linkedEntity;

#define SPAWNBLOCK	({					\
	block = entity_create(sub_to_block(e->x), sub_to_block(e->y), 0, 0, OBJ_TOROKO_BLOCK, 0, e->direction);	\
	e->linkedEntity = block; \
	block->linkedEntity = e; \
	block->eflags &= ~NPC_INVINCIBLE;		\
	block->nflags &= ~NPC_INVINCIBLE;		\
})
#define THROWBLOCK	({			\
	block->x = e->x;	\
	block->y = e->y;	\
	block->eflags |= NPC_INVINCIBLE;		\
	block->x_speed = (((s32)((u16)(abs(player.x - block->x) >> 5)) / TIME(25))) << 5; \
	block->y_speed = (((s32)((u16)(abs(player.y - block->y) >> 5)) / TIME(25))) << 5; \
	if(block->x > player.x) block->x_speed = -block->x_speed; \
	if(block->y > player.y) block->y_speed = -block->y_speed; \
	sound_play(SND_EM_FIRE, 5);		\
})

	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;

	switch(e->state) {
		case 0:
		{
			e->grounded = true;
			e->state = 1;
			e->eflags &= ~(NPC_INTERACTIVE | NPC_SHOOTABLE | NPC_IGNORESOLID);
		}
		/* no break */
		case 1:		// wait a sec before morphing
		{
			if (++e->state_time > TIME(50)) {
				e->state_time = 0;
				e->state = 2;
				SPR_SAFEANIM(e->sprite, 11);
			}
		}
		break;
		case 2:		// morph into big toroko
		{
			SPR_SAFEANIM(e->sprite, (e->state_time & 1) ? 11 : 7);
			if (++e->state_time > TIME(50)) {
				e->state = 3;
				e->state_time = 0;
				e->eflags |= NPC_SHOOTABLE;
			}
		}
		break;
		case 3:		// rest a moment, then jump
		{
			SPR_SAFEANIM(e->sprite, 0);
			if (++e->state_time > TIME(5)) {
				e->state = 10;
				e->eflags |= NPC_SHOOTABLE;
			}
		}
		break;
		case 10:	// wait a moment then ATTACK!!
		{
			e->state = 11;
			SPR_SAFEANIM(e->sprite, 0);
			e->state_time = (random() % TIME(110)) + TIME(20);
			e->x_speed = 0;
		}
		/* no break */
		case 11:
		{
			FACE_PLAYER(e);
			if (!e->state_time) {
				e->state = (random() & 1) ? 20 : 50;
			}
			else e->state_time--;
		}
		break;
		case 20:	// init for a jump
		{
			e->state = 21;
			SPR_SAFEANIM(e->sprite, 1);
			e->state_time = 0;
		}
		/* no break */
		case 21:	// preparing to jump
		{
			if (++e->state_time > TIME(10)) {
				e->state = 22;
				e->state_time = 0;
				SPR_SAFEANIM(e->sprite, 2);
				e->grounded = false;
				e->y_speed = SPEED(-0x5ff);
				MOVE_X(SPEED(0x200));
			}
		}
		break;
		case 22:	// jumping up
		{
			if (++e->state_time > TIME(10)) {
				e->state = 23;
				e->state_time = 0;
				SPR_SAFEANIM(e->sprite, 5);
				SPAWNBLOCK; 
			}
		}
		break;
		case 23:	// preparing to throw block
		{
			if (++e->state_time > TIME(30)) {
				e->state = 24;
				e->state_time = 0;
				FACE_PLAYER(e);
				SPR_SAFEANIM(e->sprite, 6);
				THROWBLOCK; 
			}
		}
		break;
		case 24:	// threw block
		{
			if (++e->state_time > TIME(5)) {
				e->state = 25;
				SPR_SAFEANIM(e->sprite, 2);
			}
		}
		break;
		case 25:	// coming back down
		{
			if ((e->grounded = collide_stage_floor(e))) {
				e->state = 26;
				e->state_time = 0;
				SPR_SAFEANIM(e->sprite, 1);
				sound_play(SND_QUAKE, 5);
				camera_shake(20);
			}
		}
		break;
		case 26:	// landed
		{
			e->x_speed *= 8;
			e->x_speed /= 9;
			if (++e->state_time > TIME(20)) {
				e->state = 10;
				SPR_SAFEANIM(e->sprite, 0);
			}
		}
		break;
		case 50:	// throw a block (standing on ground)
		{
			e->state = 51;
			e->state_time = 0;
			SPR_SAFEANIM(e->sprite, 3);
			SPAWNBLOCK;
		}
		/* no break */
		case 51:
		{
			if (++e->state_time > TIME(30)) {
				e->state = 52;
				e->state_time = 0;
				FACE_PLAYER(e);
				SPR_SAFEANIM(e->sprite, 4);
				THROWBLOCK;
			}

		}
		break;
		case 52:
		{
			if (++e->state_time > TIME(5)) {
				e->state = 10;
				SPR_SAFEANIM(e->sprite, 0);
			}
		}
		break;
		case 100:		// defeated (set by Script On Death)
		{
			SPR_SAFEANIM(e->sprite, 2);
			e->state = 101;
			e->eflags &= ~NPC_SHOOTABLE;
			//SmokeClouds(o, 8, 8, 8);
		}
		/* no break */
		case 101:		// wait till fall to ground
		{
			if ((e->grounded = collide_stage_floor(e))) {
				e->state = 102;
				e->state_time = 0;
				SPR_SAFEANIM(e->sprite, 1);
				sound_play(SND_QUAKE, 5);
				camera_shake(20);
			}
		}
		break;
		case 102:		// crouching & smoking
		{
			e->x_speed *= 8;
			e->x_speed /= 9;
			if (++e->state_time > TIME(50)) {
				e->state = 103;
				e->state_time = 0;
				SPR_SAFEANIM(e->sprite, 7);
			}
		}
		break;
		case 103:		// looks pained
		{
			if (++e->state_time > TIME(50)) {
				e->state = 104;
				e->state_time = 0;
				SPR_SAFEANIM(e->sprite, 8);
			}
		}
		break;
		case 104:		// morphing back into normal toroko
		{
			SPR_SAFEANIM(e->sprite, (e->state_time & 1) ? 11 : 7);
			if (++e->state_time > TIME(100)) {
				e->state = 105;
				e->state_time = 0;
				SPR_SAFEANIM(e->sprite, 11);
			}
		}
		break;
		case 105:		// back to normal
		{
			if (++e->state_time > TIME(50)) {
				e->state = 106;
				e->state_time = 0;
				SPR_SAFEANIM(e->sprite, 8);
			}
		}
		break;
		case 106:		// red goes out of her eyes, she falls down
		{
			if (++e->state_time > TIME(50)) SPR_SAFEANIM(e->sprite, 9);
		}
		break;
		case 140:		// disappear
		{
			e->state = 141;
			e->state_time = 0;
			sound_play(SND_TELEPORT, 5);
		}
		/* no break */
		case 141:
		{
			SPR_SAFEVISIBILITY(e->sprite, e->state_time & 1);
			if (++e->state_time > TIME(100)) {
				// normal SmokeClouds() doesn't spawn in the right place because the sprite
				// is still sized for big toroko, so I made an Action Point for this frame.
				//SmokePuff(e->ActionPointX(), e->ActionPointY());
				//SmokePuff(e->ActionPointX(), e->ActionPointY());
				//SmokePuff(e->ActionPointX(), e->ActionPointY());
				//SmokePuff(e->ActionPointX(), e->ActionPointY());
				e->state = STATE_DELETE;
			}
		}
		break;
	}
	
	if(e->x_speed > 0) collide_stage_rightwall(e);
	if(e->x_speed < 0) collide_stage_leftwall(e);

	e->x = e->x_next;
	e->y = e->y_next;

	//if (e->state > 100 && e->state <= 105)
	//{
	//	if ((e->timer % 9)==1)
	//	{
	//		SmokeClouds(o, 1, 8, 8);
	//	}
	//}
	
	if(!e->grounded) e->y_speed += SPEED(0x20);
}

void ai_torokoBoss_onState(Entity *e) {
	if(e->state == STATE_DEFEATED) {
		//e->state = STATE_DELETE;
		e->eflags &= ~NPC_SHOOTABLE;
		e->nflags &= ~NPC_SHOOTABLE;
		tsc_call_event(e->event);
	}
}

// the blocks Frenzied Toroko throws
void ai_torokoBlock_onUpdate(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(collide_stage_leftwall(e) || collide_stage_rightwall(e) || collide_stage_floor(e)) {
		sound_play(SND_BLOCK_DESTROY, 5);
		SPR_SAFERELEASE(e->sprite);
		entity_default(e, OBJ_TOROKO_FLOWER, 0);
		entity_sprite_create(e);
		e->state = 20;
		e->grounded = false;
		e->x_speed = 0;
		e->eflags &= ~NPC_INVINCIBLE;
		e->eflags |= NPC_SHOOTABLE;
	} else if(entity_overlapping(e, &player)) {
		if(!player_invincible()) player_inflict_damage(e->attack);
		sound_play(SND_BLOCK_DESTROY, 5);
		e->state = STATE_DELETE;
	} else {
		e->x = e->x_next;
		e->y = e->y_next;
	}
}

void ai_torokoFlower_onUpdate(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		case 10:
		{
			e->state = 11;
			e->state_time = 0;
		}
		/* no break */
		case 11:
		{
			if (++e->state_time > TIME(30)) {
				e->state = 12;
				e->state_time = 0;
			}
		}
		break;
		case 12:
		{
			if (++e->state_time > TIME(30)) {
				e->state = 20;
				e->y_speed = SPEED(-0x200);
				e->grounded = false;
				FACE_PLAYER(e);
				MOVE_X(SPEED(0x200));
			}
		}
		break;
		case 20:	// falling/jumping
		{
			if ((e->grounded = collide_stage_floor(e))) {
				e->state = 21;
				e->state_time = e->x_speed = 0;
				sound_play(SND_THUD, 3);
			}
		}
		break;
		case 21:
		{
			if (++e->state_time > TIME(10)) e->state = 10;
		}
		break;
	}
	
	if(!e->grounded) e->y_speed += SPEED(0x40);
	LIMIT_Y(SPEED(0x5ff));
	e->x = e->x_next;
	e->y = e->y_next;
}
