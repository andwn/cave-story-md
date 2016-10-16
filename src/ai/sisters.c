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
#include "sprite.h"

// mainstates
#define STATE_CIRCLE_1		100		// circling right
#define STATE_CIRCLE_0		110		// circling left
#define STATE_MEGAFIRING		120		// runs pause and circling during low-hp megafire
#define STATE_DEFEATED			1000	// script-triggered; must be constant
#define STATE_DEFEATED_CRASH	1010
#define STATE_STARFLASH			1020

// head states
#define STATE_HEAD_CLOSED		10		// closed, but periodically opens by itself and fires
#define STATE_HEAD_OPEN			20		// mouth open, can be shot
#define STATE_HEAD_FIRE			30		// actually firing
#define STATE_HEAD_MEGAFIRE		40		// firing a whole lot during low-hp
#define STATE_HEAD_BIT_TONGUE	50		// bit tongue after got shot
#define STATE_HEAD_DEFEATED		100		// defeated, eyes closed

// body states
#define STATE_BODY_NOMOVE		10		// do nothing but animate
#define STATE_BODY_FACE_PLAYER	20		// follow angle set by main and face player
#define STATE_BODY_FACE_CENTER	30		// follow angle set by main and face center
#define STATE_BODY_LOCK_DIR		40		// follow angle set by main but don't change direction

#define SISTERS_HP				500		// total HP
#define SISTERS_ATTACK2_HP		300		// HP below which we can do the lots-of-fire attack
#define SISTERS_DAMAGE			10		// how much damage you can take if you run into one

enum Pieces {
	BODY1, BODY2, HEAD1, HEAD2
};

// coordinates of per-frame bboxes for each frame of the head object.
// these are all for the right-facing frame and are automatically flipped-over
// at runtime if the dragon is facing left.
const struct {
	u8 left, top, right, bottom;
	u16 flags;
} head_bboxes[] = {
	{ 12, 12, 12, 12,  NPC_SHOOTABLE | NPC_INVINCIBLE },	// closed
	{ 12, 12, 12, 12,  NPC_SHOOTABLE | NPC_INVINCIBLE },	// partway open
	{ 12, 12, 12, 12,  NPC_SHOOTABLE },		// open (shootable)
	{ 12, 12, 12, 12,  NPC_SHOOTABLE },		// bit tongue (shootable)
	{ 12, 12, 12, 12,  0 }					// eyes closed (shots pass through)
};

#define mainangle	curly_target_x

static void SetHeadStates(u16 newstate) {
	for(u8 i=0;i<2;i++)
		pieces[HEAD1+i]->state = newstate;
}

static void SetBodyStates(u16 newstate) {
	for(u8 i=0;i<2;i++)
		pieces[BODY1+i]->state = newstate;
}

// select an appropriate bbox for the current frame
static void head_set_bbox(Entity *e) {
	e->hit_box.top = head_bboxes[e->frame].top;
	e->hit_box.bottom = head_bboxes[e->frame].bottom;
	if(!e->dir) {
		e->hit_box.left = head_bboxes[e->frame].right;
		e->hit_box.right = head_bboxes[e->frame].left;
	} else {
		e->hit_box.left = head_bboxes[e->frame].left;
		e->hit_box.right = head_bboxes[e->frame].right;
	}
	e->eflags &= ~(NPC_SHOOTABLE | NPC_INVINCIBLE);
	e->eflags |= head_bboxes[e->frame].flags;
}

