#include "ai_common.h"

/*
	From King's Table, here's the Doctor's first form.
	He teleports around the room firing red wave shots at you with
	the Red Crystal following him. After every fourth teleport, he
	substitutes the wave attack for a aerial "explosion" of bouncy
	red shots (OBJ_DOCTOR_BLAST).
*/

#define savedhp		id
#define angle		jump_time

void ai_boss_doctor(Entity *e) {
	enum { STAND1, STAND2, HOVER, SHOOT1, SHOOT2, BLAST1, BLAST2, DEFEAT };
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	if(e->x_speed < 0) collide_stage_leftwall(e);
	if(e->x_speed > 0) collide_stage_rightwall(e);
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);
	
	switch(e->state) {
		case 0:
		{
			e->flags &= ~NPC_SHOOTABLE;
			e->y_next += (8 << CSF);
			e->frame = STAND1;
			e->state = 1;
			//e->BringToFront();		// make sure in front of doctor_crowned
			//crystal_tofront = true;	// make sure front crystal is in front of us
		}
		break;
		
		case 2:		// transforming (script)
		{
			e->timer++;
			//e->frame = (e->timer & 4) ? 0 : 3;
			
			if (e->timer > TIME(50)) e->state = 10;
		}
		break;
		
		case 10:	// base state/falling (script)
		{
			e->y_speed += SPEED(0x80);
			e->flags |= NPC_SHOOTABLE;
			e->attack = 3;
			
			if (e->grounded) {
				e->state = 20;
				e->timer = 0;
				e->frame = STAND1;
				
				e->savedhp = e->health;
				FACE_PLAYER(e);
			}
		}
		break;
		
		// fire wave shot
		case 20:
		{
			e->timer++;
			
			if (e->timer < TIME(50)) {
				if ((e->health - e->savedhp) > 20)
					e->timer = TIME(50);
			}
			
			if (e->timer == TIME(50)) {	// arm across chest
				FACE_PLAYER(e);
				e->frame = SHOOT1;
			}
			
			if (e->timer == TIME(80)) {
				e->frame = SHOOT2;	// arm cast out
				
				entity_create(e->x_next, e->y_next, OBJ_DOCTOR_SHOT, e->dir ? NPC_OPTION1 : 0);
				entity_create(e->x_next, e->y_next, OBJ_DOCTOR_SHOT, (e->dir ? NPC_OPTION1 : 0) | NPC_OPTION2);
				
				sound_play(SND_FUNNY_EXPLODE, 5);
			}
			
			if (e->timer == TIME(120))
				e->frame = STAND1;	// arm down
			
			if (e->timer > TIME(130)) {
				if ((e->health - e->savedhp) > 50) {
					e->state = 100;
					e->timer = 0;
				}
				
				if (e->timer > TIME(160)) {
					e->state = 100;
					e->timer = 0;
				}
			}
		}
		break;
		
		// big "explosion" blast
		case 30:
		{
			e->state = 31;
			e->timer = 0;
			e->frame = BLAST1;
			e->x_mark = e->x_next;
			e->flags |= NPC_SHOOTABLE;
		} /* fallthrough */
		case 31:
		{
			e->x_next = e->x_mark;
			if (++e->timer & 2) e->x_next += (1 << CSF);
			
			if (e->timer > TIME(50)) {
				e->state = 32;
				e->timer = 0;
				e->frame = BLAST2;
				
				sound_play(SND_LIGHTNING_STRIKE, 5);
				
				for(uint16_t angle = 8; angle < 256; angle += 16) {
					FIRE_ANGLED_SHOT(OBJ_DOCTOR_BLAST, e->x, e->y, angle, SPEED(0x400));
				}
			}
		}
		break;
		case 32:	// after blast
		{
			if (++e->timer > TIME(50)) e->state = 100;
		}
		break;
		
		// teleport away
		case 100:
		{
			e->state = 101;
			e->flags &= ~NPC_SHOOTABLE;
			e->attack = 0;
			
			//dr_tp_out_init(o);
		} /* fallthrough */
		case 101:
		{
			//if (dr_tp_out(o)) {
				e->state = 102;
				e->timer = 0;
				e->hidden = TRUE;
				
				// decide where we're going to go now, so the red crystal
				// can start moving towards it. But, it's important not to
				// actually move until the last possible second, or we could
				// drag our floattext along with us (and give away our position).
				e->x_mark = block_to_sub(5 + (rand() & 31));
				e->y_mark = block_to_sub(5 + (rand() & 3));
			//}
		}
		break;
		
		case 102:	// invisible: waiting to reappear
		{
			if (++e->timer > TIME(40)) {
				e->state = 103;
				e->timer = 16;
				e->frame = HOVER;
				e->y_speed = 0;
				
				e->x_next = e->x_mark;
				e->y_next = e->y_mark;
				
				FACE_PLAYER(e);
			}
		}
		break;
		
		// tp back in
		case 103:
		{
			e->state++;
			e->hidden = FALSE;
			//dr_tp_in_init(o);
		} /* fallthrough */
		case 104:
		{
			//if (dr_tp_in(o))
			//{
				e->flags |= NPC_SHOOTABLE;
				e->attack = 3;
				
				if (++e->timer2 >= 4) {	// big explode
					e->timer2 = 0;
					e->state = 30;
				} else {	// another wave shot
					e->state = 10;
				}
			//}
		}
		break;
		
		// defeated!
		case 500:
		{
			e->flags &= ~NPC_SHOOTABLE;
			e->frame = DEFEAT;
			
			// fall to earth
			e->y_speed += SPEED(0x10);
			if (e->grounded && e->y_speed >= 0) {
				e->state = 501;
				e->timer = 0;
				
				e->x_mark = e->x_next;
				FACE_PLAYER(e);
			}
		}
		break;
		
		case 501:	// flashing (transforming into Doctor 2)
		{
			FACE_PLAYER(e);
			e->frame = DEFEAT;
			
			e->x_next = e->x_mark;
			if (!(++e->timer & 2))
				e->x_next += (1 << CSF);
		}
		break;
	}
	
	// set crystal follow position
	if (e->state >= 10) {
		if (e->hidden)	// teleporting
		{
			crystal_xmark = e->x_mark;
			crystal_ymark = e->y_mark;
		} else {
			crystal_xmark = e->x_next;
			crystal_ymark = e->y_next;
		}
	}
	
	// Move in front of crystal if it needs to go behind
	if(crystal_state == CRYSTAL_TOBACK) {
		moveMeToFront = TRUE;
		crystal_state = CRYSTAL_INBACK;
	}
	
	e->x = e->x_next;
	e->y = e->y_next;
	
	LIMIT_Y(SPEED(0x5ff));
}

