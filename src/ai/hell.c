#include "ai_common.h"

#define BUTE_HP			4
#define MESA_HP			64
#define DELEET_HP		32
#define STATUE_HP		100

void ai_bute_dying(Entity *e) {
	e->x += e->x_speed;
	e->y += e->y_speed;

	switch(e->state) {
		case 0:
		{
			e->eflags &= ~(NPC_SHOOTABLE | NPC_IGNORESOLID | NPC_SHOWDAMAGE);
			e->attack = 0;
			e->frame = 0;
			e->animtime = 0;
			e->state = 1;
			
			e->y_speed = -0x200;
		} /* fallthrough */
		case 1:
		{
			if (blk(e->x, 0, e->y, 8) == 0x41 && e->y_speed >= 0) {
				e->state = 2;
				e->timer = 0;
				e->frame = 1;
			}
		}
		break;
		
		case 2:
		{
			e->x_speed *= 8;
			e->x_speed /= 9;
			
			ANIMATE(e, 3, 1, 2);
			
			if (++e->timer > 50)
				e->state = STATE_DESTROY;
		}
		break;
	}
	
	e->y_speed += 0x20;
	LIMIT_Y(0x5ff);
}

static uint8_t run_bute_defeated(Entity *e, uint16_t hp) {
	if (e->health <= (1000 - hp)) {
		if (e->type == OBJ_MESA) {
			e->type = OBJ_MESA_DYING;
		} else {
			e->x -= (4 << CSF);
			e->y -= (4 << CSF);
			e->type = OBJ_BUTE_DYING;
			
			sound_play(SND_ENEMY_SQUEAK, 5);
			MOVE_X(-0x100);
		}
		
		e->attack = 0;
		e->state = 0;
		e->timer = 0;
		ai_bute_dying(e);
		return 1;
	}
	
	return 0;
}

void ai_bute_flying(Entity *e) {
	if (run_bute_defeated(e, BUTE_HP))
		return;
	
	e->x += e->x_speed;
	e->y += e->y_speed;
	
	switch(e->state) {
		case 0:
		{
			e->hidden = TRUE;
			e->state = 1;
		} /* fallthrough */
		case 1:
		{
			if (e->dir == LEFT) {
				if (player.x > (e->x - (288<<CSF)) && player.x < (e->x - (272<<CSF))) {
					e->state = 10;
				}
			} else {
				if (player.x < (e->x + (288<<CSF)) && player.x > (e->x + (272<<CSF))) {
					e->state = 10;
				}
			}
		}
		break;
		
		case 10:
		{
			e->state = 11;
			e->hidden = FALSE;
			e->eflags |= NPC_SHOOTABLE;
			e->attack = 5;
		} /* fallthrough */
		case 11:
		{
			FACE_PLAYER(e);
			ANIMATE(e, 4, 0,1);
			
			ACCEL_X(0x10);
			e->y_speed += (e->y > player.y) ? -0x10 : 0x10;
			
			LIMIT_X(0x5ff);
			LIMIT_Y(0x5ff);
			
			if ((e->x_speed < 0 && blk(e->x, -6, e->y, 0) == 0x41) || 
				(e->x_speed > 0 && blk(e->x,  6, e->y, 0) == 0x41)) {
				e->x_speed = -e->x_speed;
			}
			if ((e->y_speed > 0 && blk(e->x, 0, e->y,  6) == 0x41) || 
				(e->y_speed < 0 && blk(e->x, 0, e->y, -6) == 0x41)) {
				e->y_speed = -e->y_speed;
			}
		}
		break;
	}
	
}

// Butes that come down from ceiling
void ai_bute_spawner(Entity *e) {
	static const int NUM_BUTES = 8;
	
	switch(e->state) {
		case 10:	// script trigger (dir set by script at same time)
		{
			e->state = 11;
			e->timer = 0;
		} /* fallthrough */
		case 11:
		{
			e->timer++;
			
			if ((e->timer % TIME(50)) == 1) {
				Entity *bute = entity_create(e->x, e->y, OBJ_BUTE_FALLING, 0);
				bute->dir = e->dir;
				
				if (e->timer == ((NUM_BUTES - 1) * TIME(50)) + 1)
					e->state = 0;
			}
		}
		break;
	}
}

