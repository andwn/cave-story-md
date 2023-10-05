#include "ai_common.h"

#define angle	jump_time

void ai_heli(Entity *e) {
	if(e->dir) { // STOP changing direction!
		e->dir = 0;
		e->frame = 1;
	}
	
	switch(e->state) {
		case 0:		// stopped
		{
			Entity *b = entity_create(e->x + pixel_to_sub(16), e->y - pixel_to_sub(57), OBJ_HELICOPTER_BLADE, 0);
			b->linkedEntity = e;
			b = entity_create(e->x - pixel_to_sub(36), e->y - pixel_to_sub(52), OBJ_HELICOPTER_BLADE2, 0);
			b->linkedEntity = e;
			e->state++;
		}
		break;
		
		case 20:	// blades running
		break;
		
		case 30:	// blades running, spawn momorin
		{
			entity_create(e->x - pixel_to_sub(11), e->y - pixel_to_sub(14), OBJ_MOMORIN, 0)->dir = 0;
			e->frame = 1;		// open hatch
			e->state++;
		}
		break;
		
		case 40:	// blades running, spawn momorin, santa, and chako (from credits)
		{
			entity_create(e->x - pixel_to_sub(9), e->y - pixel_to_sub(14), OBJ_MOMORIN, 0)->dir = 0;
			entity_create(e->x - pixel_to_sub(22), e->y - pixel_to_sub(14), OBJ_SANTA, 0)->dir = 0;
			entity_create(e->x - pixel_to_sub(35), e->y - pixel_to_sub(14), OBJ_CHACO, 0)->dir = 0;
			e->frame = 1;		// open hatch
			e->state++;
		}
		break;
	}
}

void onspawn_heliblade1(Entity *e) {
	e->display_box = (bounding_box) {{ 56, 8, 56, 8 }};
}

void onspawn_heliblade2(Entity *e) {
	e->display_box = (bounding_box) {{ 36, 8, 36, 8 }};
}

void ai_heli_blade(Entity *e) {
	switch(e->state) {
		case 0:
		case 1:
		{
			if (e->linkedEntity && e->linkedEntity->state >= 20)
				e->state = 10;
		}
		break;
		
		case 10:
		{
			static const uint8_t f[] = { 0, 1, 2, 1 };
			if(++e->animtime >= 16) e->animtime = 0;
			e->frame = f[e->animtime >> 2];
		}
		break;
	}
}

void ai_igor_balcony(Entity *e) {
	enum Frame { STAND1, STAND2, WALK1, WALK2, PUNCH1, PUNCH2, MOUTH1, 
				 JUMP, LAND, DEFEAT1, MOUTH2, DEFEAT2, DEFEAT3, DEFEAT4 };
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	uint8_t blockl = e->x_speed < 0 && collide_stage_leftwall(e);
	uint8_t blockr = e->x_speed > 0 && collide_stage_rightwall(e);
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else if(e->x_speed != 0) e->grounded = collide_stage_floor_grounded(e);
	
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->grounded = FALSE;
			e->display_box.top += 4;
			e->frame = STAND1;
		} /* fallthrough */
		case 1:
		{
			if(++e->animtime >= TIME_8(20)) {
				e->animtime = 0;
				if(++e->frame > STAND2) e->frame = STAND1;
			}
			
			if((PLAYER_DIST_X(e, pixel_to_sub(112)) && PLAYER_DIST_Y2(e, pixel_to_sub(48), pixel_to_sub(112))) || e->damage_time) {
				e->state = 10;
			}
		}
		break;
		
		case 10:		// walking towards player
		{
			e->state = 11;
			//e->frame = STAND1;
			e->animtime = 0;
			FACE_PLAYER(e);
			moveMeToFront = TRUE;
		} /* fallthrough */
		case 11:
		{
			static const uint8_t f[] = { WALK1,STAND1,WALK2,STAND1 };
			if(++e->animtime >= 32) e->animtime = 0;
			e->frame = f[e->animtime >> 3];
			MOVE_X(SPEED(0x200));
			
			if (blockr || blockl || PLAYER_DIST_X(e, pixel_to_sub(64))) {
				e->x_speed = 0;
				e->state = 20;
				e->timer = 0;
			}
		}
		break;
		
		case 20:	// prepare to jump...
		{
			e->frame = LAND;	// jump-prepare frame
			
			if (++e->timer > 10) {
				e->state = 21;
				e->y_speed = -SPEED_12(0x5ff);
				e->grounded = FALSE;
				MOVE_X(SPEED_10(0x200));
				sound_play(SND_IGOR_JUMP, 5);
				moveMeToFront = TRUE;
			}
		}
		break;
		
		case 21:	// jumping
		{
			e->frame = JUMP;	// in-air frame
			MOVE_X(SPEED_10(0x200));
			if (e->grounded) {
				camera_shake(20);
				e->x_speed = 0;
				
				e->state = 22;
				e->timer = 0;
				e->frame = LAND;
			}
		}
		break;
		
		case 22:	// landed
		{
			if (++e->timer > 30)
				e->state = 30;
		}
		break;
		
		case 30:	// mouth-blast attack
		{
			e->state = 31;
			e->timer = 0;
			FACE_PLAYER(e);
			moveMeToFront = TRUE;
		} /* fallthrough */
		case 31:
		{
			e->timer++;
			
			// flash mouth
			e->frame = MOUTH1;
			if (e->timer < TIME(50) && (e->timer & 4)) e->frame = MOUTH2;
			
			// fire shots
			if (e->timer > 30) {
				if ((e->timer & 7) == 1) {
					sound_play(SND_BLOCK_DESTROY, 5);
					Entity *shot = entity_create(e->x + (e->dir ? 0x800 : -0x800),
												 e->y, OBJ_IGOR_SHOT, 0);
					shot->x_speed = e->dir ? SPEED_12(0x600) : -SPEED_12(0x600);
					shot->y_speed = -SPEED_12(0x280) + SPEED_12((rand() & 0x3FF));
				}
			}
			
			if (e->timer > 82) {
				FACE_PLAYER(e);
				e->state = 10;
			}
		}
		break;
	}
	
	e->x = e->x_next;
	e->y = e->y_next;
	
	if(!e->grounded) e->y_speed += SPEED_8(0x33);
	LIMIT_Y(SPEED_12(0x5ff));
}

