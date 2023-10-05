#include "ai_common.h"
extern uint8_t windowOpen;

#define savedhp		id
#define timer3		jump_time

enum STATES
{
	BP_FIGHTING_STANCE		= 100,		// show fighting stance, then prepare to fly lr
	
	BP_PREPARE_FLY_LR		= 110,		// duck a moment, then fly horizontally at player
	BP_PREPARE_FLY_UD		= 120,		// duck a moment, then fly vertically at player
	
	BP_FLY_LR				= 130,		// flying horizontally
	BP_FLY_UP				= 140,		// flying up
	BP_FLY_DOWN				= 150,		// flying down
	
	BP_HIT_WALL				= 160,		// hit wall while flying horizontally
	BP_HIT_CEILING			= 170,		// hit ceiling while flying up
	BP_HIT_FLOOR			= 180,		// hit floor while flying down
	
	BP_RETURN_TO_GROUND		= 190,		// faces screen and floats down to ground
	BP_LIGHTNING_STRIKE		= 200,		// lightning attack
	
	BP_DEFEATED				= 1000		// defeated (script-triggered)
};

#define DMG_NORMAL		3		// normal damage for touching him
#define DMG_RUSH		10		// damage when he is rushing/flying at you

#define RUSH_SPEED		0x800		// how fast he flies
#define RUSH_DIST		0x2000	// how close he gets to you before changing direction

#define FLOAT_Y			block_to_sub(11)	// Y position to rise to during lightning attack
#define LIGHTNING_Y		block_to_sub(19)	// Y position lightning strikes hit (i.e., the floor)

// creates the two bone spawners which appear when he crashes into the floor or ceiling.
// pass UP if he has hit the ceiling, DOWN if he has hit the floor.
static void spawn_bones(Entity *e, uint8_t up) {
	int32_t y;

	if (up)
		y = (e->y - pixel_to_sub(12));
	else
		y = (e->y + pixel_to_sub(12));
	
	entity_create(e->x - pixel_to_sub(12), y, OBJ_BALLOS_BONE_SPAWNER, 0)->dir = 0;
	entity_create(e->x + pixel_to_sub(12), y, OBJ_BALLOS_BONE_SPAWNER, 0)->dir = 1;
}

