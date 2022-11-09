#include "ai_common.h"

// dragonfly creature
void ai_stumpy(Entity *e) {
	e->frame ^= 1;
	// Collide up front
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	e->x_mark = e->x_speed; // Remember speed
	e->y_mark = e->y_speed;
	uint8_t blockl = e->x_speed < 0 && collide_stage_leftwall(e),
		blockr = e->x_speed > 0 && collide_stage_rightwall(e),
		blocku = e->y_speed < 0 && collide_stage_ceiling(e),
		blockd = e->y_speed > 0 && collide_stage_floor(e);

	switch(e->state) {
		case 0:
		{
			e->state++;
			e->timer = 0;
			FACE_PLAYER(e);
		} /* fallthrough */
		case 1:
		{
			if (++e->timer > TIME(50)) e->state++;
		}
		break;
		case 2:
		{
			e->state++;
			e->timer = 0;
			THROW_AT_TARGET(e, player.x, player.y, 0x400);
			e->dir = e->x_speed >= 0;
		} /* fallthrough */
		case 3:
		{
			if(blockl || blockr) e->x_speed = -e->x_mark;
			if(blocku || blockd) e->y_speed = -e->y_mark;
			if (++e->timer > TIME(50)) {
				e->state = 0;
				e->x_speed = 0;
				e->y_speed = 0;
			}
		}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

// very happy green guy who runs back and forth
void ai_midorin(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(e->grounded) e->grounded = collide_stage_floor_grounded(e);
	else e->grounded = collide_stage_floor(e);
	
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->timer = 0;
			e->frame = 0;
			e->x_speed = 0;
		} /* fallthrough */
		case 1:
		{
			if (!(rand() & 31)) {
				e->state = 2 + (rand() & 1);
				e->frame = 1;
			}
		}
		break;
		
		case 2:		// blinking
		{
			if (++e->timer > TIME_8(8)) {
				e->state = 1;
				e->timer = 0;
				e->frame = 0;
			}
		}
		break;
		
		case 3:		// running
		{
			moveMeToFront = TRUE; // Stay in front of doors
			e->state = 4;
			e->frame = 0;	// this will be toggled into frame 2 just below
			e->animtime = 0;
			
			e->timer = 40 + (rand() & 15);		// how long to run
			e->dir = rand() & 1;
		} /* fallthrough */
		case 4:
		{
			if (e->animtime ^= 1) e->frame ^= 2;
			
			if ((!e->dir && collide_stage_leftwall(e)) ||
				(e->dir && collide_stage_rightwall(e))) {
				e->dir ^= 1;
			}
			
			MOVE_X(SPEED(0x400));
			
			if (--e->timer <= 0) e->state = 0;
		}
		break;
	}
	
	if(!e->grounded) e->y_speed += SPEED(0x20);
	LIMIT_Y(SPEED(0x5ff));
	e->x = e->x_next;
	e->y = e->y_next;
}

