#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "effect.h"
#include "camera.h"
#include "system.h"
#include "sheet.h"
#include "resources.h"
#include "npc.h"

#define OMEGA_RISE_HEIGHT			48
#define OMEGA_SINK_DEPTH			60
#define OMEGA_WAIT_TIME				7
#define OMEGA_SPEED					(1<<CSF)

#define OMG_APPEAR					20  // this MUST be 20 because misery sets this to begin the battle
#define OMG_WAIT					30
#define OMG_MOVE					40
#define OMG_JAWS_OPEN				50
#define OMG_FIRE					60
#define OMG_JAWS_CLOSE				70
#define OMG_UNDERGROUND				80
#define OMG_JUMP					90
#define OMG_EXPLODING				100	// start fancy victory animation
#define OMG_EXPLODED				110 // full-screen flash in progress

#define LEGD_MIN				(25<<CSF)
#define LEGD_MAX				(46<<CSF)

#define OMEGA_DAMAGE			20
#define HP_TRIGGER_POINT		280

// Field aliases - wtf is a union?
#define omgorgx		curly_target_x
#define omgorgy		curly_target_y
#define omgmovetime	curly_target_time

#define nextstate		timer2
#define endfirestate	timer2
#define firecounter		deathSound
#define form			jump_time
#define firefreq		underwater
#define shotxspd		x_mark
#define leg_descend		y_mark

enum Pieces {
	LEFTLEG, RIGHTLEG,
	LEFTSTRUT, RIGHTSTRUT,
	NUM_PIECES
};

void onspawn_omega(Entity *e) {
	e->alwaysActive = TRUE;
	e->enableSlopes = FALSE;
	e->health = 400;
	e->eflags |= NPC_SHOWDAMAGE | NPC_SOLID;
	e->frame = 0;
	e->attack = 5;
	e->hurtSound = 52;
	e->hit_box = (bounding_box) { 32, 24, 32, 32 };
	e->display_box = (bounding_box) { 40, 32, 40, 32 };
	
	e->form = 1;
	e->firefreq = 3;
	e->shotxspd = 0x100;
	e->endfirestate = 200;
	omgmovetime = OMEGA_RISE_HEIGHT;
	
	// *MUST* create in this order so that the z-order is correct
	memset(pieces, 0, sizeof(pieces) * 4);
	pieces[LEFTLEG] = entity_create(0, 0, OBJ_OMEGA_LEG, 0);
	pieces[RIGHTLEG] = entity_create(0, 0, OBJ_OMEGA_LEG, 0);
	pieces[LEFTSTRUT] = entity_create(0, 0, OBJ_OMEGA_STRUT, 0);
	pieces[RIGHTSTRUT] = entity_create(0, 0, OBJ_OMEGA_STRUT, 0);
	
	pieces[LEFTLEG]->dir = 0;
	pieces[RIGHTLEG]->dir = 1;
	pieces[LEFTSTRUT]->dir = 0;
	pieces[RIGHTSTRUT]->dir = 1;
	
	e->leg_descend = LEGD_MIN;
	
	e->x = ((217 * 16) + 5) << CSF;
	e->y = ((14 * 16) - 5) << CSF;
	omgorgx = e->x;
	omgorgy = e->y;
	
	bossEntity = e;
	e->state = OMG_APPEAR;
}