// handles his "looping" flight/rush attacks
static void run_flight(Entity *e) {
	switch(e->state) {
		// flying left or right
		case BP_FLY_LR:
		{
			e->state++;
			e->animtime = 0;
			e->frame = 8;		// flying horizontally
			
			e->y_speed = 0;
			e->attack = DMG_RUSH;
			
			FACE_PLAYER(e);
			MOVE_X(RUSH_SPEED);
		} /* fallthrough */
		case BP_FLY_LR+1:
		{
			ANIMATE(e, 4, 8,9);
			
			// smacked into wall?
			if ((!e->dir && collide_stage_leftwall(e)) || 
				(e->dir && collide_stage_rightwall(e))) {
				e->x_speed = 0;
				e->state = BP_HIT_WALL;
				e->attack = DMG_NORMAL;
				e->timer = 0;
				camera_shake(10);
			}
			
			// reached player?
			// this has to be AFTER smacked-into-wall check for proper behavior
			// if player stands in spikes at far left/right of arena.
			if (PLAYER_DIST_X(e, RUSH_DIST))
				e->state = BP_PREPARE_FLY_UD;
		}
		break;
		
		// smacked into wall while flying L/R
		case BP_HIT_WALL:
		{
			e->frame = 8;
			
			if (++e->timer > TIME_8(30)) {
				if (e->timer2 <= 3)
					e->state = BP_PREPARE_FLY_LR;
				else
					e->state = BP_RETURN_TO_GROUND;
			}
		}
		break;
		
		
		// flying up
		case BP_FLY_UP:
		{
			e->state++;
			e->timer = 0;
			e->animtime = 0;
			
			e->frame = 10;		// vertical flight
			//e->dir = 0;		// up-facing frame
			
			e->y_speed = -RUSH_SPEED;
			e->x_speed = 0;
			e->attack = DMG_RUSH;
		} /* fallthrough */
		case BP_FLY_UP+1:
		{
			ANIMATE(e, 4, 10,11);
			
			// hit ceiling? (to make this happen, break his loop and jump ABOVE him
			// while he is in the air, at the part where he would normally be
			// coming back down at you).
			if (collide_stage_ceiling(e)) {
				e->state = BP_HIT_CEILING;
				e->attack = DMG_NORMAL;
				e->timer = 0;
				
				//SmokeXY(e->x, e->Top(), 8);
				camera_shake(10);
				
				spawn_bones(e, UP);
			}
			
			// reached player? (this check here isn't exactly the same as pdistly;
			// it's important that it checks the player's top and not his center).
			if ((abs(player.y - e->y) < RUSH_DIST) && e->timer2 < 4)
				e->state = BP_PREPARE_FLY_LR;
		}
		break;
		
		case BP_HIT_CEILING:	// hit ceiling
		{
			e->frame = 10;
			
			if (++e->timer > TIME_8(30)) {
				if (e->timer2 <= 3)
					e->state = BP_PREPARE_FLY_LR;
				else
					e->state = BP_RETURN_TO_GROUND;
			}
		}
		break;
		
		
		// flying down
		case BP_FLY_DOWN:
		{
			e->state++;
			e->timer = 0;
			e->animtime = 0;
			
			e->frame = 13;		// vertical flight
			//e->dir = RIGHT;		// down-facing frame
			
			e->y_speed = RUSH_SPEED;
			e->x_speed = 0;
			e->attack = DMG_RUSH;
		} /* fallthrough */
		case BP_FLY_DOWN+1:
		{
			ANIMATE(e, 4, 13, 14);
			
			if (collide_stage_floor(e))
			{
				e->state = BP_HIT_FLOOR;
				e->attack = DMG_NORMAL;
				e->timer = 0;
				
				//SmokeXY(e->x, e->Bottom(), 8);
				camera_shake(10);
				
				spawn_bones(e, DOWN);
				FACE_PLAYER(e);
			}
			
			if (PLAYER_DIST_Y(e, RUSH_DIST) && e->timer2 < 4)
				e->state = BP_PREPARE_FLY_LR;
		}
		break;
		
		case BP_HIT_FLOOR:	// hit floor
		{
			e->frame = 3;
			
			if (++e->timer > TIME_8(30)) {
				e->state = BP_FIGHTING_STANCE;
				e->timer = TIME_8(120);
			}
		}
		break;
		
		
		// come back to ground while facing head on
		case BP_RETURN_TO_GROUND:
		{
			e->frame = 1;		// face screen frame
			e->dir = LEFT;		// non-flashing version
			
			e->state++;
		} /* fallthrough */
		case BP_RETURN_TO_GROUND+1:
		{
			ANIMATE(e, 4, 1,2);
			
			e->y_speed += SPEED_8(0x40);
			LIMIT_Y(SPEED_12(0x5ff));
			
			if (e->y_speed >= 0 && collide_stage_floor(e)) {
				e->state++;
				e->timer = 0;
				e->frame = 3; 	// landed
				
				FACE_PLAYER(e);
			}
		}
		break;
		
		case BP_RETURN_TO_GROUND+2:
		{
			e->x_speed *= 3;
			e->x_speed /= 4;
			
			if (++e->timer > TIME_8(10)) {
				e->state = BP_FIGHTING_STANCE;
				e->timer = 140;
			}
		}
		break;
	}
}

