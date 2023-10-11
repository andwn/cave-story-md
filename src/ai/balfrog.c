#include "ai_common.h"

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
#define SPAWN_RANGE_RIGHT		15
#define SPAWN_RANGE_TOP			0
#define SPAWN_RANGE_BOTTOM		4

// twiddle adjustment to get the proper Y coordinate when switching
// between normal and jumping sprites.
#define JUMP_SPRITE_ADJ			(pixel_to_sub(16))

#define frog_attack_count	underwater
#define bbox_damage			curly_target_time
#define bbox_attack			curly_target_x
#define bbox_mode			curly_target_y

static bounding_box bbox[2];

static void place_bboxes(Entity *e);
static void set_jump_sprite(Entity *e, uint8_t enable);
static uint8_t player_bbox_collide(Entity *e, uint8_t index);
static Bullet* bullets_bbox_collide(Entity *e, uint8_t index);
static void deflect_bullet(Bullet *b);
static void hurt_by_bullet(Entity *e, Bullet *b);
static void spawn_frogs(uint16_t objtype, uint8_t count);

void onspawn_balfrog(Entity *e) {
	e->x = FROG_START_X;
	e->y = FROG_START_Y;
	
	e->health = 300;
	e->attack = 0;	// damage comes from our bbox puppets, not our own bbox
	e->experience = 1;
	e->dir = 1;
	e->enableSlopes = FALSE;
	e->flags |= NPC_IGNORE44 | NPC_SHOWDAMAGE;
	// now disable being able to hit the Balfrog boss object itself.
	//e->eflags &= ~(NPC_SHOOTABLE | NPC_SOLID);
	e->flags &= ~(NPC_SHOOTABLE | NPC_SOLID);
	
	e->hurtSound = 52;
	e->deathSound = 72;
	e->hit_box = (bounding_box) {{ 32, 24, 32, 32 }};
	e->display_box = (bounding_box) {{ 40, 32, 40, 32 }};
	
	// timer2 recycled for feet's VRAM index
	uint16_t sheet = NOSHEET;
	SHEET_FIND(sheet, SHEET_FROGFEET);
	e->timer2 = sheets[sheet].index;
	
	// setup the bounding box objects--this boss has an irregular bounding box
	// and so we simulate that by having three invisible objects which are wired
	// to transmit hits to the real Balfrog boss object.
	bbox[0] = (bounding_box) {0};
	bbox[1] = (bounding_box) {0};
	bbox_attack = 5;
	bbox_mode = BM_DISABLED;
	bbox_damage = 0;
	
	e->frog_attack_count = 0;
}

