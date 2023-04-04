#include "ai_common.h"

void onspawn_sunstone(Entity *e) {
	e->x += pixel_to_sub(8);
	e->y += pixel_to_sub(8);
}

void onspawn_jenka(Entity *e) {
	if(e->type == OBJ_JENKA_COLLAPSED) {
		e->frame = 2;;
	}
}

void ai_polish(Entity *e) {
	#define POLISH_ACCEL	0x20
	#define POLISH_SPEED	0x200
	#define POLISH_BOUNCE	0x100
	
	#define POLISH_CCW_LEFT		1
	#define POLISH_CCW_UP		2
	#define POLISH_CCW_RIGHT	3
	#define POLISH_CCW_DOWN		4
	
	#define POLISH_CW_LEFT		5
	#define POLISH_CW_UP		6
	#define POLISH_CW_RIGHT		7
	#define POLISH_CW_DOWN		8
	
	// Split after 20 damage
	if(e->health <= 100) {
		entity_create(e->x - (8<<CSF), e->y, OBJ_POLISHBABY, 0)->dir = 0;
		entity_create(e->x + (8<<CSF), e->y, OBJ_POLISHBABY, 0)->dir = 1;
		e->state = STATE_DELETE;
		effect_create_smoke(sub_to_pixel(e->x), sub_to_pixel(e->y));
		sound_play(e->deathSound, 5);
		return;
	}
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	uint8_t blockl = collide_stage_leftwall(e);
	uint8_t blockr = collide_stage_rightwall(e);
	uint8_t blocku = collide_stage_ceiling(e);
	uint8_t blockd = collide_stage_floor(e);

	switch(e->state) {
		case 0:		// initilization
			if(e->flags & NPC_OPTION2) {
				e->state = POLISH_CW_RIGHT;
			} else {
				e->state = POLISH_CCW_LEFT;
			}
		break;
		
		// -------------- Traveling around counter-clockwise --------------
		
		case POLISH_CCW_LEFT:	// traveling left on ceiling
			e->y_speed -= POLISH_ACCEL;
			if (blocku)
			{
				e->y_speed = POLISH_BOUNCE;
				e->x_speed -= POLISH_BOUNCE;
			}
			
			if (blockl) e->state = POLISH_CCW_DOWN;
		break;
		
		case POLISH_CCW_UP:	// traveling up right wall
		{
			e->x_speed += POLISH_ACCEL;
			if (blockr)
			{
				e->x_speed = -POLISH_BOUNCE;
				e->y_speed -= POLISH_BOUNCE;
			}
			
			if (blocku) e->state = POLISH_CCW_LEFT;
		}
		break;
		
		case POLISH_CCW_RIGHT:	// traveling right on floor
		{
			e->y_speed += POLISH_ACCEL;
			if (blockd)
			{
				e->y_speed = -POLISH_BOUNCE;
				e->x_speed += POLISH_BOUNCE;
			}
			
			if (blockr) e->state = POLISH_CCW_UP;
		}
		break;
		
		case POLISH_CCW_DOWN:	// traveling down left wall
		{
			e->x_speed -= POLISH_ACCEL;
			if (blockl)
			{
				e->x_speed = POLISH_BOUNCE;
				e->y_speed += POLISH_BOUNCE;
			}
			
			if (blockd) e->state = POLISH_CCW_RIGHT;
		}
		break;
		
		// -------------- Traveling around clockwise --------------
		
		case POLISH_CW_LEFT:		// traveling left on floor
		{
			e->y_speed += POLISH_ACCEL;
			if (blockd)
			{
				e->y_speed = -POLISH_BOUNCE;
				e->x_speed -= POLISH_BOUNCE;
			}
			
			if (blockl) e->state = POLISH_CW_UP;
		}
		break;
		
		case POLISH_CW_UP:		// traveling up left wall
		{
			e->x_speed -= POLISH_ACCEL;
			if (blockl)
			{
				e->x_speed = POLISH_BOUNCE;
				e->y_speed -= POLISH_BOUNCE;
			}
			
			if (blocku) e->state = POLISH_CW_RIGHT;
		}
		break;
		
		case POLISH_CW_RIGHT:		// traveling right on ceiling
		{
			e->y_speed -= POLISH_ACCEL;
			if (blocku)
			{
				e->y_speed = POLISH_BOUNCE;
				e->x_speed += POLISH_BOUNCE;
			}
			
			if (blockr) e->state = POLISH_CW_DOWN;
		}
		break;
		
		case POLISH_CW_DOWN:		// traveling down right wall
		{
			e->x_speed += POLISH_ACCEL;
			if (blockr)
			{
				e->x_speed = -POLISH_BOUNCE;
				e->y_speed += POLISH_BOUNCE;
			}
			
			if (blockd) e->state = POLISH_CW_LEFT;
		}
		break;
	}
	
	LIMIT_X(POLISH_SPEED);
	LIMIT_Y(POLISH_SPEED);
	
	e->x = e->x_next;
	e->y = e->y_next;
	
	ANIMATE(e, 4, 0,1);
	e->dir = 0;
}