// his lightning-strike attack
static void run_lightning(Entity *e) {
	switch(e->state) {
		// lightning strikes (targeting player)
		case BP_LIGHTNING_STRIKE:
		{
			e->x_mark = player.x;
			e->y_speed = -SPEED_12(0x600);
			
			e->timer = 0;
			e->timer2 = 0;
			e->animtime = 0;
			
			e->frame = 4;		// facing screen
			//e->dir = LEFT;		// not flashing
			
			e->state++;
		} /* fallthrough */
		case BP_LIGHTNING_STRIKE+1:
		{
			ANIMATE(e, 4, 4,5);
			e->x_speed += (e->x < e->x_mark) ? SPEED_8(0x40) : -SPEED_8(0x40);
			e->y_speed += (e->y < FLOAT_Y) ? SPEED_8(0x40) : -SPEED_8(0x40);
			LIMIT_X(SPEED_10(0x3FF));
			LIMIT_Y(SPEED_10(0x3FF));
			
			// run firing
			e->timer++;
			switch(e->timer) {
				case 200:
				case 200+40:
				case 200+80:
				case 200+120:
				case 200+160:
				case 200+200:
				case 200+240:
				case 200+280:
				{
					// spawn lightning target
					entity_create(player.x, LIGHTNING_Y, OBJ_BALLOS_TARGET, 0);
					//e->dir = 1;		// switch to flashing frames
					e->animtime = 0;
					
					// after 8 attacks, switch to even-spaced strikes
					if (++e->timer2 >= 8) {
						e->x_speed = 0;
						e->y_speed = 0;
						
						//e->dir = 1;		// flashing
						e->frame = 7;		// flash red then white during screen flash
						e->animtime = 1;	// desync animation from screen flashes so it's visible
						
						e->state++;
						e->timer = 0;
						e->timer2 = 0;
					}
				}
				break;
			}
		}
		break;
		
		// lightning strikes (evenly-spaced everywhere)
		case BP_LIGHTNING_STRIKE+2:
		{
			ANIMATE(e, 4, 6,7);
			e->timer++;
			
			if (e->timer == TIME_8(40)) {
				SCREEN_FLASH(3);
			}
			
			if (e->timer > TIME_8(50)) {
				if ((e->timer & 15) == 1) {
					entity_create(block_to_sub(e->timer2), LIGHTNING_Y, OBJ_BALLOS_TARGET, 0);
					e->timer2 += 4;
					
					if (e->timer2 >= 40) e->state = BP_RETURN_TO_GROUND;
				}
			}
		}
		break;
	}
}

// intro cinematic sequence
static void run_intro(Entity *e) {
	switch(e->state) {
		// idle/talking to player
		case 0:
		{
			// setup
			e->y_next -= (6<<CSF);
			e->dir = 0;
			e->attack = 0;
			
			// ensure copy pfbox first time
			//e->dirparam = -1;
			
			// closed eyes/mouth
			e->linkedEntity = entity_create(e->x, e->y_next - pixel_to_sub(16), OBJ_BALLOS_SMILE, 0);
			e->state = 1;
		}
		break;
		
		// fight begin
		// he smiles, then enters base attack state
		case 10:
		{
			// When entering Seal Chamber from the stage select, Curly is missing.
			// If that happens, spawn her at teh start of the fight
			if(!entity_find_by_type(OBJ_CURLY_CARRIED_SHOOTING)) {
				entity_create(player.x, player.y, OBJ_CURLY_CARRIED_SHOOTING, 0);
			}
			e->state++;
		} /* fallthrough */
		case 11:
		{
			e->timer++;
			
			// animate smile/open eyes
			if (e->timer > TIME_8(50)) {
				Entity *smile = e->linkedEntity;
				if (smile) {
					if (++smile->animtime > 4) {
						smile->animtime = 0;
						smile->frame++;
						
						if (smile->frame > 2)
							smile->state = STATE_DELETE;
					}
				}
				
				if (e->timer > TIME_8(100)) {
					e->state = BP_FIGHTING_STANCE;
					e->timer = TIME_8(150);
					
					e->flags |= NPC_SHOOTABLE;
					e->flags &= ~NPC_INVINCIBLE;
				}
			}
		}
		break;
	}
}


