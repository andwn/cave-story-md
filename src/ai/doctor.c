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

/*
	From King's Table, here's the Doctor's first form.
	He teleports around the room firing red wave shots at you with
	the Red Crystal following him. After every fourth teleport, he
	substitutes the wave attack for a aerial "explosion" of bouncy
	red shots (OBJ_DOCTOR_BLAST).
*/

#define savedhp		id

void ai_boss_doctor(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->y += (8 << CSF);
			e->frame = 3;
			e->state = 1;
			//e->BringToFront();		// make sure in front of doctor_crowned
			//crystal_tofront = true;	// make sure front crystal is in front of us
		}
		break;
		
		case 2:		// transforming (script)
		{
			e->timer++;
			e->frame = (e->timer & 4) ? 0 : 3;
			
			if (e->timer > TIME(50))
				e->state = 10;
		}
		break;
		
		case 10:	// base state/falling (script)
		{
			e->y_speed += SPEED(0x80);
			e->eflags |= NPC_SHOOTABLE;
			e->attack = 3;
			
			if (e->grounded) {
				e->state = 20;
				e->timer = 0;
				e->frame = 0;
				
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
			
			if (e->timer == TIME(50))
			{	// arm across chest
				FACE_PLAYER(e);
				e->frame = 4;
			}
			
			if (e->timer == TIME(80)) {
				//Entity *shot;
				e->frame = 5;	// arm cast out
				
				entity_create(e->x, e->y, OBJ_DOCTOR_SHOT, e->dir ? NPC_OPTION1 : 0);
				entity_create(e->x, e->y, OBJ_DOCTOR_SHOT, (e->dir ? NPC_OPTION1 : 0) | NPC_OPTION2);
				//FIRE_ANGLED_SHOT(OBJ_DOCTOR_SHOT, e->x, e->y, A_LEFT, SPEED(0x400));
				//FIRE_ANGLED_SHOT(OBJ_DOCTOR_SHOT, e->x, e->y, A_RIGHT, SPEED(0x400));
				//shot = SpawnEntityAtActionPoint(o, OBJ_DOCTOR_SHOT);
				//shot->dir = e->dir;
				//shot->angle = 0;
				
				//shot = SpawnEntityAtActionPoint(o, OBJ_DOCTOR_SHOT);
				//shot->dir = e->dir;
				//shot->angle = 0x80;
				
				sound_play(SND_FUNNY_EXPLODE, 5);
			}
			
			if (e->timer == TIME(120))
				e->frame = 0;	// arm down
			
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
			e->frame = 6;
			e->x_mark = e->x;
			e->eflags |= NPC_SHOOTABLE;
		}
		case 31:
		{
			e->x = e->x_mark;
			if (++e->timer & 2) e->x += (1 << CSF);
			
			if (e->timer > TIME(50)) {
				e->state = 32;
				e->timer = 0;
				e->frame = 7;
				
				sound_play(SND_LIGHTNING_STRIKE, 5);
				
				for(u16 angle = 8; angle < 256; angle += 16) {
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
			e->eflags &= ~NPC_SHOOTABLE;
			e->attack = 0;
			
			//dr_tp_out_init(o);
		}
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
				e->x_mark = (5 + (random() % 31)) << CSF << 4;
				e->y_mark = (5 + (random() % 3)) << CSF << 4;
			//}
		}
		break;
		
		case 102:	// invisible: waiting to reappear
		{
			if (++e->timer > TIME(40)) {
				e->state = 103;
				e->timer = 16;
				e->frame = 2;
				e->y_speed = 0;
				
				e->x = e->x_mark;
				e->y = e->y_mark;
				
				FACE_PLAYER(e);
			}
		}
		break;
		
		// tp back in
		case 103:
		{
			e->state++;
			//dr_tp_in_init(o);
		}
		case 104:
		{
			//if (dr_tp_in(o))
			//{
				e->eflags |= NPC_SHOOTABLE;
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
			e->eflags &= ~NPC_SHOOTABLE;
			e->frame = 6;
			
			// fall to earth
			e->y_speed += SPEED(0x10);
			if (e->grounded && e->y_speed >= 0) {
				e->state = 501;
				e->timer = 0;
				
				e->x_mark = e->x;
				FACE_PLAYER(e);
			}
		}
		break;
		
		case 501:	// flashing (transforming into Doctor 2)
		{
			FACE_PLAYER(e);
			e->frame = 8;
			
			e->x = e->x_mark;
			if (!(++e->timer & 2))
				e->x += (1 << CSF);
		}
		break;
	}
	
	// enable per-frame bbox
	//COPY_PFBOX;
	
	// set crystal follow position
	if (e->state >= 10) {
		if (e->hidden)	// teleporting
		{
			crystal_xmark = e->x_mark;
			crystal_ymark = e->y_mark;
		} else {
			crystal_xmark = e->x;
			crystal_ymark = e->y;
		}
	}
	
	LIMIT_Y(SPEED(0x5ff));
}

