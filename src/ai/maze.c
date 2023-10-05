#include "ai_common.h"

void onspawn_block(Entity *e) {
	e->x += pixel_to_sub(8);
	e->y += pixel_to_sub(8);
	e->hit_box = (bounding_box) {{ 16, 16, 16, 16 }};
	e->display_box = (bounding_box) {{ 16, 16, 16, 16 }};
	e->flags |= NPC_SPECIALSOLID | NPC_IGNORE44;
	e->flags &= ~NPC_SOLID;
	e->enableSlopes = FALSE;
	e->attack = 0;
	e->state = (e->flags & NPC_OPTION2) ? 20 : 10;
}

void ai_blockh(Entity *e) {
	switch(e->state) {
		case 10:
		if((player.x > e->x && player.x - e->x < 0x3200) ||
			(player.x < e->x && e->x - player.x < 0x32000)) {
			if(PLAYER_DIST_Y(e, 0x3200)) {
				e->state = 30;
				e->timer = 0;
			}
		}
		break;
		case 20:
		if((player.x > e->x && player.x - e->x < 0x32000) ||
			(player.x < e->x && e->x - player.x < 0x3200)) {
			if(PLAYER_DIST_Y(e, 0x3200)) {
				e->state = 30;
				e->timer = 0;
			}
		}
		break;
		case 30:
		{
			uint16_t dir = e->flags & NPC_OPTION2;
			e->x_speed += dir ? SPEED_8(0x20) : -SPEED_8(0x20);
			if(e->x_speed > SPEED_10(0x200)) e->x_speed = SPEED_10(0x200);
			if(e->x_speed < -SPEED_10(0x200)) e->x_speed = -SPEED_10(0x200);
			e->x_next = e->x + e->x_speed;
			// hit edge
			if((e->x_speed > 0 && stage_get_block_type(
					sub_to_block(e->x_next + 0x1C00), sub_to_block(e->y)) == 0x41) ||
				(e->x_speed < 0 && stage_get_block_type(
					sub_to_block(e->x_next - 0x1C00), sub_to_block(e->y)) == 0x41)) {
				camera_shake(10);
				e->x_speed = 0;
				e->flags ^= NPC_OPTION2;
				e->state = dir ? 10 : 20;
			} else {
				e->x = e->x_next;
				if((++e->timer & 15) == 6) {
					sound_play(SND_BLOCK_MOVE, 2);
				}
			}
		}
		break;
	}
}

void ai_blockv(Entity *e) {
	switch(e->state) {
		case 10:
		if((player.y > e->y && player.y - e->y < 0x3200) ||
			(player.y < e->y && e->y - player.y < 0x32000)) {
			if(PLAYER_DIST_X(e, 0x3200)) {
				e->state = 30;
				e->timer = 0;
			}
		}
		break;
		case 20:
		if((player.y > e->y && player.y - e->y < 0x32000) ||
			(player.y < e->y && e->y - player.y < 0x3200)) {
			if(PLAYER_DIST_X(e, 0x3200)) {
				e->state = 30;
				e->timer = 0;
			}
		}
		break;
		case 30:
		{
			uint16_t dir = e->flags & NPC_OPTION2;
			e->y_speed += dir ? SPEED_8(0x20) : -SPEED_8(0x20);
			if(e->y_speed > SPEED_10(0x200)) e->y_speed = SPEED_10(0x200);
			if(e->y_speed < -SPEED_10(0x200)) e->y_speed = -SPEED_10(0x200);
			e->y_next = e->y + e->y_speed;
			// hit edge
			if((e->y_speed > 0 && stage_get_block_type(
					sub_to_block(e->x - 0x200), sub_to_block(e->y_next + 0x1C00)) == 0x41) ||
				(e->y_speed < 0 && stage_get_block_type(
					sub_to_block(e->x - 0x200), sub_to_block(e->y_next - 0x1C00)) == 0x41)) {
				camera_shake(10);
				e->y_speed = 0;
				e->flags ^= NPC_OPTION2;
				e->state = dir ? 10 : 20;
			} else {
				e->y = e->y_next;
				if((++e->timer & 15) == 6) {
					sound_play(SND_BLOCK_MOVE, 2);
				}
			}
		}
		break;
	}
}

