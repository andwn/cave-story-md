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
#include "resources.h"

#ifndef KDB_AI
#define puts(x) /**/
#define printf(...) /**/
#endif

void ai_jenka(Entity *e) {
	switch(e->state) {
		case 0: 
		{
			e->state = 1; 
			e->frame = 0;
		}
		/* no break */
		case 1: 
		{
			if (++e->state_time > 30 && (random() % TIME(120)) < 10) {
				e->state = 2; 
				e->state_time = 0; 
				e->frame = 1;
			} 
		}
		break;
		case 2: 
		{
			if (++e->state_time > TIME(8)) {
				e->state = 1;
				e->state_time = 0;
				e->frame = 0;
			} 
		}
		break;
	}
}

void ai_doctor(Entity *e) {
	switch(e->state) {
		case 10:	// he chuckles
		{
			e->state = 11;
			e->state_time2 = 0;
			ANIMATE(e, 10, 1,2);
		}
		/* no break */
		case 11:
		{
			if (++e->state_time2 > TIME(8*6)) {
				e->frame = 0;
				e->state = 1; 
			}
		}
		break;
		case 20:	// he rises up and hovers
		{
			e->state = 21;
			e->state_time = 0;
			e->frame = 3;
			e->y_mark = e->y - (32 << CSF);
		}
		/* no break */
		case 21:
		{
			e->y_speed += (e->y > e->y_mark) ? SPEED(-0x20) : SPEED(0x20);
			LIMIT_Y(SPEED(0x200));
		}
		break;
		case 30:	// he teleports away
		{
			e->state_time = 0;
			e->frame = 3;
			e->y_speed = 0;
			e->state++;
		}
		/* no break */
		case 31:
		{
			//if (DoTeleportOut(e, 1)) {
				e->state = STATE_DELETE;
			//}
		}
		break;
		case 40:	// he teleports in and hovers
		{
			e->state_time = 0;
			e->state = 41;
			e->frame = 3;
		}
		/* no break */
		case 41:
		{
			//if (DoTeleportIn(o, 1)) {
				e->state = 20;
				e->y_speed = SPEED(-0x200);
			//}
		}
		break;
	}
	e->y += e->y_speed;
}

void ai_toroko(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;

	if(!(e->eflags & NPC_IGNORESOLID)) {
		if(!e->grounded) e->grounded = collide_stage_floor(e);
		else e->grounded = collide_stage_floor_grounded(e);
	}

	switch(e->state) {
		case 0:		// stand and blink
		{
			e->frame = 0;
			e->x_speed = 0;
			e->state = 1;
		}
		/* no break */
		case 1:
		{
			//randblink(o, 1, 4);
		}
		break;
		case 3:		// run away!!
		{
			e->state = 4;
			e->frame = 1;
		}
		/* no break */
		case 4:
		{
			if((e->y_speed < 0 && collide_stage_leftwall(e)) ||
				(e->y_speed > 0 && collide_stage_rightwall(e))) { 
				TURN_AROUND(e);
			}
			MOVE_X(SPEED(0x400));
		}
		break;
		case 6:		// hop and run away!!
		{
			e->state = 7;
			e->frame = 1;
			e->y_speed = SPEED(-0x400);
			e->grounded = false;
		}
		/* no break */
		case 7:
		{
			MOVE_X(SPEED(0x100));
			if(e->grounded) e->state = 3;
		}
		break;
		// small hop straight up/down from Balrog
		// shaking the ground, used in Shack
		case 8:
		{
			e->frame = 1;
			e->state_time = 0;
			e->state = 9;
			e->y_speed = SPEED(-0x200);
			e->grounded = false;
		}
		/* no break */
		case 9:
		{
			if(e->grounded) e->state = 0;
		}
		break;
		case 10:	// eeks and falls down
		{
			e->state = 11;
			e->frame = 3;
			e->y_speed = SPEED(-0x400);
			e->grounded = false;
			sound_play(SND_ENEMY_SQUEAK, 5);
			MOVE_X(SPEED(0x200));
		}
		break;
		case 11:	// falling down
		{
			if (e->grounded) {
				e->state = 12;
				e->frame = 4;
				e->eflags |= NPC_INTERACTIVE;
				e->x_speed = 0;
			}
		}
		break;
	}
	
	e->x = e->x_next;
	e->y = e->y_next;

	if(!e->grounded) e->y_speed += SPEED(0x40);
	LIMIT_Y(SPEED(0x5FF));
}