// defeat sequence
// he flies away, then the script triggers the next form
static void run_defeated(Entity *e) {
	switch(e->state) {
		// defeated (script triggered; constant value 1000)
		case BP_DEFEATED:
		{
			e->state++;
			e->timer = 0;
			e->frame = 12;
			
			e->flags &= ~NPC_SHOOTABLE;
			//effect(e->x, e->y, EFFECT_BOOMFLASH);
			//SmokeClouds(o, 16, 16, 16);
			sound_play(SND_BIG_CRASH, 5);
			
			e->x_mark = e->x;
			e->x_speed = 0;
		} /* fallthrough */
		case BP_DEFEATED+1:		// fall to ground, shaking
		{
			e->y_speed += SPEED_8(0x20);
			LIMIT_Y(SPEED_12(0x5ff));
			
			e->x = e->x_mark;
			if (++e->timer & 2) e->x += pixel_to_sub(1);
						   else e->x -= pixel_to_sub(1);
			
			if (e->y_speed >= 0 && collide_stage_floor(e)) {
				if (++e->timer > TIME_8(150)) {
					e->state++;
					e->timer = 0;
					e->frame = 3;
					FACE_PLAYER(e);
				}
			}
		}
		break;
		
		case BP_DEFEATED+2:		// prepare to jump
		{
			if (++e->timer > TIME_8(30)) {
				e->y_speed = -SPEED_12(0xA00);
				
				e->state++;
				e->frame = 10;
				e->flags |= NPC_IGNORESOLID;
			}
		}
		break;
		
		case BP_DEFEATED+3:		// jumping
		{
			ANIMATE(e, 4, 10, 11);
			
			if (e->y < 0) {
				//flashscreen.Start();
				SCREEN_FLASH(3);
				sound_play(SND_TELEPORT, 5);
				
				e->x_speed = 0;
				e->y_speed = 0;
				e->state++;
			}
		}
		break;
	}
}

void ai_ballos_priest(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	run_intro(e);
	run_defeated(e);
	
	run_flight(e);
	run_lightning(e);
	
	switch(e->state) {
		// show "ninja" stance for "timer" ticks,
		// then prepare to fly horizontally
		case BP_FIGHTING_STANCE:
		{
			e->frame = 1;
			e->animtime = 0;
			e->state++;
			
			e->attack = DMG_NORMAL;
			e->savedhp = e->health;
		} /* fallthrough */
		case BP_FIGHTING_STANCE+1:
		{
			ANIMATE(e, 10, 1, 2);
			FACE_PLAYER(e);
			
			if (e->timer-- == 0 || (e->savedhp - e->health) > 50) {
				if (++e->timer3 > 4) {
					e->state = BP_LIGHTNING_STRIKE;
					e->timer3 = 0;
				} else {
					e->state = BP_PREPARE_FLY_LR;
					e->timer2 = 0;
				}
			}
		}
		break;
		
		// prepare for flight attack
		case BP_PREPARE_FLY_LR:
		case BP_PREPARE_FLY_UD:
		{
			e->timer2++;
			e->state++;
			
			e->timer = 0;
			e->frame = 3;	// fists in
			e->attack = DMG_NORMAL;
			
			// Fly/UD faces player only once, at start
			//FACE_PLAYER(e);
		} /* fallthrough */
		case BP_PREPARE_FLY_LR+1:
		{
			FACE_PLAYER(e);
		} /* fallthrough */
		case BP_PREPARE_FLY_UD+1:
		{
			// braking, if we came here out of another fly state
			e->x_speed *= 8; e->x_speed /= 9;
			e->y_speed *= 8; e->y_speed /= 9;
			
			if (++e->timer > TIME_8(20)) {
				sound_play(SND_FUNNY_EXPLODE, 5);
				
				if (e->state == BP_PREPARE_FLY_LR+1) {
					e->state = BP_FLY_LR;		// flying left/right
				} else if (player.y < (e->y + pixel_to_sub(12))) {
					e->state = BP_FLY_UP;		// flying up
				} else {
					e->state = BP_FLY_DOWN;		// flying down
				}
			}
		}
		break;
	}
	
	e->x = e->x_next;
	e->y = e->y_next;
	
	// his bounding box is in a slightly different place on L/R frames
	//if (e->dirparam != e->dir)
	//{
	//	sprites[e->sprite].bbox = sprites[e->sprite].frame[0].dir[e->dir].pf_bbox;
	//	e->dirparam = e->dir;
	//}
}

void ondeath_ballosp(Entity *e) {
	e->flags &= ~NPC_SHOOTABLE;
	tsc_call_event(e->event);
}