void ai_balfrog(Entity *e) {
	if(!e->grounded && e->y_speed < SPEED_12(0x5C0)) e->y_speed += SPEED_8(0x40);
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		// transforming from Balrog
		// the flicker is calibrated to be interlaced exactly out-of-phase
		// with Balrog's flicker, which is entirely separate.
		case STATE_TRANSFORM:
		{
			e->state++;
			e->timer = 0;
			e->frame = 2;
		}
		/* fallthrough */
		case STATE_TRANSFORM+1:
		{
			e->timer++;
			e->hidden = !(e->timer & 2);
		}
		break;
		// transformation complete: puff away balrog, and appear solid now
		case STATE_READY:
		{
			e->state++;
			e->frame = 2;
			e->hidden = FALSE;
		}
		break;
		
		case STATE_FIGHTING:
		{
			e->frame = 0;
			bbox_mode = BM_STAND;
			e->state++;
			e->timer = 0;
			e->x_speed = 0;
		}
		/* fallthrough */
		case STATE_FIGHTING+1:
		{
			e->timer++;
			// prepare to jump
			if(e->timer < 50) {
				e->frame = 0;
			}
			if(e->timer == 50) {
				e->frame = 1;
			}
			if(e->timer == 60) {
				e->frame = 0;
			}
			// jump
			if(e->timer > 64) {
				e->state = STATE_JUMPING;
			}
		}
		break;
		
		case STATE_JUMPING:
		{
			sound_play(SND_FUNNY_EXPLODE, 8);
			e->frame = 4;
			set_jump_sprite(e, TRUE);
			e->y_speed = -SPEED(0x400);
			e->grounded = FALSE;
			e->timer = 0;
			e->state++;
		}
		/* fallthrough */
		case STATE_JUMPING+1:
		{
			// turn around at walls
			if(e->dir && collide_stage_rightwall(e)) {
				e->dir = !e->dir;
			} else if(!e->dir && collide_stage_leftwall(e)) {
				e->dir = !e->dir;
			}
			e->x_speed = e->dir ? SPEED(0x200) : -SPEED(0x200);
			// landed?
			if(++e->timer > 3 && collide_stage_floor(e)) {
				e->grounded = TRUE;
				camera_shake(30);
				e->frame = 0;
				set_jump_sprite(e, FALSE);
				// passed player? turn around and fire!
				if((e->dir && e->x >= player.x) ||
					(!e->dir && e->x <= player.x)) {
					e->dir ^= 1;
					e->state = STATE_OPEN_MOUTH;
				} else {
					e->state = STATE_FIGHTING;
				}
				// shake a small frog loose from the ceiling on every landing
				spawn_frogs(OBJ_MINIFROG, 1);
				SMOKE_AREA((e->x >> CSF) - 32, (e->y >> CSF) + 16, 64, 8, 2);
			}
		}
		break;
		case STATE_BIG_JUMP:
		{
			e->state++;
			e->timer = 0;
			e->x_speed = 0;
		}
		/* fallthrough */
		case STATE_BIG_JUMP+1:		// animation of preparing to jump
		{
			e->timer++;
			if(e->timer < 50) {
				e->frame = 0;
			}
			if(e->timer == 50) {
				e->frame = 1;
			}
			if(e->timer == 70) {
				e->frame = 0;
			}
			if(e->timer > 74) {
				e->state++;
				e->frame = 4;
				set_jump_sprite(e, TRUE);
				e->y_speed = -SPEED(0xA00);
				e->grounded = FALSE;
			}
		}
		break;
		case STATE_BIG_JUMP+2:		// in air, waiting to hit ground
		{
			if(e->grounded) {
				e->frame = 0;
				set_jump_sprite(e, FALSE);
				camera_shake(60);
				spawn_frogs(OBJ_MINIFROG, 4);
				spawn_frogs(OBJ_FROG, 2);
				SMOKE_AREA((e->x >> CSF) - 32, (e->y >> CSF) + 16, 64, 8, 4);
				// player ran under us? turn around and fire!
				if((e->dir && e->x >= player.x) ||
					(!e->dir && e->x <= player.x)) {
					e->state = STATE_OPEN_MOUTH;
				} else {
					e->state = STATE_FIGHTING;
				}
				FACE_PLAYER(e);
			}
		}
		break;
		
		case STATE_OPEN_MOUTH:		// open mouth and fire shots
		{
			e->frame = 0;
			e->x_speed = 0;
			e->timer = 0;
			e->state++;
		}
		/* fallthrough */
		case STATE_OPEN_MOUTH+1:
		{
			e->timer++;
			if(e->timer == 50) {
				e->frame = 1;
			}
			if(e->timer > 54) {
				e->state = STATE_SHOOTING;
				e->timer = 0;
				e->frame = 2;
				bbox_mode = BM_MOUTH_OPEN;
			}
		}
		break;
		case STATE_SHOOTING:
		{
			e->timer++;
			if(e->damage_time > 0) {
				if((e->damage_time & 7) == 1) {
					e->frame = 2;
				} else if((e->damage_time & 7) == 5) {
					e->frame = 3;
				}
			}
			if((e->timer & 15) == 1) {
				uint8_t angle = (e->dir ? A_RIGHT : A_LEFT) - 16 + (rand() & 31);
				FIRE_ANGLED_SHOT(OBJ_BALFROG_SHOT, e->x + (e->dir ? 0x1000 : -0x1000),
						e->y + 0x1000, angle, 0x200);
				sound_play(SND_EM_FIRE, 5);
				if(e->timer > TIME_8(160) || bbox_damage > 90) {
					e->frame = 1;
					e->state = STATE_CLOSE_MOUTH;
					bbox_mode = BM_STAND;
					bbox_damage = 0;
					e->timer = 0;
				}
			}
		}
		break;
		case STATE_CLOSE_MOUTH:
		{
			if(++e->timer > TIME_8(10)) {
				e->timer = 0;
				e->frame = 0;
				// Big jump after 3 attacks
				if(++e->frog_attack_count >= 3) {
					e->frog_attack_count = 0;
					e->state = STATE_BIG_JUMP;
				} else {
					e->state = STATE_FIGHTING;
				}
			}
		}
		break;
		
		case STATE_DEATH:			// BOOM!
		{
			e->frame = 2;
			sound_play(SND_BIG_CRASH, 10);
			e->x_speed = 0;
			e->timer = 0;
			e->state++;
			SMOKE_AREA((e->x >> CSF) - 32, (e->y >> CSF) - 24, 64, 48, 4);
		}
		/* fallthrough */
		case STATE_DEATH+1:			// shaking with mouth open
		{
			e->timer++;
			if((e->timer & 7) == 0) {
				SMOKE_AREA((e->x >> CSF) - 32, (e->y >> CSF) - 24, 64, 48, 1);
			}
			// at a glance it might seem like this has it alternate
			// slowly between 2 X coordinates, but in fact, it
			// alternates quickly between 3.
			e->x += (e->timer & 2) ? 0x200 : -0x200;
			if(e->timer > TIME_8(100)) {
				e->timer = 0;
				e->state++;
			}
		}
		break;
		case STATE_DEATH+2:			// begin flashing back and forth between frog and balrog
		{ // Scope for balrog pointer
			// spawn balrog puppet
			Entity *balrog = entity_create(e->x, e->y, OBJ_BALROG, 0);
			balrog->dir = e->dir;
			balrog->state = 500;	// tell him to give us complete control
			balrog->frame = 5;
			e->state++;
		}
		/* fallthrough */
		case STATE_DEATH+3:		// flashing
		{
			e->timer++;
			if((e->timer & 15) == 0) {
				SMOKE_AREA((e->x >> CSF) - 32, (e->y >> CSF) - 24, 64, 48, 1);
			}
			if(e->timer <= TIME_8(150)) {
				Entity *balrog = entity_find_by_type(OBJ_BALROG);
				if(balrog) {
					e->hidden = (e->timer & 2);
					balrog->hidden = !e->hidden;
				}
			}
			if(e->timer > TIME_8(156)) {
				e->timer = 0;
				e->state++;
			}
		}
		break;
		case STATE_DEATH+4:		// balrog falling to ground
		{
			Entity *balrog = entity_find_by_type(OBJ_BALROG);
			// should start to move exactly when timer hits 160
			//
			// 10 frames until starts to fall
			// 14 frames until changes to landed frame
			if(balrog) {
				e->hidden = TRUE;
				balrog->hidden = FALSE;
				balrog->y_speed += SPEED_8(0x40);
				balrog->y_next = balrog->y + balrog->y_speed;
				balrog->x_next = balrog->x;
				if(collide_stage_floor(balrog)) {
					balrog->y_speed = 0;
					balrog->frame = 2;
					if(++e->timer > TIME_8(30)) {
						balrog->frame = 0;
						balrog->grounded = TRUE;
						e->state++;
					}
				}
				balrog->y = balrog->y_next;
			}
		}
		break;
		case STATE_DEATH+5:		// balrog flying away
		{
			if(++e->timer > TIME_8(30)) {
				Entity *balrog = entity_find_by_type(OBJ_BALROG);
				// it's all over, destroy ourselves and clean up
				if(balrog) {
					balrog->frame = 3;
					balrog->y_speed = -SPEED_12(0x5FF);
					balrog->y_next = balrog->y + balrog->y_speed;
					balrog->y = balrog->y_next;
					balrog->flags |= NPC_IGNORESOLID;
					if(balrog->y < -0x400) {
						entity_delete(balrog);
						bossEntity = NULL;
						e->state = STATE_DELETE;
						return;
					}
				}
			}
		}
		break;
	}
	// Player collision
	if(bbox_mode != BM_DISABLED) {
		if(!playerIFrames && (player_bbox_collide(e, 0) || player_bbox_collide(e, 1))) {
			player_inflict_damage(5);
		}
		Bullet *b1 = bullets_bbox_collide(e, 0);
		Bullet *b2 = bullets_bbox_collide(e, 1);
		if(b1) deflect_bullet(b1);
		if(b2) {
			if(bbox_mode == BM_MOUTH_OPEN) {
				hurt_by_bullet(e, b2);
			} else {
				deflect_bullet(b2);
			}
		}
	}
	if(!e->grounded) {
		if(e->y_speed > 0) e->grounded = collide_stage_floor_grounded(e);
	} else {
		e->grounded = collide_stage_floor(e);
	}
	e->x = e->x_next;
	e->y = e->y_next;
	// Draw feet if jumping
	static const int16_t xoff1[2] = { -16, -32 },
					 xoff2[2] = { 8, -8 };
	if(e->jump_time) {
		Sprite feet[2] = {
			(Sprite) {
				.x = (e->x >> CSF) - (camera.x >> CSF) + ScreenHalfW + xoff1[e->dir] + 128,
				.y = (e->y >> CSF) - (camera.y >> CSF) + ScreenHalfH + 24 + 128,
				.size = SPRITE_SIZE(3, 3),
				.attr = TILE_ATTR(PAL3,0,0,e->dir,e->timer2+(e->dir?9:0)),
			},
			(Sprite) {
				.x = (e->x >> CSF) - (camera.x >> CSF) + ScreenHalfW + xoff2[e->dir] + 128,
				.y = (e->y >> CSF) - (camera.y >> CSF) + ScreenHalfH + 24 + 128,
				.size = SPRITE_SIZE(3, 3),
				.attr = TILE_ATTR(PAL3,0,0,e->dir,e->timer2+(e->dir?0:9)),
			},
		};
	vdp_sprites_add(feet, 2);
	}
	// link bboxes to our real object
	place_bboxes(e);
}