// wave shot
void ai_doctor_shot(Entity *e) {
	if ((e->x < 0) || e->x > ((int32_t)stageWidth) << 13) {
		e->state = STATE_DELETE;
		return;
	}
	
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			if(e->flags & NPC_OPTION2) e->angle = 0x80;
			if(e->flags & NPC_OPTION1) e->dir = 1;
			e->x_mark = e->x;
			e->y_mark = e->y;
		} /* fallthrough */
		case 1:
		{
			// distance apart from each other
			if (e->timer2 < 128) e->timer2++;
			
			// spin
			e->angle += 6;
			
			// travel
			e->x_speed += (e->dir) ? 0x15 : -0x15;
			e->x_mark += e->x_speed;
			
			e->x = e->x_mark + (((int32_t)cos[e->angle] * (int32_t)e->timer2) >> 3);
			e->y = e->y_mark + (((int32_t)sin[e->angle] * (int32_t)e->timer2) >> 1);
		}
		break;
	}
}

// from his "explosion" attack
void ai_doctor_blast(Entity *e) {
	e->timer++;
	// they're bouncy
	if (e->timer & 1) {
		if (e->x_speed < 0 && blk(e->x, -6, e->y, 0) == 0x41)
			e->x_speed = abs(e->x_speed);
			
		if (e->x_speed > 0 && blk(e->x, 6, e->y, 0) == 0x41)
			e->x_speed = -abs(e->x_speed);
		
		if (e->y_speed < 0 && blk(e->x, 0, e->y, -6) == 0x41)
			e->y_speed = SPEED(0x200);
		
		if (e->y_speed > 0 && blk(e->x, 0, e->y, 6) == 0x41)
			e->y_speed = -SPEED(0x200);
	}
	
	e->x += e->x_speed;
	e->y += e->y_speed;
	
	if (e->timer & 2) e->frame ^= 1;
	
	if (e->timer > TIME(250)) {
		e->state = STATE_DELETE;
		effect_create_smoke(e->x >> CSF, e->y >> CSF);
	}
}

