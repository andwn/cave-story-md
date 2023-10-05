#include "ai_common.h"

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
	uint8_t left, top, right, bottom;
	uint16_t flags;
} head_bboxes[] = {
	{ 12, 12, 12, 12,  NPC_SHOOTABLE | NPC_INVINCIBLE },	// closed
	{ 12, 12, 12, 12,  NPC_SHOOTABLE | NPC_INVINCIBLE },	// partway open
	{ 12, 12, 12, 12,  NPC_SHOOTABLE },		// open (shootable)
	{ 12, 12, 12, 12,  NPC_SHOOTABLE },		// bit tongue (shootable)
	{ 12, 12, 12, 12,  0 }					// eyes closed (shots pass through)
};

#define mainangle	curly_target_x

static void SetHeadStates(uint16_t newstate) {
	for(uint8_t i=0;i<2;i++)
		pieces[HEAD1+i]->state = newstate;
}

static void SetBodyStates(uint16_t newstate) {
	for(uint8_t i=0;i<2;i++)
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
	e->flags &= ~(NPC_SHOOTABLE | NPC_INVINCIBLE);
	e->flags |= head_bboxes[e->frame].flags;
}

void onspawn_sisters(Entity *e) {
	e->alwaysActive = TRUE;
	e->x = pixel_to_sub(160);
	e->y = pixel_to_sub((8*16)-4);
	e->hidden = TRUE;
	
	mainangle = 0;
	e->x_mark = 192; // Start outside of the room, changes to 112 when they appear
	e->y_mark = 60;
	e->timer2 = TIME_10(rand() & 511) + TIME_10(700);
	e->health = 500;
	e->event = 1000;
	e->flags |= NPC_EVENTONDEATH;
	
	// Create body before head, so the head will be on top
	pieces[BODY1] = entity_create(-pixel_to_sub(64), pixel_to_sub(80), OBJ_SISTERS_BODY, 0);
	pieces[HEAD1] = entity_create(-pixel_to_sub(64), pixel_to_sub(64), OBJ_SISTERS_HEAD, 0);
	pieces[HEAD1]->linkedEntity = pieces[BODY1];
	pieces[BODY1]->linkedEntity = pieces[HEAD1];
	
	pieces[BODY2] = entity_create(pixel_to_sub(384), pixel_to_sub(80), OBJ_SISTERS_BODY, NPC_OPTION2);
	pieces[HEAD2] = entity_create(pixel_to_sub(384), pixel_to_sub(64), OBJ_SISTERS_HEAD, NPC_OPTION2);
	pieces[HEAD2]->linkedEntity = pieces[BODY2];
	pieces[BODY2]->linkedEntity = pieces[HEAD2];

	//bossEntity = e;
}

void onspawn_sisters_body(Entity *e) {
	e->alwaysActive = TRUE;
	e->attack = SISTERS_DAMAGE;
	e->display_box = (bounding_box) {{ 16, 20, 16, 20 }};
	e->hit_box = (bounding_box) {{ 12, 16, 12, 16 }};
}

void onspawn_sisters_head(Entity *e) {
	e->alwaysActive = TRUE;
	e->attack = SISTERS_DAMAGE;
	e->health = 1000;
	e->hurtSound = SND_ENEMY_HURT_COOL;
	e->damage_time = 10;
	//sound_play(e->hurtSound, 5);
	e->display_box = (bounding_box) {{ 16, 16, 16, 16 }};
}

void ondeath_sisters(Entity *e) {
	//e->state = STATE_DEFEATED; // <BOA does this for us
	tsc_call_event(e->event);
}

void ai_sisters(Entity *e) {
	switch(e->state) {
		case 20:	// fight begin (script-triggered)
		{
			if (++e->timer > TIME_8(68)) {
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
			
			if (e->timer < TIME_8(100))					mainangle += 1;
			else if (e->timer < TIME_8(120))			mainangle += 2;
			else if (e->timer < e->timer2)				mainangle += 4;
			else if (e->timer < e->timer2 + TIME_8(40))	mainangle += 2;
			else if (e->timer < e->timer2 + TIME_8(60))	mainangle += 1;
			else {
				e->timer = 0;
				e->state = STATE_CIRCLE_0;
				e->timer2 = TIME_8(rand() & 0xFF) + TIME_10(420);
			}
		}
		break;
		
		case STATE_CIRCLE_0:
		{
			e->timer++;
			
			if (e->timer < TIME_8(20))					mainangle -= 1;
			else if (e->timer < TIME_8(60))				mainangle -= 2;
			else if (e->timer < e->timer2)				mainangle -= 4;
			else if (e->timer < e->timer2 + TIME_8(40))	mainangle -= 2;
			else if (e->timer < e->timer2 + TIME_8(60))	mainangle -= 1;
			else {
				if (e->health < SISTERS_ATTACK2_HP) {
					e->state = STATE_MEGAFIRING;
					SetHeadStates(STATE_HEAD_MEGAFIRE);
					e->timer = 0;
				} else {
					e->state = STATE_CIRCLE_1;
					e->timer2 = TIME_8(rand() & 0xFF) + TIME_10(420);
					e->timer = 0;
				}
			}
		}
		break;
		
		// runs the circling while the heads are doing their low-hp "megafire" state.
		// first they stop completely, then spin around and around clockwise for a while.
		case STATE_MEGAFIRING:
		{
			if (++e->timer > TIME_8(100)) {
				e->state++;
				e->timer = 0;
			}
		}
		break;
		case STATE_MEGAFIRING+1:
		{
			e->timer++;
			
			if (e->timer < TIME_8(100))	       mainangle += 1;
			else if (e->timer < TIME_8(120))   mainangle += 2;
			else if (e->timer < TIME_10(500))  mainangle += 4;
			else if (e->timer < TIME_10(540))  mainangle += 2;
			else if (e->timer < TIME_10(560))  mainangle += 1;
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
			
			for(uint16_t i=0;i<2;i++) {
				pieces[HEAD1+i]->attack = 0;
				pieces[BODY1+i]->attack = 0;
			}
			
			SetHeadStates(STATE_HEAD_DEFEATED);
			SetBodyStates(STATE_BODY_FACE_CENTER);
			
			e->state++;
			e->timer = 0;
		} /* fallthrough */
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
					SCREEN_FLASH(3);
					sound_play(SND_EXPLOSION1, 5);
					// Do this before the fade wears off
					entities_clear_by_type(OBJ_DRAGON_ZOMBIE_SHOT);
					entities_clear_by_type(OBJ_SPIKE_SMALL);
					
					for(uint8_t i=0;i<2;i++) {
						pieces[HEAD1+i]->state = STATE_DELETE;
						pieces[BODY1+i]->state = STATE_DELETE;
					}
				
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
				e->state = STATE_DELETE;
				bossEntity = NULL;
				
				return;
			}
		}
		break;
	}
	
	//mainangle &= 1023;
}