void ondeath_balfrog(Entity *e) {
	if(bbox_mode == BM_JUMPING) {
		set_jump_sprite(e, FALSE);
	}
	bbox_mode = BM_DISABLED;
	e->state = STATE_DEATH;
	e->timer = 0;
	tsc_call_event(e->event); // Boss defeated event
}

static void place_bboxes(Entity *e) {
	// Set coordinates for the bounding boxes
	switch(bbox_mode) {
		case BM_STAND:
			bbox[0] = (bounding_box) {{ 16, 0, 32, 32 }};	// body
			bbox[1] = (bounding_box) {{ 32, 24, 0, 8 }};	// head
		break;
		case BM_JUMPING:
			bbox[0] = (bounding_box) {{ 24, 24, 8, 0 }};	// head
			bbox[1] = (bounding_box) {{ 8, 4, 28, 40 }};	// body
		break;
		case BM_MOUTH_OPEN:
			bbox[0]	= (bounding_box) {{ 0, 8, 32, 32 }};	// backside
			bbox[1] = (bounding_box) {{ 32, 8, 0, 32 }};	// mouth
		break;
	}
	if(e->dir) { // Swap when facing right
		uint8_t temp;
		temp = bbox[0].left;
		bbox[0].left = bbox[0].right;
		bbox[0].right = temp;
		temp = bbox[1].left;
		bbox[1].left = bbox[1].right;
		bbox[1].right = temp;
	}
}