void ai_baby(Entity *e) {
	if(!e->state) {
		e->state = 1;
		if(rand() & 1) {
			e->x_speed = (rand() & 0xFF) + 0x100;
		} else {
			e->x_speed = (rand() & 0xFF) - 0x300;
		}
		if(rand() & 1) {
			e->y_speed = (rand() & 0xFF) + 0x100;
		} else {
			e->y_speed = (rand() & 0xFF) - 0x300;
		}
	}
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	// Collide functions set speed to 0. Remember using mark vars
	e->x_mark = e->x_speed;
	e->y_mark = e->y_speed;
	if (e->x_speed > 0 && collide_stage_rightwall(e)) e->x_speed = -e->x_mark;
	if (e->x_speed < 0 && collide_stage_leftwall(e)) e->x_speed = -e->x_mark;
	if (e->y_speed > 0 && collide_stage_floor(e)) e->y_speed = -e->y_mark;
	if (e->y_speed < 0 && collide_stage_ceiling(e)) e->y_speed = -e->y_mark;
	
	e->x = e->x_next;
	e->y = e->y_next;
	
	ANIMATE(e, 4, 0,1);
}

void ai_sandcroc(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->timer = 0;
			e->x_mark = e->x;
			e->y_mark = e->y;
			//e->eflags &= ~(NPC_SOLID | NPC_SHOOTABLE | NPC_INVINCIBLE | NPC_IGNORESOLID);
			e->flags &= ~(NPC_SOLID | NPC_SHOOTABLE | NPC_INVINCIBLE | NPC_IGNORESOLID);
		} /* fallthrough */
		case 1:
		{
			// track player invisibly underground
			if(e->type != OBJ_SANDCROC_OSIDE) {
				e->x_speed = (e->x < player.x) ? 0x400:-0x400;
				e->x_next = e->x + e->x_speed;
				e->y_next = e->y - 0x200;
				collide_stage_leftwall(e);
				collide_stage_rightwall(e);
				e->x = e->x_next;
			}
			if(PLAYER_DIST_X(e, pixel_to_sub(19))) {
				// check if bottoms of player and croc are near
				if((player.y < e->y + 0x200) && sub_to_pixel(player.y) +
					player.hit_box.bottom + 12 > sub_to_pixel(e->y) + e->hit_box.bottom) {
					// attack!!
					e->x_speed = 0;
					e->state = 2;
					e->timer = 0;
					sound_play(SND_JAWS, 5);
					e->hidden = FALSE;
				}
			} else {
				e->hidden = TRUE;
			}
		}
		break;
		case 2:		// attacking
		{
			e->timer++;
			if(e->timer == 6) {
				e->frame = 1;
			} else if(e->timer == 12) {
				e->attack = (e->type == OBJ_SANDCROC_OSIDE) ? 15 : 10;
				e->frame = 2;
			} else if(e->timer==16) {
				e->flags |= NPC_SHOOTABLE;
				e->flags |= NPC_SOLID;
				e->attack = 0;
				e->state = 3;
				e->timer = 0;
				e->frame = 3;
			}
		}
		break;
		case 3:
		{
			e->timer++;
			if(e->damage_time) {
				e->state = 4;
				e->timer = 0;
				e->y_speed = 0;
				e->damage_time += 25;		// delay floattext until after we're underground
			}
		}
		break;
		case 4:		// retreat
		{
			e->y += 0x280;
			e->flags &= ~(NPC_SOLID);
			
			if (++e->timer == 30) {
				e->flags &= ~(NPC_SHOOTABLE);
				e->hidden = TRUE;
				e->state = 5;
				e->timer = 0;
			}
		}
		break;
		case 5:
		{
			e->frame = 0;
			e->y = e->y_mark;
			
			if(e->timer < 100) {
				e->timer++;
				// have to wait before moving: till floattext goes away
				// else they can see us jump
				if(e->timer==98) {
					e->x_speed = player.x - e->x;
				} else {
					e->x_speed = 0;
				}
			} else {
				e->state = 0;
			}
		}
		break;
	}
}

