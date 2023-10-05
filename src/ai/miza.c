#include "ai_common.h"

#define SUE_BASE				20
#define SUE_PREPARE_ATTACK		30
#define SUE_SOMERSAULT			40
#define SUE_DASH				50
#define SUE_SOMERSAULT_HIT		60

// both sue and misery
#define SIDEKICK_CORE_DEFEATED		99		// core defeated (script-triggered)
#define SIDEKICK_DEFEATED			100		// sidekick defeated
#define SIDEKICK_CORE_DEFEATED_2	110

#define savedhp		id
#define angle		jump_time
#define spawner		underwater

uint8_t sue_being_hurt;
uint8_t sue_was_killed;

// Prototypes
//static Entity *fm_spawn_missile(Entity *e, uint8_t angindex);
static void set_ignore_solid(Entity *e);
static void sidekick_run_defeated(Entity *e, uint16_t health);

void ai_misery_frenzied(Entity *e) {
	enum Frame {
		STAND1, STAND2, FLOAT1, FLOAT2, SPELL1, SPELL2, SPELL3,
		IDK1, IDK2, IDK3, EGADS, KNOCKOUT, IDK4, IDK5, DASH,
		MISSL1, MISSL2, IDK6
	};
	
	sidekick_run_defeated(e, 600);
	
	switch(e->state) {
		case 0:
		{
			e->hit_box = (bounding_box) {{ 10, 12, 10, 12 }};
			e->display_box = (bounding_box) {{ 16, 16, 16, 16 }};
			e->y -= 8 <<CSF;
			
			e->state = 1;
			sue_being_hurt = sue_was_killed = FALSE;
			
			e->savedhp = e->health;
			
			sound_play(SND_TELEPORT, 5);
			e->timer = 1;
		} /* fallthrough */
		case 1:		// transforming
		{
			e->timer++;
			if (e->timer == 2) {	// frenzied
				e->frame = EGADS;
				e->hidden = FALSE;
			}
			if (e->timer == 4) {	// normal
				e->timer = 0;
				e->hidden = TRUE;
			}
			
			if (++e->timer2 >= TIME(50)) {
				e->timer2 = 0;
				e->state = 2;
				e->hidden = FALSE;	// let's not leave misery invisible the whole fight
			}
		}
		break;
		case 10:	// hold at "being transformed" frame
		{
			e->state = 11;
			e->frame = EGADS;
		}
		break;
		case 20:	// fight begin / base state
		{
			e->state = 21;
			e->timer = 0;
			e->frame = FLOAT1;
			e->animtime = 0;
		} /* fallthrough */
		case 21:
		{
			e->x_speed *= 7; e->x_speed /= 8;
			e->y_speed *= 7; e->y_speed /= 8;
			
			ANIMATE(e, 20, FLOAT1,FLOAT2);
			
			if (++e->timer > TIME(100)) e->state = 30;
			
			FACE_PLAYER(e);
		}
		break;
		case 30: // Charging spell
		{
			e->state = 31;
			e->timer = 0;
			e->frame = SPELL1;
			e->savedhp = e->health;
		} /* fallthrough */
		case 31:
		{
			e->frame = (e->timer & 4) ? SPELL1 : SPELL2;
			
			if (e->grounded) e->y_speed = -SPEED(0x200);
			
			int32_t core_x = bossEntity ? bossEntity->x : 0;
			
			e->x_speed += (e->x > core_x) ? -SPEED(0x20) : SPEED(0x20);
			e->y_speed += (e->y > player.y) ? -SPEED(0x10) : SPEED(0x10);
			LIMIT_X(SPEED(0x200));
			LIMIT_Y(SPEED(0x200));
			
			FACE_PLAYER(e);
			
			if (++e->timer > TIME(150)) {
				// she attacks with normal critters if you attack either her or Sue.
				if ((e->savedhp - e->health) > 20 || sue_being_hurt) {
					sue_being_hurt = FALSE;
					e->state = 40;
				}
			}
			
			// she attacks periodically with fishy missiles if you killed Sue.
			if (e->timer > TIME(250) && sue_was_killed)
				e->state = 50;
		}
		break;
		
		case 40:	// spawn bats/critters
		{
			e->state = 41;
			e->timer = 0;
			e->x_speed = 0;
			e->y_speed = 0;
			FACE_PLAYER(e);
			sound_play(SND_CHARGE_GUN, 5);
			
			// if you are below the 2nd little platform on the left,
			// she spawns critters, else bats.
			e->spawner = player.y >= block_to_sub(10);
		} /* fallthrough */
		case 41:
		{
			e->timer++;
			e->frame = (e->timer & 4) ? SPELL1 : SPELL2;
			
			if ((e->timer & 15) == 1) {
				int x, y;
				
				if (e->spawner) {
					x = e->x - pixel_to_sub(64 + ((rand() & 127)));
					y = e->y - pixel_to_sub(32 + ((rand() & 63)));
				} else {
					x = e->x - pixel_to_sub(32 + ((rand() & 63)));
					y = e->y - pixel_to_sub(64 + ((rand() & 127)));
				}
				
				if (x < block_to_sub(2)) x = block_to_sub(2);
				if (x > block_to_sub(stageWidth - 3)) x = block_to_sub(stageWidth - 3);
				
				if (y < block_to_sub(2)) y = block_to_sub(2);
				if (y > block_to_sub(stageHeight - 3)) y = block_to_sub(stageHeight - 3);
				
				sound_play(SND_EM_FIRE, 5);
				entity_create(x, y, 
						e->spawner ? OBJ_MISERY_CRITTER : OBJ_MISERY_BAT, 0)->hidden = TRUE;
			}
			
			if (e->timer > TIME_8(50)) {
				e->state = 42;
				e->timer = 0;
				FACE_PLAYER(e);
			}
		}
		break;
		
		case 42:
		{
			e->frame = DASH;
			
			if (++e->timer > TIME_8(50)) {
				e->y_speed = -SPEED_10(0x200);
				MOVE_X(-SPEED_10(0x200));
				
				e->state = 30;
			}
		}
		break;
		
		case 50:	// spawn fishy missiles
		{
			e->state = 51;
			e->timer = 0;
			e->x_speed = 0;
			e->y_speed = 0;
			FACE_PLAYER(e);
			sound_play(SND_CHARGE_GUN, 5);
		} /* fallthrough */
		case 51:
		{
			e->timer++;
			e->frame = (e->timer & 4) ? SPELL1 : SPELL2;
			
			//uint8_t rate = (playerEquipment & EQUIP_BOOSTER20) ? 10 : 24;
			
			//if ((e->timer % rate) == 1) {
				// pattern: booster=[0,1,3,1,2,0], no-booster=[0,0,0]:
			//	uint8_t angindex = (e->timer >> 3) & 3;
			//	fm_spawn_missile(e, angindex);
			//}
			
			if (++e->timer > TIME(50)) {
				e->state = 42;
				e->timer = 0;
				FACE_PLAYER(e);
			}
		}
		break;
	}
	
	// Stay within bounds
	if(e->y_speed > 0 && e->y > block_to_sub(stageHeight - 3)) e->y_speed = 0;
	if(e->y_speed < 0 && e->y < block_to_sub(3)) e->y_speed = 0;
	
	e->x += e->x_speed;
	e->y += e->y_speed;
}