void ai_boulder(Entity *e) {
	//e->inback = TRUE;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		// shaking
		case 10:
		{
			e->state = 11;
			e->timer = 0;
			e->x_mark = e->x;
		}
		/* fallthrough */
		case 11:
		{
			if ((++e->timer & 2) != 0)
				e->x = e->x_mark + (1 << 9);
			else
				e->x = e->x_mark;
		}
		break;
		
		// thrown away by Balrog
		case 20:
		{
			e->y_speed = -SPEED(0x400);
			e->x_speed = SPEED(0x100);
			sound_play(SND_FUNNY_EXPLODE, 5);
			
			e->state = 21;
			e->timer = 0;
		}
		/* fallthrough */
		case 21:
		{
			e->y_speed += SPEED(0x10);
			
			if (e->y_speed >= 0 && collide_stage_floor(e)) {
				sound_play(SND_EXPLOSION1, 5);
				camera_shake(40);
				
				e->x_speed = 0;
				e->y_speed = 0;
				e->state = 0;
			}
		}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

#define GAUDI_HP 15
#define GAUDI_FLYING_HP 15
#define GAUDI_ARMORED_HP 15

void ai_gaudiDying(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		case 0:		// just died (initializing)
		{
			//e->eflags &= ~(NPC_SHOOTABLE | NPC_IGNORESOLID | NPC_SHOWDAMAGE);
			e->flags &= ~(NPC_SHOOTABLE | NPC_IGNORESOLID | NPC_SHOWDAMAGE);
			e->attack = 0;
			
			e->frame = 4;
			
			e->y_speed = -SPEED(0x200);
			MOVE_X(-SPEED(0x100));
			sound_play(SND_ENEMY_HURT_SMALL, 5);
			e->grounded = FALSE;
			
			e->state = 1;
		}
		break;
		
		case 1:		// flying backwards through air
		{
			if (e->y_speed >= 0 && (e->grounded = collide_stage_floor(e))) {
				e->frame = 5;
				e->state = 2;
				e->timer = 0;
			}
		}
		break;
		
		case 2:		// landed, shake
		{
			e->x_speed -= e->x_speed >> 4;
			e->frame = (e->timer & 7) > 3 ? 6 : 5;
			
			if (++e->timer > TIME(50)) {
				// this deletes Entity while generating smoke effects and boom
				e->state = STATE_DESTROY;
			}
		}
		break;
	}
	
	e->x = e->x_next;
	e->y = e->y_next;
	
	if(!e->grounded) e->y_speed += SPEED(0x20);
	LIMIT_Y(SPEED(0x5ff));
}

void ai_gaudi(Entity *e) {
	if (e->health <= (1000 - GAUDI_HP)) {
		e->type = OBJ_GAUDI_DYING;
        e->onFrame = npc_info[OBJ_GAUDI_DYING].onFrame;
		e->attack = 0;
		e->state = 0;
		e->timer = 0;
		ai_gaudiDying(e);
		return;
	}
	
	// Use different palette in Labyrinth Shop
	if(stageID == 0x2A) sprite_pal(&e->sprite[0], PAL3);
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	switch(e->state) {
		case 0:
		{
			// Gaudi's in shop
			if (e->flags & NPC_INTERACTIVE) {
				e->attack = 0;
				e->flags &= ~NPC_SHOOTABLE;
			}
			
			e->x_speed = 0;
			e->state = 1;
		}
		/* fallthrough */
		case 1:
		{
			e->frame = 0;
			RANDBLINK(e, 3, 200);
			if(!(rand() & 127)) {
				if(rand() & 1) {
					TURN_AROUND(e);
				} else {
					e->state = 10;
				}
			}
		}
		break;
		case 10:		// walking
		{
			e->state = 11;
			e->timer = TIME_8(rand() & 63) + TIME_8(30);		// how long to walk for
			moveMeToFront = TRUE;
		}
		/* fallthrough */
		case 11:
		{
			ANIMATE(e, 8, 1,0,2,0);
			// time to stop walking?
			if (--e->timer <= 0) e->state = 0;
				
			MOVE_X(SPEED_10(0x200));
			
			// try to jump over any walls we come to
			if ((e->x_speed < 0 && collide_stage_leftwall(e)) ||
				(e->x_speed > 0 && collide_stage_rightwall(e))) {
				e->y_speed = -SPEED_12(0x5ff);
				e->grounded = FALSE;
				e->state = 20;
				e->timer = 0;
				
				if (!controlsLocked)	// no sound during ending cutscene
					sound_play(SND_ENEMY_JUMP, 5);
			}
		}
		break;
		case 20:		// jumping
		{
			// landed?
			if (e->y_speed >= 0 && (e->grounded = collide_stage_floor(e))) {
				e->x_speed = 0;
				e->state = 21;
				e->timer = 0;
				moveMeToFront = TRUE;
				
				if (!controlsLocked)	// no sound during ending cutscene
					sound_play(SND_THUD, 5);
			}
			
			// count how long we've been touching the wall
			// we're trying to jump over..if it's not working
			// go the other way.
			if ((e->dir == 0 && collide_stage_leftwall(e)) ||
				(e->dir == 1 && collide_stage_rightwall(e))) {
				if (++e->timer > TIME_8(10)) {
					e->timer = 0;
					TURN_AROUND(e);
				}
			} else {
				e->timer = 0;
			}
			
			MOVE_X(SPEED_10(0x100));
		}
		break;
		case 21:	// landed from jump
		{
			if (++e->timer > TIME_8(10)) e->state = 0;
		}
		break;
	}
	if(e->y_speed >= 0) {
		if(!e->grounded) e->grounded = collide_stage_floor(e);
		else e->grounded = collide_stage_floor_grounded(e);
	} else {
		collide_stage_ceiling(e);
	}
	
	e->x = e->x_next;
	e->y = e->y_next;
	
	if(!e->grounded) e->y_speed += SPEED_8(0x40);
	LIMIT_Y(SPEED_12(0x5ff));
}