// targeter for lightning strikes
void ai_ballos_target(Entity *e) {
	switch(e->state) {
		case 0:
		{
			// position to shoot lightning at passed as x,y
			e->x_mark = e->x; //((sprites[SPR_LIGHTNING].w / 2) << CSF);
			e->y_mark = e->y;
			
			// adjust our Y coordinate to match player's
			e->y = player.y;
			
			sound_play(SND_CHARGE_GUN, 5);
			e->state = 1;
		} /* fallthrough */
		case 1:
		{
			e->hidden = (++e->timer & 2);
			if (e->timer == TIME_8(20)) {	// lightning attack
				if(!e->dir) entity_create(e->x_mark, e->y_mark - 0x2000, OBJ_LIGHTNING, NPC_OPTION2);
			} else if(e->timer >= TIME_8(30)) {
				e->state = STATE_DELETE;
			}
		}
		break;
	}
	
}

// white sparky thing that moves along floor throwing out bones,
// spawned he hits the ground.
// similar to the red smoke-spawning ones from Undead Core.
void ai_ballos_bone_spawner(Entity *e) {
	e->x += e->x_speed;
	
	switch(e->state) {
		case 0:
		{
			sound_play(SND_MISSILE_HIT, 5);
			e->state++;
			MOVE_X(SPEED_10(0x3FF));
		} /* fallthrough */
		case 1:
		{
			e->timer++;
			if((e->timer & 15) == 1 && entity_on_screen(e)) {
				int16_t xi = SPEED_10(rand() & 0x3FF);
				if (!e->dir) xi = -xi;
				
				Entity *bone = entity_create(e->x, e->y, OBJ_BALLOS_BONE, 0);
				bone->x_speed = xi;
				bone->y_speed = -SPEED_10(0x3FF);
				sound_play(SND_BLOCK_DESTROY, 5);
			}
			
			if(blk(e->x, 0, e->y, -2) == 0x41) {
				e->state = STATE_DELETE;
			}
		}
		break;
	}
	
}

// bones emitted by bone spawner
void ai_ballos_bone(Entity *e) {
	e->x += e->x_speed;
	e->y += e->y_speed;
	
	if(++e->animtime > 4) {
		e->animtime = 0;
		if(++e->frame > 3) {
			e->frame = 0;
		}
	}
	
	if (e->y_speed >= 0 && blk(e->x, 0, e->y, 6) == 0x41) {
		if (e->state == 0) {
			e->y_speed = -SPEED_10(0x200);
			e->state = 1;
		} else {
			//effect(e->x, e->y, EFFECT_FISHY);
			e->state = STATE_DELETE;
		}
	}
	
	if(e->y_speed < SPEED_12(0x5C0)) e->y_speed += SPEED_8(0x40);
}

void ai_ballos_skull(Entity *e) {
	if(++e->animtime > 8) {
		e->animtime = 0;
		if(++e->frame > 3) {
			e->frame = 0;
		}
	}
	
	switch(e->state) {
		case 0:
		{
			e->state = 100;
			e->timer = 0;
			e->frame = rand() & 3;
			e->animtime = 0;
		} /* fallthrough */
		case 100:
		{
			e->y_speed += SPEED_8(0x40);
			LIMIT_Y(SPEED_12(0x700));
			
			e->timer++;
			//if (e->timer & 2) {
			//	(SmokePuff(e->x, e->y))->PushBehind(o);
			//}
			
			if (e->y > 0x10000) {
				if (blk(e->x, 0, e->y, 7) == 0x41) {
					e->y_speed = -SPEED_10(0x200);
					e->state = 110;
					
					//quake(10, SND_BLOCK_DESTROY);
					
					//for(int i=0;i<4;i++) {
					//	Entity *s = SmokePuff(e->x + rand(-12<<CSF, 12<<CSF), e->y + 0x2000);
					//	
					//	s->x_speed = rand(-0x155, 0x155);
					//	s->y_speed = rand(-0x600, 0);
					//	s->PushBehind(o);
					//}
				}
			}
		}
		break;
		
		case 110:
		{
			e->y_speed += SPEED_8(0x40);
			
			if (e->y >= block_to_sub(stageHeight)) {
				e->state = STATE_DELETE;
			}
		}
		break;
	}
}