void ai_toroko_teleport_in(Entity *e) {
	e->x_next = e->x;
	e->y_next = e->y + e->y_speed;
	e->grounded = collide_stage_floor(e);

	switch(e->state) {
		case 0:
		{
			e->grounded = false;
			e->state = 1;
			e->state_time = 0;
			e->nflags &= ~NPC_IGNORESOLID;		// this is set in npc.tbl, but uh, why?
		}
		/* no break */
		case 1:
		{
			//if (DoTeleportIn(o, 2)) {
			if(++e->state_time > TIME(50)) {
				e->frame = 1;
				e->state = 2;
			}
		}
		break;
		case 2:
		{
			if (e->grounded) {
				e->state = 4;
				e->frame = 4;
				sound_play(SND_THUD, 5);
			}
		}
		break;
		case 4: break;			// knocked out
	}
	
	e->y = e->y_next;
	// fall unless teleporting
	if (e->state >= 2) {
		if(!e->grounded) e->y_speed += SPEED(0x20);
		LIMIT_Y(SPEED(0x5FF));
	}
}

// Use the shared global x and y mark for red crystal
#define crystal_xmark curly_target_x
#define crystal_ymark curly_target_y

void ai_sue(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;

	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);

	switch(e->state) {
		case 0:		// stand and blink
			e->state_time = 0;
			SPR_SAFEANIM(e->sprite, 0);
			e->x_speed = 0;
			e->linkedEntity = NULL;
			//randblink(o, 1, 4);
		break;
		case 3:		// walking
		case 4:		// walking
		case 5:		// face away
			generic_npc_states(e);
		break;
		case 6:		// got punched by Igor
		{
			e->state = 7;
			SPR_SAFEANIM(e->sprite, 4);
			e->state_time = 0;
			sound_play(SND_ENEMY_SQUEAK, 5);
		}
		/* no break */
		case 7:
		{
			if (++e->state_time > TIME(10)) e->state = 0;
		}
		break;
		// got punched extra hard by Igor
		// flys through air backwards and crashes
		case 8:
		{
			e->state = 9;
			SPR_SAFEANIM(e->sprite, 4);
			e->state_time = 0;
			sound_play(SND_ENEMY_SQUEAK, 5);
			e->y_speed = SPEED(-0x200);
			MOVE_X(SPEED(-0x400));
		}
		/* no break */
		case 9:
		{
			if (++e->state_time > 3 && e->grounded) {
				e->state = 10;
				TURN_AROUND(e);
			}
		}
		break;
		case 10:
		{
			e->x_speed = 0;
			SPR_SAFEANIM(e->sprite, 5);
		}
		break;
		// punching the air (when she tells Igor "I'm not afraid of you!")
		case 11:
		{
			e->state = 12;
			e->state_time = 0;
		}
		/* no break */
		case 12:
		{
			e->state_time++;
			if((e->state_time % 16) == 8) {
				SPR_SAFEANIM(e->sprite, 7);
			} else if((e->state_time % 16) == 0) {
				SPR_SAFEANIM(e->sprite, 0);
			}
		}
		break;
		// picked up & carried away by Igor
		case 13:
		{
			SPR_SAFEANIM(e->sprite, 6);
			e->x_speed = 0;
			e->y_speed = 0;
			e->state = 14;
			// find Igor
			e->linkedEntity = entity_find_by_event(501);
			if(e->linkedEntity) {
				puts("Found Igor");
			} else {
				puts("Couldn't find Igor");
			}
			e->alwaysActive = true;
		}
		/* no break */
		case 14:	// being carried--see aftermove routine
		{
			Entity *link = e->linkedEntity;
			if (link != NULL) {
				e->x_next = link->x + link->dir ? (32 << CSF) : -(32 << CSF);
				e->y_next = link->y - (32 << CSF);
				if(e->dir != link->dir) TURN_AROUND(e);
			}
		}
		break;
		// spawn red crystal and call it to us (Undead Core intro)
		case 15:
		{
			//e->PushBehind(dr_create_red_crystal(e->x+(128<<CSF), e->y));
			e->state = 16;
			e->x_speed = 0;
			SPR_SAFEANIM(e->sprite, 0);
		}
		/* no break */
		case 16:
		{
			crystal_xmark = e->x - (18<<CSF);
			crystal_ymark = e->y - (8<<CSF);
		}
		break;
		case 17:	// look up (still followed by red crystal)
		{
			e->x_speed = 0;
			SPR_SAFEANIM(e->sprite, 9);
			crystal_xmark = e->x;
			crystal_ymark = e->y - (8<<CSF);
		}
		break;
		// run away from DOCTOR_GHOST and hide behind player
		case 20:
		{
			e->state = 21;
			SPR_SAFEANIM(e->sprite, 1);
		}
		/* no break */
		case 21:
		{
			MOVE_X(SPEED(0x400));
			if (e->x < player.x - (8<<CSF)) {
				e->dir = 1;
				e->state = 0;
			}
		}
		break;
		// run, during "we've got to get out of here" post-undead core cutscene.
		case 30:
		{
			e->state = 31;
			SPR_SAFEANIM(e->sprite, 1);
		}
		/* no break */
		case 31:
		{
			MOVE_X(SPEED(0x400));
		}
		break;
		case 40:	// she jumps off the island
		{
			e->state = 41;
			SPR_SAFEANIM(e->sprite, 9);
			e->y_speed = SPEED(-0x400);
		}
		break;
	}

	e->x = e->x_next;
	e->y = e->y_next;

	if(!e->grounded) e->y_speed += SPEED(0x40);
	LIMIT_Y(SPEED(0x5FF));
}

