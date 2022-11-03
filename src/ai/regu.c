#include "ai_common.h"

void ai_jenka(Entity *e) {
	e->dir = 1;
	e->frame = 0;
	RANDBLINK(e, 1, 200);
}

void ai_doctor(Entity *e) {
	enum { STAND1, STAND2, FLOAT, HAND1, HAND2 };
	switch(e->state) {
		case 10:	// he chuckles
		{
			e->state = 11;
			e->timer2 = 0;
			ANIMATE(e, 10, 1,0);
		}
		/* fallthrough */
		case 11:
		{
			if (++e->timer2 > TIME(8*6)) {
				e->frame = 0;
				e->state = 1; 
			}
		}
		break;
		case 20:	// he rises up and hovers
		{
			e->state = 21;
			e->timer = 0;
			e->frame = 2;
			e->y_mark = e->y - (32 << CSF);
		}
		/* fallthrough */
		case 21:
		{
			e->y_speed += (e->y > e->y_mark) ? -SPEED(0x20) : SPEED(0x20);
			LIMIT_Y(SPEED(0x200));
		}
		break;
		case 30:	// he teleports away
		{
			e->timer = 0;
			e->frame = 2;
			e->y_speed = 0;
			e->state++;
		}
		/* fallthrough */
		case 31:
		{
			//if (DoTeleportOut(e, 1)) {
				e->state = STATE_DELETE;
			//}
		}
		break;
		case 40:	// he teleports in and hovers
		{
			e->timer = 0;
			e->state = 41;
			e->frame = 2;
		}
		/* fallthrough */
		case 41:
		{
			//if (DoTeleportIn(o, 1)) {
				e->state = 20;
				e->y_speed = -SPEED(0x200);
			//}
		}
		break;
	}
	e->y += e->y_speed;
}

void ai_toroko(Entity *e) {
	if(e->state != 500) {
		e->x_next = e->x + e->x_speed;
		e->y_next = e->y + e->y_speed;
		if(!e->grounded) e->grounded = collide_stage_floor(e);
		else e->grounded = collide_stage_floor_grounded(e);
	}
	switch(e->state) {
		case 0:		// stand and blink
		{
			e->x_speed = 0;
			e->state = 1;
		}
		/* fallthrough */
		case 1:
		{
			e->frame = 0;
			RANDBLINK(e, 7, 200);
		}
		break;
		case 3:		// run away!!
		case 4:
		{
			ANIMATE(e, 8, 1,2);
			if((e->y_speed < 0 && collide_stage_leftwall(e)) ||
				(e->y_speed > 0 && collide_stage_rightwall(e))) { 
				TURN_AROUND(e);
			}
			MOVE_X(SPEED(0x3E0));
		}
		break;
		case 6:		// hop and run away!!
		{
			e->state = 7;
			e->frame = 1;
			e->y_speed = -SPEED(0x400);
			e->grounded = FALSE;
		}
		/* fallthrough */
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
			e->timer = 0;
			e->state = 9;
			e->y_speed = -SPEED(0x200);
			e->grounded = FALSE;
		}
		/* fallthrough */
		case 9:
		{
			if(e->grounded) e->state = 0;
		}
		break;
		case 10:	// eeks and falls down
		{
			e->state = 11;
			e->frame = 5;
			e->y_speed = -SPEED(0x400);
			e->grounded = FALSE;
			sound_play(SND_ENEMY_SQUEAK, 5);
			MOVE_X(SPEED(0x200));
		}
		break;
		case 11:	// falling down
		{
			if (e->grounded) {
				e->state = 12;
				e->frame = 6;
				e->flags |= NPC_INTERACTIVE;
				e->x_speed = 0;
			}
		}
		break;
		case 500: // In bubble - don't move or collide with anything
		{
			e->frame = 1;
		}
		break;
		default: // Toroko getting up after you shoot her, don't know the real state
			FACE_PLAYER(e);
			e->alwaysActive = TRUE;
			if(stageID != 0x23) e->frame = 0;
	}
	if(e->state != 500) {
		e->x = e->x_next;
		e->y = e->y_next;
		if(!e->grounded) e->y_speed += SPEED(0x40);
		LIMIT_Y(SPEED(0x5FF));
	}

	// I hate this
	if(stageID == STAGE_MIMIGA_SHACK && e->frame == 0) e->dir = 0;
}