void ai_ballos_spikes(Entity *e) {
	uint16_t x = sub_to_block(e->x) & ~1; // Even number
	if(x >= stageWidth - 2) {
		e->state = STATE_DELETE;
		return;
	}
	if (++e->timer < 128) {
		e->y -= 0x80;
	} else {
		// Need to use some special tiles which are blended with foreground objects
		// Otherwise the bottom will look cut off and bad
		uint16_t blk1 = 54, blk2 = 54;
		switch(x) {
			case 10: case 16: case 20: case 24: // Pillar
			blk1 = 101;
			blk2 = 102;
			break;

			case 30: case 34: // Pillar (left)
			blk2 = 101;
			break;

			case 32: case 36: // Pillar (right)
			blk1 = 102;
			break;

			case 26: // Throne (left)
			blk2 = 103;
			break;

			case 28: // Throne (right)
			blk1 = 104;
			blk2 = 105;
			break;
		}
		stage_replace_block(x, sub_to_block(e->y) - 1, blk1);
		stage_replace_block(x + 1, sub_to_block(e->y) - 1, blk2);
		effect_create_smoke(sub_to_pixel(e->x), sub_to_pixel(e->y));
		effect_create_smoke(sub_to_pixel(e->x) + 16, sub_to_pixel(e->y));
		e->state = STATE_DELETE;
	}
}

void ai_green_devil_spawner(Entity *e) {
	if(e->timer == 0) {
		Entity *dv = entity_create(e->x, e->y - 0x1000, OBJ_GREEN_DEVIL, 0);
		dv->dir = e->dir;
		dv->x_speed = SPEED_10(0x3FF) + (rand() & 0x3FF);
		dv->y_speed = (rand() & 0x7FF) - 0x3FF;
		if(!dv->dir) dv->x_speed = -dv->x_speed;
		dv->y_mark = dv->y;
		dv->attack = 3;
		
		e->timer = TIME_8(50) + (rand() & 0x3F);
	} else {
		e->timer--;
	}
}

void ai_green_devil(Entity *e) {
	e->flags ^= NPC_SHOOTABLE;
	
	e->x += e->x_speed;
	e->y += e->y_speed;
	
	if((++e->animtime & 3) == 0) e->frame ^= 1;
	e->y_speed += (e->y < e->y_mark) ? SPEED_8(0x80) : -SPEED_8(0x80);
	if(!e->dir) {
		if(e->x < 0) e->state = STATE_DELETE;
	} else {
		if(e->x > block_to_sub(stageWidth)) e->state = STATE_DELETE;
	}
}

void onspawn_bute_sword_red(Entity *e) {
	e->alwaysActive = TRUE;
	e->y_speed = -SPEED_12(0x600);
	e->hit_box = (bounding_box) {{ 6,6,6,6 }};
	e->display_box = (bounding_box) {{ 8,8,8,8 }};
}

void ai_bute_sword_red(Entity *e) {
	e->flags ^= NPC_SHOOTABLE;
	e->x += e->x_speed;
	e->y += e->y_speed;
	if((++e->animtime & 3) == 0) e->frame ^= 1;
	if(!e->state) {
		if(++e->timer >= TIME_8(16)) {
			e->state++;
			e->frame = 2;
			e->flags |= NPC_SHOOTABLE;
			e->attack = 5;
		}
	} else {
		e->flags ^= NPC_SHOOTABLE;
		FACE_PLAYER(e);
		// when player is below them, they come towards him,
		// when player is above, they sweep away.
		if(player.y > e->y) ACCEL_X(SPEED_8(0x20));
		else ACCEL_X(-SPEED_8(0x20));
		e->y_speed += (e->y <= player.y) ? SPEED_8(0x20) : -SPEED_8(0x20);
		LIMIT_X(SPEED_10(0x3FF));
		LIMIT_Y(SPEED_10(0x3FF));
		
		if ((e->x_speed < 0 && blk(e->x, -7, e->y, 0) == 0x41) || 
			(e->x_speed > 0 && blk(e->x,  7, e->y, 0) == 0x41)) {
			e->x_speed = -e->x_speed;
		}
		if ((e->y_speed < 0 && blk(e->x, 0, e->y, -7) == 0x41) || 
			(e->y_speed > 0 && blk(e->x, 0, e->y,  7) == 0x41)) {
			e->y_speed = -e->y_speed;
		}
	}
}