void ai_gaudiFlying(Entity *e) {
	if (e->health <= (1000 - GAUDI_FLYING_HP)) {
		if (e->dir == 0)
			e->x -= (2 << 9);
		else
			e->x += (2 << 9);
		
		e->type = OBJ_GAUDI_DYING;
        e->onFrame = npc_info[OBJ_GAUDI_DYING].onFrame;
		e->attack = 0;
		e->state = 0;
		e->timer = 0;
		ai_gaudiDying(e);
		return;
	}
	
	switch(e->state) {
		case 0:
		{
			uint8_t angle = rand();
			e->x_speed = cos[angle];
			e->y_speed = sin[angle];
			e->x_mark = e->x + (e->x_speed << 3);
			e->y_mark = e->y + (e->y_speed << 3);
			e->state = 1;
			e->timer2 = 120;
		}
		/* fallthrough */
		case 1:
		{
			e->timer = TIME_8(80) + TIME_8(rand() & 63);
			e->state = 2;
		}
		/* fallthrough */
		case 2:
		{
			ANIMATE(e, 4, 7,8);
			
			if (!e->timer) {
				e->state = 3;
			} else e->timer--;
		}
		break;
		
		case 3:		// preparing to fire
		{
			ANIMATE(e, 4, 9,8);
			
			e->timer++;
			if (++e->timer > TIME_8(30)) {
				Entity *shot = entity_create(e->x, e->y, OBJ_GAUDI_FLYING_SHOT, 0);
				THROW_AT_TARGET(shot, player.x, player.y, SPEED_12(0x400));
				sound_play(SND_EM_FIRE, 5);
				
				e->state = 1;
			}
		}
	}
	
	FACE_PLAYER(e);
	// sinusoidal circling pattern
	e->x_speed += (e->x > e->x_mark) ? -SPEED_8(0x10) : SPEED_8(0x10);
	e->y_speed += (e->y > e->y_mark) ? -SPEED_8(0x10) : SPEED_8(0x10);
	LIMIT_X(SPEED_10(0x200));
	LIMIT_Y(SPEED_12(0x5ff));
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_gaudiArmor(Entity *e) {
	if (e->health <= (1000 - GAUDI_ARMORED_HP)) {
		e->type = OBJ_GAUDI_DYING;
		e->attack = 0;
		e->state = 0;
		e->timer = 0;
		ai_gaudiDying(e);
		return;
	}
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	FACE_PLAYER(e);
	
	switch(e->state) {
		case 0:
		{
			e->frame = 0;
			e->x_mark = e->x;
			e->state = 1;
		}
		/* fallthrough */
		case 1:
		{
			e->x_speed = 0;
			
			if (++e->timer >= TIME(5)) {
				if (PLAYER_DIST_X(e, pixel_to_sub(192)) && PLAYER_DIST_Y(e, pixel_to_sub(160))) {	// begin hopping
					e->state = 10;
					e->timer = 0;
					e->frame = 1;
				}
			}
		}
		break;
		case 10:	// on ground inbetween hops
		{
			if (++e->timer > 3) {
				sound_play(SND_ENEMY_JUMP, 5);
				e->frame = 2;
				e->timer = 0;
				
				if (++e->timer2 < 3) {	// hopping back and forth
					e->state = 20;
					e->y_speed = -SPEED(0x200);
					e->grounded = FALSE;
					e->x_speed = (e->x < e->x_mark) ? SPEED(0x200) : -SPEED(0x200);
				} else {	// big jump and attack
					e->state = 30;
					e->y_speed = -SPEED(0x600);
					e->grounded = FALSE;
					e->x_speed = (e->x < e->x_mark) ? SPEED(0x80) : -SPEED(0x80);
					
					e->timer2 = 0;
				}
			}
		}
		break;
		case 20:	// jumping (small hop)
		{
			// landed?
			if (e->y_speed >= 0 && (e->grounded = collide_stage_floor(e))) {
				// drop sub-pixel precision
				// (required to maintain stability of back-and-forth sequence).
				e->y >>= 9; e->y <<= 9;
				
				sound_play(SND_THUD, 5);
				e->state = 40;
				e->frame = 1;
				e->timer = 0;
			}
		}
		break;
		case 30:	// jumping (big jump + attack)
		{
			e->timer++;
			
			// throw attacks at player
			if (e->timer == TIME(30) || e->timer == TIME(40)) {
				Entity *shot = entity_create(e->x, e->y, OBJ_GAUDI_ARMORED_SHOT, 0);
				THROW_AT_TARGET(shot, player.x, player.y, SPEED(0x500));
				sound_play(SND_EM_FIRE, 5);
				
				e->frame = 3;
				CURLY_TARGET_HERE(e);
			}
			
			// stop throwing animation
			if (e->timer == TIME(35) || e->timer == TIME(45)) e->frame = 2;
			
			if (e->y_speed > 0 && (e->grounded = collide_stage_floor(e))) {
				sound_play(SND_THUD, 5);
				e->state = 40;
				e->frame = 1;
				e->timer = 0;
			}
		}
		break;
		case 40:		// landed
		{
			e->x_speed -= e->x_speed >> 4;
			
			if (++e->timer >= 2) {
				e->frame = 0;
				e->x_speed = 0;
				
				e->state = 1;
				e->timer = 0;
			}
		}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
	
	if(!e->grounded) e->y_speed += SPEED(0x33);
	LIMIT_Y(SPEED(0x5ff));
}

void ai_gaudiArmorShot(Entity *e) {
	ANIMATE(e, 4, 0,1,2);
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		case 0:
		{
			uint8_t bounced = FALSE;
			if (e->x_speed <= 0 && collide_stage_leftwall(e)) { 
				e->x_speed = SPEED(0x200);
				bounced = TRUE; 
			}
			if (e->x_speed >= 0 && collide_stage_rightwall(e)) { 
				e->x_speed = -SPEED(0x200);
				bounced = TRUE; 
			}
			if (e->y_speed >= 0 && collide_stage_floor(e)) { 
				e->y_speed = -SPEED(0x200);
				bounced = TRUE; 
			}
			if (e->y_speed <= 0 && collide_stage_ceiling(e)) { 
				e->y_speed = SPEED(0x200);
				bounced = TRUE; 
			}
			
			if (bounced) {
				e->state = 1;
				sound_play(SND_TINK, 5);
			}
		}
		break;
		
		case 1:
		{
			e->y_speed += SPEED(0x40);
			LIMIT_Y(SPEED(0x5ff));
			
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

#define FRAME_STAND		0
#define FRAME_DYING		1
#define FRAME_LANDED	2
#define FRAME_FLYING	3

#define bubble_xmark curly_target_x
#define bubble_ymark curly_target_y

void ai_pooh_black(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;

	switch(e->state) {
		case 0:
		{
			e->alwaysActive = TRUE;

			e->frame = FRAME_FLYING;
			FACE_PLAYER(e);
			
			e->y_speed = SPEED_12(0xA00);
			e->flags |= NPC_IGNORESOLID;
			
			if (e->y >= block_to_sub(8)) {
				e->flags &= ~NPC_IGNORESOLID;
				e->state = 1;
			}
		}
		break;
		case 1:
		{
			e->frame = FRAME_FLYING;
			e->y_speed = SPEED_12(0xA00);
			
			if ((e->grounded = collide_stage_floor(e))) {
				//SmokeSide(o, 8, DOWN);
				sound_play(SND_BIG_CRASH, 5);
				camera_shake(30);
				
				entities_clear_by_type(OBJ_POOH_BLACK_BUBBLE);
				e->state = 2;
			}
			// damage player if he falls on him
			e->attack = (e->y < player.y && player.grounded) ? 20 : 0;
		}
		break;
		case 2:		// landed, showing landed frame
		{
			e->frame = FRAME_LANDED;
			e->attack = 0;
			if (++e->timer > TIME_8(24)) {
				e->state = 3;
				e->timer = 0;
			}
		}
		break;
		case 3:		// standing, stare at player till he shoots us.
		{
			e->frame = FRAME_STAND;
			bubble_xmark = e->x;
			bubble_ymark = e->y;
			
			// spawn bubbles when hit
			if (e->damage_time) {
			    if((e->damage_time & 7) == 1) {
                    Entity *bubble = entity_create(e->x, e->y, OBJ_POOH_BLACK_BUBBLE, 0);
                    bubble->alwaysActive = TRUE;
                    bubble->x = e->x - 0x2000 + (rand() & 0x3FFF);
                    bubble->y = e->y - 0x2000 + (rand() & 0x3FFF);
                    bubble->x_speed = -SPEED_12(0x400) + SPEED_12(rand() & 0x7FF);
                    bubble->y_speed = -SPEED_12(0x400) + SPEED_12(rand() & 0x7FF);
			    }
				// fly away after hit enough times
				if (++e->timer > TIME_8(100)) {
					e->state = 4;
					e->timer = 0;
					
					e->flags |= NPC_IGNORESOLID;
					e->y_speed = -SPEED_12(0xC00);
				}
			}
		}
		break;
		case 4:		// flying away off-screen
		{
			e->frame = FRAME_FLYING;;
			e->timer++;
			
			// bubbles shoot down past player just before
			// he falls.
			if (e->timer == TIME_8(50)) {
				bubble_xmark = player.x;
				bubble_ymark = pixel_to_sub(10000);
			} else if (e->timer < TIME_8(50)) {
				bubble_xmark = e->x;
				bubble_ymark = e->y;
			}
			if (e->timer >= TIME_8(170)) {
				// Fall on player, but keep outside the walls
				e->x_next = player.x;
				if(e->x_next < pixel_to_sub(5 * 16)) e->x_next = pixel_to_sub(5 * 16);
				if(e->x_next > pixel_to_sub(15 * 16)) e->x_next = pixel_to_sub(15 * 16);
				e->y_next = 0;
				e->y_speed = SPEED_12(0x5ff);
				
				e->state = 0;
				e->timer = 0;
			}
		}
		break;
	}
	
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_poohblk_bubble(Entity *e) {
	if (e->health < 100 || (e->state && e->y < 0)) {
		e->state = STATE_DELETE;
		return;
	}
	if (!(rand() & 7)) e->frame = 0;
	else e->frame = 1;
	e->x_speed += (e->x > bubble_xmark) ? -SPEED_8(0x40) : SPEED_8(0x40);
	e->y_speed += (e->y > bubble_ymark) ? -SPEED_8(0x40) : SPEED_8(0x40);
	LIMIT_X(SPEED_12(0xAFF));
	LIMIT_Y(SPEED_12(0xAFF));
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ondeath_pooh_black(Entity *e) {
	e->type = OBJ_POOH_BLACK_DYING;
	e->state = 0;
	e->attack = 0;
	e->flags &= ~(NPC_SHOOTABLE|NPC_SOLID);
	//e->eflags &= ~(NPC_SHOOTABLE|NPC_SOLID);
	tsc_call_event(e->event);
}

void ai_poohblk_dying(Entity *e) {
	bubble_xmark = e->x;
	bubble_ymark = -pixel_to_sub(10000);

	switch(e->state) {
		case 0:
		{
		    //e->x += 4 << CSF; // Sprite is 8px thinner so push forward a bit
			e->frame = 0; //FRAME_DYING;
			FACE_PLAYER(e);
			
			sound_play(SND_BIG_CRASH, 5);
			//SmokeClouds(o, 10, 12, 12);
			entities_clear_by_type(OBJ_POOH_BLACK_BUBBLE);
			
			e->state = 1;
			e->timer = 0;
			e->timer2 = 0;
		}
		break;
		
		case 1:
		case 2:
		{
			camera_shake(2);
			if (++e->timer > TIME(150)) {
				e->state = 2;
				e->timer2++;
				if ((e->timer2 & 7) == 3) {
					//e->hidden = FALSE;
					sound_play(SND_BUBBLE, 5);
				} else if((e->timer2 & 7) == 0) {
				    if(e->frame < 11) e->frame++;
					//e->hidden = TRUE;
				}
				if(e->timer2 > TIME(80)) {
					//entities_clear_by_type(OBJ_POOH_BLACK_BUBBLE);
					e->state = STATE_DELETE;
					return;
				}
			}
		}
		break;
	}
	
	if ((e->timer & 3) == 1) {
		Entity *bubble = entity_create(e->x, e->y, OBJ_POOH_BLACK_BUBBLE, 0);
		bubble->alwaysActive = TRUE;
		bubble->x = e->x - 0x2000 + (rand() & 0x3FFF);
		bubble->y = e->y - 0x2000 + (rand() & 0x3FFF);
		bubble->attack = 0;
		bubble->state = 1;
		bubble->x_speed = -0x200 + (rand() & 0x3FF);
		bubble->y_speed = -0x100;
	}

}

// MazeM enemies

void ai_firewhirr(Entity *e) {
	FACE_PLAYER(e);
	
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->timer = rand() & 63;
			e->y_mark = e->y;
		}
		/* fallthrough */
		case 1:
		{
			if (!e->timer) {
				e->state = 10;
				e->timer = TIME_8(100);
				e->y_speed = -SPEED_10(0x200);
			}
			else e->timer--;
		}
		/* fallthrough */
		case 10:
		{
			e->y_speed += (e->y < e->y_mark) ? SPEED_8(0x10) : -SPEED_8(0x10);
			LIMIT_Y(SPEED_10(0x200));
			
			// inc time-to-fire while player near
			if (PLAYER_DIST_Y(e, pixel_to_sub(80))) {
				if (!e->dir && player.x < e->x && PLAYER_DIST_X(e, pixel_to_sub(160))) e->timer2++;
				if (e->dir && player.x > e->x && PLAYER_DIST_X(e, pixel_to_sub(160))) e->timer2++;
			}
			
			// if time to fire, spawn a shot
			if (e->timer2 > TIME(120)) {
				Entity *shot = entity_create(e->x, e->y, OBJ_FIREWHIRR_SHOT, 0);
				shot->dir = e->dir;
				shot->alwaysActive = TRUE;
				shot->x = e->x;
				shot->y = e->y;
				e->timer2 = rand() & 15;
				// tell Curly to acquire us as a target
				CURLY_TARGET_HERE(e);
			}
		}
		break;
	}
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_firewhirr_shot(Entity *e) {
	ANIMATE(e, 8, 0,1,2);
	e->x_next = e->x + (!e->dir ? -SPEED_10(0x200) : SPEED_10(0x200));
	e->y_next = e->y;
	
	if ((!e->dir && collide_stage_leftwall(e)) ||
		(e->dir && collide_stage_rightwall(e))) {
		//effect(e->x, e->CenterY(), EFFECT_FISHY);
		e->state = STATE_DELETE;
	}

	e->x = e->x_next;
}

void ai_gaudi_egg(Entity *e) {
	if (!e->state) {
		if (!(e->flags & NPC_OPTION2)) {	// on floor
			// align properly with ground
			e->y -= 0x800;
			e->x -= 0x800;
		} else {	// on ceiling
			sprite_vflip(&e->sprite[0], 1);
			// for the egg @ entrance point that is on a ceiling slope
			if (!collide_stage_ceiling(e)) {
				e->y -= (14 << CSF);
			}
		}
		e->state = 1;
	} else if (e->state == 1) {
		if (e->health < 90) {
			e->frame = 1;
			e->attack = 0;
			//e->eflags &= ~NPC_SHOOTABLE;
			e->flags &= ~NPC_SHOOTABLE;
			entity_drop_powerup(e);
			sound_play(e->deathSound, 5);
			SMOKE_AREA((e->x>>CSF) - 8, (e->y>>CSF) - 8, 16, 16, 2);
			e->state = 2;
		}
	}
}

// I do something a bit weird with the Fuzzes. We want them to always be in sync,
// rotating the same distance between eachother. Easy, just mark them "alwaysActive" right?
// Yes, but if the player runs through that bottom section without killing anything the game 
// will lag. To get around this I signal the mini fuzz to delete themselves after the fuzz core 
// leaves the screen, then the core will respawn them after coming back on screen.

static void spawn_minifuzz(Entity *e) {
	uint8_t angle = 0;
	for(uint16_t i = 0; i < 5; i++) {
		Entity *f = entity_create(e->x, e->y, OBJ_FUZZ, 0);
		e->flags &= ~NPC_SHOOTABLE;
		f->linkedEntity = e;
		f->jump_time = angle;
		angle += 0x100 / 5;
	}
}

void ai_fuzz_core(Entity *e) {
	e->alwaysActive = TRUE;
    e->flags ^= NPC_SHOOTABLE;
	switch(e->state) {
		case 0:
		{
			// spawn mini-fuzzes, use jump_time as the angle since it is u8
			spawn_minifuzz(e);
			moveMeToFront = TRUE; // Fuzz Core will always run first, and can signal minis
			e->timer = rand() & 63;
			e->state = 1;
		}
		/* fallthrough */
		case 1:		// de-syncs the Y positions when multiple cores are present at once
		{
			if (e->timer == 0) {
				e->state = 2;
				e->y_speed = SPEED_10(0x300);
				e->y_mark = e->y;
			} else e->timer--;
		}
		break;
		case 2:
		{
			if(entity_on_screen(e)) {
				FACE_PLAYER(e);
				if (e->y > e->y_mark) e->y_speed -= SPEED_8(0x10);
				if (e->y < e->y_mark) e->y_speed += SPEED_8(0x10);
				LIMIT_Y(SPEED_10(0x355));
			} else {
				e->alwaysActive = FALSE; // Next frame we will deactivate
				e->state = 3; // This'll run the below case after reactivation
			}
		}
		break;
		case 3:
		{	// Respawn minis
			spawn_minifuzz(e);
			moveMeToFront = TRUE;
			e->state = 2;
		}
		break;
	}
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_fuzz(Entity *e) {
	e->flags ^= NPC_SHOOTABLE;
	
	if (e->state) {
		// base destroyed, simple sinusoidal player-seek
		e->x_speed += (e->x > player.x) ? -SPEED_8(0x20) : SPEED_8(0x20);
		e->y_speed += (e->y > player.y) ? -SPEED_8(0x20) : SPEED_8(0x20);
		LIMIT_X(SPEED_12(0x800));
		LIMIT_Y(SPEED_10(0x200));
		e->x += e->x_speed;
		e->y += e->y_speed;
		FACE_PLAYER(e);
	} else {
		if (e->linkedEntity->state == STATE_DESTROY) {
			e->alwaysActive = TRUE;
			e->x_speed = -SPEED_10(0x1FF) + SPEED_10((rand() & 0x3FF));
			e->y_speed = -SPEED_10(0x1FF) + SPEED_10((rand() & 0x3FF));
			e->state = 1;
		} else if(!e->linkedEntity->alwaysActive) {
			e->state = STATE_DELETE;
		} else {
			e->jump_time++;
			int16_t xoff = cos2[e->jump_time] << 4; // cosine * 24
			int16_t yoff = sin[e->jump_time] << 5;
			e->x = e->linkedEntity->x + xoff;
			e->y = e->linkedEntity->y + yoff;
			FACE_PLAYER(e);
		}
	}
}

#define BUYOBUYO_BASE_HP		60

void ai_buyobuyo_base(Entity *e) {
	if (e->state < 3 && e->health < (1000 - BUYOBUYO_BASE_HP)) {
		//SmokeClouds(o, objprop[e->type].death_smoke_amt, 8, 8);
		e->attack = 0;
		//e->eflags &= ~NPC_SHOOTABLE;
		e->flags &= ~NPC_SHOOTABLE;
		entity_drop_powerup(e);
		sound_play(e->deathSound, 5);
		e->state = 10;
		e->frame = 2;
	}
	
	switch(e->state) {
		case 0:
		{
			// pixel does some weird stuff with the hitboxes, normalize them
			e->hit_box = (bounding_box) {{ 12, 12, 12, 12 }};
			e->display_box = (bounding_box) {{ 16, 16, 16, 16 }};
			// OPTION2 means we are on the ceiling
			if(e->flags & NPC_OPTION2) {
				e->y -= 0x1000;
			} else {
				e->y += 0x1000;
			}
			e->timer = TIME(10);
			e->state = 1;
		}
		/* fallthrough */
		case 1:
		{
			if (PLAYER_DIST_X(e, 0x14000)) {
				if ((!(e->flags & NPC_OPTION2) && PLAYER_DIST_Y2(e, 0x14000, 0x2000)) ||
					((e->flags & NPC_OPTION2) && PLAYER_DIST_Y2(e, 0x2000, 0x14000))) {
					if (--e->timer == 0) {
						e->state = 2;
						e->timer = 0;
						e->frame = 1;
					}
				}
			}
		}
		break;
		case 2:
		{
			if (++e->timer > TIME(10)) {
				Entity *buyo = entity_create(e->x, e->y, OBJ_BUYOBUYO, e->flags & NPC_OPTION2);
				if(e->flags & NPC_OPTION2) {
					// On ceiling, buyo are fine
				} else {
					// On floor, buyo delete immediately, so push them up
					buyo->y -= 0x800;
				}
				
				sound_play(SND_EM_FIRE, 5);
				CURLY_TARGET_HERE(e);
				
				// cyclic: three firings then pause
				e->state = 1;
				if (++e->timer2 > 2) {
					e->timer = TIME(100);
					e->timer2 = 0;
				} else {
					e->timer = TIME(20);
				}
				e->frame = 0;
			}
		}
		break;
	}
}

void ai_buyobuyo(Entity *e) {
	e->flags ^= NPC_SHOOTABLE;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	ANIMATE(e, 8, 0,1);
	
	switch(e->state) {
		case 0:
		{
			// shoot up down at player...
			e->y_speed = (e->flags & NPC_OPTION2) ? SPEED(0x600) : -SPEED(0x600);
			e->state = 1;
			e->timer = 0;
		}
		/* fallthrough */
		case 1:
		{
			e->timer++;		// inc fly time
			// reached height of player yet?
			if (PLAYER_DIST_Y(e, 0x2000)) {
				e->state = 2;
			} else break;
		}
		/* fallthrough */
		case 2:
		{
			// this slight "minimum fly time" keeps the underwater ones from
			// smacking into the floor if the player is underwater with them
			if (++e->timer > 4) {
				e->x_mark = e->x;
				e->y_mark = e->y;
				
				e->x_speed = (rand() & 1) ? SPEED(0x200) : -SPEED(0x200);
				e->y_speed = (rand() & 1) ? SPEED(0x200) : -SPEED(0x200);
				
				e->state = 3;
			}
		}
		break;
		case 3:
		{
			if (e->x > e->x_mark) e->x_speed -= SPEED_8(0x20);
			if (e->x < e->x_mark) e->x_speed += SPEED_8(0x20);
			if (e->y > e->y_mark) e->y_speed -= SPEED_8(0x20);
			if (e->y < e->y_mark) e->y_speed += SPEED_8(0x20);
			LIMIT_X(SPEED_10(0x3FF));
			LIMIT_Y(SPEED_10(0x3FF));
			
			// move the point we are bobbling around
			e->x_mark += e->dir ? SPEED_10(0x200) : -SPEED_10(0x200);
			
			if (++e->timer > TIME_10(300)) {
				e->state = STATE_DELETE;
				return;
			}
		}
		break;
	}
	// Quicker collision
	if (blk(e->x, 0, e->y, 0) == 0x41) {
		e->state = STATE_DELETE;
		return;
	}
	
	e->x = e->x_next;
	e->y = e->y_next;
}

void onspawn_gero(Entity *e) {
	e->x -= 8 << CSF;
}

void ai_gero(Entity *e) {
	e->frame = 0;
    RANDBLINK(e, 1, 200);
}