void ai_toroko_tele_in(Entity *e) {
	e->x_next = e->x;
	e->y_next = e->y + e->y_speed;
	e->grounded = collide_stage_floor(e);

	switch(e->state) {
		case 0:
		{
			e->grounded = FALSE;
			e->state = 1;
			e->timer = 0;
			e->flags &= ~NPC_IGNORESOLID;		// this is set in npc.tbl, but uh, why?
		}
		/* fallthrough */
		case 1:
		{
			//if (DoTeleportIn(o, 2)) {
			if(++e->timer > TIME(50)) {
				e->frame = 1;
				e->state = 2;
			}
		}
		break;
		case 2:
		{
			if (e->grounded) {
				e->state = 4;
				e->frame = 6;
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

void onspawn_sue(Entity *e) {
	if(e->flags & NPC_OPTION2) e->dir = TRUE;
	// This keeps Sue behind the cage in Mimiga Village
	if(stageID == STAGE_MIMIGA_VILLAGE) e->alwaysActive = TRUE;
	if(stageID == STAGE_FALLING) {
		e->alwaysActive = TRUE;
		//e->state = 50;
	} else {
		SNAP_TO_GROUND(e);
	}
}

void ai_sue(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;

	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);

	switch(e->state) {
		case 0:		// stand and blink
		{
			e->timer = 0;
			e->frame = 0;
			e->x_speed = 0;
			e->linkedEntity = NULL;
			RANDBLINK(e, 3, 200);
		}
		break;
		case 3:		// walking
		case 4:		// walking
		case 5:		// face away
		{
			generic_npc_states(e);
		}
		break;
		case 6:		// got punched by Igor
		{
			e->state = 7;
			e->frame = 5;
			e->timer = 0;
			sound_play(SND_ENEMY_SQUEAK, 5);
		}
		/* fallthrough */
		case 7:
		{
			if (++e->timer > TIME_8(10)) e->state = 0;
		}
		break;
		// got punched extra hard by Igor
		// flys through air backwards and crashes
		case 8:
		{
			e->state = 9;
			e->frame = 5;
			e->timer = 0;
			sound_play(SND_ENEMY_SQUEAK, 5);
			e->y_speed = -SPEED_10(0x200);
			MOVE_X(-SPEED_10(0x3FF));
		}
		/* fallthrough */
		case 9:
		{
			if (++e->timer > 3 && e->grounded) {
				e->state = 10;
				TURN_AROUND(e);
			}
		}
		break;
		case 10:
		{
			e->x_speed = 0;
			e->frame = 6;
		}
		break;
		// punching the air (when she tells Igor "I'm not afraid of you!")
		case 11:
		{
			e->state = 12;
			e->timer = 0;
		}
		/* fallthrough */
		case 12:
		{
			e->timer++;
			if((e->timer & 15) == 8) {
				e->frame = 8;
			} else if((e->timer & 15) == 0) {
				e->frame = 0;
			}
		}
		break;
		// picked up & carried away by Igor
		case 13:
		{
			e->frame = 7;
			e->x_speed = 0;
			e->y_speed = 0;
			e->state = 14;
			// Find Igor
			Entity *find = entityList;
			while(find && find->event != 501) find = find->next;
			e->linkedEntity = find;
			e->alwaysActive = TRUE;
		}
		/* fallthrough */
		case 14:	// being carried
		{
			Entity *link = e->linkedEntity;
			if (link) {
				e->x_next = link->x + (link->dir ? (8 << CSF) : -(8 << CSF));
				e->y_next = link->y + (8 << CSF);
				e->dir = link->dir;
			}
		}
		break;
		// spawn red crystal and call it to us (Undead Core intro)
		case 15:
		{
			entity_create(e->x+pixel_to_sub(128), e->y, OBJ_RED_CRYSTAL, 0);
			e->state = 16;
			e->x_speed = 0;
			e->frame = 0;
		}
		/* fallthrough */
		case 16:
		{
			crystal_xmark = e->x - (18<<CSF);
			crystal_ymark = e->y - (8<<CSF);
		}
		break;
		case 17:	// look up (still followed by red crystal)
		{
			e->x_speed = 0;
			e->frame = 10;
			crystal_xmark = e->x;
			crystal_ymark = e->y - (8<<CSF);
		}
		break;
		// run away from DOCTOR_GHOST and hide behind player
		case 20:
		case 21:
		{
			ANIMATE(e, 8, 1,0,2,0);
			MOVE_X(SPEED_10(0x3FF));
			if (e->x < player.x - (8<<CSF)) {
				e->dir = 1;
				e->state = 0;
			}
		}
		break;
		// run, during "we've got to get out of here" post-undead core cutscene.
		case 30:
		case 31:
		{
			ANIMATE(e, 8, 1,0,2,0);
			MOVE_X(SPEED_10(0x3FF));
		}
		break;
		case 40:	// she jumps off the island
		{
			e->state = 41;
			e->frame = 8;
			e->y_speed = -SPEED_10(0x3FF);
			e->grounded = FALSE;
		}
		break;
		case 50:
		{
			moveMeToFront = TRUE;
			e->dir = !e->linkedEntity->dir;
			e->frame = 7;
			e->x_next = e->linkedEntity->x + (e->dir ? (16 << CSF) : -(16 << CSF));
			e->y_next = e->linkedEntity->y + (4 << CSF);
			e->y_speed = 0;
			e->grounded = TRUE;
		} break;
	}

	e->x = e->x_next;
	e->y = e->y_next;

	if(!e->grounded) e->y_speed += SPEED(0x40);
	LIMIT_Y(SPEED(0x5FF));
}

void ai_sue_teleport_in(Entity *e) {
	e->x_next = e->x;
	e->y_next = e->y + e->y_speed;
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);

	switch(e->state) {
		case 0:
		{
			e->x += (6 << CSF);
			e->timer = 0;
			e->state = 1;
			e->frame = 7;	// her "hanging on a hook" frame
		}
		/* fallthrough */
		case 1:
		{
			//if (DoTeleportIn(o, 2)) {
				e->state = 2;
				e->timer = 0;
				e->grounded = FALSE;
			//}
		}
		break;
		case 2:
		{
			if (e->grounded) {
				e->frame = 9;	// crumpled;
				e->state = 3;
				e->timer = 0;
				sound_play(SND_THUD, 5);
			}
		}
		break;
	}

	e->y = e->y_next;
	// fall unless teleporting
	if (e->state >= 2) {
		if(!e->grounded) e->y_speed += SPEED(0x20);
		LIMIT_Y(SPEED(0x5FF));
	}
}

// Kazuma has no physics, and doesn't blink
void ai_kazuma(Entity *e) {
	generic_npc_states(e);
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_king(Entity *e) {
	Entity *sword = e->linkedEntity;
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);
	
	switch(e->state) {
		case 0:
		{
			e->enableSlopes = TRUE;
			e->frame = 0;
			e->x_speed = e->y_speed = 0;
		}
		break;
		case 5:			// he is knocked out
		{
			e->frame = 5;
			e->x_speed = 0;
		}
		break;
		case 6:			// hop in air then fall and knocked out
		{
			e->state = 7;
			e->timer = 0;
			e->y_speed = -SPEED(0x400);
			e->grounded = FALSE;
		}
		/* fallthrough */
		case 7:			// he falls and is knocked out
		{
			e->frame = 4;
			MOVE_X(SPEED(0x280));
			e->y_speed += 0x40;
			if (e->timer++ && e->grounded) e->state = 5;
		}
		break;
		case 8:		// walking
		case 9:
		{
			ANIMATE(e, 8, 1,0,2,0);
			MOVE_X(SPEED(0x200));
		}
		break;
		case 10:		// run
		{
			ANIMATE(e, 8, 1,0,2,0);
			MOVE_X(SPEED(0x400));
		}
		break;
		case 20:		// pull out sword
		{
			sword = entity_create(e->x, e->y, OBJ_KINGS_SWORD, 0);
			sword->frame = 6;
			sword->dir = 1;
			e->linkedEntity = sword;
			e->state = 0;
		}
		break;
		case 30:		// he goes flying in spec'd direction and smacks wall
		{
			e->enableSlopes = FALSE;
			e->state = 31;
			e->timer = 0;
			e->frame = 4;
			MOVE_X(SPEED(0x600));
			e->y_speed = 0;
		}
		/* fallthrough */
		case 31:
		{
			// Don't follow slopes
			e->grounded = FALSE;
			e->y_speed = 0;
			e->y_next = e->y;
			// Hit the wall
			if (collide_stage_leftwall(e)) {
				e->dir = 1;
				e->state = 7;
				e->timer = 0;
				e->y_speed = -SPEED(0x400);
				e->grounded = FALSE;
				e->x_speed = SPEED(0x280);
				sound_play(SND_LITTLE_CRASH, 5);
				effect_create_smoke(e->x >> CSF, e->y >> CSF);
			}
		}
		break;
		case 40:		// he flickers away and is replaced by the Blade
		{
			e->state = 41;
			e->timer = 0;
		}
		/* fallthrough */
		case 41:
		{
			e->hidden = (++e->timer & 2);
			if (e->timer > TIME_8(100)) {
				effect_create_smoke(e->x >> CSF, e->y >> CSF);
				e->state = 42;
				e->hidden = FALSE;
				e->frame = 6; // Sword
			}
		}
		break;
		case 42: break;
		case 60:		// jump (used when he lunges with sword)
			e->frame = 0;
			e->state = 61;
			e->y_speed = -SPEED(0x5FF);
			e->x_speed = SPEED(0x380);
			sword->dir = 0;
		break;
		case 61:		// jumping
			e->y_speed += SPEED(0x80);
			if (e->grounded) {
				e->state = 0;
				e->x_speed = 0;
				sword->dir = 1;
			}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
	
	if(sword) {
		sword->x = e->x + (sword->dir ? (10 << CSF) : -(10 << CSF));
		sword->y = e->y;
	}
	
	if(stageID == 0x23) e->dir = 1; // Stop turning left asshole
	
	LIMIT_Y(SPEED(0x5FF));
}

void ai_blue_robot(Entity *e) {
	e->x_next = e->x; // x_next must be set or collision will be wrong
	e->y_next = e->y + e->y_speed;
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);
	e->frame = 0;
	RANDBLINK(e, 1, 200);
	e->y = e->y_next;
	if(!e->grounded) e->y_speed += SPEED(0x40);
	LIMIT_Y(SPEED(0x5FF));
}

void ai_kanpachi_fish(Entity *e) {
	if (!e->state) {
		SNAP_TO_GROUND(e);
		e->state = 1;
	}
	// open eyes when player comes near
	if(PLAYER_DIST_X(e, 64<<CSF) && PLAYER_DIST_Y2(e, 64<<CSF, 16<<CSF)) {
		e->frame = 1;
		RANDBLINK(e, 0, 200);
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
			e->timer = 0;
			e->x_speed = e->y_speed = 0;
		}
		/* fallthrough */
		case 1:
		{
			e->frame = 0;
			RANDBLINK(e, 3, 200);
		}
		break;
		case 30:	// teleporting in at Shelter
		{
			e->frame = 0;
			sound_play(SND_TELEPORT, 5);
			// move into middle of teleporter
			e->x_next -= 16 << CSF;
			e->y_next += 8 << CSF;
			
			e->state++;
			e->timer = 0;
		}
		/* fallthrough */
		case 31:	// teleporting-in animation
		{
			//if (DoTeleportIn(o, 2)) {
				e->state++;
				e->timer = 0;
			//}
		}
		break;
		case 32:	// wait before hop out of teleporter
		{
			if (++e->timer > 20) {
				e->state++;
				e->frame = 1;
			}
		}
		break;
		case 33:	// hopping out of teleporter
		{
			if (e->grounded) {
				e->state++;
				e->timer = 0;
				e->frame = 0;
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
			e->frame = 6;
		break;
		case 10:		// falling
		{
			e->frame = 5;
			e->y_speed += SPEED(0x40);
			LIMIT_Y(SPEED(0x5FF));
		}
		break;
		case 20:		// dying (flickering away)
		{
			sound_play(SND_TELEPORT, 5);
			e->state = 21;
			e->timer = 0;
		}
		/* fallthrough */
		case 21:
		{
			e->hidden = (++e->timer & 2);
			if (e->timer > TIME(100)) {
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
		// Remove duplicate Kazuma from Arthur's house
		e->state = STATE_DELETE; 
	}
	enum { INIT=0, TYPING, PAUSE_SLOUCH, PAUSE_UPRIGHT };
	switch(e->state) {
		case INIT:
		{
			SNAP_TO_GROUND(e);
			e->state = TYPING;
			e->frame = 0;
			e->x -= 4<<CSF;
		} /* fallthrough */
		case TYPING:
		{
			ANIMATE(e, 4, 0,1);
			if(!(rand() & 63)) {
				e->state = PAUSE_SLOUCH;
				e->frame = 1;
				e->timer = 0;
			}
			else if(!(rand() & 127)) {
				e->state = PAUSE_UPRIGHT;
				e->frame = 2;
				e->timer = 0;
			}
		}
		break;
		
		case PAUSE_SLOUCH:
		{
			if (++e->timer > TIME_8(40)) {
				e->state = PAUSE_UPRIGHT;
				e->frame = 2;
				e->timer = 0;
			}
		}
		break;
		
		case PAUSE_UPRIGHT:
		{
			if (++e->timer > TIME_8(80)) {
				e->state = TYPING;
				e->frame = 0;
				e->timer = 0;
			}
		}
		break;
	}
}
