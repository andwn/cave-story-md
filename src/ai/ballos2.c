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
#define RUSH_DIST		(16<<CSF)	// how close he gets to you before changing direction

#define FLOAT_Y			block_to_sub(11)	// Y position to rise to during lightning attack
#define LIGHTNING_Y		block_to_sub(19)	// Y position lightning strikes hit (i.e., the floor)

// creates the two bone spawners which appear when he crashes into the floor or ceiling.
// pass UP if he has hit the ceiling, DOWN if he has hit the floor.
static void spawn_bones(Entity *e, uint8_t up) {
	int32_t y;

	if (up)
		y = (e->y - (12 << CSF));
	else
		y = (e->y + (12 << CSF));
	
	entity_create(e->x - (12<<CSF), y, OBJ_BALLOS_BONE_SPAWNER, 0)->dir = 0;
	entity_create(e->x + (12<<CSF), y, OBJ_BALLOS_BONE_SPAWNER, 0)->dir = 1;
}

// handles his "looping" flight/rush attacks
static void run_flight(Entity *e)
{
	switch(e->state)
	{
		// flying left or right
		case BP_FLY_LR:
		{
			e->state++;
			e->animtime = 0;
			e->frame = 6;		// flying horizontally
			
			e->y_speed = 0;
			e->attack = DMG_RUSH;
			
			FACE_PLAYER(e);
			MOVE_X(RUSH_SPEED);
		}
		case BP_FLY_LR+1:
		{
			ANIMATE(e, 4, 6,7);
			
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
			if (PLAYER_DIST_X(RUSH_DIST))
				e->state = BP_PREPARE_FLY_UD;
		}
		break;
		
		// smacked into wall while flying L/R
		case BP_HIT_WALL:
		{
			e->frame = 6;
			
			if (++e->timer > 30)
			{
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
			
			e->frame = 8;		// vertical flight
			e->dir = 0;		// up-facing frame
			
			e->y_speed = -RUSH_SPEED;
			e->x_speed = 0;
			e->attack = DMG_RUSH;
		}
		case BP_FLY_UP+1:
		{
			ANIMATE(e, 4, 8,9);
			
			// hit ceiling? (to make this happen, break his loop and jump ABOVE him
			// while he is in the air, at the part where he would normally be
			// coming back down at you).
			if (collide_stage_ceiling(e))
			{
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
			e->frame = 8;
			
			if (++e->timer > 30)
			{
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
			
			e->frame = 8;		// vertical flight
			e->dir = RIGHT;		// down-facing frame
			
			e->y_speed = RUSH_SPEED;
			e->x_speed = 0;
			e->attack = DMG_RUSH;
		}
		case BP_FLY_DOWN+1:
		{
			ANIMATE(e, 4, 8, 9);
			
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
			
			if (PLAYER_DIST_Y(RUSH_DIST) && e->timer2 < 4)
				e->state = BP_PREPARE_FLY_LR;
		}
		break;
		
		case BP_HIT_FLOOR:	// hit floor
		{
			e->frame = 3;
			
			if (++e->timer > 30)
			{
				e->state = BP_FIGHTING_STANCE;
				e->timer = 120;
			}
		}
		break;
		
		
		// come back to ground while facing head on
		case BP_RETURN_TO_GROUND:
		{
			e->frame = 4;		// face screen frame
			e->dir = LEFT;		// non-flashing version
			
			e->state++;
		}
		case BP_RETURN_TO_GROUND+1:
		{
			ANIMATE(e, 4, 4,5);
			
			e->y_speed += 0x40;
			LIMIT_Y(0x5ff);
			
			if (e->y_speed >= 0 && collide_stage_floor(e))
			{
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
			
			if (++e->timer > 10)
			{
				e->state = BP_FIGHTING_STANCE;
				e->timer = 140;
			}
		}
		break;
	}
}

// his lightning-strike attack
static void run_lightning(Entity *e)
{
	switch(e->state)
	{
		// lightning strikes (targeting player)
		case BP_LIGHTNING_STRIKE:
		{
			e->x_mark = player.x;
			e->y_speed = -0x600;
			
			e->timer = 0;
			e->timer2 = 0;
			e->animtime = 0;
			
			e->frame = 4;		// facing screen
			e->dir = LEFT;		// not flashing
			
			e->state++;
		}
		case BP_LIGHTNING_STRIKE+1:
		{
			ANIMATE(e, 4, 4,5);
			
			e->x_speed += (e->x < e->x_mark) ? 0x40 : -0x40;
			e->y_speed += (e->y < FLOAT_Y) ? 0x40 : -0x40;
			LIMIT_X(0x400);
			LIMIT_Y(0x400);
			
			// run firing
			if (++e->timer > 200)
			{
				uint8_t pos = (e->timer % 40);
				
				if (pos == 1)
				{
					// spawn lightning target
					entity_create(player.x, LIGHTNING_Y, OBJ_BALLOS_TARGET, 0);
					e->dir = RIGHT;		// switch to flashing frames
					e->animtime = 0;
					
					// after 8 attacks, switch to even-spaced strikes
					if (++e->timer2 >= 8)
					{
						e->x_speed = 0;
						e->y_speed = 0;
						
						e->dir = 1;		// flashing
						e->frame = 5;		// flash red then white during screen flash
						e->animtime = 1;	// desync animation from screen flashes so it's visible
						
						e->state++;
						e->timer = 0;
						e->timer2 = 0;
					}
				}
				else if (pos == 20)
				{
					e->dir = 0;		// stop flashing
				}
			}
		}
		break;
		
		// lightning strikes (evenly-spaced everywhere)
		case BP_LIGHTNING_STRIKE+2:
		{
			ANIMATE(e, 4, 4,5);
			e->timer++;
			
			if (e->timer == 40)
				SCREEN_FLASH(20);
				//flashscreen.Start();
			
			if (e->timer > 50) {
				if ((e->timer % 10) == 1) {
					entity_create(block_to_sub(e->timer2), LIGHTNING_Y, OBJ_BALLOS_TARGET, 0);
					e->timer2 += 4;
					
					if (e->timer2 >= 40)
						e->state = BP_RETURN_TO_GROUND;
				}
			}
		}
		break;
	}
}

// intro cinematic sequence
static void run_intro(Entity *e)
{
	switch(e->state)
	{
		// idle/talking to player
		case 0:
		{
			// setup
			e->y -= (6<<CSF);
			e->dir = 0;
			e->attack = 0;
			
			// ensure copy pfbox first time
			//e->dirparam = -1;
			
			// closed eyes/mouth
			e->linkedEntity = entity_create(e->x, e->y - (16 << CSF), OBJ_BALLOS_SMILE, 0);
			e->state = 1;
		}
		break;
		
		// fight begin
		// he smiles, then enters base attack state
		case 10:
		{
			e->timer++;
			
			// animate smile/open eyes
			if (e->timer > 50)
			{
				Entity *smile = e->linkedEntity;
				if (smile)
				{
					if (++smile->animtime > 4)
					{
						smile->animtime = 0;
						smile->frame++;
						
						if (smile->frame > 2)
							smile->state = STATE_DELETE;
					}
				}
				
				if (e->timer > 100)
				{
					e->state = BP_FIGHTING_STANCE;
					e->timer = 150;
					
					e->eflags |= NPC_SHOOTABLE;
					e->eflags &= ~NPC_INVINCIBLE;
				}
			}
		}
		break;
	}
}


// defeat sequence
// he flies away, then the script triggers the next form
static void run_defeated(Entity *e)
{
	switch(e->state)
	{
		// defeated (script triggered; constant value 1000)
		case BP_DEFEATED:
		{
			e->state++;
			e->timer = 0;
			e->frame = 10;
			
			e->eflags &= ~NPC_SHOOTABLE;
			//effect(e->x, e->y, EFFECT_BOOMFLASH);
			//SmokeClouds(o, 16, 16, 16);
			sound_play(SND_BIG_CRASH, 5);
			
			e->x_mark = e->x;
			e->x_speed = 0;
		}
		case BP_DEFEATED+1:		// fall to ground, shaking
		{
			e->y_speed += 0x20;
			LIMIT_Y(0x5ff);
			
			e->x = e->x_mark;
			if (++e->timer & 2) e->x += (1 << CSF);
						   else e->x -= (1 << CSF);
			
			if (e->y_speed >= 0 && collide_stage_floor(e))
			{
				if (++e->timer > 150)
				{
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
			if (++e->timer > 30)
			{
				e->y_speed = -0xA00;
				
				e->state++;
				e->frame = 8;
				e->eflags |= NPC_IGNORESOLID;
			}
		}
		break;
		
		case BP_DEFEATED+3:		// jumping
		{
			ANIMATE(e, 1, 8, 9);
			e->dir = LEFT;		// up frame
			
			if (e->y < 0)
			{
				//flashscreen.Start();
				SCREEN_FLASH(20);
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
	
	switch(e->state)
	{
		// show "ninja" stance for "timer" ticks,
		// then prepare to fly horizontally
		case BP_FIGHTING_STANCE:
		{
			e->frame = 1;
			e->animtime = 0;
			e->state++;
			
			e->attack = DMG_NORMAL;
			e->savedhp = e->health;
		}
		case BP_FIGHTING_STANCE+1:
		{
			ANIMATE(e, 10, 1, 2);
			FACE_PLAYER(e);
			
			if (e->timer-- == 0 || (e->savedhp - e->health) > 50)
			{
				if (++e->timer3 > 4)
				{
					e->state = BP_LIGHTNING_STRIKE;
					e->timer3 = 0;
				}
				else
				{
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
			FACE_PLAYER(e);
		}
		case BP_PREPARE_FLY_LR+1:
		{
			FACE_PLAYER(e);
		}
		case BP_PREPARE_FLY_UD+1:
		{
			// braking, if we came here out of another fly state
			e->x_speed *= 8; e->x_speed /= 9;
			e->y_speed *= 8; e->y_speed /= 9;
			
			if (++e->timer > 20)
			{
				sound_play(SND_FUNNY_EXPLODE, 5);
				
				if (e->state == BP_PREPARE_FLY_LR+1)
				{
					e->state = BP_FLY_LR;		// flying left/right
				}
				else if (player.y < (e->y + (12 << CSF)))
				{
					e->state = BP_FLY_UP;		// flying up
				}
				else
				{
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

// targeter for lightning strikes
void ai_ballos_target(Entity *e)
{
	switch(e->state)
	{
		case 0:
		{
			// position to shoot lightning at passed as x,y
			e->x_mark = e->x - (8 << CSF); //((sprites[SPR_LIGHTNING].w / 2) << CSF);
			e->y_mark = e->y;
			
			// adjust our Y coordinate to match player's
			e->y = player.y;
			
			sound_play(SND_CHARGE_GUN, 5);
			e->state = 1;
		}
		case 1:
		{
			ANIMATE(e, 4, 0,1);
			e->timer++;
			
			if (e->timer == 20 && !e->dir)
			{	// lightning attack
				// setting lightning dir=left: tells it do not flash screen
				entity_create(e->x_mark, e->y_mark, OBJ_LIGHTNING, 0);
			}
			
			if (e->timer > 40)
				e->state = STATE_DELETE;
		}
		break;
	}
	
}


// white sparky thing that moves along floor throwing out bones,
// spawned he hits the ground.
// similar to the red smoke-spawning ones from Undead Core.
void ai_ballos_bone_spawner(Entity *e) {
	e->x += e->x_speed;
	
	switch(e->state)
	{
		case 0:
		{
			sound_play(SND_MISSILE_HIT, 5);
			e->state = 1;
			
			MOVE_X(0x400);
		}
		case 1:
		{
			ANIMATE(e, 4, 0,1,2);
			e->timer++;
			
			if ((e->timer % 6) == 1) {
				int16_t xi = 4 + ((random() % 12) << (CSF-3));
				
				if (!e->dir)
					xi = -xi;
				
				Entity *bone = entity_create(e->x, e->y, OBJ_BALLOS_BONE, 0);
				bone->x_speed = xi;
				bone->y_speed = -0x400;
				sound_play(SND_BLOCK_DESTROY, 5);
			}
			
			if (blk(e->x, 0, e->y, -2) == 0x41) {
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
	
	ANIMATE(e, 3, 0, 2);
	
	if (e->y_speed >= 0 && blk(e->x, 0, e->y, 6) == 0x41) {
		if (e->state == 0) {
			e->y_speed = -0x200;
			e->state = 1;
		} else {
			//effect(e->x, e->y, EFFECT_FISHY);
			e->state = STATE_DELETE;
		}
	}
	
	e->y_speed += 0x40;
	LIMIT_Y(0x5ff);
}

void ai_ballos_skull(Entity *e) {
	ANIMATE(e, 8, 0,1,2,3);
	
	switch(e->state) {
		case 0:
		{
			e->state = 100;
			e->frame = random() & 3;
		}
		case 100:
		{
			e->y_speed += 0x40;
			LIMIT_Y(0x700);
			
			e->timer++;
			//if (e->timer & 2) {
			//	(SmokePuff(e->x, e->y))->PushBehind(o);
			//}
			
			if (e->y > 0x10000) {
				e->eflags &= ~NPC_IGNORESOLID;
				
				if (blk(e->x, 0, e->y, 7) == 0x41) {
					e->y_speed = -0x200;
					e->state = 110;
					e->eflags |= NPC_IGNORESOLID;
					
					//quake(10, SND_BLOCK_DESTROY);
					
					//for(int i=0;i<4;i++) {
					//	Entity *s = SmokePuff(e->x + random(-12<<CSF, 12<<CSF), e->y + 0x2000);
					//	
					//	s->x_speed = random(-0x155, 0x155);
					//	s->y_speed = random(-0x600, 0);
					//	s->PushBehind(o);
					//}
				}
			}
		}
		break;
		
		case 110:
		{
			e->y_speed += 0x40;
			
			if (e->y >= block_to_sub(stageHeight)) {
				e->state = STATE_DELETE;
			}
		}
		break;
	}
}

void ai_ballos_spikes(Entity *e) {
	switch(e->state) {
		case 0:
		{
			if (++e->timer < TIME(128)) {
				e->y -= SPEED(0x80);
				e->frame = (e->timer & 2) ? 0 : 1;
			} else {
				e->state = 1;
				e->attack = 2;
			}
		}
		break;
	}
}

void ai_green_devil_spawner(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->timer = random() % TIME(40);
			e->state = 1;
		}
		case 1:
		{
			if (e->timer-- == 0) {
				Entity *dv = entity_create(e->x, e->y, OBJ_GREEN_DEVIL, 0);
				dv->x_speed = (random() % (32<<CSF)) - (16<<CSF);
				dv->dir = e->dir;
				
				e->state = 0;
			}
		}
		break;
	}
	
}

void ai_green_devil(Entity *e) {
	e->nflags ^= NPC_SHOOTABLE;
	switch(e->state) {
		case 0:
		{
			e->y_mark = e->y;
			e->y_speed = (random() % (10<<CSF)) - (5<<CSF);
			e->attack = 3;
			e->state = 1;
		}
		case 1:
		{
			ANIMATE(e, 4, 0,1);
			e->y_speed += (e->y < e->y_mark) ? 0x80 : -0x80;
			
			ACCEL_X(0x20);
			LIMIT_X(0x400);
			
			if (!e->dir) {
				if (e->x < 0) e->state = STATE_DELETE;
			} else {
				if (e->x > block_to_sub(stageWidth)) e->state = STATE_DELETE;
			}
		}
		break;
	}
	
}

void ai_bute_sword_red(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			//e->sprite = SPR_BUTE_SWORD_RED_FALLING;
			//e->MoveAtDir(e->dir, 0x600);
			e->dir = 0;
		}
		case 1:
		{
			ANIMATE(e, 4, 0,1,2,3);
			
			if (++e->timer == 8)
				e->eflags &= ~NPC_IGNORESOLID;
			
			if (e->timer >= 16) {
				e->state = 10;
				//e->sprite = SPR_BUTE_SWORD_RED;
				e->frame = 0;
				
				e->eflags |= NPC_SHOOTABLE;
				e->attack = 5;
			}
		}
		break;
		
		case 10:
		{
			ANIMATE(e, 4, 0,1);
			FACE_PLAYER(e);
			
			// when player is below them, they come towards him,
			// when player is above, they sweep away.
			if (player.y > (e->y + (24 << CSF))) {
				ACCEL_X(0x10);
			} else {
				ACCEL_X(-0x10);
			}
			
			e->y_speed += (e->y <= player.y) ? 0x10 : -0x10;
			
			if ((e->x_speed < 0 && blk(e->x, -7, e->y, 0) == 0x41) || 
				(e->x_speed > 0 && blk(e->x,  7, e->y, 0) == 0x41)) {
				e->x_speed = -e->x_speed;
			}
			
			if ((e->y_speed < 0 && blk(e->x, 0, e->y, -7) == 0x41) || 
				(e->y_speed > 0 && blk(e->x, 0, e->y,  7) == 0x41)) {
				e->y_speed = -e->y_speed;
			}
			
			LIMIT_X(0x5ff);
			LIMIT_Y(0x5ff);
		}
		break;
	}
}

void ai_bute_archer_red(Entity *e) {
	//DebugCrosshair(e->x, e->y, 0, 255, 255);
	
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			
			e->x_mark = e->x;
			e->y_mark = e->y;
			
			if (!e->dir)
				e->x_mark -= (128<<CSF);
			else
				e->x_mark += (128<<CSF);
			
			e->x_speed = (random() % 0x800) - 0x400;
			e->y_speed = (random() % 0x800) - 0x400;
		}
		case 1:		// come on screen
		{
			ANIMATE(e, 4, 0,1);
			
			if ((!e->dir && e->x < e->x_mark) || (e->dir && e->x > e->x_mark)) {
				e->state = 20;
			}
		}
		break;
		
		case 20:	// aiming
		{
			e->state = 21;
			e->timer = random() % TIME(150);
			
			e->frame = 2;
			e->animtime = 0;
		}
		case 21:
		{
			ANIMATE(e, 4, 2,3);
			
			if (++e->timer > 300 || (PLAYER_DIST_X(112<<CSF) && PLAYER_DIST_Y(16<<CSF))) {
				e->state = 30;
			}
		}
		break;
		
		case 30:	// flashing to fire
		{
			e->state = 31;
			e->timer = 0;
			e->animtime = 0;
			e->frame = 3;
		}
		case 31:
		{
			ANIMATE(e, 4, 3,4);
			
			if (++e->timer > 30) {
				e->state = 40;
				e->frame = 5;
				
				Entity *arrow = entity_create(e->x, e->y, OBJ_BUTE_ARROW, 0);
				arrow->dir = e->dir;
				arrow->x_speed = e->dir ? 0x800 : -0x800;
			}
		}
		break;
		
		case 40:	// fired
		{
			e->state = 41;
			e->timer = 0;
			e->animtime = 0;
		}
		case 41:
		{
			ANIMATE(e, 4, 5,6);
			
			if (++e->timer > 40) {
				e->state = 50;
				e->timer = 0;
				e->x_speed = 0;
				e->y_speed = 0;
			}
		}
		break;
		
		case 50:	// retreat offscreen
		{
			ANIMATE(e, 4, 0,1);
			ACCEL_X(-0x20);
			
			if (e->x < 0 || e->x > block_to_sub(stageWidth)) e->state = STATE_DELETE;
		}
		break;
	}
	
	// sinusoidal hover around set point
	if (e->state != 50) {
		e->x_speed += (e->x < e->x_mark) ? 0x2A : -0x2A;
		e->y_speed += (e->y < e->y_mark) ? 0x2A : -0x2A;
		LIMIT_X(0x400);
		LIMIT_Y(0x400);
	}
	
}

// This object is responsible for collapsing the walls in the final best-ending sequence.
// All the original object does is collapse one tile further every 101 frames.
// However, since it's triggered at the beginning of the cinematic and then is let to run
// through almost the entire thing it needs to be sync'd really-really perfect with a
// number of other systems; the textboxes, etc.
//
// I spent several hours trying to get my events to run in perfect frame-by-frame
// exactness with the original engine, and found several things that were slightly off.
// However, I've decided that even if I got it absolutely perfect, it's too liable to
// get broken by some minor innocent change in the future, and requires too much of
// the engine to be tuned just so.
//
// So, I've added some event-based triggers to the object, that are NOT technically supposed
// to be there. These will make extra sure that nothing embarrassing happens during this great
// finale, such as the walls being one tile too far at one point, or even worse, having
// them collapse onto Balrog before he makes it to the exit. Because there are no triggers
// in the script and I can't change the script, I had to do a bit of sneaky spying on program
// state to implement them.
void ai_wall_collapser(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->hidden = TRUE;
			e->timer = 0;
			e->state = 1;
		}
		break;
		
		case 10:	// trigger
		{
			if (++e->timer > TIME(100)) {
				e->timer2++;
				e->timer = 0;
				
				uint16_t xa = sub_to_block(e->x);
				uint16_t ya = sub_to_block(e->y);
				for(uint16_t y=0;y<20;y++) {
					// pushing the smoke behind all objects prevents it from covering
					// up the NPC's on the collapse just before takeoff.
					stage_replace_block(xa, ya+y, 109);
				}
				
				sound_play(SND_BLOCK_DESTROY, 5);
				camera_shake(20);
				
				if (!e->dir) e->x -= (16 << CSF);
						else e->x += (16 << CSF);
				
				// reached the solid tile in the center of the throne.
				// it isn't supposed to cover this tile until after Curly
				// says we're gonna get crushed.
				if (e->timer2 == TIME(6)) e->state = 20;
				
				// balrog is about to take off/rescue you.
				if (e->timer2 == TIME(9)) e->state = 30;
			}
		}
		break;
		
		// "gonna get crushed" event
		case 20:
		{
			// wait for text to come up
			if (windowOpen) e->state = 21;
		}
		break;
		case 21:
		{
			// wait for text to dismiss, then tile immediately collapses
			if (!windowOpen) {
				e->state = 10;
				e->timer = TIME(1000);
			}
		}
		break;
		
		// balrog is about to take off. the video I took shows that
		// the walls are supposed to collapse into your space on the
		// exact same frame that he breaks the first ceiling tile.
		case 30:
		{
			e->linkedEntity = entity_find_by_type(OBJ_BALROG_DROP_IN);
			if (e->linkedEntity) e->state = 31;
		}
		break;
		case 31:
		{
			//debug("%x", e->linkedEntity->y);
			if (e->linkedEntity && e->linkedEntity->y <= 0x45800) {
				e->state = 10;
				e->timer = TIME(1000);
			}
		}
		break;
	}
}