void onspawn_sisters(Entity *e) {
	e->alwaysActive = TRUE;
	e->x = (10*16)<<CSF;
	e->y = ((8*16)-4)<<CSF;
	e->hidden = TRUE;
	
	mainangle = 0;
	e->x_mark = 180;
	e->y_mark = 61;
	e->timer2 = (random() % TIME(500)) + TIME(700);
	e->health = 500;
	e->event = 1000;
	e->eflags |= NPC_EVENTONDEATH;
	
	// Create body before head, so the head will be on top
	pieces[BODY1] = entity_create((64<<CSF), 80<<CSF, OBJ_SISTERS_BODY, 0);
	pieces[HEAD1] = entity_create((64<<CSF), 64<<CSF, OBJ_SISTERS_HEAD, 0);
	pieces[HEAD1]->linkedEntity = pieces[BODY1];
	pieces[BODY1]->linkedEntity = pieces[HEAD1];
	
	pieces[BODY2] = entity_create((64<<CSF) + (50<<CSF), 80<<CSF, OBJ_SISTERS_BODY, NPC_OPTION2);
	pieces[HEAD2] = entity_create((64<<CSF) + (50<<CSF), 64<<CSF, OBJ_SISTERS_HEAD, NPC_OPTION2);
	pieces[HEAD2]->linkedEntity = pieces[BODY2];
	pieces[BODY2]->linkedEntity = pieces[HEAD2];
}

void onspawn_sisters_body(Entity *e) {
	e->alwaysActive = TRUE;
	e->attack = SISTERS_DAMAGE;
	e->display_box = (bounding_box) { 16, 20, 16, 20 };
	e->hit_box = (bounding_box) { 12, 16, 12, 16 };
}

void onspawn_sisters_head(Entity *e) {
	e->alwaysActive = TRUE;
	e->attack = SISTERS_DAMAGE;
	e->health = 1000;
	e->hurtSound = SND_ENEMY_HURT_COOL;
	e->damage_time = 10;
	sound_play(e->hurtSound, 5);
	e->display_box = (bounding_box) { 16, 16, 16, 16 };
}

void ondeath_sisters(Entity *e) {
	//e->state = STATE_DEFEATED; // <BOA does this for us
	tsc_call_event(e->event);
}

