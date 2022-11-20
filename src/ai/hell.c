#include "ai_common.h"

#define BUTE_HP			4
#define MESA_HP			64
#define DELEET_HP		32
#define STATUE_HP		100

enum ButeFrame {
	BF_FLYING1, BF_FLYING2,
	BF_SWORD1, BF_SWORD2, BF_SWORD3, BF_SWORD4, BF_SWORD5,
	BF_ARCHER1, BF_ARCHER2, BF_ARCHER3, BF_ARCHER4, BF_ARCHER5, BF_ARCHER6, BF_ARCHER7,
	BF_FALLING1, BF_FALLING2
};

enum MesaFrame {
	MS_STAND1, MS_STAND2,
	MS_THROW1, MS_THROW2,
	MS_DYING1, MS_DYING2, MS_DYING3
};

void ai_bute_dying(Entity *e) {
	e->x += e->x_speed;
	e->y += e->y_speed;
	
	uint16_t baseframe = (e->type == OBJ_MESA_DYING) ? MS_DYING1 : 0;

	switch(e->state) {
		case 0:
		{
			//e->eflags &= ~(NPC_SHOOTABLE | NPC_IGNORESOLID | NPC_SHOWDAMAGE);
			e->flags &= ~(NPC_SHOOTABLE | NPC_IGNORESOLID | NPC_SHOWDAMAGE);
			e->attack = 0;
			e->frame = baseframe;
			e->animtime = 0;
			e->state = 1;
			
			e->y_speed = -SPEED_10(0x1FF);
		} /* fallthrough */
		case 1:
		{
			e->y_speed += SPEED_8(0x20);
			LIMIT_Y(SPEED_12(0x5ff));
			
			if (blk(e->x, 0, e->y, 8) == 0x41 && e->y_speed >= 0) {
				e->state = 2;
				e->timer = 0;
				e->frame = baseframe + 1;
				e->y_speed = 0;
			}
		}
		break;
		
		case 2:
		{
			e->x_speed -= (e->x_speed >> 3) + 1;
			
			if (++e->timer > TIME_8(50)) {
				e->state = STATE_DESTROY;
			} else {
				if((e->timer & 15) == 1) e->frame = baseframe + 2;
				if((e->timer & 15) == 9) e->frame = baseframe + 1;
			}
		}
		break;
	}
	
}