/*
// spawn a fishy missile in the given direction
static Entity *fm_spawn_missile(Entity *e, uint8_t angindex) {
	static const int ang_table_left[]  = { 0xD8, 0xEC, 0x14, 0x28 };
	static const int ang_table_right[] = { 0x58, 0x6C, 0x94, 0xA8 };

	Entity *shot = entity_create(e->x, e->y, OBJ_MISERY_MISSILE, 0);
	sound_play(SND_EM_FIRE, 3);
	
	if (!e->dir) {
		shot->x += (10 << CSF);
		shot->angle = ang_table_left[angindex];
	} else {
		shot->x -= (10 << CSF);
		shot->angle = ang_table_right[angindex];
	}
	
	return shot;
}
*/
void ai_misery_critter(Entity *e) {
	e->flags ^= NPC_SHOOTABLE;
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	if(e->state < 12) {
		if(e->x_speed > 0) collide_stage_rightwall(e);
		if(e->x_speed < 0) collide_stage_leftwall(e);
		//if(e->y_speed < 0) collide_stage_ceiling(e);
		if(!e->grounded) e->grounded = collide_stage_floor(e);
	}
	
	switch(e->state) {
		case 0:
		{
			if (++e->timer > TIME(16)) {
				e->frame = 2;
				e->hidden = FALSE;
				e->grounded = FALSE;
				FACE_PLAYER(e);
				
				e->state = 10;
				e->attack = 2;
			}
		}
		break;
		
		case 10:
		{
			if (e->grounded) {
				e->state = 11;
				e->frame = 0;
				e->timer = 0;
				e->x_speed = 0;
				
				FACE_PLAYER(e);
			}
		}
		break;
		
		case 11:
		{
			if (++e->timer > TIME(10)) {
				if (++e->timer2 > 4)
					e->state = 12;
				else
					e->state = 10;
				
				sound_play(SND_ENEMY_JUMP, 5);
				e->grounded = FALSE;
				e->y_speed = -SPEED(0x600);
				MOVE_X(SPEED(0x200));
				
				e->frame = 2;
			}
		}
		break;
		
		case 12:
		{
			if (e->y_next > block_to_sub(stageHeight)) {
				e->state = STATE_DELETE;
			}
		}
		break;
	}
	
	e->x = e->x_next;
	e->y = e->y_next;
	if (e->state >= 10) {
		if(!e->grounded) e->y_speed += SPEED(0x40);
		LIMIT_Y(SPEED(0x5FF));
	}
}