void ai_bute_falling(Entity *e) {
	ANIMATE(e, 4, 0,3);
	
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			//e->MoveAtDir(e->dir, 0x600);
			MOVE_X(0x600);
			e->eflags |= NPC_IGNORESOLID;
		} /* fallthrough */
		case 1:
		{
			e->timer++;
			
			if (e->timer == 16) {
				e->eflags &= ~NPC_IGNORESOLID;
			} else if (e->timer > 16 && blk(e->x, 0, e->y, 0) == 0x41) {
				e->state = 10;
			}
			
			if (e->timer > 20) {
				switch(e->dir) {
					case LEFT:
						if (e->x <= player.x + (32<<CSF))
							e->state = 10;
					break;
					
					case RIGHT:
						if (e->x >= player.x - (32<<CSF))
							e->state = 10;
					break;
					
					case UP:
						if (e->y <= player.y + (32<<CSF))
							e->state = 10;
					break;
					
					case DOWN:
						if (e->y >= player.y - (32<<CSF))
							e->state = 10;
					break;
				}
			}
		}
		break;
		
		case 10:
		{
			e->y += (4 << CSF);
			e->type = OBJ_BUTE_FLYING;
			e->state = 10;	// trigger flight immediately
			
			e->frame = 0;
			e->x_speed = 0;
			e->y_speed = 0;
		}
		break;
	}
}

void ai_bute_sword(Entity *e) {
	if (run_bute_defeated(e, BUTE_HP))
		return;
		
	e->x += e->x_speed;
	e->y += e->y_speed;
	
	switch(e->state) {
		case 0:
		{
			e->eflags |= (NPC_SHOOTABLE | NPC_INVINCIBLE);
			//e->nxflags |= NXFLAG_FOLLOW_SLOPE;
			e->attack = 0;
			e->state = 1;
		} /* fallthrough */
		case 1:		// lying in wait
		{
			FACE_PLAYER(e);
			
			if (PLAYER_DIST_X(128<<CSF) && PLAYER_DIST_Y2(128<<CSF, 16<<CSF)) {
				e->state = 10;
			}
		}
		break;
		
		// wait a moment, then start running at player
		case 10:
		{
			e->eflags |= NPC_INVINCIBLE;
			e->attack = 0;
			e->frame = 0;
			
			e->state = 11;
			e->timer = 0;
		} /* fallthrough */
		case 11:
		{
			if (++e->timer > 30) {
				e->state = 20;
				e->timer = 0;
			}
		}
		break;
		
		// run at player and jump
		case 20:
		{
			e->eflags &= ~NPC_INVINCIBLE;
			e->state = 21;
			FACE_PLAYER(e);
		} /* fallthrough */
		case 21:
		{
			ANIMATE(e, 3, 0, 1);
			MOVE_X(0x400);
			
			if (PLAYER_DIST_X(40<<CSF)) {
				e->x_speed /= 2;
				e->y_speed = -0x300;
				
				e->state = 30;
				e->frame = 2;	// sword back, jumping
				sound_play(SND_ENEMY_JUMP, 5);
			} else if (++e->timer > 50) {	// timeout, p got away
				e->state = 10;
				e->x_speed = 0;
			}
		}
		break;
		
		// jumping up
		case 30:
		{
			if (e->y_speed > -0x80) {
				e->frame = 3;	// sword swipe fwd
				e->attack = 9;
				
				e->state = 31;
				e->timer = 0;
			}
		}
		break;
		
		// swiping sword, in air
		case 31:
		{
			if (++e->timer > 2) {
				e->timer = 0;
				e->frame = 4;	// sword down, in front
			}
			
			if (e->y_speed > 0 && blk(e->x, 0, e->y, 8) == 0x41) {
				e->x_speed = 0;
				e->attack = 3;
				
				e->state = 32;
				e->timer = 0;
			}
		}
		break;
		
		case 32:
		{
			if (++e->timer > 30) {
				e->state = 10;
				e->timer = 0;
			}
		}
		break;
	}
	
	e->y_speed += 0x20;
	LIMIT_Y(0x5ff);
}