void ai_sunstone(Entity *e) {
	switch(e->state) {
		case 0:
		{
			// Don't allow bullets through
			e->flags |= (NPC_SHOOTABLE | NPC_INVINCIBLE);
			e->state = 1;
		}
		break;
		case 10:	// triggered to move by hvtrigger script
		{
			// The sunstones push the player to the right here for some reason
			// Disable solid to work around this
			e->flags &= ~(NPC_SOLID|NPC_SPECIALSOLID);
			// Always face left, don't flip the sprite while moving
			MOVE_X(SPEED(0x80));
			e->dir = 0;
			e->timer = 0;
			e->state++;
		} /* fallthrough */
		case 11:
		{
			e->x += e->x_speed;
			if((e->timer & 7) == 0) sound_play(SND_QUAKE, 5);
			e->timer++;
			camera_shake(20);
		}
		break;
	}
}

void ai_armadillo(Entity *e) {
	ANIMATE(e, 16, 0,1);
	if(!e->grounded) e->y_speed += SPEED(0x40);
	LIMIT_Y(SPEED(0x5ff));
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		case 0:
		{
			FACE_PLAYER(e);
			e->state = 1;
		} /* fallthrough */
		case 1:
		{
			if((!e->dir && collide_stage_leftwall(e)) ||
				(e->dir && collide_stage_rightwall(e))) {
				TURN_AROUND(e);
			}
			MOVE_X(SPEED(0x100));
		}
		break;
	}
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_crow(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;

	// for crows carrying skullheads
	if (e->state >= 100) {
		// if our skullhead dies, go into attack mode
		if (!e->linkedEntity) e->state = 2;
	}

	switch(e->state) {
		case 0:
		{
			uint8_t angle = rand();
			e->x_speed = cos[angle];
			e->y_speed = sin[angle];
			e->x_mark = e->x + (e->x_speed << 3);
			e->y_mark = e->y + (e->y_speed << 3);
			e->frame = 0;
			e->state = 1;
		} /* fallthrough */
		case 1:
		case 101:
		{
			if 		(e->x > e->x_mark) e->x_speed -= SPEED_8(16);
			else if (e->x < e->x_mark) e->x_speed += SPEED_8(16);
			if 		(e->y > e->y_mark) e->y_speed -= SPEED_8(16);
			else if (e->y < e->y_mark) e->y_speed += SPEED_8(16);
			
			FACE_PLAYER(e);
			LIMIT_X(SPEED_10(0x200));
			LIMIT_Y(SPEED_10(0x200));

			if (e->damage_time) {
				e->state++;		// state 2/102
				//e->timer = 0;
				if (!e->linkedEntity) MOVE_X(SPEED_10(0x200));
				e->y_speed = 0;
			}
		}
		break;
		case 2:
		case 102:
		{
			FACE_PLAYER(e);
			if (e->damage_time) {
				// fall while hurt
				e->y_speed += SPEED(0x20);
				e->x_speed = 0;
				//e->timer = 0;
			} else {
				if(!e->linkedEntity) {
					// move towards player
					if(e->x < player.x) e->x_speed += SPEED_8(0x10);
					else e->x_speed -= SPEED_8(0x10);
					if(e->y < player.y) e->y_speed += SPEED_8(0x10);
					else e->y_speed -= SPEED_8(0x10);
				} else {
					// carrying a skull; don't chase him
					e->state--; // state 1/101
				}
				//if(!e->timer) e->timer++;
			}
			// bounce off walls
			if (e->x_speed < 0 && collide_stage_leftwall(e)) e->x_speed = SPEED_10(0x200);
			if (e->x_speed > 0 && collide_stage_rightwall(e)) e->x_speed = -SPEED_10(0x200);
			if (e->y_speed < 0 && collide_stage_ceiling(e)) e->y_speed = SPEED_10(0x200);
			if (e->y_speed > 0 && collide_stage_floor(e)) e->y_speed = -SPEED_10(0x200);
			
			LIMIT_X(SPEED_12(0x5ff));
			LIMIT_Y(SPEED_12(0x5ff));
		}
		break;
	}

	if(e->damage_time) e->frame = 2;
	else if((++e->animtime & 3) == 0 && ++e->frame > 1) e->frame = 0;

	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_crow_with_skull(Entity *e) {
	// create the skullhead we're carrying
	Entity *skull = entity_create(e->x, e->y, OBJ_SKULLHEAD, 0);
	skull->linkedEntity = e;
	skull->timer = TIME_8(50);
	e->linkedEntity = skull;
	
	// switch over to the main crow AI, but only move up & down
	e->y_speed = -SPEED_10(0x100 + (rand() & 0xFF));
	e->x_mark = e->x;
	e->y_mark = e->y - (32<<CSF) + (rand() & 0x7FFF);
	e->state = 101;
	e->type = OBJ_CROW;
    e->onFrame = &ai_crow;
	
	// run the ai for the normal crow for this first frame
	ai_crow(e);
}

void ai_skullhead(Entity *e) {
	if(e->linkedEntity) {
		e->x = e->linkedEntity->x;
		e->y = e->linkedEntity->y + 0x2600;
		// Carried by crow
		switch(e->state) {
			case 0:
			{
				e->frame = 0;
				e->state = 101;
			} /* fallthrough */
			case 101:			// mouth closed
			{
				// shoot only when player near
				if ((abs(player.x - e->x) < pixel_to_sub(130)) && (abs(player.y - e->y) < pixel_to_sub(100))) {
					e->timer++;
				} else {
					e->timer = TIME_8(49);
				}
				
				if (e->timer >= TIME_8(50)) {
					FACE_PLAYER(e);
					e->frame = 1;
					e->state = 102;
					e->timer = 0;
				}
			}
			break;
			case 102:			// mouth opened
			{
				e->timer++;
				if (e->timer == TIME_8(30)) {
					Entity *shot = entity_create(e->x, e->y, OBJ_SKELETON_SHOT, 0);
					THROW_AT_TARGET(shot, player.x, player.y, SPEED_10(0x300));
					sound_play(SND_EM_FIRE, 5);
				} else if (e->timer > TIME_8(50)) {
					e->frame = 0;
					e->state = 101;
					e->timer = 0;
				}
			}
			break;
		}
	} else {
		// Normal skullhead
		e->x_next = e->x + e->x_speed;
		e->y_next = e->y + e->y_speed;
		switch(e->state) {
			case 0:
			{
				e->state = 1;
				e->timer = rand() & 3;
			} /* fallthrough */
			case 1:
			{
				if(++e->timer > 8) {
					e->y_speed = -0x350;
					e->state = 2;
					e->frame = 1;
					MOVE_X(0x100);
				} else {
					break;
				}
			} /* fallthrough */
			case 2:
			{
				e->y_speed += 0x40;
				LIMIT_Y(0x5ff);
				if(e->y_speed > 0) {
					if(collide_stage_floor(e)) {
						e->x_speed = 0;
						e->y_speed = 0;
						e->state = 1;
						e->timer = 0;
						e->frame = 0;
					}
				} else {
					collide_stage_ceiling(e);
				}
			}
			break;
			default: // Crow was killed
			{
				e->state = 2;
				MOVE_X(SPEED_10(0x200));
				ai_skullhead(e);
				return;
			}
			break;
		}
		if(e->x_speed) {
			if (collide_stage_leftwall(e)) { e->dir = 1; e->x_speed = 0x100; }
			if (collide_stage_rightwall(e)) { e->dir = 0; e->x_speed = -0x100; }
		}
		e->x = e->x_next;
		e->y = e->y_next;
	}
}

extern void ondeath_default(Entity *e);

void ondeath_crowskull(Entity *e) {
	if(e->linkedEntity) e->linkedEntity->linkedEntity = NULL;
	ondeath_default(e);
}

void ai_skelShot(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	// bounce off walls
	if((e->x_speed < 0 && collide_stage_leftwall(e)) || 
		(e->x_speed > 0 && collide_stage_rightwall(e))) {
		e->x_speed = -e->x_speed;
		e->timer += 5;
	}
	
	// bounce off ceiling
	if(e->y_speed < 0 && collide_stage_ceiling(e)) {
		e->y_speed = -e->y_speed;
		e->timer += 5;
	}
	
	// if hit floor bounce along it...
	if (collide_stage_floor(e)) {
		e->y_speed = -0x180;
		e->state = 1;	// begin falling
		e->timer += 4;
	}
	
	if(e->state == 1) {
		e->y_speed += 0x10;
		LIMIT_Y(0x5ff);
	}
	
	if(e->timer >= 10) {
		e->state = STATE_DELETE;
	}
	
	e->x = e->x_next;
	e->y = e->y_next;
}

// curly's mimiga's
void ai_curlys_mimigas(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		case 0:		// init/set initial anim state
		{
			if(e->type == OBJ_MIMIGAC1) {
				e->state = 100;
			} else { // OBJ_MIMIGAC2 & OBJ_MIMIGA_ENEMY
				e->state = 2;			// stand and blink
				if(e->type == OBJ_MIMIGAC2) {
					if(e->flags & NPC_OPTION2) e->state = 110; // sleeping
				}
			}
		}
		break;
		case 2:		// init stand and blink
		{
			e->state = 3;
		}
		/* fallthrough */
		case 3:		// stand and blink
		{
			e->frame = 0;
			RANDBLINK(e, 3, 200);
		}
		break;
		// sitting mimiga (when facing right)
		// facing away mimiga (when facing left)
		case 100:
		{
			e->frame = 4;
		}
		break;
		case 110:	// sleeping facing left mimiga
		{
			e->frame = 7;
			//ai_zzzz_spawner(o);
		}
		break;
		/// ******************** Fighting Mimiga Code ********************
		case 10:
		{
			e->flags |= NPC_SHOOTABLE;
			e->health = 1000;
			e->state = 11;
			e->timer = rand() & 63;
			e->frame = 0;
		} /* fallthrough */
		case 11:
			if(e->timer) e->timer--;
			else e->state = 13;
		break;
		case 13:
		{
			e->state = 14;
			e->timer = rand() & 63;
			FACE_PLAYER(e);
			e->frame = 1;
		} /* fallthrough */
		case 14:
		{
			ANIMATE(e, 8, 1,0,2,0);
			if(e->dir) e->x_speed += 0x40;
			else e->x_speed -= 0x40;
			
			if(e->timer) {
				e->timer--;
			} else if(e->grounded) {	
				// enter hop state
				e->state = 15;
				e->y_speed = -0x200;
				e->attack = 2;
				e->frame = 2;
			}
		}
		break;
		case 15:	// hopping
		{
			if(e->grounded) {	
				// landed
				e->x_speed = 0;
				e->state = 10;
				e->attack = 0;
			}
		}
		break;
		case 20:	// aiiie! got shot!!
		{
			if(e->grounded) {
				e->x_speed = 0;
				e->state = 21;
				if(e->frame < 7) e->frame += 2;
				e->timer = 280 + (rand() & 127);
			}
		}
		break;
		case 21:	// lying on ground knocked out
		{
			if (e->timer) {
				e->timer--;
			} else {
				e->flags |= NPC_SHOOTABLE;
				e->health = 1000;
				e->state = 11;
				e->timer = rand() & 63;
				e->frame = 0;
			}
		}
		break;
	}
	if (e->state > 10 && e->state < 20 && e->health != 1000) {
		// got shot by player
		e->state = 20;
		e->y_speed = -0x200;
		e->frame = (rand() & 1) + 5;
		e->attack = 0;
		e->flags &= ~NPC_SHOOTABLE;
	}
	
	if(!e->grounded) e->y_speed += SPEED(0x20);
	LIMIT_Y(SPEED(0x5ff));
	LIMIT_X(SPEED(0x1ff));
	collide_stage_leftwall(e);
	collide_stage_rightwall(e);
	if(e->grounded) e->grounded = collide_stage_floor_grounded(e);
	else e->grounded = collide_stage_floor(e);
	
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_skeleton_shot(Entity *e) {
	//Bullet *b = bullet_colliding(e);
	//if(b) {
		//b->ttl = 0;
		//e->state = STATE_DELETE;
		//return;
	//	entity_handle_bullet(e, b);
	//}
	
	ANIMATE(e, 8, 0,1,2,3);
	
	if ((e->x_speed < 0 && blk(e->x, -6, e->y, 0) == 0x41) || 
			(e->x_speed > 0 && blk(e->x, 6, e->y, 0) == 0x41)) {
		e->x_speed = -e->x_speed;
		e->timer += 5;
	}
	
	if ((e->y_speed < 0 && blk(e->x, 0, e->y, -6) == 0x41)) {
		e->y_speed = -e->y_speed;
		e->timer += 5;
	}
	
	// Have to collide with slopes, so use the normal method
	e->x_next = e->x;
	e->y_next = e->y;
	if (collide_stage_floor(e)) {
		e->y_speed = -SPEED(0x180);
		e->state = 1;	// begin falling
		e->timer += 4;
	}
	
	if (e->state == 1) {
		e->y_speed += SPEED(0x10);
		LIMIT_Y(SPEED(0x5ff));
	}
	
	e->x += e->x_speed;
	e->y += e->y_speed;
	
	if (e->timer >= 10) {
		//effect(e->CenterX(), e->CenterY(), EFFECT_FISHY);
		e->state = STATE_DELETE;
	}
}

#define SKNEAR_BELOW	pixel_to_sub(160)
#define SKNEAR_ABOVE	pixel_to_sub(64)

void ai_skeleton(Entity *e) {
	uint8_t pnear = PLAYER_DIST_Y2(e, SKNEAR_ABOVE, SKNEAR_BELOW);
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);
	
	switch(e->state) {
		case 0:
		{
			e->frame = e->grounded;
			e->x_speed = 0;
			
			// if player comes near, enter attack state
			// also if he shoots us while we're inactive, don't just stand there
			// like a ninny, jump around!
			if (e->damage_time) e->state = 20;
			if (pnear) e->state = 10;
		}
		break;
		
		case 10:	// prepare for jump
			e->timer = e->timer2 = e->frame = e->x_speed = 0;
			e->state = 11;
			/* fallthrough */
		case 11:
			if (++e->timer > 5 && e->grounded) {
				// jump if player near, else go to idle
				e->state = pnear ? 20:0;
			}
		if (e->state != 20) break;
		/* fallthrough */
		case 20:
		{
			e->state = 21;
			e->frame = 1;
			e->timer2 = 0;
			e->y_speed = -(SPEED_10(0x200) + SPEED_10(rand() & 0x3FF));
			e->grounded = FALSE;
			
			// jump towards player, unless we've been hurt; in that case jump away
			if (!e->damage_time) {
				e->x_speed += (e->x_next > player.x) ? -SPEED(0x100):SPEED(0x100);
			} else {
				e->x_speed += (e->x_next > player.x) ? SPEED(0x100):-SPEED(0x100);
			}
		} /* fallthrough */
		case 21:
		{
			if (e->y_speed > 0) {
				if (e->timer2 == 0) {
					e->timer2++;
					
					FIRE_ANGLED_SHOT(OBJ_SKELETON_SHOT, e->x, e->y, 
							e->dir ? A_RIGHT+1:A_LEFT-1, 0x300);
					sound_play(SND_EM_FIRE, 3);
				}
				
				
			} else collide_stage_ceiling(e);
			if (e->grounded) {
				e->state = 10;
				e->frame = 0;
			}
		}
		break;
	}
	
	if (e->state >= 10) FACE_PLAYER(e);
	if(!e->grounded) e->y_speed += SPEED(0x33);
	LIMIT_X(SPEED(0x5ff));
	LIMIT_Y(SPEED(0x5ff));
	
	e->x = e->x_next;
	e->y = e->y_next;
}