void ai_misery_bat(Entity *e) {
	e->flags ^= NPC_SHOOTABLE;
	switch(e->state) {
		case 0:
		{
			if (++e->timer > TIME(16)) {
				e->frame = 2;
				e->hidden = FALSE;
				FACE_PLAYER(e);
				
				e->state = 1;
				e->attack = 2;
				e->flags &= ~NPC_INVINCIBLE;
				
				e->y_mark = e->y;
				e->y_speed = SPEED(0x400);
			}
		}
		break;
		
		case 1:
		{
			ANIMATE(e, 2, 0,1,2);
			
			e->y_speed += (e->y < e->y_mark) ? SPEED(0x40) : -SPEED(0x40);
			ACCEL_X(SPEED(0x10));
			
			if ((e->x < 0) || e->x > block_to_sub(stageWidth) ||
                    (e->y < 0) || e->y > block_to_sub(stageHeight)) {
				e->state = STATE_DELETE;
			}
		}
		break;
	}
	
	e->x += e->x_speed;
	e->y += e->y_speed;
}
/*
void ai_misery_missile(Entity *e) {
	// cut & pasted from ai_x_fishy_missile
	vector_from_angle(e->angle, 0x400, &e->x_speed, &e->y_speed);
	int desired_angle = GetAngle(e->x, e->y, player.x, player.y);
	
	if (e->angle >= desired_angle)
	{
		if ((e->angle - desired_angle) < 128)
		{
			e->angle--;
		}
		else
		{
			e->angle++;
		}
	}
	else
	{
		if ((e->angle - desired_angle) < 128)
		{
			e->angle++;
		}
		else
		{
			e->angle--;
		}
	}
	
	// smoke trails
	if (++e->timer2 > 2)
	{
		e->timer2 = 0;
		Caret *c = effect(e->ActionPointX(), e->ActionPointY(), EFFECT_SMOKETRAIL_SLOW);
		c->x_speed = -e->x_speed >> 2;
		c->y_speed = -e->y_speed >> 2;
	}
	
	e->frame = (e->angle + 16) / 32;
	if (e->frame > 7) e->frame = 7;
}
*/