void ai_sue_teleport_in(Entity *e) {
	e->y_next = e->y + e->y_speed;
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);

	switch(e->state) {
		case 0:
		{
			e->x += (6 << CSF);
			e->state_time = 0;
			e->state = 1;
			SPR_SAFEANIM(e->sprite, 6);	// her "hanging on a hook" frame
		}
		/* no break */
		case 1:
		{
			//if (DoTeleportIn(o, 2)) {
				e->state = 2;
				e->state_time = 0;
			//}
		}
		break;
		case 2:
		{
			if (e->grounded) {
				SPR_SAFEANIM(e->sprite, 8);	// crumpled
				e->state = 3;
				e->state_time = 0;
				sound_play(SND_THUD, 5);
			}
		}
		break;
	}

	e->y = e->y_next;
	// fall unless teleporting
	if (e->state >= 2) {
		e->y_speed += SPEED(0x20);
		LIMIT_Y(SPEED(0x5FF));
	}
}

void ai_kazuma(Entity *e) {
	generic_npc_states(e);
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_king(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	entity_update_collision(e);
	switch(e->state) {
		case 0:
		{
			SPR_SAFEANIM(e->sprite, 0);
			e->x_speed = e->y_speed = 0;
		}
		break;
		case 5:			// he is knocked out
		{
			SPR_SAFEANIM(e->sprite, 4);
			e->x_speed = 0;
		}
		break;
		case 6:			// hop in air then fall and knocked out
		{
			e->state = 7;
			e->state_time = 0;
			e->y_speed = SPEED(-0x400);
			e->grounded = false;
		}
		/* no break */
		case 7:			// he falls and is knocked out
		{
			SPR_SAFEANIM(e->sprite, 3);
			MOVE_X(SPEED(0x280));
			e->y_speed += 0x40;
			if (e->state_time++ && e->grounded) e->state = 5;
		}
		break;
		case 8:		// walking
		{
			e->state = 9;
			SPR_SAFEANIM(e->sprite, 1);
		}
		/* no break */
		case 9:
		{
			MOVE_X(SPEED(0x200));
		}
		break;
		case 10:		// run
		{
			e->state = 11;
			SPR_SAFEANIM(e->sprite, 1);
		}
		/* no break */
		case 11:
		{
			MOVE_X(SPEED(0x400));
		}
		break;
		case 20:		// pull out sword
		{
			if (e->linkedEntity == NULL) {
				Entity *sword = entity_create(sub_to_block(e->x), sub_to_block(e->y), 
						0, 0, OBJ_KINGS_SWORD, 0, 0);
				sword->linkedEntity = e;
				e->linkedEntity = sword;
			}
			SPR_SAFEANIM(e->sprite, 0);
			e->state = 0;
		}
		break;
		case 30:		// he goes flying in spec'd direction and smacks wall
		{
			e->state = 31;
			e->state_time = 0;
			SPR_SAFEANIM(e->sprite, 3);
			MOVE_X(SPEED(0x600));
			e->y_speed = 0;
		}
		/* no break */
		case 31:
		{
			// Don't follow slopes
			e->grounded = false;
			e->y_speed = 0;
			e->y_next = e->y;
			// Hit the wall
			if (e->x_speed == 0) {
				e->dir = 1;
				e->state = 7;
				e->state_time = 0;
				e->y_speed = SPEED(-0x400);
				e->grounded = false;
				e->x_speed = SPEED(0x280);
				sound_play(SND_LITTLE_CRASH, 5);
				//SmokeClouds(o, 4, 8, 8);
			}
		}
		break;
		case 40:		// he flickers away and is replaced by the Blade
		{
			e->state = 41;
			e->state_time = 0;
		}
		/* no break */
		case 41:
		{
			SPR_SAFEVISIBILITY(e->sprite, (e->state_time % 4) > 1 ? AUTO_FAST : HIDDEN);
			if (++e->state_time > 100) {
				//SmokeClouds(o, 4, 8, 8);
				e->state = 42;
				SPR_SAFERELEASE(e->sprite);
				SPR_SAFEADD(e->sprite, &SPR_Sword, 0, 0, TILE_ATTR(PAL0, 0, 0, 0), 3);
				// Just in case the player walks away
				e->alwaysActive = true;
			}
		}
		break;
		case 42: break;
		case 60:		// jump (used when he lunges with sword)
			SPR_SAFEANIM(e->sprite, 0);
			e->state = 61;
			e->y_speed = SPEED(-0x5FF);
			e->x_speed = SPEED(0x380);
			SPR_SAFEHFLIP(e->linkedEntity->sprite, 1);
		break;
		case 61:		// jumping
			e->y_speed += SPEED(0x80);
			if (e->grounded) {
				e->state = 0;
				e->x_speed = 0;
				SPR_SAFEHFLIP(e->linkedEntity->sprite, 0);
			}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
	
	LIMIT_Y(SPEED(0x5FF));
}

void ai_blue_robot(Entity *e) {
	//e->y_next = e->y + e->y_speed;
	//if(!e->grounded) e->grounded = collide_stage_floor(e);
	//else e->grounded = collide_stage_floor_grounded(e);
	//randblink(o, 1, 4);
	//e->y = e->y_next;
	//if(!e->grounded) e->y_speed += SPEED(0x40);
	//LIMIT_Y(SPEED(0x5FF));
}

void ai_kanpachi_fishing(Entity *e) {
	if (!e->state) {
		SNAP_TO_GROUND(e);
		e->state = 1;
	}
	// open eyes when player comes near
	if(PLAYER_DIST_X(64<<CSF) && PLAYER_DIST_Y2(64<<CSF, 16<<CSF)) {
		e->frame = 1;
	} else {
		e->frame = 0;
	}
}

void ai_booster(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);

	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->state_time = 0;
			e->x_speed = e->y_speed = 0;
			SPR_SAFEANIM(e->sprite, 0);
		}
		/* no break */
		case 1:
		{
			//randblink(o, 1, 8, 120);
		}
		break;
		case 30:	// teleporting in at Shelter
		{
			SPR_SAFEANIM(e->sprite, 0);
			sound_play(SND_TELEPORT, 5);
			// move into middle of teleporter
			e->x -= 16 << CSF;
			e->y += 8 << CSF;
			
			e->state++;
			e->state_time = 0;
		}
		/* no break */
		case 31:	// teleporting-in animation
		{
			//if (DoTeleportIn(o, 2)) {
				e->state++;
				e->state_time = 0;
			//}
		}
		break;
		case 32:	// wait before hop out of teleporter
		{
			if (++e->state_time > 20) {
				e->state++;
				SPR_SAFEANIM(e->sprite, 1);
			}
		}
		break;
		case 33:	// hopping out of teleporter
		{
			if (e->grounded) {
				e->state++;
				e->state_time = 0;
				SPR_SAFEANIM(e->sprite, 0);
			}
		}
		break;
		default:
			generic_npc_states(e);
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
	// no fall when teleporting in
	if (!e->grounded && e->state != 31 && e->state != 32) e->y_speed += 0x40;
}

