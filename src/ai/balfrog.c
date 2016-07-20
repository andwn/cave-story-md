#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "camera.h"
#include "effect.h"
#include "resources.h"

enum States {
	STATE_TRANSFORM			= 20,			// script-triggered: must stay at this value
	STATE_READY				= 10,			// script-triggered: must stay at this value
	STATE_DEATH				= 130,			// script-triggered: must stay at this value
	STATE_FIGHTING			= 100,			// script-triggered: must stay at this value
	
	STATE_JUMPING			= 50,
	
	STATE_OPEN_MOUTH		= 60,
	STATE_SHOOTING			= 70,
	STATE_CLOSE_MOUTH		= 80,
	
	STATE_BIG_JUMP			= 90
};

enum BBox_States {
	BM_STAND,
	BM_JUMPING,
	BM_MOUTH_OPEN,
	BM_DISABLED
};

#define FROG_START_X			(block_to_sub(7))
#define FROG_START_Y			(block_to_sub(12))

// when he lands he spawns frogs from ceiling--
// this is the range of where they should spawn at
#define SPAWN_RANGE_LEFT		4
#define SPAWN_RANGE_RIGHT		16
#define SPAWN_RANGE_TOP			0
#define SPAWN_RANGE_BOTTOM		4

// twiddle adjustment to get the proper Y coordinate when switching
// between normal and jumping sprites.
#define JUMP_SPRITE_ADJ			(pixel_to_sub(16))

// Speed and time values adjusted depending on TV framerate
#ifdef PAL
#define FROG_GRAVITY		0x40
#define FROG_MAX_FALL		0x5FF
#else
#define FROG_GRAVITY		0x36
#define FROG_MAX_FALL		0x4FF
#endif

u8 frog_attack_count;
u8 bbox_mode;
u8 bbox_attack;
u16 bbox_damage;
bounding_box bbox[2];

void place_bboxes(Entity *e);
void set_jump_sprite(Entity *e, bool enable);
bool player_bbox_collide(Entity *e, u8 index);
Bullet* bullets_bbox_collide(Entity *e, u8 index);
void deflect_bullet(Bullet *b);
void hurt_by_bullet(Entity *e, Bullet *b);

void ai_balfrog_onCreate(Entity *e) {
	e->x = FROG_START_X;
	e->y = FROG_START_Y;
	
	e->health = 300;
	e->attack = 0;	// damage comes from our bbox puppets, not our own bbox
	e->experience = 1;
	e->direction = 1;
	e->eflags |= NPC_IGNORE44;
	e->eflags |= NPC_SHOWDAMAGE;
	// now disable being able to hit the Balfrog boss object itself.
	e->eflags &= ~NPC_SHOOTABLE;
	
	e->hurtSound = 52;
	e->deathSound = 72;
	e->deathSmoke = 3;
	e->hit_box = (bounding_box) { 32, 24, 32, 32 };
	e->display_box = (bounding_box) { 40, 32, 40, 32 };
	
	// setup the bounding box objects--this boss has an irregular bounding box
	// and so we simulate that by having three invisible objects which are wired
	// to transmit hits to the real Balfrog boss object.
	bbox[0] = (bounding_box) { 0, 0, 0, 0 };
	bbox[1] = (bounding_box) { 0, 0, 0, 0 };
	bbox_attack = 5;
	bbox_mode = BM_DISABLED;
	bbox_damage = 0;
	
	frog_attack_count = 0;
}

