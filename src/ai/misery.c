#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "vdp_ext.h"
#include "effect.h"

void ai_misery_float(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->x_next += (1 << CSF);	// check Undead Core intro to prove this is correct
			e->x_mark = e->x_next;
			e->y_mark = e->y_next;
			e->frame = 0;
			e->timer = 0;
		}
		/* no break */
		case 1:
		{
			//if (DoTeleportIn(o, 1)) {
				e->state = 10;
			//}
		}
		break;
		case 10:	// floating
		{
			e->state = 11;
			e->timer = 0;
			e->y_speed = SPEED(1 << CSF);
		}
		/* no break */
		case 11:
		{
			e->frame = 0;
			RANDBLINK(e, 1, 200);
			if (e->y_next > e->y_mark) e->y_speed -= SPEED(16);
			if (e->y_next < e->y_mark) e->y_speed += SPEED(16);
			if (e->y_speed > SPEED(0x100)) e->y_speed = SPEED(0x100);
			if (e->y_speed < SPEED(-0x100)) e->y_speed = SPEED(-0x100);
		}
		break;
		case 13:	// fall from floaty
			e->frame = 2;
			e->y_speed += SPEED(0x40);
			LIMIT_Y(SPEED(0x5FF));
			
			if ((e->grounded = collide_stage_floor(e))) {
				sound_play(SND_THUD, 5);
				e->state = 14;
			}
		break;
		case 14: break;			// standing
		// spawn the bubble which picks up Toroko in Shack
		case 15:
		{
			e->frame = 4;
			e->timer = 0;
			e->state = 16;
		}
		/* no break */
		case 16:
		{
			if (++e->timer >= TIME(20)) {
				sound_play(SND_BUBBLE, 5);
				entity_create(e->x, e->y - (16<<CSF), OBJ_MISERYS_BUBBLE, 0);
				e->state = 17;
				e->timer = 0;
			}
		}
		break;
		case 17:
		{
			if (++e->timer >= TIME(50)) e->state = 14;
		}
		/* no break */
		case 20: 	// fly away
		{
			e->state = 21;
			e->frame = 0;
			e->y_speed = 0;
		}
		/* no break */
		case 21:
		{
			e->y_speed -= SPEED(0x20);
			if (e->y_next < -0x1000) e->state = STATE_DELETE;
		}
		break;
		case 25:	// big spell
		{
			e->state = 26;
			e->timer = 0;
			e->frame = 4;
		}
		/* no break */
		case 26:	// she flashes, then a clap of thunder
		{
			ANIMATE(e, 2, 4, 5);
			if (++e->timer >= TIME(20)) {
				sound_play(SND_LIGHTNING_STRIKE, 5);
				SCREEN_FLASH(10);
				e->state = 27;
				e->timer = 0;
			}
		}
		break;
		case 27:	// return to standing after lightning strike
		{
			if (++e->timer > TIME(16)) e->state = 14;
		}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_misery_stand(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		case 0:
		{
			e->state = 1;
		}
		case 1:
		{
			e->frame = 2;
			RANDBLINK(e, 3, 200);
		}
		break;
		case 20:	// she flys away
		{
			e->state = 21;
			e->frame = 0;
			e->y_speed = 0;
			e->eflags |= NPC_IGNORESOLID;
		}
		case 21:
		{
			e->y_speed -= 0x20;
			if (e->y < -0x1000) e->state = STATE_DELETE;
		}
		break;
		// big spell
		// she flashes, then a clap of thunder,
		// and she teleports away.
		case 25:
		{
			e->state = 26;
			e->timer = 0;
			e->frame = 5;
			e->animtime = 0;
		}
		case 26:
		{
			ANIMATE(e, 2, 4, 5);
			if (++e->timer >= TIME(20)) {
				sound_play(SND_LIGHTNING_STRIKE, 5);
				// Flash screen white
				VDP_flashWhite();
				e->state = 27;
				e->timer = 0;
			}
		}
		break;
		case 27:
		{
			if (++e->timer > 50) {	
				// return to standing
				e->state = 0;
			}
		}
		break;
		case 30:	// she throws up her staff like she's summoning something
		{
			e->timer = 0;
			e->state++;
			e->frame = 2;
		}
		case 31:
		{
			if (e->timer==10) e->frame = 4;
			if (e->timer==130) e->state = 1;
			e->timer++;
		}
		break;
		// fire at DOCTOR_GHOST
		case 40:
		{
			e->state = 41;
			e->timer = 0;
			e->frame = 4;
		}
		case 41:
		{
			e->timer++;
			
			if (e->timer == 30 || \
				e->timer == 40 || \
				e->timer == 50)
			{
				Entity *shot = entity_create(e->x+(16<<CSF), e->y, OBJ_IGOR_SHOT, 0);
				shot->x_speed = 0x600;
				shot->y_speed = -(random() % 0x200);
				
				sound_play(SND_SNAKE_FIRE, 5);
			}
			
			if (e->timer > 50)
				e->state = 0;
		}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_misery_bubble(Entity *e) {
	// find the Toroko object we are to home in on
	Entity *target = entity_find_by_type(0x3C);
	if(!target) {
		e->state = STATE_DELETE;
		return;
	} else if(e->y < 0) {
		e->state = STATE_DELETE;
		entity_delete(target);
		return;
	}
	switch(e->state) {
		case 0:
		{
			// Wait a bit
			if(++e->timer > TIME(20)) e->state = 1;
		}
		break;
		case 1:
		{
			// Calculate the speed it will take to reach the target in 1 second
			// Genesis can't divide 32 bit integers so here is a fun hack have fun deciphering it
			e->x_speed = (((s32)((u16)(abs(target->x - e->x) >> 5)) / TIME(50))) << 5;
			e->y_speed = (((s32)((u16)(abs(target->y - e->y) >> 5)) / TIME(50))) << 5;
			if(e->x > target->x) e->x_speed = -e->x_speed;
			if(e->y > target->y) e->y_speed = -e->y_speed;
			e->state = 2;
			e->timer = 0;
		}
		/* no break */
		case 2:
		{
			e->x += e->x_speed;
			e->y += e->y_speed;
			// Did we reach the target?
			if(++e->timer == TIME(50)) {
				sound_play(SND_BUBBLE, 5);
				e->state = 3;
				e->x = target->x;
				e->y = target->y;
				e->x_speed = 0;
				e->y_speed = 0;
				target->state = 500;
			}
		}
		break;
		case 3: // Carry Toroko away
		{
			e->y_speed -= SPEED(0x1C);
			e->y += e->y_speed;
			target->x = e->x + 0x200;
			target->y = e->y - 0x200;
		}
		break;
	}
}

#define STATE_FIGHTING			100		// script-triggered, must be constant

#define STATE_FLASH_FOR_SPELL	200		// flashes then casts either FIRE_SHOTS or SUMMON_BLOCk
#define STATE_FIRE_SHOTS		210		// fires black shots at player
#define STATE_SUMMON_BLOCK		220		// summons falling block over player's head
#define STATE_SUMMON_BALLS		230		// summons black-lightning balls
#define STATE_TP_AWAY			240		// teleports away then reappears

#define STATE_DEFEATED			1000	// script-triggered, must be constant

#define savedhp		curly_target_time

static void run_intro(Entity *e);
static void run_spells(Entity *e);
static void run_teleport(Entity *e);
static void run_defeated(Entity *e);

static Entity* CreateRing(Entity *e, u8 angle);

void ai_boss_misery(Entity *e) {
	switch(e->state) {
		// fight begin and default/base state
		case STATE_FIGHTING:
		{
			e->eflags |= NPC_SHOOTABLE;
			savedhp = e->health;
			
			e->timer = 0;
			e->frame = 0;
			e->x_speed = 0;
			e->state++;
		}
		case STATE_FIGHTING+1:
		{
			FACE_PLAYER(e);
			
			e->y_speed += (e->y < e->y_mark) ? SPEED(0x20) : -SPEED(0x20);
			LIMIT_Y(SPEED(0x200));
			
			if (++e->timer > TIME(200) || (e->health - savedhp) >= 80) {
				e->state = STATE_FLASH_FOR_SPELL;
				e->timer = 0;
			}
		}
		break;
	}
	
	run_spells(e);
	run_teleport(e);
	
	run_intro(e);
	run_defeated(e);
	
	LIMIT_X(SPEED(0x200));
	LIMIT_Y(SPEED(0x400));
}

void ondeath_boss_misery(Entity *e) {
	e->eflags &= ~NPC_SHOOTABLE;
	e->attack = 0;
	tsc_call_event(e->event);
}

// her 3 attacks: black shots, black balls, and summon falling block.
static void run_spells(Entity *e) {
	switch(e->state) {
		// flashes for spell...
		// then either fires shots or casts the falling-block spell
		case STATE_FLASH_FOR_SPELL:
		{
			e->eflags &= ~NPC_SHOOTABLE;
			e->x_speed = 0;
			e->y_speed = 0;
			
			e->timer = 0;
			e->state++;
		}
		case STATE_FLASH_FOR_SPELL+1:
		{
			e->timer++;
			e->frame = 5 + (e->timer & 1);
			
			if (e->timer > 30) {
				e->timer = 0;
				e->frame = 4;
				
				if (++e->timer >= 3) {
					e->state = STATE_SUMMON_BLOCK;
					e->timer2 = 0;
				} else {
					e->state = STATE_FIRE_SHOTS;
				}
			}
		}
		break;
		
		// fire black shots at player
		case STATE_FIRE_SHOTS:
		{
			if ((++e->timer & 7) == 0) {
				u8 angle = get_angle(e->x, e->y, player.x, player.y);
				Entity *shot = entity_create(e->x, e->y, OBJ_MISERY_SHOT, 0);
				angle -= 3;
				angle += random() & 7;
				shot->x_speed = (cos[angle] * SPEED(0x800)) >> CSF;
				shot->y_speed = (sin[angle] * SPEED(0x800)) >> CSF;
				sound_play(SND_FIREBALL, 3);
			}
			
			if (e->timer > 40) {
				e->timer = 0;
				e->state = STATE_TP_AWAY;
			}
		}
		break;
		
		// summon falling block
		case STATE_SUMMON_BLOCK:
		{
			if (++e->timer == 10) {
				//int x = player->x - (8 << CSF);
				//int y = player->y - (64 << CSF);
				//CreateObject(x, y, OBJ_FALLING_BLOCK);
				//e->sprite = SPR_BALCONY_BLOCK_LARGE;
				//e->dir = DOWN;	// tell block it was spawned by Misery
			}
			
			if (e->timer > 30) {
				e->state = STATE_TP_AWAY;
				e->timer = 0;
			}
		}
		break;
		
		// summon black balls
		case STATE_SUMMON_BALLS:
		{
			FACE_PLAYER(e);
			e->frame = 4;
			
			e->timer = 0;
			e->state++;
		}
		case STATE_SUMMON_BALLS+1:
		{
			e->y_speed += (e->y < e->y_mark) ? SPEED(0x20) : -SPEED(0x20);
			LIMIT_Y(SPEED(0x200));
			
			if ((++e->timer % 24) == 0) {
				entity_create(e->x, e->y+(4<<CSF), OBJ_MISERY_BALL, 0);
				sound_play(SND_FIREBALL, 3);
			}
			
			if (e->timer > 72) {
				e->state = 100;
				e->timer = 0;
			}
		}
		break;
	}
}


// runs her teleport-away and reappear states.
static void run_teleport(Entity *e) {
	switch(e->state) {
		// teleport away, then reappear someplace else
		case STATE_TP_AWAY:
		{
			e->state++;
			e->timer = 0;
			e->hidden = TRUE;
			e->eflags &= ~NPC_SHOOTABLE;
			
			//CreateObject(e->x, e->y, OBJ_MISERY_PHASE)->dir = LEFT;
			//CreateObject(e->x, e->y, OBJ_MISERY_PHASE)->dir = RIGHT;
			
			sound_play(SND_TELEPORT, 5);
		}
		case STATE_TP_AWAY+1:
		{
			e->timer++;
			
			// it takes exactly 8 frames for the phase-in animation to complete
			if (e->timer == 42) {
				// we don't actually move until the last possible second
				// in order not to bring the floattext/damage numbers with us,
				// which gives away our position.
				e->x_mark = block_to_sub(9 + (random() % 23));
				e->y_mark = block_to_sub(5 + (random() % 3));
				
				//CreateObject(e->x_mark + 0x2000, e->y_mark, OBJ_MISERY_PHASE)->dir = LEFT;
				//CreateObject(e->x_mark - 0x2000, e->y_mark, OBJ_MISERY_PHASE)->dir = RIGHT;
			} else if (e->timer == 50) {
				// switch back to showing real misery instead of the phase-in effect
				e->eflags |= NPC_SHOOTABLE;
				e->hidden = FALSE;
				e->frame = 0;
				e->dir = 0;
				
				e->x = e->x_mark;
				e->y = e->y_mark;
				
				// spawn rings
				if (e->health < 340) {
					CreateRing(e, A_RIGHT);
					CreateRing(e, A_LEFT);
					
					if (e->health < 180) {
						CreateRing(e, A_UP);
						CreateRing(e, A_DOWN);
					}
				}
				
				// after tp we can summon the black balls if the player
				// is far enough away from us that they won't immediately trigger
				if (abs(player.x - e->x) > 112<<CSF) {
					e->state = STATE_SUMMON_BALLS;
				} else {
					e->state = STATE_FIGHTING;
				}
				
				// setup sinusoidal hover, both of those possible states
				// are in-air states that do it.
				e->timer = 0;
				e->y_speed = -SPEED(0x200);
				// counteracts y_speed of first visible frame, so it's a
				// seamless transition from the phase-in effect.
				e->y += SPEED(0x220);
			}
		}
		break;
	}
}


// intro states: stuff that happens before the fight actually starts.
static void run_intro(Entity *e) {
	switch(e->state) {
		case 0:
		{
			// fixes her position on throne; don't use a spawn point or it'll
			// glitch when she turns to misery_stand in defeated cinematic
			//e->y += (6 << CSF);
			
			// her initial target height when fight begins
			e->y_mark = (64 << CSF);
			
			e->state = 1;
		}
		case 1:
		{
			e->frame = 0;
			RANDBLINK(e, 1, 200);
		}
		break;
		
		case 20:		// fall from throne (script-triggered)
		{
			if (e->grounded) {
				e->state = 21;
				e->frame = 2;
			} else {
				e->y_speed += SPEED(0x40);
			}
		}
		break;
		case 21:		// standing/talking after fall from throne
		{
			e->frame = 2;
			RANDBLINK(e, 3, 200);
		}
		break;
	}
}


// defeated states--they're all run by the ondeath script.
static void run_defeated(Entity *e) {
	// these states are all script-triggered and must be constant.
	switch(e->state) {
		// defeated! "gaah" in air
		case 1000:
		{
			e->eflags &= ~NPC_SHOOTABLE;
			entities_clear_by_type(OBJ_MISERY_RING);
			//SmokeClouds(o, 3, 2, 2);
			
			e->x_speed = 0;
			e->y_speed = 0;
			
			e->state = 1001;
			e->timer = 0;
			e->frame = 4;
			e->x_mark = e->x;
		}
		case 1001:		// shake until script tells us otherwise
		{
			e->x = e->x_mark;
			if (++e->timer & 2)
				e->x += (1 << CSF);
		}
		break;
		
		case 1010:		// fall to ground and do defeated frame: "ergh"
		{
			e->y_speed += 10;
			LIMIT_Y(SPEED(0x400));
			e->y += e->y_speed;
			if (blk(e->x, 0, e->y, 8) == 0x41) {
				e->frame = 7;
				e->state = 1011;
			}
		}
		break;
	}
}

static Entity *CreateRing(Entity *e, u8 angle) {
	Entity *ring = entity_create(0, 0, OBJ_MISERY_RING, 0);
	ring->jump_time = angle;
	ring->linkedEntity = e;
	
	return ring;
}

void ai_misery_ring(Entity *e) {
	if (!e->linkedEntity) {
		effect_create_smoke(e->x >> CSF, e->y >> CSF);
		e->state = STATE_DELETE;
		return;
	}
	
	switch(e->state) {
		case 0:
		{
			e->frame = 3;
			e->state = 1;
			e->timer = 0;
		}
		case 1:
		{
			// distance from misery
			if (e->timer < 192) e->timer++;
			
			// angle
			e->jump_time += 2;
			e->x = e->linkedEntity->x + cos[e->jump_time] * e->timer;
			e->y = e->linkedEntity->y + sin[e->jump_time] * e->timer;
			
			// turn to bats when misery teleports
			if (e->linkedEntity->state >= STATE_TP_AWAY &&
				e->linkedEntity->state < STATE_TP_AWAY+10) {
				e->state = 10;
			}
		}
		break;
		
		case 10:	// transform to bat
		{
			e->eflags |= NPC_SHOOTABLE;
			e->eflags &= ~NPC_INVINCIBLE;
			
			THROW_AT_TARGET(e, player.x, player.y, SPEED(0x200));
			FACE_PLAYER(e);
			
			// Change to bat sprite
			SHEET_FIND(e->sheet, SHEET_BAT);
			e->frame = 0;
			e->oframe = 255;
			
			e->state = 11;
		}
		case 11:
		{
			ANIMATE(e, 4, 0,1,2);
			// Disappear when touching with any solid blocks
			if (blk(e->x, 0, e->y, 0) == 0x41) {
				effect_create_smoke(e->x >> CSF, e->y >> CSF);
				e->state = STATE_DELETE;
			}
		}
		break;
	}
}

// this is her phasy teleport out/teleport in effect
// it's a 2-dir interlaced picture of her with each dir
// containing only half the lines. We spawn two objects
// in opposite dirs and then separate them.
//void ai_misery_phase(Object *o)
//{
//	XMOVE(0x400);
//	if (++e->timer >= 8) e->state = STATE_DELETE;
//}


void ai_misery_ball(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->y_mark = e->y;
			e->x_speed = 0;
			e->y_speed = -0x200;
		}
		case 1:
		{
			ANIMATE(e, 2, 0,1);
			
			e->x_speed += (e->x < player.x) ? 0x10 : -0x10;
			e->y_speed += (e->y < e->y_mark) ? 0x20 : -0x20;
			LIMIT_X(0x200);
			LIMIT_Y(0x200);
			
			if (PLAYER_DIST_X(8<<CSF) && player.y > e->y) {
				e->state = 10;
				e->timer = 0;
			}
		}
		break;
		
		case 10:	// black lightning
		{
			if (++e->timer > 10) {
				sound_play(SND_LIGHTNING_STRIKE, 5);
				entity_create(e->x, e->y, OBJ_BLACK_LIGHTNING, 0);
				e->state = STATE_DELETE;
			}
			
			e->frame = (e->timer & 2) ? 2 : 1;
		}
		break;
	}
	
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_black_lightning(Entity *e) {
	ANIMATE(e, 2, 0,1);
	if (blk(e->x, 0, e->y, 15) == 0x41) {
		//effect(e->CenterX(), e->Bottom(), EFFECT_BOOMFLASH);
		effect_create_smoke(e->x >> CSF, e->y >> CSF);
		e->state = STATE_DELETE;
	}
	e->y += SPEED(0x1000);
}