// shake loose frogs from the ceiling
static void spawn_frogs(uint16_t objtype, uint8_t count) {
	for(int i=0;i<count;i++) {
		entity_create((pixel_to_sub(7 + (rand() & 31))) << 3,
					  block_to_sub(1 + (rand() & 3)), objtype, NPC_OPTION1);
	}
}

// switches on and off the jumping frame/sprite
static void set_jump_sprite(Entity *e, uint8_t enable) {
	if(enable) {
		e->jump_time = TRUE;
		e->display_box.top += 8;
		bbox_mode = BM_JUMPING;
	} else {
		e->jump_time = FALSE;
		e->display_box.top -= 8;
		bbox_mode = BM_STAND;
	}
}

static uint8_t player_bbox_collide(Entity *e, uint8_t index) {
	int16_t ax1 = sub_to_pixel(player.x) - (player.dir ? player.hit_box.right : player.hit_box.left),
		ax2 = sub_to_pixel(player.x) + (player.dir ? player.hit_box.left : player.hit_box.right),
		ay1 = sub_to_pixel(player.y) - player.hit_box.top,
		ay2 = sub_to_pixel(player.y) + player.hit_box.bottom,
		bx1 = sub_to_pixel(e->x) - (e->dir ? bbox[index].right : bbox[index].left),
		bx2 = sub_to_pixel(e->x) + (e->dir ? bbox[index].left : bbox[index].right),
		by1 = sub_to_pixel(e->y) - bbox[index].top,
		by2 = sub_to_pixel(e->y) + bbox[index].bottom;
	return (ax1 < bx2 && ax2 > bx1 && ay1 < by2 && ay2 > by1);
}

static Bullet* bullets_bbox_collide(Entity *e, uint8_t index) {
	for(uint8_t i = 0; i < MAX_BULLETS; i++) {
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

static void deflect_bullet(Bullet *b) {
	b->ttl = 0;
	sound_play(SND_TINK, 5);
}

static void hurt_by_bullet(Entity *e, Bullet *b) {
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
		if(b->damage < e->health) sound_play(e->hurtSound, 5);
	}
	if(e->health <= b->damage) {
		effect_create_damage(e->damage_value - b->damage, e, 0, 0);
		e->damage_time = e->damage_value = 0;
		// Killed enemy
		e->health = 0;
		ENTITY_ONDEATH(e);
	}
	if((/*e->eflags|*/e->flags) & NPC_SHOWDAMAGE) {
		e->damage_value -= b->damage;
		e->damage_time = 30;
	}
	e->health -= b->damage;
}
