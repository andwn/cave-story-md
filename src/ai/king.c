#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

void ai_king_onUpdate(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	entity_update_collision(e);
	switch(e->state) {
		case 0:
			SPR_SAFEANIM(e->sprite, 0);
			e->x_speed = e->y_speed = 0;
		break;
		
		case 5:			// he is knocked out
			SPR_SAFEANIM(e->sprite, 4);
			e->x_speed = 0;
		break;
		
		case 6:			// hop in air then fall and knocked out
			e->state = 7;
			e->state_time = 0;
			e->y_speed = -0x400;
			e->grounded = false;
		case 7:			// he falls and is knocked out
			SPR_SAFEANIM(e->sprite, 3);
			MOVE_X(0x200);
			e->y_speed += 0x40;
			if (e->state_time++)
			{
				if (e->grounded)
				{
					e->state = 5;
				}
			}
		break;
		
		case 8:		// walking
			e->state = 9;
			SPR_SAFEANIM(e->sprite, 1);
			//o->animtimer = 0;
		case 9:
			//ANIMATE(3, 4, 7);
			MOVE_X(0x200);
		break;
		
		case 10:		// run
			e->state = 11;
			SPR_SAFEANIM(e->sprite, 1);
			//o->animtimer = 0;
		case 11:
			//ANIMATE(2, 4, 7);
			MOVE_X(0x400);
		break;
		
		case 20:		// pull out sword
			if (!e->linkedEntity)
			{
				Entity *sword = entity_create(sub_to_block(e->x), sub_to_block(e->y), 
						0, 0, OBJ_KINGS_SWORD, 0, 0);
				sword->linkedEntity = e;
				e->linkedEntity = sword;
			}
			
			SPR_SAFEANIM(e->sprite, 0);
			e->state = 0;
		break;
		
		case 30:		// he goes flying in spec'd direction and smacks wall
			e->state = 31;
			e->state_time = 0;
			SPR_SAFEANIM(e->sprite, 3);
			MOVE_X(0x600);
			e->y_speed = 0;
			//o->nxflags &= ~NXFLAG_FOLLOW_SLOPE;		// necessary to keep him from going down slope...
		case 31:
			// Don't follow slopes
			e->grounded = false;
			e->y_speed = 0;
			e->y_next = e->y;
			if (e->x_speed == 0) // Hit the wall
			{
				e->direction = 1;
				e->state = 7;
				e->state_time = 0;
				e->y_speed = -0x400;
				e->grounded = false;
				e->x_speed = 0x200;
				sound_play(SND_LITTLE_CRASH, 5);
				//SmokeClouds(o, 4, 8, 8);
				//o->nxflags |= NXFLAG_FOLLOW_SLOPE;
			}
		break;
		
		case 40:		// he flickers away and is replaced by the Blade
			e->state = 41;
			e->state_time = 0;
		case 41:
			SPR_SAFEVISIBILITY(e->sprite, (e->state_time % 4) > 1 ? AUTO_FAST : HIDDEN);
			if (++e->state_time > 100)
			{
				//SmokeClouds(o, 4, 8, 8);
				e->state = 42;
				SPR_SAFEVISIBILITY(e->sprite, AUTO_FAST);
				//SPR_SAFEANIM(e->sprite, 5);
				//e->sprite = SPR_BLADE;
				//o->frame = o->yinertia = o->invisible = 0;
			}
		break;
		case 42: break;
		
		case 60:		// jump (used when he lunges with sword)
			SPR_SAFEANIM(e->sprite, 0);
			e->state = 61;
			e->y_speed = -0x4FF;
			e->x_speed = 0x400;
			SPR_SAFEHFLIP(e->linkedEntity->sprite, 1);
		break;
		case 61:		// jumping
			e->y_speed += 0x80;
			if (e->grounded)
			{
				e->state = 0;
				e->x_speed = 0;
				SPR_SAFEHFLIP(e->linkedEntity->sprite, 0);
			}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
	
	//if(!e->grounded) e->y_speed += GRAVITY_JUMP;
	LIMIT_X(0x400);
	LIMIT_Y(0x5FF);
}

void ai_king_onState(Entity *e) {
	
}