void ai_bute_archer(Entity *e) {
	if (run_bute_defeated(e, BUTE_HP))
		return;
	
	e->x += e->x_speed;
	e->y += e->y_speed;
	
	switch(e->state) {
		case 0:		// waiting for player (when haven't seen him yet)
		{
			if ((!e->dir && player.x < e->x) || (e->dir && player.x > e->x)) {
				if (PLAYER_DIST_X(320<<CSF) && PLAYER_DIST_Y(160<<CSF)) {
					e->state = 10;
				}
			}
		}
		break;
		
		// aiming--can track player here for a brief period
		case 10:
		{
			FACE_PLAYER(e);
			
			if (!PLAYER_DIST_X(224<<CSF) || player.y <= (e->y - (8<<CSF))) {
				e->frame = 4;	// shooting up
				e->timer2 = 1;
			} else {
				e->frame = 1;	// shooting straight
				e->timer2 = 0;
			}
			
			if (++e->timer > 10) {
				e->state = 20;
				e->timer = 0;
			}
		}
		break;
		
		// flashing to fire
		case 20:
		{
			if (e->timer2 == 0)
				ANIMATE(e, 4, 1,2)
			else
				ANIMATE(e, 4, 4,5)
			
			if (++e->timer > 30)
				e->state = 30;
		}
		break;
		
		// fire
		case 30:
		{
			e->state = 31;
			e->timer = 0;
			
			Entity *arrow = entity_create(e->x, e->y, OBJ_BUTE_ARROW, 0);
			arrow->x_speed = (e->dir) ? 0x600 : -0x600;
			
			if (e->timer2 == 1)		// shooting up
				arrow->y_speed = -0x600;
			
			// frame: arrow away
			e->frame = (e->timer2 == 1) ? 6 : 3;
		} /* fallthrough */
		case 31:
		{
			if (++e->timer > 30) {
				e->state = 40;
				e->frame = 0;
				e->timer = 50 + (random() % 100);
			}
		}
		break;
		
		// after fire, and the "woken up" waiting-for-player state
		case 40:
		{
			if (PLAYER_DIST_X(352<<CSF) && PLAYER_DIST_Y(240<<CSF)) {
				if (e->timer-- == 0)
					e->state = 10;	// fire again
			} else {	// player got away, do nothing until he returns
				e->timer = 150;
			}
		}
		break;
	}
}


void ai_bute_arrow(Entity *e) {
	e->x += e->x_speed;
	e->y += e->y_speed;
	// check for hit wall/floor etc
	if (e->state < 20 && !(e->eflags & NPC_IGNORESOLID)) {
		if ((e->x_speed < 0 && blk(e->x, -6, e->y,  0) == 0x41) || 
			(e->x_speed > 0 && blk(e->x,  6, e->y,  0) == 0x41) ||
			(e->y_speed < 0 && blk(e->x,  0, e->y, -6) == 0x41) || 
			(e->y_speed > 0 && blk(e->x,  0, e->y,  6) == 0x41)) {
				e->state = 20;
		}
	}
	
	switch(e->state) {
		case 0:
		{
			// set appropriate frame for initial direction
			e->dir = e->x_speed > 0;
			e->frame = (e->y_speed < 0) ? 0 : 2;
			e->state = 1;
		} /* fallthrough */
		case 1:
		{
			if (++e->timer == 4)
				e->eflags &= ~NPC_IGNORESOLID;
			
			if (e->timer > 10)
				e->state = 10;
		}
		break;
		
		case 10:
		{
			e->state = 11;
			
			// slow down a bit (was going real fast from bow)
			e->x_speed *= 3;
			e->x_speed /= 4;
			
			e->y_speed *= 3;
			e->y_speed /= 4;
		} /* fallthrough */
		case 11:
		{
			e->y_speed += 0x20;
			
			//ANIMATE_FWD(10);
			if (++e->animtime > 10) {
				e->frame++;
				e->animtime = 0;
			}
			if (e->frame > 4) e->frame = 4;
		}
		break;
		
		case 20:	// hit something
		{
			e->state = 21;
			e->timer = 0;
			e->attack = 0;
			e->x_speed = 0;
			e->y_speed = 0;
		} /* fallthrough */
		case 21:
		{
			e->timer++;
			
			if (e->timer > 30)
				e->hidden = (e->timer & 2);
				
			if (e->timer > 61)
				e->state = STATE_DELETE;
		}
		break;
	}
	
	LIMIT_Y(0x5ff);
}

