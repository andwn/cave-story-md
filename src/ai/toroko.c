#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "camera.h"
#include "effect.h"

void onspawn_torokoAtk(Entity *e) {
	e->y -= 16 << CSF;
	e->x_speed = SPEED(0x400); // 1.5px
	e->state = 3; // Running back and forth
	e->frame = 3;
}

void ai_torokoAtk(Entity *e) {
	switch(e->state) {
	case 0: // Stand still
		e->frame = 0;
		break;
	case 3: // Run back and forth
	case 4:
		ANIMATE(e, 4, 3,4);
		Bullet *b = bullet_colliding(e);
		if(b) {
			sound_play(e->hurtSound, 10); // Squeak
			e->attack = 0; // Don't hurt the player anymore
			e->eflags |= NPC_INTERACTIVE; // Enable interaction
			e->state = 10; // Change animation to falling on ground
			e->y_speed = -SPEED(0x200);
			e->x_speed >>= 1;
			e->grounded = FALSE;
			e->frame = 5;
			b->ttl = 0;
		}
		// Switch direction in specific range
		if((e->x_speed > 0 && e->x > block_to_sub(15)) || 
			(e->x_speed < 0 && e->x < block_to_sub(11))) {
			e->dir ^= 1;
			e->x_speed = e->dir ? SPEED(0x400) : -SPEED(0x400);
		}
		break;
	case 10: // Falling down
		if(e->grounded) {
			e->x_speed = 0;
			e->state = 11;
			e->frame = 6;
		}
		break;
	case 11: // After falling on ground
		e->dir = 0;
		break;
	default:
		e->dir = 0;
		e->frame = 0;
	}
	if(!e->grounded) e->y_speed += SPEED(0x20);
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	entity_update_collision(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

void onspawn_torokoBoss(Entity *e) {
	e->frame = 10;
	e->y -= 8 << CSF;
}

void ai_torokoBoss(Entity *e) {
	Entity *block = e->linkedEntity;

#define SPAWNBLOCK ({                                                                          \
	block = entity_create(e->x, e->y, OBJ_TOROKO_BLOCK, 0);	                                   \
	e->linkedEntity = block;                                                                   \
	block->dir = e->dir;                                                                       \
	block->linkedEntity = e;                                                                   \
	block->eflags &= ~NPC_INVINCIBLE;                                                          \
	block->nflags &= ~NPC_INVINCIBLE;                                                          \
})

#define THROWBLOCK ({                                                                          \
	if(block) {                                                                                \
		block->x = e->x;                                                                       \
		block->y = e->y;                                                                       \
		block->eflags |= NPC_INVINCIBLE;                                                       \
		block->x_speed = (((s32)((u16)(abs(player.x - block->x) >> 5)) / TIME(25))) << 5;      \
		block->y_speed = (((s32)((u16)(abs(player.y - block->y) >> 5)) / TIME(25))) << 5;      \
		if(block->x > player.x) block->x_speed = -block->x_speed;                              \
		if(block->y > player.y) block->y_speed = -block->y_speed;                              \
		sound_play(SND_EM_FIRE, 5);                                                            \
		e->linkedEntity = NULL;                                                                \
		block->linkedEntity = NULL;                                                            \
	}                                                                                          \
})

	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;

	switch(e->state) {
		case 0:
		{
			e->grounded = TRUE;
			e->state = 1;
			e->eflags &= ~(NPC_INTERACTIVE | NPC_SHOOTABLE | NPC_IGNORESOLID);
		}
		/* no break */
		case 1:		// wait a sec before morphing
		{
			if (++e->timer > TIME(50)) {
				e->timer = 0;
				e->state = 2;
				e->frame = 12;
			}
		}
		break;
		case 2:		// morph into big toroko
		{
			e->frame = (e->timer & 1) ? 12 : 8;
			if (++e->timer > TIME(50)) {
				e->state = 3;
				e->timer = 0;
				e->eflags |= NPC_SHOOTABLE;
			}
		}
		break;
		case 3:		// rest a moment, then jump
		{
			ANIMATE(e, 8, 0,1);
			if (++e->timer > TIME(5)) {
				e->state = 10;
				e->eflags |= NPC_SHOOTABLE;
			}
		}
		break;
		case 10:	// wait a moment then ATTACK!!
		{
			e->state = 11;
			e->timer = (random() % TIME(110)) + TIME(20);
			e->x_speed = 0;
		}
		/* no break */
		case 11:
		{
			ANIMATE(e, 8, 0,1);
			FACE_PLAYER(e);
			if (!e->timer) {
				e->state = (random() & 1) ? 20 : 50;
			}
			else e->timer--;
		}
		break;
		case 20:	// init for a jump
		{
			e->state = 21;
			e->frame = 2;
			e->timer = 0;
		}
		/* no break */
		case 21:	// preparing to jump
		{
			if (++e->timer > TIME(10)) {
				e->state = 22;
				e->timer = 0;
				e->frame = 3;
				e->grounded = FALSE;
				e->y_speed = SPEED(-0x5ff);
				MOVE_X(SPEED(0x200));
			}
		}
		break;
		case 22:	// jumping up
		{
			if (++e->timer > TIME(10)) {
				e->state = 23;
				e->timer = 0;
				e->frame = 6;
				SPAWNBLOCK; 
			}
		}
		break;
		case 23:	// preparing to throw block
		{
			if (++e->timer > TIME(30)) {
				e->state = 24;
				e->timer = 0;
				FACE_PLAYER(e);
				e->frame = 7;
				THROWBLOCK; 
			}
		}
		break;
		case 24:	// threw block
		{
			if (++e->timer > TIME(5)) {
				e->state = 25;
				e->frame = 3;
			}
		}
		break;
		case 25:	// coming back down
		{
			if ((e->grounded = collide_stage_floor(e))) {
				e->state = 26;
				e->timer = 0;
				e->frame = 2;
				sound_play(SND_QUAKE, 5);
				camera_shake(20);
			}
		}
		break;
		case 26:	// landed
		{
			e->x_speed *= 8;
			e->x_speed /= 9;
			if (++e->timer > TIME(20)) {
				e->state = 10;
				e->frame = 0;
			}
		}
		break;
		case 50:	// throw a block (standing on ground)
		{
			e->state = 51;
			e->timer = 0;
			e->frame = 4;
			SPAWNBLOCK;
		}
		/* no break */
		case 51:
		{
			if (++e->timer > TIME(30)) {
				e->state = 52;
				e->timer = 0;
				FACE_PLAYER(e);
				e->frame = 5;
				THROWBLOCK;
			}

		}
		break;
		case 52:
		{
			if (++e->timer > TIME(5)) {
				e->state = 10;
				e->frame = 0;
			}
		}
		break;
		case 100:		// defeated (set by Script On Death)
		{
			e->frame = 3;
			e->state = 101;
			e->eflags &= ~NPC_SHOOTABLE;
			SMOKE_AREA((e->x >> CSF) - 16, (e->y >> CSF) - 16, 32, 32, 4);
		}
		/* no break */
		case 101:		// wait till fall to ground
		{
			if ((e->grounded = collide_stage_floor(e))) {
				e->state = 102;
				e->timer = 0;
				e->frame = 2;
				sound_play(SND_QUAKE, 5);
				camera_shake(20);
			}
		}
		break;
		case 102:		// crouching & smoking
		{
			e->x_speed *= 8;
			e->x_speed /= 9;
			if (++e->timer > TIME(50)) {
				e->state = 103;
				e->timer = 0;
				e->frame = 8;
			}
		}
		break;
		case 103:		// looks pained
		{
			if (++e->timer > TIME(50)) {
				e->state = 104;
				e->timer = 0;
				e->frame = 9;
			}
		}
		break;
		case 104:		// morphing back into normal toroko
		{
			e->frame = (e->timer & 1) ? 12 : 8;
			if (++e->timer > TIME(100)) {
				e->state = 105;
				e->timer = 0;
				e->frame = 12;
			}
		}
		break;
		case 105:		// back to normal
		{
			if (++e->timer > TIME(50)) {
				e->state = 106;
				e->timer = 0;
				e->frame = 9;
			}
		}
		break;
		case 106:		// red goes out of her eyes, she falls down
		{
			if (++e->timer > TIME(50)) e->frame = 10;
		}
		break;
		case 140:		// disappear
		{
			e->state = 141;
			e->timer = 0;
			sound_play(SND_TELEPORT, 5);
		}
		/* no break */
		case 141:
		{
			e->hidden ^= 1;
			if (++e->timer > TIME(100)) {
				SMOKE_AREA((e->x >> CSF) - 8, e->y >> CSF, 16, 16, 2);
				e->state = STATE_DELETE;
			}
		}
		break;
	}
	
	if(e->x_speed > 0) collide_stage_rightwall(e);
	if(e->x_speed < 0) collide_stage_leftwall(e);

	e->x = e->x_next;
	e->y = e->y_next;

	if (e->state > 100 && e->state <= 105) {
		if (!(e->timer % 16)) {
			SMOKE_AREA((e->x >> CSF) - 16, (e->y >> CSF) - 16, 32, 32, 3);
		}
	}
	
	if(!e->grounded) e->y_speed += SPEED(0x20);
}

