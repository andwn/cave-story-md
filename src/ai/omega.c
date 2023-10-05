#include "ai_common.h"

#define OMEGA_RISE_HEIGHT			62
#define OMEGA_SINK_DEPTH			64
#define OMEGA_WAIT_TIME				7

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

#define LEGD_MIN				pixel_to_sub(14)
#define LEGD_MAX				pixel_to_sub(34)

#define OMEGA_DAMAGE			20
#define HP_TRIGGER_POINT		280

// Field aliases - wtf is a union?
//#define omgorgx		curly_target_x
//#define omgorgy		curly_target_y
//#define omgmovetime	curly_target_time

#define omgmovetime		timer2
#define endfirestate	id
#define nextstate		event
#define firecounter		deathSound
#define form			jump_time
#define leg_descend		y_next

enum Pieces {
	LEFTLEG, RIGHTLEG,
	LEFTSTRUT, RIGHTSTRUT,
	NUM_PIECES
};

void onspawn_omega(Entity *e) {
	// Trying something
	//entities_clear_by_type(OBJ_BEETLE_BROWN);
	//entities_clear_by_type(OBJ_CROW);
	//entities_clear_by_type(OBJ_CROWWITHSKULL);
	
	e->alwaysActive = TRUE;
	e->enableSlopes = FALSE;
	e->health = 400;
	e->flags |= NPC_SHOWDAMAGE | NPC_SOLID;
	e->flags = 0;
	e->frame = 0;
	e->attack = 5;
	e->hurtSound = 52;
	e->hit_box = (bounding_box) {{ 24, 20, 24, 24 }};
	e->display_box = (bounding_box) {{ 36, 28, 36, 28 }};
	
	e->form = 1;
	e->endfirestate = 200;
	e->omgmovetime = TIME_8(OMEGA_RISE_HEIGHT);
	
	// *MUST* create in this order so that the z-order is correct
	//memset(pieces, 0, NUM_PIECES * sizeof(Entity*));
	pieces[LEFTLEG] = entity_create(0, 0, OBJ_OMEGA_LEG, 0);
	pieces[RIGHTLEG] = entity_create(0, 0, OBJ_OMEGA_LEG, NPC_OPTION2);
	pieces[LEFTSTRUT] = entity_create(0, 0, OBJ_OMEGA_STRUT, 0);
	pieces[RIGHTSTRUT] = entity_create(0, 0, OBJ_OMEGA_STRUT, NPC_OPTION2);
	
	e->leg_descend = LEGD_MIN;
	
	//e->x = pixel_to_sub((217 * 16) + 5);
	//e->y = pixel_to_sub((14 * 16) - 5);
	e->x = player.x;
	e->y = player.y + pixel_to_sub(52);
	
	e->x_mark = e->x;
	e->y_mark = e->y;
	
	bossEntity = e;
	e->state = OMG_APPEAR;
}

void onspawn_omega_leg(Entity *e) {
	e->alwaysActive = TRUE;
	e->enableSlopes = FALSE;
	if(e->flags & NPC_OPTION2) e->dir = 1;
	//e->flags = 0;
	e->flags = 0;
	e->hit_box = (bounding_box) {{ 12, 8, 12, 8 }};
	e->display_box = (bounding_box) {{ 16, 16, 16, 16 }};
}

void onspawn_omega_strut(Entity *e) {
	e->alwaysActive = TRUE;
	e->enableSlopes = FALSE;
	if(e->flags & NPC_OPTION2) e->dir = 1;
	//e->eflags = 0;
	e->flags = 0;
	e->hit_box = (bounding_box) {{ 8, 8, 8, 8 }};
	e->display_box = (bounding_box) {{ 12, 8, 12, 8 }};
}