void ai_mesa(Entity *e) {
	if (run_bute_defeated(e, MESA_HP))
		return;
	
	switch(e->state) {
		case 0:
		{
			e->y += (4<<CSF);
			e->state = 1;
		} /* fallthrough */
		case 1:
		{
			ANIMATE(e, 40, 0, 1);
			FACE_PLAYER(e);
			
			if (PLAYER_DIST_X(320<<CSF) && PLAYER_DIST_Y(160<<CSF)) {
				if (++e->timer > 50)
					e->state = 10;
			}
		}
		break;
		
		case 10:
		{
			e->state = 11;
			e->timer = 0;
			e->frame = 2;	// hand down
			
			int32_t x = e->x + ((!e->dir) ? (7<<CSF) : -(7<<CSF));
			int32_t y = e->y + (10<<CSF);
			
			e->linkedEntity = entity_create(x, y, OBJ_MESA_BLOCK, 0);
			e->linkedEntity->linkedEntity = e;
		} /* fallthrough */
		case 11:
		{
			if (++e->timer > 50) {
				e->state = 20;
				e->timer = 0;
				e->frame = 3;	// hand up, throwing
				
				if (e->linkedEntity) {
					Entity *block = e->linkedEntity;
					
					block->y = (e->y - (4<<CSF));
					block->x_speed = (e->dir) ? 0x400 : -0x400;
					block->y_speed = -0x400;
					block->state = 1;
					
					sound_play(SND_EM_FIRE, 5);
					block->linkedEntity = NULL;
					e->linkedEntity = NULL;
				}
			}
		}
		break;
		
		case 20:
		{
			if (++e->timer > 20) {	// throw again, if player still near
				e->state = 1;
				e->timer = 0;
			}
		}
		break;
	}
	
	e->y_speed += 0x55;
	LIMIT_Y(0x5ff);
}

void ai_mesa_block(Entity *e) {
	e->x += e->x_speed;
	e->y += e->y_speed;
	
	ANIMATE(e, 4, 0,1);
	
	switch(e->state) {
		case 0:		// being held
		{
			if (!e->linkedEntity || e->linkedEntity->type == OBJ_MESA_DYING) {
				e->state = STATE_DELETE;
			}
		}
		break;
		
		case 1:		// launched
		{
			if (++e->timer == 4)
				e->eflags &= ~NPC_IGNORESOLID;
			
			e->y_speed += 0x2A;
			LIMIT_Y(0x5ff);
			
			if (blk(e->x, 0, e->y, 8) == 0x41 && e->y_speed >= 0) {
				sound_play(SND_BLOCK_DESTROY, 5);
				e->state = STATE_DELETE;
			}
		}
		break;
	}
	
	if (e->state == STATE_DELETE) {
		effect_create_smoke(e->x>>CSF, e->y>>CSF);
		//SmokeClouds(o, 3, 0, 0);
		//effect(e->x, e->y, EFFECT_BOOMFLASH);
	}
}

void ai_deleet(Entity *e) {
	// trigger counter
	if (e->health < (1000 - DELEET_HP) && e->state < 2) {
		e->state = 2;
		e->timer = 0;
		e->frame = 2;
		
		e->eflags |= NPC_INVINCIBLE;
		sound_play(SND_CHEST_OPEN, 5);
	}
	
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->x += 8 << CSF;
			e->y += 8 << CSF;
			
			if (!e->dir)
				e->y += (8<<CSF);
			else
				e->x += (8<<CSF);
			
			//e->dir = 0;
		} /* fallthrough */
		case 1:
		{
			if (e->damage_time)
				e->timer2++;
			else
				e->timer2 = 0;
			
			e->frame = (e->timer2 & 2) ? 1 : 0;
		}
		break;
		
		case 2:
		{
			int16_t counter = -1;
			
			switch(e->timer) {
				case 0:		counter = 0; break;	// 5
				case 50:	counter = 1; break;	// 4
				case 100:	counter = 2; break;	// 3
				case 150:	counter = 3; break;	// 2
				case 200:	counter = 4; break;	// 1
				
				case 250:
				{
					e->state = 3;
					//e->sprite = SPR_BBOX_PUPPET_1;
					e->hidden = TRUE;
					
					//sprites[e->sprite].bbox.x1 = -48;
					//sprites[e->sprite].bbox.x2 = 48;
					//sprites[e->sprite].bbox.y1 = -48;
					//sprites[e->sprite].bbox.y2 = 48;
					e->hit_box.left = 48;
					e->hit_box.right = 48;
					e->hit_box.top = 48;
					e->hit_box.bottom = 48;
					e->attack = 12;
					
					camera_shake(10);
					SMOKE_AREA((e->x>>CSF) - 48, (e->y>>CSF) - 48, 96, 96, 10);
					//SmokeXY(e->x, e->y, 40, 48, 48);
					
					e->eflags &= ~NPC_SHOOTABLE;
					e->eflags &= ~NPC_INVINCIBLE;
					
					if (!e->dir) {
						int16_t x = (e->x >> CSF) >> 4;
						int16_t y = ((e->y >> CSF) - 8) >> 4;
						
						stage_replace_block(x, y,   0);
						stage_replace_block(x, y+1, 0);
					} else {
						int16_t x = ((e->x >> CSF) - 8) >> 4;
						int16_t y = (e->y >> CSF) >> 4;
						
						stage_replace_block(x,   y, 0);
						stage_replace_block(x+1, y, 0);
					}
				}
				break;
			}
			
			if (counter != -1) {
				Entity *num = entity_create(e->x, e->y - (8<<CSF), OBJ_COUNTER_BOMB_NUMBER, 0);
				num->frame = counter;
			}
			
			e->timer++;
		}
		break;
		
		case 3:
		{
			// make the explosion sound and drop powerups
			e->state = STATE_DESTROY;
		}
		break;
	}
}