void ondeath_torokoBoss(Entity *e) {
	e->eflags &= ~NPC_SHOOTABLE;
	e->nflags &= ~NPC_SHOOTABLE;
	e->nflags &= ~NPC_SHOWDAMAGE;
	tsc_call_event(e->event);
}

// the blocks Frenzied Toroko throws
void ai_torokoBlock(Entity *e) {
	if(e->linkedEntity) {
		e->x = e->linkedEntity->x + (e->linkedEntity->dir ? 16 << CSF : -(16 << CSF));
		e->y = e->linkedEntity->y - (16 << CSF);
		return;
	}
	ANIMATE(e, 4, 0,1);
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(collide_stage_leftwall(e) || collide_stage_rightwall(e) || collide_stage_floor(e)) {
		sound_play(SND_BLOCK_DESTROY, 5);
		e->type = OBJ_TOROKO_FLOWER;
		e->oframe = 255;
		e->frame = 0;
		effect_create_smoke(e->x >> CSF, e->y >> CSF);
		e->state = 20;
		e->grounded = FALSE;
		e->x_speed = 0;
		e->eflags &= ~NPC_INVINCIBLE;
		e->nflags &= ~NPC_INVINCIBLE;
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

void ai_torokoFlower(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	ANIMATE(e, 8, 0,1,2,3,4);
	switch(e->state) {
		case 10:
		{
			e->state = 11;
			e->timer = 0;
		}
		/* no break */
		case 11:
		{
			if (++e->timer > TIME(30)) {
				e->state = 12;
				e->timer = 0;
			}
		}
		break;
		case 12:
		{
			if (++e->timer > TIME(30)) {
				e->state = 20;
				e->y_speed = SPEED(-0x200);
				e->grounded = FALSE;
				FACE_PLAYER(e);
				MOVE_X(SPEED(0x200));
			}
		}
		break;
		case 20:	// falling/jumping
		{
			if ((e->grounded = collide_stage_floor(e))) {
				e->state = 21;
				e->timer = e->x_speed = 0;
				sound_play(SND_THUD, 3);
			}
		}
		break;
		case 21:
		{
			if (++e->timer > TIME(10)) e->state = 10;
		}
		break;
	}
	
	if(!e->grounded) e->y_speed += SPEED(0x40);
	LIMIT_Y(SPEED(0x5ff));
	e->x = e->x_next;
	e->y = e->y_next;
}