void ai_omega(Entity *e) {
	switch(e->state) {
		case 0:	break;	// waiting for trigger by script
		case OMG_WAIT:	// waits for a moment then go to omg.nextstate
		{
			e->state++;
			e->timer = 0;
		}
		case OMG_WAIT+1:
		{
			if (++e->timer >= OMEGA_WAIT_TIME) {
				e->timer = 0;
				e->state = e->nextstate;
			}
		}
		break;
		
		case OMG_APPEAR:
		{
			e->timer = 0;
			e->frame = 0;
			e->state = OMG_MOVE;
			e->y_speed = -OMEGA_SPEED;
			e->eflags |= NPC_SOLID;
		}
		case OMG_MOVE:	// rising up/going back into ground
		{
			e->frame = 0;
			e->y += e->y_speed;
			camera_shake(2);
			e->timer++;
			if (!(e->timer & 3)) sound_play(SND_QUAKE, 3);
			
			if (e->timer >= omgmovetime) {
				if (e->y_speed < 0) {	// was rising out of ground
					e->nextstate = OMG_JAWS_OPEN;
					e->state = OMG_WAIT;
				} else {	// was going back into ground
					e->timer = 0;
					e->state = OMG_UNDERGROUND;
					e->eflags &= ~(NPC_SOLID | NPC_SPECIALSOLID);
				}
			}
		}
		break;
		case OMG_JAWS_OPEN:			// jaws opening
		{
			e->state++;
			e->animtime = 0;
			sound_play(SND_JAWS, 5);
			//e->sprite = SPR_OMG_OPENED;			// select "open" bounding box
		}
		case OMG_JAWS_OPEN+1:
		{
			e->animtime++;
			if (e->animtime > 3) {
				e->animtime = 0;
				e->frame++;
				if (e->frame >= 3) {
					e->state = OMG_FIRE;
					e->firecounter = 0;
					e->eflags |= NPC_SHOOTABLE;
				}
			}
		}
		break;
		case OMG_FIRE:	// throwing out red stuff
		{
			e->firecounter++;
			if (e->firecounter > 20 && e->firecounter < 80) {
				if ((e->firecounter % e->firefreq)==0) {
					sound_play(SND_EM_FIRE, 5);
					Entity *shot = entity_create(e->x, e->y, OBJ_OMEGA_SHOT, 0);
					shot->x_speed = random(-e->shotxspd, e->shotxspd);
					shot->y_speed = -0x333;
					if (e->form==2 || !(random() & 7)) {
						shot->frame = 0;
						shot->eflags = NPC_SHOOTABLE;
					} else {
						shot->frame = 1;
						shot->eflags = (NPC_SHOOTABLE | NPC_INVINCIBLE);
					}
					shot->timer = (random() % 7 >= 4) ? 300 + random() % 100 : 0;
					shot->attack = 4;
				}
			} else if (e->firecounter >= e->endfirestate || bullet_missile_is_exploding()) {
				// snap jaws shut
				e->animtime = 0;
				e->state = OMG_JAWS_CLOSE;
				sound_play(SND_JAWS, 5);
			}
		}
		break;
		case OMG_JAWS_CLOSE:	// jaws closing
		{
			e->animtime++;
			if (e->animtime > 3) {
				e->animtime = 0;
				e->frame--;
				if (e->frame == 0) {
					sound_play(SND_BLOCK_DESTROY, 6);
					//o->sprite = SPR_OMG_CLOSED;		// select "closed" bounding box
					e->eflags &= ~NPC_SHOOTABLE;
					e->attack = 0;
					if (e->form == 1) {	// form 1: return to sand
						e->state = OMG_WAIT;
						e->nextstate = OMG_MOVE;
						e->y_speed = OMEGA_SPEED;
						omgmovetime = OMEGA_SINK_DEPTH;
					} else {	// form 2: jump
						sound_play(SND_FUNNY_EXPLODE, 5);
						if (e->x < player.x) e->x_speed = 0xC0;
										 else e->x_speed = -0xC0;
						e->state = OMG_JUMP;
						e->y_speed = -0x5FF;
						omgorgy = e->y;
					}
				}
			}
			// hurt player if he was standing in the middle when the jaws shut
			if (playerPlatform == e) player_inflict_damage(OMEGA_DAMAGE);
		}
		break;
		case OMG_UNDERGROUND:		// underground waiting to reappear
		{
			if (++e->timer >= 120) {
				e->timer = 0;
				e->state = OMG_APPEAR;
				
				e->x = omgorgx + (-64) + ((random() % 128) << CSF);
				e->y = omgorgy;
				omgmovetime = OMEGA_RISE_HEIGHT;
				
				// switch to jumping out of ground when we get low on life
				if (e->form==1 && e->health <= HP_TRIGGER_POINT) {
					e->eflags |= NPC_SOLID;
					
					e->form = 2;
					e->firefreq = 5;
					e->shotxspd = 0x155;
					e->timer = 50; // Start firing immediately and for only 30 frames
					e->endfirestate = 50;
					omgmovetime = OMEGA_RISE_HEIGHT+3;
				}
			}
		}
		break;
		case OMG_JUMP:	// init for jump
		{
			omgorgy = e->y;
			e->state++;
			e->timer = 0;
		}
		case OMG_JUMP+1:	// jumping
		{
			e->y_speed += 0x24;
			LIMIT_Y(0x5FF);
			if (e->y_speed > 0) {	// coming down
				pieces[LEFTLEG]->frame = pieces[RIGHTLEG]->frame = 0;
				// retract legs a little when we hit the ground
				if (collide_stage_floor(pieces[LEFTLEG]) || 
						collide_stage_floor(pieces[RIGHTLEG])) {
					e->x_speed = 0;
					e->leg_descend -= e->y_speed;
					if (++e->timer >= 3) {
						e->y_speed = 0;
						e->state = OMG_JAWS_OPEN;
					}
				}
				// --- squash player if we land on him -------------
				if (player.grounded && player.y >= e->y + (16 << CSF)) {
					if (entity_overlapping(&player, e)) {	// SQUISH!
						player_inflict_damage(OMEGA_DAMAGE);
					}
				}
			} else {	// jumping up; extend legs
				e->leg_descend = (omgorgy - e->y) + LEGD_MIN;
				if (e->leg_descend > LEGD_MAX) e->leg_descend = LEGD_MAX;
				pieces[LEFTLEG]->frame = pieces[RIGHTLEG]->frame = 1;
			}
			e->x += e->x_speed;
			e->y += e->y_speed;
		}
		break;
		/// victory
		case OMG_EXPLODING:
		{
			e->timer = 0;
			e->state++;
		}
		case OMG_EXPLODING+1:
		{
			e->x_speed = e->y_speed = 0;
			if(e->timer & 1) SMOKE_AREA((e->x >> CSF) - 48, (e->y >> CSF) - 48, 96, 96, 1);
			camera_shake(2);
			
			if (!(e->timer % 12)) sound_play(SND_ENEMY_HURT_BIG, 5);
			
			if (++e->timer > 100) {
				e->timer = 0;
				//starflash.Start(o->CenterX(), o->CenterY());
				e->state = OMG_EXPLODED;
			} else if (e->timer==24) {
				tsc_call_event(210);
			}
		}
		break;
		case OMG_EXPLODED:
		{
			camera_shake(40);
			if (++e->timer > 50) {
				e->state = STATE_DELETE;
				for(int i=0;i<NUM_PIECES;i++) pieces[i]->state = STATE_DELETE;
			}
		}
		break;
	}
	
	// implement shaking when shot
	if(e->damage_time) e->x += e->damage_time & 1 ? 0x200 : -0x200;
	
	if (e->state) {
		e->x_next = e->x;
		e->y_next = e->y;
		collide_stage_leftwall(e);
		collide_stage_rightwall(e);
		e->x = e->x_next;
		pieces[LEFTLEG]->x = e->x - (4 << CSF); pieces[LEFTLEG]->y = e->y + e->leg_descend;
		pieces[RIGHTLEG]->x = e->x + (38 << CSF); pieces[RIGHTLEG]->y = e->y + e->leg_descend;
		pieces[LEFTSTRUT]->x = e->x + (9 << CSF); pieces[LEFTSTRUT]->y = e->y + (27 << CSF);
		pieces[RIGHTSTRUT]->x = e->x + (43 << CSF); pieces[RIGHTSTRUT]->y = e->y + (27 << CSF);
	}
}

void ondeath_omega(Entity *e) {
	e->eflags &= ~NPC_SHOOTABLE;
	entities_clear_by_type(OBJ_OMEGA_SHOT);
	e->state = OMG_EXPLODING;
	bossEntity = NULL;
}

void ai_omega_shot(Entity *e) {
	e->y_speed += 5;
	if (e->y_speed > 0 && collide_stage_floor(e)) e->y_speed = -0x100;
	if (e->y_speed > 0 && collide_stage_ceiling(e)) e->y_speed = -e->y_speed;
	if ((e->x_speed < 0 && stage_get_block_type(((e->x<<CSF)-8) / 16, (e->y<<CSF) / 16)) ||
		(e->x_speed > 0 && stage_get_block_type(((e->x<<CSF)+8) / 16, (e->y<<CSF) / 16))) {
		e->x_speed = -e->x_speed;
	}
	if (++e->animtime > 2) { e->frame ^= 1; e->animtime = 0; }
	if (++e->timer > 750) {
		//effect(o->CenterX(), o->CenterY(), EFFECT_FISHY);
		e->state = STATE_DELETE;
	}
}