void ai_rolling(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	switch(e->state) {
		case 0:
		{
			// exploding out of blocks (Rollings in Heavy Press area)
			if (blk(e->x, 0, e->y, 0) == 0x41) {
				stage_replace_block(sub_to_block(e->x), sub_to_block(e->y), 0);
			}
			e->state = 1;
		}
		break;
		
		case 1:
		{
			e->x_speed -= 0x40;
			e->y_speed = 0;
			if (collide_stage_leftwall(e)) e->state++;
		}
		break;
		case 2:
		{
			e->y_speed -= 0x40;
			e->x_speed = 0;
			if (collide_stage_ceiling(e)) e->state++;
		}
		break;
		case 3:
		{
			e->x_speed += 0x40;
			e->y_speed = 0;
			if (collide_stage_rightwall(e)) e->state++;
		}
		break;
		case 4:
		{
			e->y_speed += 0x40;
			e->x_speed = 0;
			if (collide_stage_floor(e)) e->state = 1;
		}
		break;
	}
	
	ANIMATE(e, 4, 0,2);
	LIMIT_X(0x400);
	LIMIT_Y(0x400);
	
	e->x = e->x_next;
	e->y = e->y_next;
}

// I don't think it does anything, it seems to be just an invisible marker
// used to bring up the signs when you press DOWN in front of the statues.
// But unlike OBJ_NULL, it can be positioned in-between a tile boundary.
// There's also one on top of the clockroom sign on the Outer Wall.
void ai_statue_base(Entity *e) {
	if (e->state == 0) {
		e->state = 1;
		//e->sprite = SPR_NULL;
		
		if (!e->dir) {
			e->x += (8 << CSF);
		} else {
			e->y += (16 << CSF);
		}
	}
}

// frame is passed in the ANP as if it were a dir, and all of these state numbers can
// also be set by the scripts, so they really shouldn't be messed with.
void ai_statue(Entity *e) {
	switch(e->state) {
		case 0:
		{
			//e->frame = (e->dirparam / 10);
			e->state = 1;
		}
		break;
		
		case 10:
		{
			if (system_get_flag(e->id)) {	// already been destroyed
				e->state = 20;
			} else {
				e->state = 11;
				e->eflags |= NPC_SHOOTABLE;
			}
		} /* fallthrough */
		case 11:
		{
			if (e->health < (1000 - STATUE_HP)) {
				entity_create(e->x, e->y, e->type, 0);
				//ns->dirparam = (e->frame + 4) * 10;
				e->state = STATE_DESTROY;
			}
		}
		break;
		
		case 20:
		{
			e->frame += 4;
			e->state = 1;
		}
		break;
	}
}

void ai_puppy_ghost(Entity *e) {
	e->timer++;
	
	switch(e->state) {
		case 10:	// disappear after talking
		{
			e->state = 11;
			e->timer = 2;
			sound_play(SND_TELEPORT, 5);
		} /* fallthrough */
		case 11:
		{
			e->hidden = (e->timer & 2);
			
			if (e->timer > 50)
				e->state = STATE_DELETE;
		}
		break;
	}
	
	//if ((e->timer % 8) == 1) {
	//	effect(random(e->Left(), e->Right()), e->Bottom(), EFFECT_GHOST_SPARKLE);
	//}
}