void ai_sisters(Entity *e) {
	switch(e->state) {
		case 20:	// fight begin (script-triggered)
		{
			if (++e->timer > TIME(68)) {
				e->x_mark = 112;		// bodies zoom onto screen via force of their interpolation
				e->timer = 0;
				
				e->state = STATE_CIRCLE_1;			// main begins turning angle
				SetHeadStates(STATE_HEAD_CLOSED);		// periodically opens/fires by itself
				SetBodyStates(STATE_BODY_FACE_PLAYER);	// enable body movement
			}
		}
		break;
		
		case STATE_CIRCLE_1:
		{
			e->timer++;
			
			if (e->timer < TIME(100))					mainangle += 1;
			else if (e->timer < TIME(120))				mainangle += 2;
			else if (e->timer < e->timer2)				mainangle += 4;
			else if (e->timer < e->timer2 + TIME(40))	mainangle += 2;
			else if (e->timer < e->timer2 + TIME(60))	mainangle += 1;
			else {
				e->timer = 0;
				e->state = STATE_CIRCLE_0;
				e->timer2 = (random() % TIME(300)) + TIME(400);
			}
		}
		break;
		
		case STATE_CIRCLE_0:
		{
			e->timer++;
			
			if (e->timer < TIME(20))					mainangle -= 1;
			else if (e->timer < TIME(60))				mainangle -= 2;
			else if (e->timer < e->timer2)				mainangle -= 4;
			else if (e->timer < e->timer2 + TIME(40))	mainangle -= 2;
			else if (e->timer < e->timer2 + TIME(60))	mainangle -= 1;
			else {
				if (e->health < SISTERS_ATTACK2_HP) {
					e->state = STATE_MEGAFIRING;
					SetHeadStates(STATE_HEAD_MEGAFIRE);
					e->timer = 0;
				} else {
					e->state = STATE_CIRCLE_1;
					e->timer2 = (random() % TIME(300)) + TIME(400);
					e->timer = 0;
				}
			}
		}
		break;
		
		// runs the circling while the heads are doing their low-hp "megafire" state.
		// first they stop completely, then spin around and around clockwise for a while.
		case STATE_MEGAFIRING:
		{
			if (++e->timer > TIME(100)) {
				e->state++;
				e->timer = 0;
			}
		}
		break;
		case STATE_MEGAFIRING+1:
		{
			e->timer++;
			
			if (e->timer < TIME(100))	   mainangle += 1;
			else if (e->timer < TIME(120)) mainangle += 2;
			else if (e->timer < TIME(500)) mainangle += 4;
			else if (e->timer < TIME(540)) mainangle += 2;
			else if (e->timer < TIME(560)) mainangle += 1;
			else
			{
				e->state = STATE_CIRCLE_0;
				SetHeadStates(STATE_HEAD_CLOSED);
				e->timer = 0;
			}
		}
		break;
		
		// defeated!! make big smoke and pause a second
		// this state is entered via a <BOA in the ondeath script.
		case STATE_DEFEATED:
		{
			//SpawnScreenSmoke(40);
			
			for(u16 i=0;i<2;i++) {
				pieces[HEAD1+i]->attack = 0;
				pieces[BODY1+i]->attack = 0;
			}
			
			SetHeadStates(STATE_HEAD_DEFEATED);
			SetBodyStates(STATE_BODY_FACE_CENTER);
			
			e->state++;
			e->timer = 0;
		}
		case STATE_DEFEATED+1:
		{
			if (++e->timer > TIME(100)) {
				e->state = STATE_DEFEATED_CRASH;
				e->timer = 0;
			}
			
			//SpawnScreenSmoke(1);
		}
		break;
		
		// the two dragons crash into each other
		case STATE_DEFEATED_CRASH:
		{
			mainangle += 4;
			
			if (e->x_mark > 8) e->x_mark--;
			if (e->y_mark > 0) e->y_mark--;
			
			if (++e->timer == 40)
				SetBodyStates(STATE_BODY_LOCK_DIR);
			
			if (e->y_mark == 0) {
				if (entity_overlapping(pieces[HEAD1], pieces[HEAD2]) ||
					entity_overlapping(pieces[HEAD1], pieces[BODY2]) ||
					entity_overlapping(pieces[HEAD2], pieces[BODY1])) {
					//starflash.Start(e->CenterX(), e->CenterY());
					sound_play(SND_EXPLOSION1, 5);
					
					e->state = STATE_STARFLASH;
					e->timer = 0;
				} else {
					e->x_mark -= 2;
					SetBodyStates(STATE_BODY_LOCK_DIR);	// lock direction facing
				}
			}
		}
		break;
		
		// big starflash after dragons hit each other
		case STATE_STARFLASH:
		{
			if (++e->timer > TIME(30)) {
				entities_clear_by_type(OBJ_DRAGON_ZOMBIE_SHOT);
				entities_clear_by_type(OBJ_SPIKE_SMALL);
				
				for(u8 i=0;i<2;i++) {
					pieces[HEAD1+i]->state = STATE_DELETE;
					pieces[BODY1+i]->state = STATE_DELETE;
				}
				
				e->state = STATE_DELETE;
				bossEntity = NULL;
				
				return;
			}
		}
		break;
	}
	
	//mainangle %= 1024;
}

void ai_sisters_body(Entity *e) {
	s32 angle = mainangle;
	if(e->eflags & NPC_OPTION2) angle += 512;
	
	// main's x_mark and y_mark tell us how far from the center to circle
	s32 xoff = (sintab32[(angle) % 1024] >> 1) * (bossEntity->x_mark); // <<CSF
	s32 yoff = (sintab32[(angle + 256) % 1024] >> 1) * (bossEntity->y_mark); // <<CSF
	
	// figure out where we are supposed to be
	s32 desired_x = bossEntity->x + xoff;
	s32 desired_y = bossEntity->y + yoff;
	
	// motion
	if (e->state == 0) { // this places them offscreen before the fight
		e->x = desired_x;
		e->y = desired_y;
		e->state = STATE_BODY_NOMOVE;
	} else if (e->state != STATE_BODY_NOMOVE) {	
		// smooth interpolation, they stay in this state throughout the fight
		e->x += (desired_x - e->x) / 8;
		e->y += (desired_y - e->y) / 8;
	}
	
	// set direction facing
	if (e->state != STATE_BODY_LOCK_DIR) {
		if (e->state == STATE_BODY_FACE_CENTER) {
			if (e->x > bossEntity->x) e->dir = 0;
			else e->dir = 1;
		} else {
			FACE_PLAYER(e);
		}
	}
	
	ANIMATE(e, 4, 0,1,2);
}