void ai_sue_frenzied(Entity *e) {
	enum Frame { 
		TRANSF1, TRANSF2, STILL1, STILL2, PREPARE, DASH, 
		SPIN1, SPIN2, SPIN3, SPIN4, EGADS, KNOCKOUT, 
		IDK1, IDK2, IDK3, IDK4, IDK5
	};
	
	sidekick_run_defeated(e, 500);
	
	switch(e->state) {
		case 0:
		{
			e->hit_box = (bounding_box) {{ 10, 12, 10, 12 }};
			e->display_box = (bounding_box) {{ 16, 16, 16, 16 }};
			e->y -= 4 <<CSF;
			
			e->state = 1;
			sue_being_hurt = sue_was_killed = FALSE;
			
			e->savedhp = e->health;
			
			sound_play(SND_TELEPORT, 5);
			e->timer = 1;
		} /* fallthrough */
		case 1:		// transforming
		{
			e->timer++;
			if (e->timer == 2) {	// frenzied sue
				e->frame = TRANSF2;
			}
			if (e->timer == 4) {	// normal sue
				e->timer = 0;
				e->frame = TRANSF1;
			}
			
			if (++e->timer2 >= TIME(50)) {
				entities_clear_by_type(OBJ_RED_CRYSTAL);
				e->frame = TRANSF2;
				e->timer2 = 0;
				e->state = 2;
			}
		}
		break;
		// fight begin/base state (script-triggered)
		case SUE_BASE:
		{
			e->state++;
			e->timer = 0;
			e->frame = STILL1;
			e->animtime = 0;
			e->attack = 0;
			
			e->flags |= NPC_SHOOTABLE;
			e->flags &= ~NPC_IGNORESOLID;
		} /* fallthrough */
		case SUE_BASE+1:
		{
			ANIMATE(e, 20, STILL1,STILL2);
			FACE_PLAYER(e);
			
			e->x_speed = (e->x_speed << 1) + (e->x_speed << 2); e->x_speed >>= 3;
			e->y_speed = (e->y_speed << 1) + (e->y_speed << 2); e->y_speed >>= 3;
			
			if ((e->savedhp - e->health) > 50) {
				e->savedhp = e->health;
				sue_being_hurt = TRUE;	// trigger Misery to spawn monsters
			}
			
			if (++e->timer > TIME(80))
				e->state = SUE_PREPARE_ATTACK;
		}
		break;
		// prepare to attack
		case SUE_PREPARE_ATTACK:
		{
			e->state++;
			e->timer = 0;
			e->frame = PREPARE;
			
			e->x_speed = 0;
			e->y_speed = 0;
		} /* fallthrough */
		case SUE_PREPARE_ATTACK+1:
		{
			if (++e->timer > TIME(16)) {
				e->state = (e->timer2 ^= 1) ? SUE_SOMERSAULT : SUE_DASH;
				e->timer = 0;
			}
		}
		break;
		
		// somersault attack. this is the only time she can actually hurt you.
		
		case SUE_SOMERSAULT:
		{
			e->state++;
			e->timer = 0;
			e->attack = 4;
			e->frame = PREPARE;
			
			FACE_PLAYER(e);
			THROW_AT_TARGET(e, player.x, player.y, SPEED(0x600));
			set_ignore_solid(e);
		} /* fallthrough */
		case SUE_SOMERSAULT+1:
		{
			// passes through frame 3 (prepare/dash) before entering anim loop
			ANIMATE(e, 4, SPIN1,SPIN2,SPIN3,SPIN4);
			e->timer++;
			
			if (e->damage_time && e->timer > TIME(20)) {	// hurt fall
				e->state = SUE_SOMERSAULT_HIT;
				break;
			}
			
			// hit wall?
			// have to manually check ignore solid flag
			if (!(e->flags & NPC_IGNORESOLID) &&
					((blk(e->x, -12, e->y, 0) == 0x41) || (blk(e->x, 12, e->y, 0) == 0x41)))
				e->state = SUE_BASE;
			
			// timeout?
			if (e->timer > TIME(50)) 
				e->state = SUE_BASE;
			
			if ((e->timer & 7) == 1)
				sound_play(SND_CRITTER_FLY, 3);
		}
		break;
		// hit during somersault
		case SUE_SOMERSAULT_HIT:
		{
			e->state++;
			e->timer = 0;
			e->frame = STILL1;	// stop somersault; back to normal stand frame
			e->attack = 0;
			e->flags &= ~NPC_IGNORESOLID;
		} /* fallthrough */
		case SUE_SOMERSAULT_HIT+1:	// slowing down
		{
			e->x_speed = (e->x_speed << 1) + (e->x_speed << 2); e->x_speed >>= 3;
			e->y_speed = (e->y_speed << 1) + (e->y_speed << 2); e->y_speed >>= 3;
			
			if (++e->timer > TIME(6)) {
				e->state++;
				e->timer = 0;
				e->y_speed = -SPEED(0x200);
				MOVE_X(-SPEED(0x200));
			}
		}
		break;
		// falling/egads
		case SUE_SOMERSAULT_HIT+2:
		{
			e->frame = EGADS;	// egads!
			
			if (blk(e->x, 0, e->y, pixel_to_sub(e->hit_box.bottom)) == 0x41) {
				e->state++;
				e->timer = 0;
				e->frame = STILL1;	// standing
				e->y_speed = 0;
				FACE_PLAYER(e);
			} else {
				e->y_speed += SPEED(0x20);
				LIMIT_Y(SPEED(0x5ff));
			}
		}
		break;
		// hit ground: slide a bit then recover
		case SUE_SOMERSAULT_HIT+3:
		{
			if (++e->timer > TIME(16))
				e->state = 20;
		}
		break;
		
		// non-harmful dash. she cannot be hurt, but cannot hurt you, either.
		
		case SUE_DASH:
		{
			e->state++;
			e->timer = 0;
			e->frame = DASH;
			
			FACE_PLAYER(e);
			//e->eflags &= ~NPC_SHOOTABLE;
			e->flags &= ~NPC_SHOOTABLE;
			
			int32_t x;
			if (player.x < e->x) x = player.x - pixel_to_sub(160);
							 else x = player.x + pixel_to_sub(160);
			THROW_AT_TARGET(e, x, player.y, SPEED(0x600));
			set_ignore_solid(e);
		} /* fallthrough */
		case SUE_DASH+1:
		{
			// flash
			e->hidden = (++e->timer & 2) > 0;
			
			// hit wall?
			if (!(e->flags & NPC_IGNORESOLID) &&
					((blk(e->x, -12, e->y, 0) == 0x41) || (blk(e->x, 12, e->y, 0) == 0x41))) {
				e->state = SUE_BASE;
				e->hidden = FALSE;
			}
			
			// timeout?
			if (e->timer > TIME(50)) {
				e->state = SUE_BASE;
				e->hidden = FALSE;
			}
		}
		break;
	}
	
	// Stay within bounds
	if(e->y_speed > 0 && e->y > block_to_sub(stageHeight - 2)) e->y_speed = 0;
	if(e->y_speed < 0 && e->y < block_to_sub(2)) e->y_speed = 0;
	
	e->x += e->x_speed;
	e->y += e->y_speed;
}