void ai_balfrog_onUpdate(Entity *e) {
	if(!e->grounded) e->y_speed += FROG_GRAVITY;
	// don't limit upwards inertia or Big Jump will fail
	if(e->y_speed > FROG_MAX_FALL) e->y_speed = FROG_MAX_FALL;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		// transforming from Balrog
		// the flicker is calibrated to be interlaced exactly out-of-phase
		// with Balrog's flicker, which is entirely separate.
		case STATE_TRANSFORM:
			e->state++;
			e->state_time = 0;
			SPR_SAFEANIM(e->sprite, 2);
		case STATE_TRANSFORM+1:
			e->state_time++;
			SPR_SAFEVISIBILITY(e->sprite, (e->state_time % 4) > 1);
		break;
		// transformation complete: puff away balrog, and appear solid now
		case STATE_READY:
			e->state++;
			SPR_SAFEANIM(e->sprite, 2);
		break;
		
		case STATE_FIGHTING:
			SPR_SAFEANIM(e->sprite, 0);
			bbox_mode = BM_STAND;
			e->state++;
			e->state_time = 0;
			e->x_speed = 0;
		case STATE_FIGHTING+1:
			e->state_time++;
			// prepare to jump
			if(e->state_time < 50) {
				SPR_SAFEANIM(e->sprite, 0);
			}
			if(e->state_time == 50) {
				SPR_SAFEANIM(e->sprite, 1);
			}
			if(e->state_time == 60) {
				SPR_SAFEANIM(e->sprite, 0);
			}
			// jump
			if(e->state_time > 64) {
				e->state = STATE_JUMPING;
			}
		break;
		
		case STATE_JUMPING:
			sound_play(SND_FUNNY_EXPLODE, 8);
			set_jump_sprite(e, true);
			e->y_speed = -0x400;
			e->grounded = false;
			e->state_time = 0;
			e->state++;
		case STATE_JUMPING+1:
			// turn around at walls
			if(e->direction && collide_stage_rightwall(e)) {
				e->direction = !e->direction;
			} else if(!e->direction && collide_stage_leftwall(e)) {
				e->direction = !e->direction;
			}
			e->x_speed = e->direction ? 0x200 : -0x200;
			// landed?
			if(++e->state_time > 3 && collide_stage_floor(e)) {
				e->grounded = true;
				camera_shake(30);
				set_jump_sprite(e, false);
				// passed player? turn around and fire!
				if((e->direction && e->x >= player.x) ||
					(!e->direction && e->x <= player.x)) {
					e->direction ^= 1;
					e->state = STATE_OPEN_MOUTH;
				} else {
					e->state = STATE_FIGHTING;
				}
				// shake a small frog loose from the ceiling on every landing
				//SpawnFrogs(OBJ_MINIFROG, 1);
				//SpawnSmoke(LANDING_SMOKE_COUNT, LANDING_SMOKE_YTOP);
			}
		break;
		case STATE_BIG_JUMP:
			e->state++;
			e->state_time = 0;
			e->x_speed = 0;
		case STATE_BIG_JUMP+1:		// animation of preparing to jump
			e->state_time++;
			if(e->state_time < 50) {
				SPR_SAFEANIM(e->sprite, 0);
			}
			if(e->state_time == 50) {
				SPR_SAFEANIM(e->sprite, 1);
			}
			if(e->state_time == 70) {
				SPR_SAFEANIM(e->sprite, 0);
			}
			if(e->state_time > 74) {
				e->state++;
				set_jump_sprite(e, true);
				e->y_speed = -0xA00;
				e->grounded = false;
			}
		break;
		case STATE_BIG_JUMP+2:		// in air, waiting to hit ground
			// pass through ceiling at edges
			//if(e->y <= block_to_sub(8)) {
			//	e->eflags |= NPC_IGNORESOLID;
			//} else {
			//	e->eflags &= ~NPC_IGNORESOLID;
			//}
			if(++e->state_time > 3 && collide_stage_floor(e)) {
				e->grounded = true;
				e->eflags &= ~NPC_IGNORESOLID;
				set_jump_sprite(e, false);
				camera_shake(60);
				//SpawnFrogs(OBJ_MINIFROG, 6);
				//SpawnFrogs(OBJ_FROG, 2);
				//SpawnSmoke(LANDING_SMOKE_COUNT, LANDING_SMOKE_YTOP);
				// player ran under us? turn around and fire!
				if((e->direction && e->x >= player.x) ||
					(!e->direction && e->x <= player.x)) {
					e->state = STATE_OPEN_MOUTH;
				} else {
					e->state = STATE_FIGHTING;
				}
				FACE_PLAYER(e);
			}
		break;
		
		case STATE_OPEN_MOUTH:		// open mouth and fire shots
			SPR_SAFEANIM(e->sprite, 0);
			e->x_speed = 0;
			e->state_time = 0;
			e->state++;
		case STATE_OPEN_MOUTH+1:
			e->state_time++;
			if(e->state_time == 50) {
				SPR_SAFEANIM(e->sprite, 1);
			}
			if(e->state_time > 54) {
				e->state = STATE_SHOOTING;
				e->state_time = 0;
				SPR_SAFEANIM(e->sprite, 2);
				bbox_mode = BM_MOUTH_OPEN;
			}
		break;
		case STATE_SHOOTING:
			e->state_time++;
			if(e->damage_time > 0) {
				if((e->damage_time % 8) == 1) {
					SPR_SAFEANIM(e->sprite, 2);
				} else if((e->damage_time % 8) == 5) {
					SPR_SAFEANIM(e->sprite, 3);
				}
			}
			if((e->state_time % 16) == 1) {
				//EmFireAngledShot(o, OBJ_BALFROG_SHOT, 16, 0x200);
				sound_play(SND_EM_FIRE, 5);
				if(e->state_time > 160 || bbox_damage > 90) {
					SPR_SAFEANIM(e->sprite, 1);
					e->state = STATE_CLOSE_MOUTH;
					bbox_mode = BM_STAND;
					bbox_damage = 0;
					e->state_time = 0;
				}
			}
		break;
		case STATE_CLOSE_MOUTH:
			if(++e->state_time > 10) {
				e->state_time = 0;
				SPR_SAFEANIM(e->sprite, 0);
				// Big jump after 3 attacks
				if(++frog_attack_count >= 3) {
					frog_attack_count = 0;
					e->state = STATE_BIG_JUMP;
				} else {
					e->state = STATE_FIGHTING;
				}
			}
		break;
		
		case STATE_DEATH:			// BOOM!
			SPR_SAFEANIM(e->sprite, 2);
			sound_play(SND_BIG_CRASH, 10);
			e->x_speed = 0;
			e->state_time = 0;
			e->state++;
			//SpawnSmoke(DEATH_SMOKE_COUNT, DEATH_SMOKE_YTOP);
		case STATE_DEATH+1:			// shaking with mouth open
			e->state_time++;
			if((e->state_time % 8) == 0) {
				effect_create_smoke(0, e->x - 28 + (random() % 56),
					e->y - 24 + (random() % 48));
			}
			// at a glance it might seem like this has it alternate
			// slowly between 2 X coordinates, but in fact, it
			// alternates quickly between 3.
			e->x += (e->state_time & 2) ? pixel_to_sub(1) : pixel_to_sub(-1);
			if(e->state_time > 100) {
				e->state_time = 0;
				e->state++;
			}
		break;
		case STATE_DEATH+2:			// begin flashing back and forth between frog and balrog
		{ // Scope for balrog pointer
			// spawn balrog puppet
			Entity *balrog = entity_create(e->x, e->y, 0, 0, 0xB, 0, e->direction);
			balrog->state = 500;	// tell him to give us complete control
			//frog.balrog->frame = 5;
			e->state++;
		}
		case STATE_DEATH+3:		// flashing
			e->state_time++;
			if((e->state_time % 16) == 0) {
				effect_create_smoke(0, e->x - 28 + (random() % 56),
					e->y - 24 + (random() % 48));
			}
			if(e->state_time <= 150) {
				SPR_SAFEVISIBILITY(e->sprite, e->state_time & 2);
				Entity *balrog = entity_find_by_type(0xB);
				if(balrog != NULL) {
					SPR_SAFEVISIBILITY(balrog->sprite, !(e->state_time & 2));
				}
			}
			if(e->state_time > 156) {
				e->state_time = 0;
				e->state++;
			}
		break;
		case STATE_DEATH+4:		// balrog falling to ground
		{
			Entity *balrog = entity_find_by_type(0xB);
			// should start to move exactly when timer hits 160
			//
			// 10 frames until starts to fall
			// 14 frames until changes to landed frame
			if(balrog != NULL) {
				balrog->y_speed += 0x40;
				if(collide_stage_floor(balrog)) {
					balrog->grounded = true;
					balrog->y_speed = 0;
					//balrog->frame = 2;
					if(++e->state_time > 30) {
						//balrog->frame = 3;
						e->state++;
					}
				}
			}
		}
		break;
		case STATE_DEATH+5:		// balrog flying away
			if(++e->state_time > 30) {
				Entity *balrog = entity_find_by_type(0xB);
				// it's all over, destroy ourselves and clean up
				if(balrog != NULL) {
					balrog->y_speed = -0xA00;
					balrog->eflags |= NPC_IGNORESOLID;
					if(balrog->y < -0x400) {
						entity_delete(balrog);
						bossEntity = NULL;
						e->state = STATE_DELETE;
						return;
					}
				}
			}
		break;
	}
	SPR_SAFEHFLIP(e->sprite, e->direction);
	// Player collision
	if(bbox_mode != BM_DISABLED) {
		if(!player_invincible() && (player_bbox_collide(e, 0) || player_bbox_collide(e, 1))) {
			player_inflict_damage(5);
		}
		Bullet *b1 = bullets_bbox_collide(e, 0);
		Bullet *b2 = bullets_bbox_collide(e, 1);
		if(b1 != NULL) deflect_bullet(b1);
		if(b2 != NULL) {
			if(bbox_mode == BM_MOUTH_OPEN) {
				hurt_by_bullet(e, b2);
			} else {
				deflect_bullet(b2);
			}
		}
	}
	if(!e->grounded) {
		e->grounded = collide_stage_floor_grounded(e);
	} else {
		e->grounded = collide_stage_floor(e);
	}
	e->x = e->x_next;
	e->y = e->y_next;
	// link bboxes to our real object
	place_bboxes(e);
}