void ai_sisters_head(Entity *e) {
	// stay connected to body
	e->dir = e->linkedEntity->dir;
	e->x = e->linkedEntity->x + ((e->dir==1) ? (4 << CSF) : -(4 << CSF));
	e->y = e->linkedEntity->y - (4 << CSF);
	
	// link hp to main object
	if (e->health < 1000) {
		if(bossEntity->health <= 1000 - e->health) {
			bossEntity->health = 0;
			ENTITY_ONDEATH(bossEntity);
		} else {
			bossEntity->health -= 1000 - e->health;
		}
		e->health = 1000;
	}
	
	// FSM
	// Thanks I didn't notice this was a state machine
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->eflags |= (NPC_SHOOTABLE | NPC_INVINCIBLE);
		}
		break;
		
		case STATE_HEAD_CLOSED:
		{
			e->frame = 0;
			e->timer = (random() % TIME(100)) + TIME(100);
			e->state++;
		}
		case STATE_HEAD_CLOSED+1:
		{
			if (--e->timer <= 0) {
				e->state = STATE_HEAD_OPEN;
				e->timer = 0;
			}
		}
		break;
		
		// open mouth and prepare to fire
		case STATE_HEAD_OPEN:
		{
			e->timer = 0;
			e->timer2 = 0;
			e->state++;
		}
		case STATE_HEAD_OPEN+1:
		{
			e->timer++;
			if (e->timer == 3) e->frame = 1;	// mouth partially open--about to fire!!
			if (e->timer == 6) e->frame = 2;	// mouth fully open
			
			if (e->timer > TIME(150)) {	// begin firing if they haven't hit us by now
				e->state = STATE_HEAD_FIRE;
				e->timer = 0;
			}
			
			// need at least 2 hits to get her to close mouth
			if (e->damage_time) e->timer2++;
			if (e->timer2 > TIME(10)) {
				sound_play(SND_ENEMY_HURT, 5);
				effect_create_smoke(e->x << CSF, e->y << CSF);
				
				e->state = STATE_HEAD_BIT_TONGUE;
				e->timer = 0;
				e->frame = 3;
			}
		}
		break;
		
		// firing (normal)
		case STATE_HEAD_FIRE:
		{
			if ((++e->timer % 8) == 1) {
				//FIRE_ANGLED_SHOT(OBJ_DRAGON_ZOMBIE_SHOT, e->x, e->y, e->dir ? 12 : 500, 0x200);
				sound_play(SND_SNAKE_FIRE, 3);
			}
			
			if (e->timer > TIME(50))
				e->state = STATE_HEAD_CLOSED;
		}
		break;
		
		// firing a WHOLE LOT during low-hp status
		case STATE_HEAD_MEGAFIRE:
		{
			e->state++;
			e->timer = 0;
			e->frame = 0;
		}
		case STATE_HEAD_MEGAFIRE+1:
		{
			e->timer++;
			if (e->timer == 3) e->frame = 1;
			if (e->timer == 6) e->frame = 2;
			
			if (e->timer > TIME(20)) {
				if ((e->timer % 32) == 1) {
					//FIRE_ANGLED_SHOT(OBJ_DRAGON_ZOMBIE_SHOT, e->x, e->y, e->dir ? 12 : 500, 0x200);
					sound_play(SND_SNAKE_FIRE, 3);
				}
			}
		}
		break;
		
		// mouth closed bit tongue after got shot
		case STATE_HEAD_BIT_TONGUE:
		{
			if (++e->timer > TIME(100)) {
				e->state = STATE_HEAD_CLOSED;
				e->timer = 0;
			}
		}
		break;
		
		case STATE_HEAD_DEFEATED:
			e->frame = 4;	// eyes closed
		break;
	}
	
	head_set_bbox(e);
}