// Nothing but dogs below

void onspawn_puppy(Entity *e) {
	e->flags |= NPC_INTERACTIVE; // Yeah..
}

void onspawn_puppyCarry(Entity *e) {
	e->alwaysActive = TRUE;
	e->x_mark = camera.x;
	e->y_mark = camera.y;
	// One's all you can manage. One's all you can manage. One's all you can manage.
	//e->eflags &= ~NPC_INTERACTIVE;
	e->flags &= ~NPC_INTERACTIVE;
}

void ai_puppyCarry(Entity *e) {
	e->dir = player.dir;
	e->x = player.x + ((e->dir ? -4 : 4) << CSF);
	e->y = player.y - (6 << CSF);
	// Keep dog in front of doors
	if(abs(e->x_mark - camera.x) > ScreenHalfW || abs(e->y_mark - camera.y) > ScreenHalfH) {
		moveMeToFront = TRUE;
		e->x_mark = camera.x;
		e->y_mark = camera.y;
	}
}

// these seem to be used for the the ones in jenka's house
// that you have already gotten.
void ai_puppy_wag(Entity *e) {
	// code shared with talking item-giving puppy from Plantation--
	// that one doesn't face you.
	if (e->type != OBJ_PUPPY_ITEMS) FACE_PLAYER(e);
	
	// needed so you can talk to them immediately after giving them to jenka
	e->flags |= NPC_INTERACTIVE;
	
	// only wag when player is near
	if (PLAYER_DIST_X(e, 56 << CSF)) {
		if (++e->animtime >= 4) {
			e->animtime = 0;
			e->frame ^= 1;
		}
	} else {
		e->frame = 0;
		e->animtime = 0;
	}
	
	RANDBLINK(e, 2, 200);
}