void onspawn_bute_archer_red(Entity *e) {
	e->alwaysActive = TRUE;
	if(e->flags & NPC_OPTION2) {
		e->dir = 1;
		e->x_mark = e->x + pixel_to_sub(128);
	} else {
		e->x_mark = e->x - pixel_to_sub(128);
	}
	e->y_mark = e->y;
	e->x_speed = (e->x > e->x_mark) ? -0xFF - (rand() & 0x1FF) : 0xFF + (rand() & 0x1FF);
	e->y_speed = (rand() & 0x3FF) - 0x1FF;
	e->hit_box = (bounding_box) {{ 6,6,6,6 }};
	e->display_box = (bounding_box) {{ 12,8,12,8 }};
}

void ai_bute_archer_red(Entity *e) {
	e->flags ^= NPC_SHOOTABLE;
	e->x += e->x_speed;
	e->y += e->y_speed;
	switch(e->state) {
		case 0:		// come on screen
		{
			if((++e->animtime & 3) == 0) e->frame ^= 1;
			if((!e->dir && e->x < e->x_mark) || (e->dir && e->x > e->x_mark)) {
				e->state++;
				e->timer = TIME_8(rand() & 0x7F);
				e->frame = 2;
			}
		}
		break;
		case 1: // aiming
		{
			if((++e->animtime & 3) == 0) e->frame ^= 1;
			if(++e->timer > TIME_10(300) || (PLAYER_DIST_X(e, pixel_to_sub(112)) && PLAYER_DIST_Y(e, pixel_to_sub(16)))) {
				e->state++;
				e->timer = 0;
				e->frame = 3;
			}
		}
		break;
		case 2: // flashing
		{
			if((++e->animtime & 3) == 0 && ++e->frame > 4) e->frame = 3;
			if(++e->timer > TIME_8(30)) {
				e->state++;
				e->timer = 0;
				e->frame = 5;
				Entity *arrow = entity_create(e->x, e->y, OBJ_BUTE_ARROW, 0);
				arrow->dir = e->dir;
				arrow->x_speed = e->dir ? SPEED_12(0x800) : -SPEED_12(0x800);
			}
		}
		break;
		case 3: // fired
		{
			//if((++e->animtime & 3) == 0 && ++e->frame > 6) e->frame = 5;
			if (++e->timer > TIME_8(40)) {
				e->state++;
				e->timer = 0;
				e->frame = 0;
				e->x_speed = 0;
				e->y_speed = 0;
			}
		}
		break;
		
		case 4:	// retreat offscreen
		{
			if((++e->animtime & 3) == 0) e->frame ^= 1;
			ACCEL_X(-SPEED_8(0x20));
			if(e->x < 0 || e->x > block_to_sub(stageWidth)) e->state = STATE_DELETE;
		}
		break;
	}
	// sinusoidal hover around set point
	if (e->state < 4) {
		if(e->x < e->x_mark && e->x_speed <  SPEED_10(0x3E0)) e->x_speed += SPEED_8(0x2A);
		if(e->x > e->x_mark && e->x_speed > -SPEED_10(0x3E0)) e->x_speed -= SPEED_8(0x2A);
		if(e->y < e->y_mark && e->y_speed <  SPEED_10(0x3E0)) e->y_speed += SPEED_8(0x2A);
		if(e->y > e->y_mark && e->y_speed > -SPEED_10(0x3E0)) e->y_speed -= SPEED_8(0x2A);
	}
	
}

// This object is responsible for collapsing the walls in the final best-ending sequence.
void ai_wall_collapser(Entity *e) {
	if(e->state == 10 && ++e->timer > TIME_8(102)) {
		e->timer = 0;
		uint16_t xa = sub_to_block(e->x);
		uint16_t ya = sub_to_block(e->y);
		for(uint16_t y = 0; y < 20; y++) stage_replace_block(xa, ya+y, 100);
		sound_play(SND_BLOCK_DESTROY, 5);
		camera_shake(20);
		e->x += e->dir ? pixel_to_sub(16) : -pixel_to_sub(16);
	}
}