void ai_omega(Entity *e) {
	/*volatile */
    uint16_t px = e->x >> CSF, py = e->y >> CSF;
	switch(e->state) {
		case 0:	break;	// waiting for trigger by script
		case OMG_WAIT:	// waits for a moment then go to omg.nextstate
		{
			e->state++;
			e->timer = 0;
		} /* fallthrough */
		case OMG_WAIT+1:
		{
			if (++e->timer >= TIME_8(OMEGA_WAIT_TIME)) {
				e->timer = 0;
				e->state = e->nextstate;
			}
		}
		break;
		
		case OMG_APPEAR:
		{
			moveMeToFront = TRUE; // Need to be in front of the "strut" objects
			e->attack = 0; // First cycle was doing 5 damage. Don't do that
			e->timer = 0;
			e->frame = 0;
			e->state = OMG_MOVE;
			e->y_speed = -SPEED_10(0x200);
			e->flags |= NPC_SOLID;
		} /* fallthrough */
		case OMG_MOVE:	// rising up/going back into ground
		{
			e->frame = 0;
			e->y += e->y_speed;
			camera_shake(2);
			e->timer++;
			if (!(e->timer & 3)) sound_play(SND_QUAKE, 3);
			
			if (e->timer >= e->omgmovetime) {
				if (e->y_speed < 0) {	// was rising out of ground
					e->nextstate = OMG_JAWS_OPEN;
					e->state = OMG_WAIT;
				} else {	// was going back into ground
					e->timer = 0;
					e->state = OMG_UNDERGROUND;
					e->flags &= ~NPC_SOLID;
				}
			}
		}
		break;
		case OMG_JAWS_OPEN:			// jaws opening
		{
			e->state++;
			e->frame = 0;
			e->timer = 0;
			e->animtime = 0;
			e->hit_box.top = 8;
			//e->eflags &= ~NPC_BOUNCYTOP;
			sound_play(SND_JAWS, 5);
			//e->sprite = SPR_OMG_OPENED;			// select "open" bounding box
		} /* fallthrough */
		case OMG_JAWS_OPEN+1:
		{
			if (++e->animtime > 4) {
				e->animtime = 0;
				if (++e->frame >= 2) {
					e->state = OMG_FIRE;
					e->firecounter = 0;
					e->flags |= NPC_SHOOTABLE;
				}
			}
		}
		break;
		case OMG_FIRE:	// throwing out red stuff
		{
			e->firecounter++;
			if (e->firecounter > 20 && e->firecounter < 80) {
				if (!(e->firecounter & 7) && entity_active_count < 30) {
					sound_play(SND_EM_FIRE, 5);
					Entity *shot = entity_create(e->x, e->y, OBJ_OMEGA_SHOT, 0);
					if(e->form == 2) {
						shot->x_speed = -SPEED_10(0x180) + SPEED_10((rand() & 0xFF) * 3);
					} else {
						shot->x_speed = -SPEED_10(0x100) + SPEED_10(rand() & 0x1FF);
					}
					shot->y_speed = -SPEED_10(0x330);
					if(e->form == 2 || (rand() & 7)) {
						shot->frame = 0;
						shot->flags = /*shot->eflags = */0;
						//shot->flags |= NPC_SHOOTABLE;
					} else {
						shot->frame = 2;
						//shot->flags |= (NPC_SHOOTABLE | NPC_INVINCIBLE);
						shot->flags = /*shot->eflags =*/ 0;
						shot->flags |= NPC_INVINCIBLE;
					}
					//shot->timer = (rand() & 1) ? (TIME_10(300) + (rand() & 0x7F)) : 0;
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
			if (++e->animtime > 4) {
				e->animtime = 0;
				e->frame--;
				if (e->frame == 0) {
					e->hit_box.top = 24;
					sound_play(SND_BLOCK_DESTROY, 6);
					//o->sprite = SPR_OMG_CLOSED;		// select "closed" bounding box
					e->flags &= ~NPC_SHOOTABLE;
					//e->eflags |= NPC_BOUNCYTOP;
					e->attack = 0;
					if (e->form == 1) {	// form 1: return to sand
						e->state = OMG_WAIT;
						e->nextstate = OMG_MOVE;
						e->y_speed = SPEED_10(0x200);
						e->omgmovetime = TIME_8(OMEGA_SINK_DEPTH);
					} else {	// form 2: jump
						//collide_stage_floor(pieces[LEFTLEG]);
						//collide_stage_floor(pieces[RIGHTLEG]);
						sound_play(SND_FUNNY_EXPLODE, 5);
						if(px < player.x >> CSF) {
							e->x_speed = SPEED_8(0xC0);
						} else {
							e->x_speed = -SPEED_8(0xC0);
						}
						e->state = OMG_JUMP;
						e->y_speed = -SPEED_12(0x5FF);
						//e->y_mark = e->y;
					}
				}
			}
			// hurt player if he was standing in the middle when the jaws shut
			if(!playerIFrames && playerPlatform == e) player_inflict_damage(OMEGA_DAMAGE);
		}
		break;
		case OMG_UNDERGROUND:		// underground waiting to reappear
		{
			if (++e->timer >= TIME_8(120)) {
				e->timer = 0;
				e->state = OMG_APPEAR;
				
				e->x = e->x_mark + pixel_to_sub((-64 + (rand() & 127)));
				e->y = e->y_mark;
				e->omgmovetime = TIME_8(OMEGA_RISE_HEIGHT);
				
				// switch to jumping out of ground when we get low on life
				if (e->form==1 && e->health <= HP_TRIGGER_POINT) {
					e->flags |= NPC_SOLID;
					
					e->form = 2;
					e->timer = 50; // Start firing immediately and for only 30 frames
					e->endfirestate = 50;
					e->omgmovetime = TIME_8(OMEGA_RISE_HEIGHT+3);
				}
			}
		}
		break;
		case OMG_JUMP:	// init for jump
		{
			e->y_mark = e->y;
			e->state++;
			e->timer = 0;
		} /* fallthrough */
		case OMG_JUMP+1:	// jumping
		{
			if(e->y_speed < SPEED_12(0x5E0)) e->y_speed += SPEED_8(0x24);
			//pieces[LEFTLEG]->x_next = pieces[LEFTLEG]->x;
			//pieces[LEFTLEG]->y_next = pieces[LEFTLEG]->y;
			//pieces[RIGHTLEG]->x_next = pieces[RIGHTLEG]->x;
			//pieces[RIGHTLEG]->y_next = pieces[RIGHTLEG]->y;
			// Check sides so we don't get stuck
			//if(++e->timer > TIME_8(5)) {
			
			if     (px < block_to_pixel(213)) e->x_speed = SPEED_8(0xFF);
			else if(px > block_to_pixel(226)) e->x_speed = -SPEED_8(0xFF);
				//if(collide_stage_leftwall(pieces[LEFTLEG])) {
				//	e->x_speed = SPEED_8(0xFF);
				//} else if(collide_stage_rightwall(pieces[RIGHTLEG])) {
				//	e->x_speed = -SPEED_8(0xFF);
				//}
			//}
			if (e->y_speed > 0) {	// coming down
				pieces[LEFTLEG]->frame = pieces[RIGHTLEG]->frame = 0;
				// retract legs a little when we hit the ground
				//pieces[LEFTLEG]->x_next = pieces[LEFTLEG]->x;
				//pieces[LEFTLEG]->y_next = pieces[LEFTLEG]->y;
				//pieces[RIGHTLEG]->x_next = pieces[RIGHTLEG]->x;
				//pieces[RIGHTLEG]->y_next = pieces[RIGHTLEG]->y;
				//if (collide_stage_floor(pieces[LEFTLEG]) || collide_stage_floor(pieces[RIGHTLEG])) {
				if(py > block_to_pixel(11) + 6) {
					e->x_speed = 0;
					e->leg_descend -= e->y_speed;
					if (++e->timer > 3) {
						e->y_speed = 0;
						e->state = OMG_JAWS_OPEN;
					}
				}
				// --- squash player if we land on him -------------
				if (!playerIFrames && player.grounded && player.y >= e->y + (16 << CSF)) {
					if (entity_overlapping(&player, e)) {	// SQUISH!
						player_inflict_damage(OMEGA_DAMAGE);
					}
				}
			} else {	// jumping up; extend legs
				e->leg_descend = (e->y_mark - e->y) + LEGD_MIN;
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
		} /* fallthrough */
		case OMG_EXPLODING+1:
		{
			e->x_speed = e->y_speed = 0;
			if(e->timer & 1) {
				SMOKE_AREA((e->x >> CSF) - 48, (e->y >> CSF) - 48, 96, 96, 1);
				camera_shake(4);
			}
			if(!(e->timer & 15)) sound_play(SND_ENEMY_HURT_BIG, 5);
			
			if(++e->timer > TIME_8(100)) {
				e->timer = 0;
				//SCREEN_FLASH(3);
				e->state = OMG_EXPLODED;
			} else if(e->timer == TIME_8(24)) {
				tsc_call_event(210);
			}
			return;
		}
		break;
		case OMG_EXPLODED:
		{
			camera_shake(40);
			if(++e->timer > TIME_8(20)) {
				SCREEN_FLASH(3);
				bossEntity = NULL;
				e->state = STATE_DELETE;
				for(uint8_t i=0;i<NUM_PIECES;i++) pieces[i]->state = STATE_DELETE;
				return;
			}
		}
		break;
	}
	
	// implement shaking when shot
	//if(e->damage_time) e->x += (e->damage_time & 1) ? 0x200 : -0x200;
	
	if (e->state) {
		//if(e->x_speed != 0 || e->state == OMG_JUMP+1) {
		//	e->x_next = e->x;
		//	e->y_next = e->y;
		//	collide_stage_leftwall(e);
		//	collide_stage_rightwall(e);
		//	e->x = e->x_next;
		//}
		pieces[LEFTLEG]->x = e->x - (18 << CSF); pieces[LEFTLEG]->y = e->y + e->leg_descend;
		pieces[RIGHTLEG]->x = e->x + (22 << CSF); pieces[RIGHTLEG]->y = e->y + e->leg_descend;
		pieces[LEFTSTRUT]->x = e->x - (17 << CSF); pieces[LEFTSTRUT]->y = e->y + (23 << CSF);
		pieces[RIGHTSTRUT]->x = e->x + (21 << CSF); pieces[RIGHTSTRUT]->y = e->y + (23 << CSF);
	}
}

void ondeath_omega(Entity *e) {
	e->flags &= ~(NPC_SHOOTABLE|NPC_SHOWDAMAGE);
	entities_clear_by_type(OBJ_OMEGA_SHOT);
	e->state = OMG_EXPLODING;
}

void ai_omega_shot(Entity *e) {
	e->flags ^= NPC_SHOOTABLE;
	if(++e->timer & 1) {
		e->y_speed += SPEED_8(12);
		uint16_t px = e->x >> CSF, py = e->y >> CSF;
		if(e->y_speed > 0 && blk(e->x, 0, e->y, 8) == 0x41) {
		//if(e->y_speed > 0 && py > block_to_pixel(13) + 8) {
			// Delete brown shots when they hit the ground, red ones bounce
			if(e->frame > 1) {
				effect_create_smoke(px, py);
				e->state = STATE_DELETE;
				return;
			}
			e->y_speed = -SPEED_8(0xFF);
		}
		//if(e->y_speed < 0 && py < block_to_pixel(3) + 8) e->y_speed = -e->y_speed;
		//if((e->x_speed < 0 && px < block_to_pixel(211) + 8) ||
		//	(e->x_speed > 0 && px > block_to_pixel(227) + 8)) {
		//		e->x_speed = -e->x_speed;
		//}
		if(e->y_speed < 0 && blk(e->x, 0, e->y, -8) == 0x41) e->y_speed = -e->y_speed;
		if((e->x_speed < 0 && blk(e->x, -8, e->y, 0) == 0x41) ||
			(e->x_speed > 0 && blk(e->x, 8, e->y, 0) == 0x41)) {
			e->x_speed = -e->x_speed;
		}
		e->frame ^= 1;
	} else if(e->timer > TIME_10(0x240)) {
		effect_create_smoke(e->x >> CSF, e->y >> CSF);
		e->state = STATE_DELETE;
	}
	e->x += e->x_speed;
	e->y += e->y_speed;
}