#define BARK	5
#define NOBARK	3
void ai_puppy_bark(Entity *e) {
	if (e->state < 100) FACE_PLAYER(e);
	
	switch(e->state) {
		case 0:
		case 100:
			e->state++;
			e->timer2 = 0;
			/* fallthrough */
		case 1:
		case 101:
			// bark when player is near
			// note: this is also supposed to run at jenka's house when balrog appears
			// but it's ok:
			// the player is always near enough because of the way the cutscene is set up
			if (PLAYER_DIST_X(e, 64 << CSF) && PLAYER_DIST_Y(e, 16 << CSF)) {
				if (++e->animtime > 6) {
					e->animtime = 0;
					
					if (e->frame==NOBARK) {
						e->frame = BARK;
						sound_play(SND_PUPPY_BARK, 5);
					} else {
						e->frame = NOBARK;
						
						if (!(rand() & 7) || ++e->timer2 > 5) {
							// stop barking for a sec
							e->state++;
							e->timer = e->timer2 = 0;
							e->frame = 0;
						}
					}
				}
			} else {
				e->frame = 0;
				e->animtime = 99;		// begin barking as SOON as player gets near
			}
		break;
		
		case 2:
		case 102:
			e->frame = 0;
			if (++e->timer > 8) {	// start barking again
				e->state--;
				e->animtime = 0;
				e->timer2 = 0;
			}
		break;
		
		// do not bark at all--set during jenka's 1st cutscene with balrog
		case 20:
		case 120:
			e->frame = 0;
		break;
	}
}