void ai_balfrog_onState(Entity *e) {
	if(e->state == STATE_DEFEATED) {
		tsc_call_event(e->event); // Boss defeated event
		if(bbox_mode == BM_JUMPING) {
			set_jump_sprite(e, false);
		}
		bbox_mode = BM_DISABLED;
		e->state = STATE_DEATH;
		e->state_time = 0;
	}
}

void place_bboxes(Entity *e) {
	// Set coordinates for the bounding boxes
	switch(bbox_mode) {
		case BM_STAND:
			bbox[0] = (bounding_box) { 16, 0, 32, 32 };	// body
			bbox[1] = (bounding_box) { 32, 24, 0, 8 };	// head
		break;
		case BM_JUMPING:
			bbox[0] = (bounding_box) { 24, 24, 8, 0 };// head
			bbox[1] = (bounding_box) { 8, 4, 28, 40 };	// body
		break;
		case BM_MOUTH_OPEN:
			bbox[0]	= (bounding_box) { 0, 8, 32, 32 };	// backside
			bbox[1] = (bounding_box) { 32, 8, 0, 32 };	// mouth
		break;
	}
	if(e->direction) { // Swap when facing right
		u8 temp;
		temp = bbox[0].left;
		bbox[0].left = bbox[0].right;
		bbox[0].right = temp;
		temp = bbox[1].left;
		bbox[1].left = bbox[1].right;
		bbox[1].right = temp;
	}
}

