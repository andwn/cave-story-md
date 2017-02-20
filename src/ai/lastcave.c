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
#include "sprite.h"

void ai_proximity_press_vert(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->grounded = TRUE;
			if (PLAYER_DIST_X(8<<CSF) && PLAYER_DIST_Y2(8<<CSF, 128<<CSF) && 
					!(blk(e->x, 0, e->y, e->hit_box.bottom + 1) == 0x41)) {
				e->state = 10;
				e->grounded = FALSE;
				e->animtime = 0;
				e->frame = 1;
			}
		}
		break;
		
		case 10:
		{
			if (e->frame < 2 && ++e->animtime > 4) {
				e->animtime = 0;
				e->frame++;
			}
			
			e->x_next = e->x;
			e->y_next = e->y + e->y_speed;
			if ((e->grounded = collide_stage_floor(e))) {
				if (e->frame >= 2) { // make sure eye fully open
					effect_create_smoke(e->x >> CSF, (e->y >> CSF) + e->hit_box.bottom);
					camera_shake(10);
				}
				
				e->eflags |= NPC_SPECIALSOLID;
				e->attack = 0;
				
				e->state = 11;
				e->frame = 0;
			} else {
				if (player.y - (player.hit_box.top << CSF) > e->y) {
					e->eflags &= ~NPC_SPECIALSOLID;
					e->attack = 127;
				} else {
					e->eflags |= NPC_SPECIALSOLID;
					e->attack = 0;
				}
			}
			e->x = e->x_next;
			e->y = e->y_next;
		}
		break;
	}
	
	if (e->state >= 5) {
		if(!e->grounded) e->y_speed += SPEED(0x80);
		LIMIT_Y(SPEED(0x5ff));
	}
}

// lava drip spawner in Last Cave (hidden) and used extensively in Final Cave.
// the id1 tag sets the frequency of the drip, the id2 tag sets an amount to
// delay the first drip in order to desync a group of drips.
void ai_lava_drip_spawner(Entity *e) {
	switch(e->state) {
		case 0:
		{
			//e->sprite = SPR_LAVA_DRIP;
			e->x += (4 << CSF);
			e->timer = abs(e->event - e->id) + 20;
			e->state = 1;
			e->frame = 1;
			e->hidden = TRUE;
		}
		case 1:
		{
			if (e->timer == 0) {
				e->state = 2;
				e->animtime = 0;
				e->timer2 = 0;
				e->hidden = FALSE;
			} else e->timer--;
		}
		break;
		
		case 2:
		{
			//e->display_xoff = (++e->timer2 & 2) ? 0 : 1;
			
			if (e->frame <= 3 && ++e->animtime > 10) {
				e->animtime = 0;
				e->frame++;
			}
			if (e->frame > 3) {
				e->frame = 1;
				e->state = 1;
				e->timer = e->id + 20;
				e->hidden = TRUE;
				entity_create(e->x, e->y, OBJ_LAVA_DRIP, 0);
			}
		}
		break;
	}
}

void ai_lava_drip(Entity *e) {
	e->frame = 0;
	e->y_speed += SPEED(0x40);
	LIMIT_Y(SPEED(0x5ff));
	e->y += e->y_speed;
	
	u8 blockd = blk(e->x, 0, e->y, 0);
	if (blockd == 0x41 || (blockd & BLOCK_WATER)) {
		//for(int i=0;i<3;i++) {
		//	Caret *c = effect(e->CenterX(), e->Bottom(), EFFECT_LAVA_SPLASH);
		//	c->x_speed = random(-0x400, 0x400);
		//	c->y_speed = random(-0x400, 0);
		//}
		
		sound_play(SND_BUBBLE, 2);
		
		e->state = STATE_DELETE;
	}
}

void ai_red_bat_spawner(Entity *e) {
	switch(e->state) {
		case 0:
		{
			//if(PLAYER_DIST_Y(64 << CSF)) {
				if(e->eflags & NPC_OPTION2) e->dir = 1;
				e->state = 1;
				e->timer = TIME(200) + (random() % TIME(200));
			//}
		}
		break;
		case 1:
		{
			if (e->timer == 0) {
				Entity *bat = entity_create(e->x, e->y - (32 << CSF) + (random() % (64 << CSF)),
											OBJ_RED_BAT, 0);
				bat->dir = e->dir;
				e->state = 0;
			} else e->timer--;
		}
	}
}