void ai_block_spawner(Entity *e) {
	switch(e->state) {
		// wait till player leaves "safe zone" at start of Balcony
		// does nothing in Hell--you enter from the left.
		case 0:
		{
			if(player.x < block_to_sub(stageWidth - 6)) {
				e->state = 1;
				e->timer = 24;
			}
		}
		break;
		
		case 1:
		{
			if(--e->timer == 0) {
				int32_t x;
				// blocks tend to follow behind the player--this goes along
				// with the text that tells you to run so as not to get squashed.
				if (playerEquipment & EQUIP_BOOSTER20) {
					x = (player.x + block_to_sub(4));
					if (x < block_to_sub(26)) x = block_to_sub(26);
				} else {
					x = (player.x + block_to_sub(6));
					if (x < block_to_sub(23)) x = block_to_sub(23);
				}
				
				if (x > block_to_sub(stageWidth - 10))
					x = block_to_sub(stageWidth - 10);
				
				if (playerEquipment & EQUIP_BOOSTER20) {
					static const int8_t pos[32] = {
						-14, -13, -11, -10, -8, -6, -4, -3, -2,  0,  2,  3,  4,  6,  7,  8,  
						 10,  11,  12,  14, 15, 16, 18, 19, 20, 21, 22, 24, 25, 26, 27, 28
					};
					x += block_to_sub(pos[rand() & 31]);
				} else {
					static const int8_t pos[32] = {
						-11, -10,  -9,  -8, -7, -6, -4, -3, -2,  0,  1,  2,  3,  4,  5,  6,  
						  7,   8,   9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22
					};
					x += block_to_sub(pos[rand() & 31]);
				}
				
				entity_create(x, (player.y - block_to_sub(14)), 
							OBJ_FALLING_BLOCK, (rand() & 1) ? NPC_OPTION2 : 0);
									  
				e->timer = TIME_8(15) + (rand() & 15);
			}
		}
		break;
	}
}

void ai_falling_block(Entity *e) {
	e->attack = (player.y > e->y) ? 10 : 0;
	
	switch(e->state) {
		case 0:
		{	
			if(e->flags & NPC_OPTION2) { // large Hell or Balcony block
				e->flags |= NPC_INVINCIBLE;
				e->state = 10;
			} else if(e->flags & NPC_OPTION1) { // Misery-spawned block
				e->state = 1;
				e->timer = 0;
			} else { // small Hell or Balcony block
				e->flags |= NPC_INVINCIBLE;
				e->state = 10; 
				e->hit_box = (bounding_box) {{ 8, 8, 8, 8 }};
				e->display_box = (bounding_box) {{ 8, 8, 8, 8 }};
				e->sheet++; // SHEET_BLOCKM always after SHEET_BLOCK
				e->vramindex = sheets[e->sheet].index;
				e->sprite[0].size = SPRITE_SIZE(2, 2);
			}
		}
		break;
		
		case 1:	// just spawned by Misery--pause a moment
		{
			if (++e->timer > 3) {
				e->flags |= NPC_INVINCIBLE;
				e->state = 10;
			}
		}
		break;
		
		case 10:	// falling
		{	// allow to pass thru Hell/Balcony ceiling
			if (e->y > pixel_to_sub(128)) {
				e->state = 11;
			}
			e->y_speed += SPEED(0x40);
			LIMIT_Y(SPEED(0x5FF));
		}
		break;
		
		case 11:	// passed thru ceiling in Hell B2
		{
			e->y_speed += SPEED(0x40);
			LIMIT_Y(SPEED(0x5FF));
			
			if (blk(e->x, 0, e->y, (NPC_OPTION2 ? 8 : 20)) == 0x41) {
				e->y_speed = -SPEED(0x280);
				
				e->state = 20;
				SMOKE_AREA((e->x >> CSF) - 8, (e->y >> CSF) + (NPC_OPTION2 ? 8 : 16), 16, 1, 1);
				camera_shake(10);
			}
		}
		break;
		
		case 20:	// already bounced on ground, falling offscreen
		{
			e->attack = 0;
			e->y_speed += SPEED(0x40);
			LIMIT_Y(SPEED(0x5FF));
			
			if (e->y >= block_to_sub(stageHeight + 1)) {
				e->state = STATE_DELETE;
			}
		}
		break;
	}
	e->x += e->x_speed;
	e->y += e->y_speed;
}