void ai_puppy_run(Entity *e) {
	switch(e->state) {
		case 0:
		{
			FACE_PLAYER(e);
			e->frame = 0;		// necessary for randblink
			
			if (PLAYER_DIST_Y2(e, (32 << CSF), (16 << CSF))) {
				if (PLAYER_DIST_X(e, 32 << CSF)) {	// run away!!!
					FACE_PLAYER(e); TURN_AROUND(e);
					e->state = 10;
					e->animtime = 0;
				} else if (PLAYER_DIST_X(e, pixel_to_sub(96))) {
					// wag tail
					if (++e->animtime >= 4) {
						e->animtime = 0;
						e->frame ^= 1;
					}
				}
			}

            RANDBLINK(e, 2, 200);
		}
		break;
		
		case 10:		// running
		{
			ANIMATE(e, 8, 4,3);
			
			e->x_next = e->x + e->x_speed;
			e->y_next = e->y + e->y_speed;
			
			if(!e->grounded) e->grounded = collide_stage_floor(e);
			else e->grounded = collide_stage_floor_grounded(e);
			
			// "bounce" off walls
			if (e->dir) {
				if (collide_stage_rightwall(e)) {
					e->x_speed = -SPEED(0x400);
					e->dir = 0;
				}
			} else {
				if (collide_stage_leftwall(e)) {
					e->x_speed = SPEED(0x400);
					e->dir = 1;
				}
			}
			
			e->x = e->x_next;
			e->y = e->y_next;
			
			e->x_speed += e->dir ? SPEED(0x40) : -SPEED(0x40);
			if(!e->grounded) e->y_speed += SPEED(0x40);
			
			LIMIT_X(SPEED(0x400));
			LIMIT_Y(SPEED(0x5FF));
		}
		break;
	}
}