// wave shot
void ai_doctor_shot(Entity *e) {
	if (e->x < 0 || e->x > stageWidth << 13) {
		e->state = STATE_DELETE;
		return;
	}
	
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			if(e->eflags & NPC_OPTION2) e->dir = 1;
			if(e->eflags & NPC_OPTION1) e->jump_time = 0x80;
			e->x_mark = e->x;
			e->y_mark = e->y;
		}
		case 1:
		{
			// distance apart from each other
			if (e->timer2 < 128)
				e->timer2++;
			
			// spin
			e->jump_time += 6;
			
			// travel
			e->x_speed += (e->dir) ? 0x15 : -0x15;
			//e->x += e->x_speed;
			e->x_mark += e->x_speed;
			
			e->x = e->x_mark + ((cos[e->jump_time] * e->timer2) >> 3);
			e->y = e->y_mark + ((sin[e->jump_time] * e->timer2) >> 1);
			
			//Entity *trail = CreateEntity(e->x, e->y, OBJ_DOCTOR_SHOT_TRAIL);
			//trail->sprite = SPR_DOCTOR_SHOT;
			//trail->frame = 1;
			//trail->PushBehind(o);
		}
		break;
	}
}

//void ai_doctor_shot_trail(Entity *e) {
	//ANIMATE_FWD(3);
	//if (e->frame > 3)
	//	e->Delete();
//}

// from his "explosion" attack
void ai_doctor_blast(Entity *e) {
	// they're bouncy
	if ((e->x_speed < 0 && blk(e->x, -8, e->y, 0) == 0x41) ||
		(e->x_speed > 0 && blk(e->x, 8, e->y, 0) == 0x41)) {
		e->x_speed = -e->x_speed;
	}
	
	if (e->y_speed > 0 && blk(e->x, 0, e->y, -8) == 0x41)
		e->y_speed = -SPEED(0x200);
	
	if (e->y_speed < 0 && blk(e->x, 0, e->y, 8) == 0x41)
		e->y_speed = SPEED(0x200);
	
	ANIMATE(e, 2, 0,1);
	
	//if ((++e->timer % 4) == 1)
	//	CreateEntity(e->x, e->y, OBJ_DOCTOR_SHOT_TRAIL)->PushBehind(o);
	
	e->x += e->x_speed;
	e->y += e->y_speed;
	
	if (e->timer > TIME(250))
		e->state = STATE_DELETE;
}

// The Doctor's red crystal.
// There are actually two, one is behind him and one is in front
// and they alternate visibility as they spin around him so it looks 3D.
//
// This function has to be an aftermove, otherwise, because one is in front
// and the other behind, one will be checking crystal_xmark before the Doctor
// updates it, and the other afterwards, and they will get out of sync.
void aftermove_red_crystal(Entity *e) {
	ANIMATE(e, 4, 0,1);
	
	switch(e->state) {
		case 0:
		{
			if (crystal_xmark != 0) {
				e->state = 1;
				//crystal_tofront = true;
			}
		}
		break;
		
		case 1:
		{
			e->x_speed += (e->x < crystal_xmark) ? 0x55 : -0x55;
			e->y_speed += (e->y < crystal_ymark) ? 0x55 : -0x55;
			LIMIT_X(0x400);
			LIMIT_Y(0x400);
			
			if ((!e->dir && e->x_speed > 0) ||
				(e->dir && e->x_speed < 0)) {
				e->hidden = TRUE;
			} else {
				e->hidden = FALSE;
			}
		}
		break;
	}
	
	//if (crystal_tofront && e->dir == LEFT) {
	//	e->BringToFront();
	//	crystal_tofront = FALSE;
	//}
}

// doctor as npc before fight
void ai_doctor_crowned(Entity *e) {
	switch(e->state) {
		case 0:
		{
			// do this manually instead of a spawn point,
			// cause he's gonna transform.
			e->x -= (8 << CSF);
			e->y -= (16 << CSF);
			
			e->state = 1;
			//crystal_xmark = crystal_ymark = 0;
			//crystal_tofront = true;
		}
		case 1:		// faces away
		{
			e->frame = 0;
		}
		break;
		
		case 10:	// goes "ho ho ho" (while still facing away)
		{
			e->frame = 0;
			e->animtime = 0;
			e->timer = 0;
			e->state = 11;
		}
		case 11:
		{
			ANIMATE(e, 8, 0,1);
			// he has to show shrug frame exactly 6 times.
			// ANIMATE(5) changes frame on every 6th tick
			// so this is 6*6*nframes(2) = 72
			if (++e->timer >= 128)
				e->state = 1;
		}
		break;
		
		case 20:	// turns around (faces screen instead of away)
		{
			e->state = 21;
			e->frame = 2;
		}
		break;
		
		case 40:	// arm up--presents red crystal
		{
			e->state = 41;
			
			// spawn the red crystal
			// one is for when it's behind him, the other is in front.
			//int x = e->x - (6 << CSF);
			//int y = e->y - (8 << CSF);
			
			//dr_create_red_crystal(x, y);
		}
		case 41:
		{
			e->frame = 4;
		}
		break;
		
		case 50:	// "ho ho ho" (while facing player)
		{
			e->frame = 4;
			e->animtime = 0;
			e->timer = 0;
			e->state = 51;
		}
		case 51:
		{
			ANIMATE(e, 8, 4,5);
			if (++e->timer >= 72)
				e->state = 41;
		}
		break;
	}
}