void onspawn_red_crystal(Entity *e) {
	e->alwaysActive = TRUE;
	crystal_entity = e;
}

void ai_red_crystal(Entity *e) {
	ANIMATE(e, 4, 0,1);
	
	switch(e->state) {
		case 0:
		{
			if (crystal_xmark != 0) {
				e->state = 1;
				e->timer = 0;
				crystal_state = CRYSTAL_TOFRONT;
			}
		}
		break;
		
		case 1:
		{
			e->x_speed += (e->x < crystal_xmark) ? SPEED(0x55) : -SPEED(0x55);
			e->y_speed += (e->y < crystal_ymark) ? SPEED(0x55) : -SPEED(0x55);
			LIMIT_X(SPEED(0x400));
			LIMIT_Y(SPEED(0x400));
			
			if ((!e->dir && e->x_speed > 0) || (e->dir && e->x_speed < 0)) {
				if(crystal_state == CRYSTAL_INBACK) crystal_state = CRYSTAL_TOFRONT;
			} else {
				if(crystal_state == CRYSTAL_INFRONT) crystal_state = CRYSTAL_TOBACK;
			}
		}
		break;
	}
	
	if(crystal_state == CRYSTAL_TOFRONT) {
		moveMeToFront = TRUE;
		crystal_state = CRYSTAL_INFRONT;
	}
	e->x += e->x_speed;
	e->y += e->y_speed;
}

// doctor as npc before fight
void ai_doctor_crowned(Entity *e) {
	enum { STAND1, STAND2, FLOAT, HAND1, HAND2, AWAY1, AWAY2 };
	switch(e->state) {
		case 0:
		{
			e->y -= (8 << CSF);
			e->state = 1;
			crystal_xmark = crystal_ymark = 0;
		} /* fallthrough */
		case 1:		// faces away
		{
			e->frame = AWAY1;
		}
		break;
		
		case 10:	// goes "ho ho ho" (while still facing away)
		{
			e->frame = AWAY1;
			e->animtime = 0;
			e->timer = 0;
			e->state = 11;
		} /* fallthrough */
		case 11:
		{
			ANIMATE(e, 8, AWAY1,AWAY2);
			if (++e->timer >= 128) e->state = 1;
		}
		break;
		
		case 20:	// turns around (faces screen instead of away)
		{
			e->state = 21;
			e->frame = STAND1;
		}
		break;
		
		case 40:	// arm up--presents red crystal
		{
			e->state = 41;
			entity_create(e->x - (14 << CSF), e->y - (16 << CSF), OBJ_RED_CRYSTAL, 0);
		} /* fallthrough */
		case 41:
		{
			e->frame = HAND1;
		}
		break;
		
		case 50:	// "ho ho ho" (while facing player)
		{
			e->frame = HAND1;
			e->animtime = 0;
			e->timer = 0;
			e->state = 51;
		} /* fallthrough */
		case 51:
		{
			ANIMATE(e, 8, HAND1,HAND2);
			if (++e->timer >= 72) e->state = 41;
		}
		break;
	}
	
	if(crystal_state == CRYSTAL_TOBACK) {
		moveMeToFront = TRUE;
		crystal_state = CRYSTAL_INBACK;
	}
}