static uint8_t run_bute_defeated(Entity *e, uint16_t hp) {
	if (e->health <= (1000 - hp)) {
		if (e->type == OBJ_MESA) {
			e->type = OBJ_MESA_DYING;
            e->onFrame = npc_info[OBJ_MESA_DYING].onFrame;
		} else {
			e->x -= (4 << CSF);
			e->y -= (4 << CSF);
			e->type = OBJ_BUTE_DYING;
            e->onFrame = npc_info[OBJ_BUTE_DYING].onFrame;
			
			sound_play(SND_ENEMY_SQUEAK, 5);
			MOVE_X(-SPEED_8(0xFF));

			e->vramindex += 16*6;
			e->frame = 0;
			e->oframe = 255;
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
	if (run_bute_defeated(e, BUTE_HP)) return;
	
	if(e->state != 11) {
		e->state = 11;
		e->hidden = FALSE;
		e->flags |= NPC_SHOOTABLE;
		e->attack = 5;
		e->frame = BF_FLYING1;
	}

	if(++e->animtime & 1) {
		FACE_PLAYER(e);
		if(e->dir) {
			if(e->x_speed <  SPEED_10(0x3E0)) e->x_speed += SPEED_8(0x20);
		} else {
			if(e->x_speed > -SPEED_10(0x3E0)) e->x_speed -= SPEED_8(0x20);
		}
		if ((e->x_speed < 0 && blk(e->x, -8, e->y, 0) == 0x41) || 
			(e->x_speed > 0 && blk(e->x,  8, e->y, 0) == 0x41)) {
			e->x_speed = -e->x_speed;
		}
	} else {
		if((e->animtime & 2) && ++e->frame > BF_FLYING2) e->frame = BF_FLYING1;
		if(e->y < player.y) {
			if(e->y_speed <  SPEED_10(0x3E0)) e->y_speed += SPEED_8(0x20);
		} else {
			if(e->y_speed > -SPEED_10(0x3E0)) e->y_speed -= SPEED_8(0x20);
		}
		if ((e->y_speed > 0 && blk(e->x, 0, e->y,  8) == 0x41) || 
			(e->y_speed < 0 && blk(e->x, 0, e->y, -8) == 0x41)) {
			e->y_speed = -e->y_speed;
		}
	}

	e->x += e->x_speed;
	e->y += e->y_speed;
}

void onspawn_bute_spawner(Entity *e) {
	if(e->event == 250 || e->event == 254) {
		e->flags = NPC_OPTION1;
	} else {
		e->flags = 0;
	}
}

// Butes that come down from ceiling
void ai_bute_spawner(Entity *e) {
	static const int NUM_BUTES = 8;

	switch(e->state) {
		case 10:	// script trigger (dir set by script at same time)
		{
			e->state = 11;
			e->timer2 = 0;
		} /* fallthrough */
		case 11:
		{
			e->timer = 0;
			if(entity_active_count >= 30) {
				e->state = 12;
				break;
			}
			e->timer2++;
			
			Entity *bute = entity_create(e->x, e->y, OBJ_BUTE_FALLING, e->flags);
			bute->dir = e->dir & 1;
			if(e->dir == UP) e->flags |= NPC_OPTION2;
				
			if(e->timer2 >= (uint16_t) NUM_BUTES) {
				e->state = STATE_DELETE;
			} else {
				e->state = 12;
			}
		}
		break;
		case 12:
		{
			if(++e->timer >= TIME_8(50)) e->state = 11;
		}
		break;
	}
}

void onspawn_bute_falling(Entity *e) {
	e->frame = BF_FALLING1;
	if(e->flags & NPC_OPTION1) {
		e->y_speed = -SPEED_12(0x600);
	} else {
		e->y_speed = SPEED_12(0x600);
	}
}

void ai_bute_falling(Entity *e) {
	e->y += e->y_speed;

	e->timer++;
	if((e->timer & 3) == 0 && ++e->frame > BF_FALLING2) e->frame = BF_FALLING1;
	if(e->timer > TIME_8(20)) {
		if((e->y_speed > 0 && e->y >= player.y - (32<<CSF)) ||
				(e->y_speed < 0 && e->y >= player.y + (32<<CSF)) ||
				blk(e->x, e->dir ? 6 : -6, e->y, e->y_speed > 0 ? 6 : -6) == 0x41) {
			FACE_PLAYER(e);
			e->type = OBJ_BUTE_FLYING;
            e->onFrame = npc_info[OBJ_BUTE_FLYING].onFrame;
			e->state = 10;	// trigger flight immediately
			e->frame = 0;
			e->x_speed = 0;
			e->y_speed = 0;
		}
	}
}

void ai_bute_sword(Entity *e) {
	if (run_bute_defeated(e, BUTE_HP)) return;
	
	if(!e->grounded) {
		if(e->y_speed < SPEED_12(0x5E0)) e->y_speed += SPEED_8(0x20);
		// Check if we hit the ground when falling
		if(e->y_speed > 0) {
			e->x_next = e->x;
			e->y_next = e->y;
			e->grounded = collide_stage_floor(e);
			e->y = e->y_next;
		}
	} else if(e->x_speed) {
		// Need to follow slopes and fall off ledges
		e->x_next = e->x;
		e->y_next = e->y;
		e->grounded = collide_stage_floor_grounded(e);
		e->y = e->y_next;
	}
	// Wall collision
	if(e->x_speed < 0 && blk(e->x, -8, e->y, 0) == 0x41) {
		e->x_speed = 0;
	} else if(e->x_speed > 0 && blk(e->x, 8, e->y, 0) == 0x41) {
		e->x_speed = 0;
	}
	e->x += e->x_speed;
	e->y += e->y_speed;
	
	switch(e->state) {
		case 0:
		{
			e->flags |= (NPC_SHOOTABLE | NPC_INVINCIBLE);
			e->y -= 4 << CSF;
			e->grounded = FALSE;
			e->attack = 0;
			e->state = 1;
			e->frame = BF_SWORD1;
		} /* fallthrough */
		case 1:		// lying in wait
		{
			if (PLAYER_DIST_X(e, pixel_to_sub(128)) && PLAYER_DIST_Y2(e, pixel_to_sub(128), 16<<CSF)) {
				FACE_PLAYER(e);
				e->state = 10;
			}
		}
		break;
		
		// wait a moment, then start running at player
		case 10:
		{
			e->flags |= NPC_INVINCIBLE;
			e->attack = 0;
			e->frame = BF_SWORD1;
			
			e->state = 11;
			e->timer = 0;
		} /* fallthrough */
		case 11:
		{
			if(++e->timer > TIME_8(30)) {
				e->state = 20;
				e->timer = 0;
			}
		}
		break;
		
		// run at player and jump
		case 20:
		{
			e->flags &= ~NPC_INVINCIBLE;
			e->state = 21;
			e->frame = BF_SWORD1;
			FACE_PLAYER(e);
		} /* fallthrough */
		case 21:
		{
			if((++e->animtime & 3) == 0 && ++e->frame > BF_SWORD2) e->frame = BF_SWORD1;
			MOVE_X(SPEED_10(0x3FF));
			
			if(PLAYER_DIST_X(e, 40<<CSF)) {
				e->x_speed >>= 1;
				e->y_speed = -SPEED_10(0x2FF);
				e->grounded = FALSE;
				
				e->state = 30;
				e->frame = BF_SWORD3;	// sword back, jumping
				sound_play(SND_ENEMY_JUMP, 5);
			} else if(++e->timer > TIME_8(50)) {	// timeout, p got away
				e->state = 10;
				e->x_speed = 0;
			}
		}
		break;
		
		// jumping up
		case 30:
		{
			if (e->y_speed > -SPEED_8(0x80)) {
				e->frame = BF_SWORD4;	// sword swipe fwd
				e->attack = 9;
				
				e->state = 31;
				e->timer = 0;
			}
		}
		break;
		
		// swiping sword, in air
		case 31:
		{
			if (++e->timer > 3) {
				e->timer = 0;
				e->frame = BF_SWORD5;	// sword down, in front
			}
			
			if (e->grounded) {
				e->x_speed = 0;
				e->y_speed = 0;
				e->attack = 3;
				
				e->state = 32;
				e->timer = 0;
			}
		}
		break;
		
		case 32:
		{
			if (++e->timer > TIME_8(30)) {
				e->state = 10;
				e->timer = 0;
			}
		}
		break;
	}
}


void ai_bute_archer(Entity *e) {
	if (run_bute_defeated(e, BUTE_HP)) return;
	
	switch(e->state) {
		case 0:		// waiting for player (when haven't seen him yet)
		{
			if(e->flags & NPC_OPTION2) e->dir = 1;
			e->y += 4<<CSF;
			e->frame = BF_ARCHER1;
			e->state++;
		} /* fallthrough */
		case 1:
		{
			if((!e->dir && player.x < e->x) || (e->dir && player.x > e->x)) e->state = 2;
		}
		break;
		
		// aiming--can track player here for a brief period
		case 2:
		{
			if(!PLAYER_DIST_X(e, pixel_to_sub(224)) || player.y <= (e->y - (8<<CSF))) {
				e->frame = BF_ARCHER5;	// shooting up
				e->timer2 = 1;
			} else {
				e->frame = BF_ARCHER2;	// shooting straight
				e->timer2 = 0;
			}
			
			if (++e->timer > TIME_8(10)) {
				e->state++;
				e->timer = 0;
			}
		}
		break;
		
		// flashing to fire
		case 3:
		{
			e->frame = e->timer2 ? BF_ARCHER5 : BF_ARCHER2;
			if(++e->timer > TIME_8(30)) {
				e->state++;
			} else if(e->timer & 2) {
				e->frame++;
			}
		}
		break;
		
		// fire
		case 4:
		{
			e->state++;
			e->timer = 0;
			
			Entity *arrow = entity_create(e->x, e->y, OBJ_BUTE_ARROW, 0);
			arrow->x_speed = (e->dir) ? SPEED_12(0x5FF) : -SPEED_12(0x5FF);
			
			if(e->timer2) arrow->y_speed = -SPEED_12(0x5FF);
			// frame: arrow away
			e->frame = e->timer2 ? BF_ARCHER7 : BF_ARCHER4;
		} /* fallthrough */
		case 5:
		{
			if (++e->timer > TIME_8(30)) {
				e->state++;
				e->frame = BF_ARCHER1;
				e->timer = TIME_8(50) + (rand() & 127);
			}
		}
		break;
		
		// after fire, and the "woken up" waiting-for-player state
		case 6:
		{
			if(--e->timer == 0) {
				e->state = 2;	// fire again
				FACE_PLAYER(e);
			}
		}
		break;
	}
}


void ai_bute_arrow(Entity *e) {
	switch(e->state) {
		case 0:
		{	// set appropriate frame for initial direction
			e->dir = e->x_speed > 0;
			e->frame = (e->y_speed < 0) ? 0 : 2;
			e->state = 1;
		} /* fallthrough */
		case 1:
		{
			if(++e->timer > TIME_8(10)) {
				e->state = 10;
				// slow down a bit (was going real fast from bow)
				e->x_speed -= e->x_speed >> 2;
				e->y_speed -= e->y_speed >> 2;
			}
		}
		break;
		
		case 10:
		{
			if(e->y_speed < SPEED_12(0x5E0)) e->y_speed += SPEED_8(0x20);
			if((e->frame < 4) && ++e->animtime > TIME_8(10)) {
				e->frame++;
				e->animtime = 0;
			}
			if(blk(e->x, 0, e->y, 0) == 0x41) {
				e->state = 20;
				e->timer = 0;
				e->attack = 0;
				e->x_speed = 0;
				e->y_speed = 0;
			}
		}
		break;
		
		case 20:	// hit something
		{
			e->hidden = (++e->timer & 2);
			if(e->timer > TIME_8(30)) e->state = STATE_DELETE;
		}
		break;
	}

	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_mesa(Entity *e) {
	if (run_bute_defeated(e, MESA_HP)) return;
	
	switch(e->state) {
		case 0:
		{
			e->flags &= ~(NPC_SOLID | NPC_SPECIALSOLID);
			e->y += (4<<CSF);
			e->frame = MS_STAND1;
			e->state++;
		} /* fallthrough */
		case 1:
		{
			if (++e->timer > TIME_8(50)) e->state++;
			else if((++e->animtime & 31) == 0 && ++e->frame > MS_STAND2) e->frame = MS_STAND1;
		}
		break;
		case 2:
		{
			e->state++;
			e->timer = 0;
			e->frame = MS_THROW1;	// hand down
			FACE_PLAYER(e);
			
			int32_t x = e->x + (e->dir ? -(7<<CSF) : (7<<CSF));
			int32_t y = e->y + (2<<CSF);
			
			e->linkedEntity = entity_create(x, y, OBJ_MESA_BLOCK, 0);
			e->linkedEntity->linkedEntity = e;
		} /* fallthrough */
		case 3:
		{
			if (++e->timer > TIME_8(50)) {
				e->state++;
				e->timer = 0;
				e->frame = MS_THROW2;	// hand up, throwing
				
				if (e->linkedEntity) {
					Entity *block = e->linkedEntity;
					
					block->y = e->y - 0x800;
					block->x_speed = e->dir ? SPEED_10(0x3FF) : -SPEED_10(0x3FF);
					block->y_speed = -SPEED_10(0x3FF);
					block->state = 1;
					
					sound_play(SND_EM_FIRE, 5);
					block->linkedEntity = NULL;
					e->linkedEntity = NULL;
				}
			}
		}
		break;
		case 4:
		{
			if (++e->timer > TIME_8(20)) {	// throw again, if player still near
				e->state = 1;
				e->timer = 0;
				e->frame = MS_STAND1;
			}
		}
		break;
	}
}

void ai_mesa_block(Entity *e) {
	switch(e->state) {
		case 0:		// being held
		{
			if (!e->linkedEntity || e->linkedEntity->type == OBJ_MESA_DYING) {
				e->state = STATE_DELETE;
				effect_create_smoke(e->x>>CSF, e->y>>CSF);
			}
		}
		break;
		
		case 1:		// launched
		{
			e->y_speed += SPEED_8(0x2A);
			LIMIT_Y(SPEED_12(0x5ff));
			
			if (blk(e->x, 0, e->y, 8) == 0x41 && e->y_speed >= 0) {
				sound_play(SND_BLOCK_DESTROY, 5);
				effect_create_smoke(e->x>>CSF, e->y>>CSF);
				e->state = STATE_DELETE;
			}
		}
		break;
	}
	if((++e->animtime & 3) == 0) e->frame ^= 1; 
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void onspawn_deleet(Entity *e) {
	if (!(e->flags & NPC_OPTION2)) {
		int16_t x = (e->x >> CSF) >> 4;
		int16_t y = ((e->y >> CSF) - 8) >> 4;
		
		stage_replace_block(x, y,   1);
		stage_replace_block(x, y+1, 1);
	} else {
		int16_t x = ((e->x >> CSF) - 8) >> 4;
		int16_t y = (e->y >> CSF) >> 4;
		
		stage_replace_block(x,   y, 1);
		stage_replace_block(x+1, y, 1);
	}
}

void ai_deleet(Entity *e) {
	// Draw above the wall
	sprite_pri(&e->sprite[0], 1);

	// trigger counter
	if (e->health < (1000 - DELEET_HP) && e->state < 2) {
		e->state = 2;
		e->timer = 0;
		e->frame = 2;
		e->alwaysActive = TRUE;
		e->flags |= NPC_INVINCIBLE;
		sound_play(SND_CHEST_OPEN, 5);
	}
	
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			if (!(e->flags & NPC_OPTION2)) e->y += (8<<CSF);
			else e->x += (8<<CSF);
		} /* fallthrough */
		case 1:
		{
			if (e->damage_time) e->timer2++;
			else e->timer2 = 0;
			
			e->frame = (e->timer2 & 2) ? 1 : 0;
		}
		break;
		
		case 2:
		{
			int16_t counter = -1;
			
			if(e->timer == 0) counter = 0;
			if(e->timer == TIME_8(50)) counter = 1;
			if(e->timer == TIME_8(100)) counter = 2;
			if(e->timer == TIME_8(150)) counter = 3;
			if(e->timer == TIME_8(200)) counter = 4;
			
			if(e->timer >= TIME_8(250)) {
				e->state = 3;
				e->hidden = TRUE;
				
				e->hit_box.left = 48;
				e->hit_box.right = 48;
				e->hit_box.top = 48;
				e->hit_box.bottom = 48;
				e->attack = 12;
				
				camera_shake(10);
				SMOKE_AREA((e->x>>CSF) - 48, (e->y>>CSF) - 48, 96, 96, 10);
				
				e->flags &= ~NPC_SHOOTABLE;
				
				if (!(e->flags & NPC_OPTION2)) {
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
			
			if (counter != -1) {
				Entity *num = entity_create(e->x + (8<<CSF), e->y, OBJ_COUNTER_BOMB_NUMBER, 0);
				num->frame = counter;
			}
			
			e->timer++;
		}
		break;
		
		case 3:
		{	// make the explosion sound and drop powerups
			e->state = STATE_DESTROY;
		}
		break;
	}
}

void ai_rolling(Entity *e) {
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
			if(e->x_speed > -SPEED_10(0x3C0)) e->x_speed -= SPEED_8(0x40);
			if(blk(e->x, -8, e->y, 0) == 0x41) {
				e->x_speed = 0;
				e->state++;
			}
		}
		break;
		case 2:
		{
			if(e->y_speed > -SPEED_10(0x3C0)) e->y_speed -= SPEED_8(0x40);
			if(blk(e->x, 0, e->y, -8) == 0x41) {
				e->y_speed = 0;
				e->state++;
			}
		}
		break;
		case 3:
		{
			if(e->x_speed < SPEED_10(0x3C0)) e->x_speed += SPEED_8(0x40);
			if(blk(e->x, 8, e->y, 0) == 0x41) {
				e->x_speed = 0;
				e->state++;
			}
		}
		break;
		case 4:
		{
			if(e->y_speed < SPEED_10(0x3C0)) e->y_speed += SPEED_8(0x40);
			if(blk(e->x, 0, e->y, 8) == 0x41) {
				e->y_speed = 0;
				e->state = 1;
			}
		}
		break;
	}
	
	if((++e->animtime & 3) == 0 && ++e->frame > 2) e->frame = 0;
	
	e->x += e->x_speed;
	e->y += e->y_speed;
}

// I don't think it does anything, it seems to be just an invisible marker
// used to bring up the signs when you press DOWN in front of the statues.
// But unlike OBJ_NULL, it can be positioned in-between a tile boundary.
// There's also one on top of the clockroom sign on the Outer Wall.
void onspawn_statue_base(Entity *e) {
	if(e->flags & NPC_OPTION2) {
		e->y += (16 << CSF);
	} else {
		e->x += (8 << CSF);
	}
}

void onspawn_statue(Entity *e) {
	e->alwaysActive = TRUE;
	e->x += 8 << CSF;
	e->y += 12 << CSF;
	// Another object with a weird dir parameter to ANP
	// We use the ID instead
	switch(e->id) {
		case 1590: e->frame = 3; break;
		case 1591: e->frame = 2; break;
		case 1592: e->frame = 1; break;
		case 1593: e->frame = 0; break;
	}
	if(system_get_flag(e->id)) e->frame += 4;
}

void ai_statue(Entity *e) {
	switch(e->state) {
		case 10:
		{
			e->dir = 0;
			if(e->frame > 3) {	// already been destroyed
				e->state = 0;
			} else {
				e->state = 11;
				e->flags |= NPC_SHOOTABLE;
			}
		} /* fallthrough */
		case 11:
		{
			if(e->health < (1000 - STATUE_HP)) {
				SMOKE_AREA((e->x >> CSF) - 12, (e->y >> CSF) - 12, 24, 24, 4);
				sound_play(SND_BLOCK_DESTROY, 5);
				entity_drop_powerup(e);
				system_set_flag(e->id, TRUE);
				e->flags &= ~NPC_SHOOTABLE;
				e->frame += 4;
				e->state = 0;
			}
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
			
			if (e->timer > TIME_8(50)) e->state = STATE_DELETE;
		}
		break;
	}
	
	//if ((e->timer & 7) == 1) {
	//	effect(rand(e->Left(), e->Right()), e->Bottom(), EFFECT_GHOST_SPARKLE);
	//}
}