/*
// shake loose frogs from the ceiling
void BalfrogBoss::SpawnFrogs(int objtype, int count)
{
Object *child;

	for(int i=0;i<count;i++)
	{
		int x = random(SPAWN_RANGE_LEFT, SPAWN_RANGE_RIGHT);
		int y = random(SPAWN_RANGE_TOP, SPAWN_RANGE_BOTTOM);
		
		child = CreateObject((x*TILE_W)<<CSF, (y*TILE_H)<<CSF, objtype);
		child->dir = DOWN;	// allow fall through ceiling
	}
}
*/

// switches on and off the jumping frame/sprite
void set_jump_sprite(Entity *e, bool enable) {
	if(enable) {
		SPR_SAFEADD(e->sprite, &SPR_Balfrog2, 0, 0, TILE_ATTR(PAL3, 0, 0, e->direction), 5);
		e->y -= JUMP_SPRITE_ADJ;
		e->display_box.left -= 4;
		bbox_mode = BM_JUMPING;
	} else {
		SPR_SAFEADD(e->sprite, &SPR_Balfrog1, 0, 0, TILE_ATTR(PAL3, 0, 0, e->direction), 5);
		e->y += JUMP_SPRITE_ADJ;
		e->display_box.left += 4;
		bbox_mode = BM_STAND;
	}
}