// sets NPC_IGNORESOLID if the object is heading towards the center
// of the room, clears it otherwise.
static void set_ignore_solid(Entity *e) {
	int32_t map_right_half = block_to_sub(stageWidth) >> 1;
	int32_t map_bottom_half = block_to_sub(stageHeight) >> 1;
	
	e->flags &= ~NPC_IGNORESOLID;
	
	if ((e->x < map_right_half && e->x_speed > 0) ||
		(e->x > map_right_half && e->x_speed < 0)) {
		if ((e->y < map_bottom_half && e->y_speed > 0) ||
			(e->y > map_bottom_half && e->y_speed < 0)) {
			e->flags |= NPC_IGNORESOLID;
		}
	}
}

// shared between both Sue and Misery.
static void sidekick_run_defeated(Entity *e, uint16_t health) {
	// die if still around when core explodes
	if (e->state == SIDEKICK_CORE_DEFEATED_2) {
		if (!bossEntity) e->health = 0;
	}
	
	// trigger die
	if (e->health < (1000 - health)) {
		//e->eflags &= ~NPC_SHOOTABLE;
		e->flags &= ~NPC_SHOOTABLE;
		e->health = 9999;	// don't re-trigger
		e->state = SIDEKICK_DEFEATED;
	}
	
	switch(e->state) {
		// the script triggers this if you defeat the core
		// without killing one or both sidekicks.
		case SIDEKICK_CORE_DEFEATED:
		{
			if (e->health == 9999)
			{	// we were already dead when core was killed--ignore.
				e->state = SIDEKICK_DEFEATED+1;
			} else {
				//e->eflags &= ~NPC_SHOOTABLE;
				e->flags &= ~NPC_SHOOTABLE;
				e->health = 9999;
				
				e->x_speed = 0;
				e->y_speed = 0;
				e->frame = 10;
				
				e->state = SIDEKICK_CORE_DEFEATED_2; // cannot "state++"; that is SIDEKICK_DEFEATED
			}
		}
		break;
		
		case SIDEKICK_DEFEATED:
		{
			e->state++;
			e->frame = 10;
			e->attack = 0;
			//e->eflags &= ~NPC_SHOOTABLE;
			e->flags &= ~NPC_SHOOTABLE;
			e->flags |= NPC_IGNORESOLID;
			
			e->y_speed = -SPEED(0x200);
			e->damage_time += 50;
			
			if (e->type == OBJ_SUE_FRENZIED)
				sue_was_killed = TRUE;	// trigger Misery to start spawning missiles
		} /* fallthrough */
		case SIDEKICK_DEFEATED+1:
		{
			e->y_speed += SPEED(0x20);
			
			#define FLOOR	pixel_to_sub(((14 * 16) - 13))
			if (e->y_speed > 0 && e->y > FLOOR) {
				e->y = FLOOR;
				e->state++;
				e->frame = 11;
				e->x_speed = 0;
				e->y_speed = 0;
			}
		}
		break;
		
		case SIDEKICK_CORE_DEFEATED_2: break;
	}
}