void ai_booster_falling(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);

	switch(e->state) {
		case 0:			// sitting on ground after fall
			SPR_SAFEANIM(e->sprite, 5);
		break;
		case 10:		// falling
		{
			SPR_SAFEANIM(e->sprite, 4);
			e->y_speed += SPEED(0x40);
			LIMIT_Y(SPEED(0x5FF));
		}
		break;
		case 20:		// dying (flickering away)
		{
			sound_play(SND_TELEPORT, 5);
			e->state = 21;
			e->state_time = 0;
		}
		/* no break */
		case 21:
		{
			SPR_SAFEVISIBILITY(e->sprite, (e->state_time & 2) ? AUTO_FAST : HIDDEN);
			if (++e->state_time > TIME(100)) {
				//SmokeClouds(o, 4, 16, 16);
				e->state = STATE_DELETE;
			}
		}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

// shared between OBJ_KAZUMA_AT_COMPUTER and OBJ_SUE_AT_COMPUTER
void ai_npc_at_computer(Entity *e) {
	if(stageID == 1 && e->type == 0x3E) {
		e->state = STATE_DELETE; // Remove duplicate Kazuma from Arthur's house
	}
	//if((++e->state_time % 30 == 1) && (random() % 4 == 0)) {
	//	s16 newAnim = e->sprite->animInd + 1;
	//	if(newAnim >= 3) newAnim = 0;
	//	SPR_SAFEANIM(e->sprite, newAnim);
	//	e->state_time = 0;
	//}
}

void generic_npc_states(Entity *e) {
	switch(e->state) {
		case 0:		// stand
		{
			SPR_SAFEANIM(e->sprite, 0);
			e->x_speed = 0;
			e->y_speed = 0;
			//randblink(o, 1, 8, 100);
		}
		break;
		case 3:		// walking
		{
			e->state++;
			SPR_SAFEANIM(e->sprite, 1);
		}
		/* no break */
		case 4:
		{
			MOVE_X(SPEED(0x200));
		}
		break;
		case 5:		// face away
		{
			SPR_SAFEANIM(e->sprite, e->type == OBJ_KAZUMA ? 2 : 3);
			e->x_speed = 0;
		}
		break;
		case 8:		// walk (alternate state used by OBJ_NPC_JACK)
		{
			if (e->type == OBJ_JACK) {
				e->state = 4;
				SPR_SAFEANIM(e->sprite, 1);
			}
		}
		break;
	}
}