void ai_red_bat(Entity *e) {
	ANIMATE(e, 4, 0,1,2);
	
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->y_mark = e->y;
			e->timer = random() % TIME(50);
		}
		case 1:
		{
			if (e->timer == 0) {
				e->state = 2;
				e->y_speed = SPEED(0x400);
			} else {
				e->timer--;
				break;
			}
		}
		case 2:
		{
			e->y_speed += (e->y < e->y_mark) ? SPEED(0x10) : -SPEED(0x10);
			LIMIT_Y(SPEED(0x300));
			MOVE_X(SPEED(0x100));
		}
		break;
	}
	
	e->x += e->x_speed;
	e->y += e->y_speed;
	
	if (e->x < 0 || e->x > block_to_sub(stageWidth)) {
		//effect(e->CenterX(), e->CenterY(), EFFECT_BOOMFLASH);
		e->state = STATE_DELETE;
	}
}

void ai_red_demon(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	if(e->x_speed > 0) collide_stage_rightwall(e);
	if(e->x_speed < 0) collide_stage_leftwall(e);
	if(e->y_speed < 0) collide_stage_ceiling(e);
	else if(!e->grounded) e->grounded = collide_stage_floor(e);
	
	switch(e->state)
	{
		case 0:
		{
			e->x_mark = e->x;
			e->x_speed = 0;
			e->frame = 0;
			e->state = 1;
		}
		case 1:
		{
			ANIMATE(e, 20, 0,1);
			FACE_PLAYER(e);
		}
		break;
		
		case 10:	// prepare to jump
		{
			e->eflags |= NPC_SHOOTABLE;
			e->state = 11;
			e->frame = 3;
			e->timer = 0;
		}
		case 11:
		{
			switch(++e->timer)
			{
				case 30:
				case 40:
				case 50:
				{
					e->frame = 4;
					//EmFireAngledShot(o, OBJ_RED_DEMON_SHOT, 0, 0x800);
					Entity *shot = entity_create(e->x, e->y, OBJ_RED_DEMON_SHOT, 0);
					THROW_AT_TARGET(shot, player.x, player.y, 0x800);
					sound_play(SND_EM_FIRE, 5);
				}
				break;
				
				case 34:
				case 44:
				case 54:
				{
					e->frame = 3;
				}
				break;
				
				case 61:
				{
					e->state = 20;
					e->timer = 0;
					e->frame = 2;
				}
				break;
			}
		}
		break;
		
		case 20:	// pause before jump
		{
			if (++e->timer > TIME(20)) {
				e->state = 21;
				e->timer = 0;
				e->frame = 5;
				e->grounded = FALSE;
				e->y_speed = -SPEED(0x5ff);
				e->x_speed = (e->x < player.x) ? SPEED(0x100) : -SPEED(0x100);
			}
		}
		break;
		
		case 21:	// in air
		{
			switch(++e->timer)
			{
				case 30:
				case 40:
				case 50:
				{
					e->frame = 6;
					//EmFireAngledShot(o, OBJ_RED_DEMON_SHOT, 0, 0x800);
					Entity *shot = entity_create(e->x, e->y, OBJ_RED_DEMON_SHOT, 0);
					THROW_AT_TARGET(shot, player.x, player.y, 0x800);
					sound_play(SND_EM_FIRE, 5);
				}
				break;
				
				case 34:
				case 44:
				{
					e->frame = 5;
				}
				break;
				
				case 54:
				{
					e->frame = 7;
				}
				break;
			}
			
			if (e->grounded) {
				camera_shake(10);
				e->state = 22;
				e->timer = 0;
				e->frame = 2;
			}
		}
		break;
		
		case 22:	// landed
		{
			e->x_speed /= 2;
			
			if (++e->timer > TIME(22)) {
				e->state = 10;
			}
		}
		break;
		
		// defeated/turned to stone (set by script)
		case 50:
		{
			e->eflags &= ~NPC_SHOOTABLE;
			e->attack = 0;
			
			if (e->grounded) {
				e->state = 51;
				e->frame = 2;
				
				camera_shake(10);
				//SmokeClouds(o, 12, 4, 4);
				//e->SpawnXP(19);
				
				sound_play(SND_BIG_CRASH, 6);
				
				// needed to prevent status bars from not disappearing
				//game.bossbar.object = NULL;
			}
		}
		break;
		
		case 51:
		{
			e->x_speed *= 7;
			e->x_speed /= 8;
			
			e->frame = 8;
		}
		break;
	}
	
	if (e->state < 50) {
		FACE_PLAYER(e);
	}
	
	e->x = e->x_next;
	e->y = e->y_next;
	
	if(!e->grounded) {
		e->y_speed += SPEED(0x20);
		LIMIT_Y(SPEED(0x5ff));
	}
}

void ondeath_red_demon(Entity *e) {
	tsc_call_event(e->event);
}