void ai_sisters_body(Entity *e) {
	// mainangle works in a range of 1024 so the sisters can circle more slowly
	uint8_t angle = mainangle >> 2;
	if(e->flags & NPC_OPTION2) angle ^= 0x80;
	
	// main's x_mark and y_mark tell us how far from the center to circle
	int32_t xoff = (int32_t)cos[angle] * bossEntity->x_mark;
	int32_t yoff = (int32_t)sin[angle] * bossEntity->y_mark;
	
	// figure out where we are supposed to be
	int32_t desired_x = bossEntity->x + xoff;
	int32_t desired_y = bossEntity->y + yoff;
	
	// motion
	if (e->state == 0) { // this places them offscreen before the fight
		e->x = desired_x;
		e->y = desired_y;
		e->state = STATE_BODY_NOMOVE;
	} else if (e->state != STATE_BODY_NOMOVE) {	
		// smooth interpolation, they stay in this state throughout the fight
		e->x += (desired_x - e->x) >> 4;
		e->y += (desired_y - e->y) >> 4;
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
		e->timer2++;
	}
	
	// FSM
	// Thanks I didn't notice this was a state machine
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->flags |= (NPC_SHOOTABLE | NPC_INVINCIBLE);
		}
		break;
		
		case STATE_HEAD_CLOSED:
		{
			e->frame = 0;
			e->timer = TIME_8(rand() & 127) + TIME_8(80);
			e->state++;
		} /* fallthrough */
		case STATE_HEAD_CLOSED+1:
		{
			if (--e->timer <= 0) {
				e->state = STATE_HEAD_OPEN;
				e->timer = 0;
				e->timer2 = 0;
			}
		}
		break;
		
		// open mouth and prepare to fire
		case STATE_HEAD_OPEN:
		{
			e->timer = 0;
			e->timer2 = 0;
			e->state++;
		} /* fallthrough */
		case STATE_HEAD_OPEN+1:
		{
			e->timer++;
			if (e->timer == 3) e->frame = 1;	// mouth partially open--about to fire!!
			if (e->timer == 6) e->frame = 2;	// mouth fully open
			
			if (e->timer > TIME(150)) {	// begin firing if they haven't hit us by now
				e->state = STATE_HEAD_FIRE;
				e->timer = 0;
			}
			
			// Close mouth after 3 hits, minimum of 1 second open
			// This might be wrong but it works better than before at least
			if (e->timer2 > 2 && e->timer > TIME(50)) {
				sound_play(SND_ENEMY_HURT, 5);
				effect_create_smoke(e->x >> CSF, e->y >> CSF);
				
				e->state = STATE_HEAD_BIT_TONGUE;
				e->timer = 0;
				e->frame = 3;
			}
		}
		break;
		
		// firing (normal)
		case STATE_HEAD_FIRE:
		{
			if ((++e->timer & 7) == 1) {
				//FIRE_ANGLED_SHOT(OBJ_DRAGON_ZOMBIE_SHOT, e->x, e->y, 
				//		e->dir ? A_RIGHT : A_LEFT, 0x200);
				// Need to aim at the player
				Entity *fire = entity_create(e->x, e->y, OBJ_DRAGON_ZOMBIE_SHOT, 0);
				THROW_AT_TARGET(fire, player.x, player.y, SPEED(0x200));
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
		} /* fallthrough */
		case STATE_HEAD_MEGAFIRE+1:
		{
			e->timer++;
			if (e->timer == 3) e->frame = 1;
			if (e->timer == 6) e->frame = 2;
			
			if (e->timer > TIME(20)) {
				if ((e->timer & 31) == 1) {
					//FIRE_ANGLED_SHOT(OBJ_DRAGON_ZOMBIE_SHOT, e->x, e->y, 
					//		e->dir ? A_RIGHT : A_LEFT, 0x200);
					Entity *fire = entity_create(e->x, e->y, OBJ_DRAGON_ZOMBIE_SHOT, 0);
					THROW_AT_TARGET(fire, player.x, player.y, SPEED(0x200));
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