// The Doctor in his red energy form.
// there is no "move" state, when he takes over Misery,
// the object is moved kind of unconventionally, using an <MNP.
void ai_doctor_ghost(Entity *e) {
	switch(e->state) {
		case 10:
		{
			e->state = 11;
			e->timer = 0;
		} /* fallthrough */
		case 11:
		{
			e->timer++;
			if((e->timer & 7) == 0) {
				Entity *r = entity_create(e->x, e->y/*+pixel_to_sub(128)*/, OBJ_RED_ENERGY, 0);
				r->angle = A_RIGHT;
				r->linkedEntity = e;
				if((e->timer & 15) == 0) r->timer = 200;
			}
			
			if(e->timer > TIME_8(150))
				e->state++;
		}
		break;
		
		case 20:
		{
			e->state = 21;
			e->timer = 0;
		} /* fallthrough */
		case 21:
		{
			if (++e->timer > TIME_8(250)) {
				entities_clear_by_type(OBJ_RED_ENERGY);
				e->state++;
			}
		}
		break;
	}
}

// red energy for doctor. In a completely different role,
// it's also used for the dripping blood from Ballos's final form.
void ai_red_energy(Entity *e) {
	switch(e->angle) {
		case A_UP:
		{
			e->frame = rand() & 1;
			e->y_speed -= SPEED_8(0x40);
			e->y += e->y_speed;
			if (blk(e->x, 0, e->y, 0) == 0x41) e->state = STATE_DELETE;
		}
		break;
		
		case A_DOWN:
		{
			e->frame = rand() & 1;
			if(e->y_speed < SPEED_12(0x5C0)) e->y_speed += SPEED_8(0x40);
			e->y += e->y_speed;
			if((++e->timer > TIME_8(50)) || (blk(e->x, 0, e->y, 0) == 0x41)) e->state = STATE_DELETE;
		}
		break;
		
		case A_RIGHT:
		{
			if (!e->linkedEntity || (e->linkedEntity->state == 21 && ++e->timer > 200)) { 
				e->state = STATE_DELETE; 
				return; 
			}
			if(!e->state) {
				e->state++;
				e->x_speed = -0xFF + (rand() & 0x1FF);
				e->y_speed = -0xFF + (rand() & 0x1FF);
			}
			if(e->x < e->linkedEntity->x)		{ if(e->x_speed < SPEED_10(0x3ff)) e->x_speed += SPEED_8(12); }
			else if(e->x > e->linkedEntity->x)	{ if(e->x_speed > -SPEED_10(0x3ff)) e->x_speed -= SPEED_8(12); }
			if(e->y < e->linkedEntity->y)		{ if(e->y_speed < SPEED_10(0x3ff)) e->y_speed += SPEED_8(12); }
			else if(e->y > e->linkedEntity->y)	{ if(e->y_speed > -SPEED_10(0x3ff)) e->y_speed -= SPEED_8(12); }
			
			e->x += e->x_speed;
			e->y += e->y_speed;
		}
	}
}

void ai_mimiga_caged(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->x -= pixel_to_sub(1);
			e->y -= pixel_to_sub(2);
		} /* fallthrough */
		case 1:
		{
			e->frame = 0;
			RANDBLINK(e, 1, 200);
			if(e->frame == 0) FACE_PLAYER(e);
		}
		break;
		
		case 10:	// blush and spawn heart
		{
			e->state = 11;
			e->frame = 2;
			entity_create(e->x, e->y - pixel_to_sub(16), OBJ_HEART, 0);
		} /* fallthrough */
		case 11:
		{
			FACE_PLAYER(e);
		}
		break;
	}
}