// large bat with many baby bats
void ai_orangebell(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state)
	{
		case 0:
		{
			e->state = 1;
			e->y_mark = e->y;
			e->y_speed = SPEED(0x200);
			
			// create baby bats
			for(uint8_t i=0;i<8;i++) {
				Entity *bat = entity_create(e->x - 0x1000 + (rand() & 0x1FFF),
										   e->y - 0x1000 + (rand() & 0x1FFF),
										   OBJ_ORANGEBELL_BABY, 0);
				bat->linkedEntity = e;
			}
		} /* fallthrough */
		case 1:
		{
			ANIMATE(e, 5, 0,1,2);
			
			if ((!e->dir && collide_stage_leftwall(e)) ||
				(e->dir && collide_stage_rightwall(e))) {
				e->dir ^= 1;
			}
			
			MOVE_X(SPEED(0x100));
			
			e->y_speed += (e->y < e->y_mark) ? 0x08 : -0x08;
			LIMIT_Y(SPEED(0x200));
		}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_orangebell_baby(Entity *e) {
	switch(e->state) {
		case 0:
		{
			uint8_t angle = rand();
			e->x_speed = cos[angle];//pixel_to_sub();
			e->y_speed = sin[angle];//pixel_to_sub();
			
			e->timer = 0;	// time until can dive-bomb
			// unique target point on main bat
			e->y_next = -pixel_to_sub(rand() & 31);
			
			e->state = 1;
		} /* fallthrough */
		case 1:
		{
			ANIMATE(e, 4, 0,1,2);
			
			if (e->linkedEntity) {
				e->x_mark = e->linkedEntity->x;
				e->y_mark = e->linkedEntity->y + e->y_next;
				e->dir = e->linkedEntity->dir;
			}
			
			e->x_speed += (e->x < e->x_mark) ? 0x08 : -0x08;
			e->y_speed += (e->y < e->y_mark) ? 0x20 : -0x20;
			LIMIT_X(SPEED(0x400));
			LIMIT_Y(SPEED(0x400));
			
			// dive-bomb
			if (e->timer) e->timer--;
			if (PLAYER_DIST_X(e, pixel_to_sub(8)) && !e->timer) {
				if (player.y > e->y && ((player.y - e->y) < pixel_to_sub(175))) {
					e->x_speed /= 4;
					e->y_speed = 0;
					e->state = 2;
				}
			}
		}
		break;
		
		case 2:		// dive-bombing
		{
			e->frame = 3;
			e->y_speed += SPEED(0x40);
			LIMIT_Y(SPEED(0x5ff));
			// Quick floor check
			if (blk(e->x, 0, e->y, 8) == 0x41) {
				e->x_speed *= 2;
				e->y_speed = 0;
				e->timer = TIME(120);
				
				e->state = 1;
			}
		}
		break;
	}
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_gunfish(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->y_speed = 0;
			e->y_mark = e->y;
			
			e->state = 1;
			e->timer = rand() & 63;
		} /* fallthrough */
		case 1:		// desync
		{
			if (e->timer == 0) {
				e->y_speed = SPEED_8(0x200);
				e->state = 2;
				e->timer = 0;
			} else e->timer--;
		}
		break;
		
		case 2:
		{
			ANIMATE(e, 4, 0,1);
			FACE_PLAYER(e);
			
			if (PLAYER_DIST_X(e, pixel_to_sub(128)) && PLAYER_DIST_Y2(e, pixel_to_sub(160), 20<<CSF)) {
				if (++e->timer > TIME_8(80)) {
					e->state = 10;
					e->timer = 0;
					e->frame += 2;
				}
			}
		}
		break;
		
		case 10:
		{
			ANIMATE(e, 4, 2,3);
			
			if (++e->timer > TIME_8(20)) {
				e->state = 20;
				e->timer = 0;
				e->timer2 = 0;
				e->frame += 2;
			}
		}
		break;
		
		case 20:
		{
			ANIMATE(e, 4, 4,5);
			
			if(++e->timer > TIME_8(10)) {
				e->timer = 0;
				e->timer2++;
				Entity *shot = entity_create(e->x, e->y, OBJ_GUNFISH_SHOT, 0);
				
				shot->x_speed = e->dir ? SPEED(0x400) : -SPEED(0x400);
				shot->y_speed = -SPEED(0x400);
				
				sound_play(SND_EM_FIRE, 3);
			}
			
			if (e->timer2 > 6) {
				e->state = 2;
				e->timer = 0;
				e->frame -= 4;
			}
		}
		break;
	}
	
	e->y_speed += (e->y < e->y_mark) ? 0x10 : -0x10;
	LIMIT_Y(SPEED(0x100));
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_gunfish_shot(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	// Delete after hitting a wall or water
	if ((e->x_speed < 0 && collide_stage_leftwall(e)) ||
		(e->x_speed > 0 && collide_stage_rightwall(e)) ||
		(e->y_speed < 0 && collide_stage_ceiling(e)) ||
		(e->y_speed > 0 && collide_stage_floor(e)) ||
		(blk(e->x, 0, e->y, -4) & 0x10)) {
		//for(uint8_t i=0;i<5;i++) {
		//	Caret *c = effect(e->CenterX(), e->CenterY(), EFFECT_GUNFISH_BUBBLE);
		//	c->x_speed = rand(-0x400, 0x400);
		//	c->y_speed = rand(-0x400, 0);
		//}
		sound_play(SND_BUBBLE, 3);
		e->state = STATE_DELETE;
	}
	
	e->y_speed += SPEED(0x20);
	LIMIT_Y(SPEED(0x5ff));
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_droll(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(e->grounded) e->grounded = collide_stage_floor_grounded(e);
	else e->grounded = collide_stage_floor(e);
	switch(e->state) {
		case 0:
		{
			e->x_mark = e->x;
			e->state = 1;
		} /* fallthrough */
		case 1:
		{
			ANIMATE(e, 40, 0,1);
			FACE_PLAYER(e);
			
			if (e->damage_time) e->state = 10;
		}
		break;
		
		case 10:	// prepare to jump
		{
			e->state = 11;
			e->frame = 2;
			e->timer = 0;
		} /* fallthrough */
		case 11:
		{
			if (++e->timer > 10) {	// jump
				e->state = 12;
				e->frame = 6;
				e->timer2 = 0;		// have not fired yet
				
				e->x_speed = (e->x >= e->x_mark) ? -SPEED(0x200) : SPEED(0x200);
				e->y_speed = -SPEED(0x600);
			}
		}
		break;
		
		case 12:	// in air
		{
			if (e->y_speed >= 0) {
				if (!e->timer2)	{	// have fired yet?
					FACE_PLAYER(e);
					e->timer2 = 1;
					e->frame = 7;
					//FIRE_ANGLED_SHOT(OBJ_DROLL_SHOT, e->x, e->y, e->dir ? A_RIGHT : A_LEFT, 0x600);
					Entity *shot = entity_create(e->x, e->y, OBJ_DROLL_SHOT, 0);
					THROW_AT_TARGET(shot, player.x, player.y, 0x600);
					sound_play(SND_EM_FIRE, 5);
				} else if (e->y_speed > SPEED(0x200)) {	// after-fire frame
					e->frame = 3;
				}
				
				if (e->grounded) {
					//e->y >>= CSF; e->y <<= CSF;
					e->x_speed = 0;
					
					e->state = 13;
					e->frame = 2;
					e->timer = 0;
				}
			}
		}
		break;
		
		case 13:	// landed
		{
			e->x_speed /= 2;
			
			if (++e->timer > 10) {
				e->x_speed = 0;
				e->state = 1;
			}
		}
		break;
	}
	
	if(!e->grounded) e->y_speed += SPEED(0x55);
	LIMIT_Y(SPEED(0x5ff));
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_droll_shot(Entity *e) {
	e->x += e->x_speed;
	e->y += e->y_speed;
	ANIMATE(e, 4, 0,1,2);
	
	if ((++e->timer & 7) == 0) sound_play(SND_DROLL_SHOT_FLY, 3);
	
	if (blk(e->x, 0, e->y, 0) & 0x41) {
		effect_create_smoke(e->x >> CSF, e->y >> CSF);
		//effect(e->CenterX(), e->CenterY(), EFFECT_BOOMFLASH);
		e->state = STATE_DELETE;
	}
}

// stomps you in Teleporter Room
void ai_droll_guard(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(e->grounded) e->grounded = collide_stage_floor_grounded(e);
	else e->grounded = collide_stage_floor(e);
	switch(e->state) {
		case 0:
		{
			e->x += pixel_to_sub(8);
			e->state = 1;
		} /* fallthrough */
		case 1:
		{
			ANIMATE(e, 30, 0,1);
			FACE_PLAYER(e);
		}
		break;
		
		case 10:
		{
			e->timer = 0;
			e->state = 11;
		} /* fallthrough */
		case 11:
		{
			e->frame = 2;
			if (++e->timer > 10) {
				e->state = 12;
				e->frame = 3;
				e->y_speed = -0x600;
				MOVE_X(0x200);
			}
		}
		break;
		
		case 12:	// in air...
		{
			if (e->y_speed >= 0 && e->grounded) {
				e->frame = 2;
				e->state = 13;
				e->timer = 0;
			}
		}
		break;
		
		case 13:	// landed
		{
			e->x_speed /= 2;
			if (++e->timer > 10) {
				e->x_speed = 0;
				e->frame = 0;
				e->state = 1;
			}
		}
		break;
	}
	
	if(!e->grounded) e->y_speed += 0x40;
	LIMIT_Y(0x5ff);
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_mimiga_farmer(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(e->grounded) e->grounded = collide_stage_floor_grounded(e);
	else e->grounded = collide_stage_floor(e);
	switch(e->state) {
		case 0:
		{
		    if(e->x > block_to_sub(18)) {
		        e->timer2 = 1;
		    }
			e->frame = 0;
			e->x_speed = 0;
			e->state = 1;
		} /* fallthrough */
		case 1:
		{
			if (!(rand() & 63)) {
				if (e->type != OBJ_MIMIGA_FARMER_STANDING && (rand() & 1)) {	// walk
					e->state = 10;
				} else {	// blink
					e->state = 2;
					e->frame = 1;
				}
			}
		}
		break;
		case 2:		// blinking
		{
			if (++e->timer >= 8) {
				e->state = 0;
				e->timer = 0;
			}
		}
		break;
		
		case 10:	// walk
		{
			e->state = 11;
			e->frame = 1;
			e->animtime = 0;
			
			e->timer = 16 + (rand() & 15);
			e->dir = rand() & 1;
		} /* fallthrough */
		case 11:
		{
		    if(e->type == OBJ_MIMIGA_JAILED) {
		        if(e->timer2) { // I give up
		            if(sub_to_pixel(e->x) < block_to_pixel(21) + 8) e->dir = 1;
                    if(sub_to_pixel(e->x) > block_to_pixel(25) - 8) e->dir = 0;
		        } else {
                    if(sub_to_pixel(e->x) < block_to_pixel(13) + 8) e->dir = 1;
                    if(sub_to_pixel(e->x) > block_to_pixel(17) - 8) e->dir = 0;
		        }
		    } else {
                if(blk(e->x, e->dir ? 9 : -9, e->y, 0) == 0x41) e->dir ^= 1;
		    }
			//if ((!e->dir && collide_stage_leftwall(e)) ||
			//	(e->dir && collide_stage_rightwall(e))) {
			//	e->dir ^= 1;
			//}
			
			MOVE_X(SPEED(0x200));
			ANIMATE(e, 8, 2,0,3,0);
			
			if (!--e->timer)
				e->state = 0;
		}
		break;
	}
	
	if(!e->grounded) e->y_speed += 0x20;
	LIMIT_Y(SPEED(0x5ff));
	e->x = e->x_next;
	e->y = e->y_next;
}


void onspawn_mimiga_cage(Entity *e) {
	e->alwaysActive = TRUE;
	e->state = 1;
	e->x -= pixel_to_sub(16);
}

void ai_npc_itoh(Entity *e) {
	enum Frame { STAND, BLINK, LEAP, COWER, WALK1, WALK2 };
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(e->grounded) e->grounded = collide_stage_floor_grounded(e);
	else e->grounded = collide_stage_floor(e);
	switch(e->state) {
		case 0:
		{
			e->state = 1;
		} /* fallthrough */
		case 1:
		{
			e->x_speed = 0;
			e->frame = STAND;
			RANDBLINK(e, BLINK, 200);
		}
		break;
		
		case 10:		// shock
		{
			e->frame = LEAP;
			e->x_speed = 0;
		}
		break;
		
		case 20:		// leap away
		{
			e->state = 21;
			e->frame = LEAP;
			e->x_speed = SPEED(0x200);
			e->y_speed = -SPEED(0x400);
			e->grounded = FALSE;
		} /* fallthrough */
		case 21:		// cower after leap
		{
			if (e->grounded) {
				e->x_speed = 0;
				
				e->frame = COWER;
				e->state = 30;
				e->timer = 0;
			}
		}
		break;
		
		case 30:		// shaking after leap
		{
			e->x_mark = e->x;
			e->state = 31;
		} /* fallthrough */
		case 31:
		{
			e->x = e->x_mark;
			if (++e->timer & 2) e->x += pixel_to_sub(1);
		}
		break;
		
		case 40:	// small vertical hop (from when he jumps out of helicopter at end)
		{
			e->state = 41;
			e->y_speed = -SPEED(0x200);
			e->frame = LEAP;
			e->grounded = FALSE;
		} /* fallthrough */
		case 41:
		{
			if (e->grounded) {
				e->x_speed = 0;
				e->state = 42;
				e->frame = STAND;
				e->dir = 1;
			}
		}
		break;
		case 42:	// stop begin shocked after blocks fall (same as 0 except he doesn't blink)
		{
			e->frame = STAND;
			e->dir = 1;
		}
		break;
		
		case 50:	// run a moment then stop
		{
			e->state = 51;
			e->timer = 0;
			e->animtime = 0;
			e->dir = 1;
			e->frame = WALK1;
		} /* fallthrough */
		case 51:
		{
			ANIMATE(e, 4, WALK1,STAND,WALK2,STAND);
			MOVE_X(SPEED(0x200));
			
			if (++e->timer > TIME(32)) {
				e->frame = STAND;
				e->x_speed = 0;
				e->state = 52;
			}
		}
		break;
	}
	
	if(!e->grounded) e->y_speed += SPEED(0x40);
	LIMIT_Y(SPEED(0x5ff));
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_kanpachi_stand(Entity *e) {
	e->dir = 1;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);
	switch(e->state) {
		case 0:		// stand
		{
			e->frame = 1;
			e->x_speed = 0;
			e->y_speed = 0;
			RANDBLINK(e, 2, 200);
		}
		break;
		case 3:		// walking
		{
			e->state++;
			moveMeToFront = TRUE; // Gotta be in front of the door
		} /* fallthrough */
		case 4:
		{
			ANIMATE(e, 8, 3,4);
			MOVE_X(SPEED(0x200));
		}
		break;
		case 5:		// face away
		{
			e->x_speed = 0;
			e->frame = 0;
		}
		break;
		case 10:	// walking
		case 11:
		{
			e->state = 3;
		}
		break;
		case 20:	// face away/enter door
		{
			e->x_speed = 0;
			e->frame = 0;
		}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
	e->dir = 0;
}


void ai_npc_momorin(Entity *e) {
	switch(e->state) {
		case 0:
			e->frame = 0;
			RANDBLINK(e, 1, 200);
			FACE_PLAYER(e);
		break;
		
		case 3:		// surprised
			e->frame = 2;
		break;
	}
}

void ai_prox_press_hoz(Entity *e) {
	switch(e->state) {
		case 0:
		{
			if(e->flags & NPC_OPTION2) e->dir = 1;
			if (!e->dir) e->x -= pixel_to_sub(8);
			e->x_mark = e->x;
			e->state = 1;
		} /* fallthrough */
		case 1:
		{
			if(PLAYER_DIST_Y2(e, 0x800, 0x800)) {
				if(!e->dir) {
					if(player.x < e->x) {
						e->state = 2;
						e->frame = 2;
						e->timer = 0;
					}
				} else {
					if(player.x > e->x) {
						e->state = 2;
						e->frame = 2;
						e->timer = 0;
					}
				}
			}
		}
		break;
		
		case 2:		// activated
		{
			e->attack = 127;
			MOVE_X(0x900);
			
			//if (++e->timer == 10) {
			//	sound_play(SND_BLOCK_DESTROY, 5);
				//SmokeSide(o, 4, e->dir);
			//}
			
			if (++e->timer > 11) {
				int16_t xx = (e->x >> CSF) + e->dir ? 12 : -12, yy = e->y >> CSF;
				effect_create_smoke(xx, yy);
				sound_play(SND_BLOCK_DESTROY, 5);
				e->attack = 0;
				e->x_speed = 0;
				e->state = 3;
				e->timer = 0;
			}
		}
		break;
		
		case 3:		// hit other press
		{
			if (++e->timer > 60) {
				e->state = 4;	// return
				e->frame = 1;
				e->timer = 0;
			}
		}
		break;
		
		case 4:		// return to start pos
		{
			MOVE_X(-0x5A0);
			
			//if(!e->dir) collide_stage_rightwall(e);
			//else collide_stage_leftwall(e);
			
			if (++e->timer > 15) {
				e->frame = 0;
				e->x_speed = 0;
				e->x = e->x_mark;
				
				e->state = 1;
				e->timer = 0;
			}
		}
		break;
	}
	e->x += e->x_speed;
}

void ai_rocket(Entity *e) {
	if(e->state) {
		e->x_next = e->x + e->x_speed;
		e->y_next = e->y + e->y_speed;
	}
	switch(e->state) {
		case 10:	// activated
		{
			e->state = 11;
			e->timer = 0;
			e->alwaysActive = TRUE;
		} /* fallthrough */
		case 11:
		{
			e->timer++;
			e->y_speed += SPEED(8);
			
			if (collide_stage_floor(e)) {
				if (e->timer < 10) {
					e->state = 12;
				} else {
					e->state = 0;
				}
			}
		}
		break;
		
		case 12:
		{
			e->flags &= ~NPC_INTERACTIVE;
			e->state = 13;
			e->timer = 0;
			e->frame = 1;
			
			//SmokeClouds(o, 10, 16, 8);
			sound_play(SND_BLOCK_DESTROY, 5);
		} /* fallthrough */
		case 13:
		{
			e->y_speed -= SPEED(0x08);
			e->timer++;
			//SpawnRocketTrail(o, (e->timer & 1) ? RIGHT : LEFT);
			
			if ((e->timer & 3) == 1) sound_play(SND_FIREBALL, 3);
			
			if (collide_stage_ceiling(e)) {
				//SmokeClouds(o, 6, 16, 8);
				sound_play(SND_BLOCK_DESTROY, 5);
				
				e->y_speed = 0;
				e->state = 14;
				
			}
		}
		break;
		
		case 14:		// falling back down
		{
			e->y_speed += 8;
			e->timer++;
			if (e->y_speed < 0) {
				if ((e->timer & 15) == 1) sound_play(SND_FIREBALL, 3);
			} else if (collide_stage_floor(e)) {
				// Apply this now, since it won't happen when the state is 0
				e->x = e->x_next;
				e->y = e->y_next;
				e->flags |= NPC_INTERACTIVE;
				e->frame = 0;
				e->state = 0;
				e->alwaysActive = FALSE;
			}
		}
		break;
	}
	LIMIT_Y(SPEED(0x5ff));
	if(e->state) {
		e->x = e->x_next;
		e->y = e->y_next;
		// Hack to prevent player from clipping off to the side
		if(!player.jump_time && PLAYER_DIST_X(e, pixel_to_sub(16)) && PLAYER_DIST_Y(e, pixel_to_sub(20))) {
			playerPlatform = e;
		}
	}
}

// in the Statue Room, if you enter via the Plantation
void ai_numahachi(Entity *e) {
	ANIMATE(e, 50, 0,1);
}

// The graphic is a bit too low
void onspawn_jailbars(Entity *e) {
	e->display_box.top += 8;
	e->flags |= NPC_SHOOTABLE | NPC_INVINCIBLE;
}

void onspawn_cent_cage(Entity *e) {
	e->alwaysActive = TRUE;
	e->x += pixel_to_sub(8);
	e->y += pixel_to_sub(16);
	// When reused in Balcony, things are a bit different
	if(stageID == 65) {
		e->x += pixel_to_sub(8);
		// Release the allocation
		TILOC_FREE(e->tiloc, e->framesize);
		e->tiloc = NOTILOC;
		// Set sheet to the smaller cage bars
		e->sprite_count = 1;
		SHEET_FIND(e->sheet, SHEET_CAGE);
		e->vramindex = sheets[e->sheet].index;
		e->framesize = 16;
		e->sprite[0] = (Sprite) {
			.size = SPRITE_SIZE(4, 4),
			.attr = TILE_ATTR(PAL2,0,0,0,e->vramindex)
		};
	}
}