bool player_bbox_collide(Entity *e, u8 index) {
	s16 ax1 = sub_to_pixel(player.x) - (player.direction ? player.hit_box.right : player.hit_box.left),
		ax2 = sub_to_pixel(player.x) + (player.direction ? player.hit_box.left : player.hit_box.right),
		ay1 = sub_to_pixel(player.y) - player.hit_box.top,
		ay2 = sub_to_pixel(player.y) + player.hit_box.bottom,
		bx1 = sub_to_pixel(e->x) - (e->direction ? bbox[index].right : bbox[index].left),
		bx2 = sub_to_pixel(e->x) + (e->direction ? bbox[index].left : bbox[index].right),
		by1 = sub_to_pixel(e->y) - bbox[index].top,
		by2 = sub_to_pixel(e->y) + bbox[index].bottom;
	return (ax1 < bx2 && ax2 > bx1 && ay1 < by2 && ay2 > by1);
}

Bullet* bullets_bbox_collide(Entity *e, u8 index) {
	for(u8 i = 0; i < MAX_BULLETS; i++) {
		if(playerBullet[i].ttl == 0) continue;
		bounding_box bb = playerBullet[i].hit_box;
		if(sub_to_pixel(playerBullet[i].x) - bb.left >= 
			sub_to_pixel(e->x) + bbox[index].right) continue;
		if(sub_to_pixel(playerBullet[i].x) + bb.right <= 
			sub_to_pixel(e->x) - bbox[index].left) continue;
		if(sub_to_pixel(playerBullet[i].y) - bb.top >= 
			sub_to_pixel(e->y) + bbox[index].bottom) continue;
		if(sub_to_pixel(playerBullet[i].y) + bb.bottom <= 
			sub_to_pixel(e->y) - bbox[index].top) continue;
		return &playerBullet[i];
	}
	return NULL;
}

void deflect_bullet(Bullet *b) {
	b->ttl = 0;
	SPR_SAFERELEASE(b->sprite);
	sound_play(SND_TINK, 5);
}

void hurt_by_bullet(Entity *e, Bullet *b) {
	bbox_damage += b->damage;
	// Copy pasting is bad but i do it anyway
	// Destroy the bullet, or if it is a missile make it explode
	if(b->type == WEAPON_MISSILE || b->type == WEAPON_SUPERMISSILE) {
		if(b->x_speed != 0 || b->y_speed != 0) {
			bullet_missile_explode(b);
			if(b->damage < e->health) sound_play(e->hurtSound, 5);
		}
	} else {
		b->ttl = 0;
		SPR_SAFERELEASE(b->sprite);
		if(b->damage < e->health) sound_play(e->hurtSound, 5);
	}
	if(e->health <= b->damage) {
		if((e->eflags|e->nflags) & NPC_SHOWDAMAGE)
			effect_create_damage(e->damage_value - b->damage,
					sub_to_pixel(e->x), sub_to_pixel(e->y), 60);
		// Killed enemy
		e->health = 0;
		ENTITY_SET_STATE(e, STATE_DEFEATED, 0);
	}
	if((e->eflags|e->nflags) & NPC_SHOWDAMAGE) {
		e->damage_value -= b->damage;
		e->damage_time = 30;
	}
	e->health -= b->damage;
}
