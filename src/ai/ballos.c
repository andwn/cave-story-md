#include "ai_common.h"

#define platform_speed	curly_target_x
#define rotators_left	curly_target_y

#define angle		jump_time
#define timer3		id

#define eye			pieces
#define body		e->linkedEntity
#define shield		body->linkedEntity
#define rotator(i)	pieces[2+(i)]

#define FLOOR_Y			0x26000							// Y coord of floor
#define CRASH_Y			(FLOOR_Y - pixel_to_sub(40))	// Y coord of main when body hits floor

enum EYE_STATES {
	EYE_OPENING		= 10,
	EYE_CLOSING		= 20,
	EYE_INVISIBLE	= 30,
	EYE_EXPLODING	= 40
};

enum BS_STATES {
	// Form 1 states
	AS_COME_DOWN	= 100,		// scripted
	AS_BEGIN_FIGHT	= 200,		// scripted
	AS_PREPARE_JUMP	= 210,
	AS_JUMPING		= 215,
	AS_DEFEATED		= 220,		// scripted
	
	// Form 2 states.
	BS_ENTER_FORM	= 300,		// scripted
	BS_FIGHT_BEGIN	= 311,		// scripted
	BS_LEFT			= 320,
	BS_UP			= 330,
	BS_RIGHT		= 340,
	BS_DOWN			= 350,
	
	// Form 3 states
	CS_ENTER_FORM		= 400,
	CS_SPAWN_SPIKES 	= 410,
	CS_EXPLODE_BLOODY	= 420,
	CS_SPIN_PLATFORMS	= 430
};

// forward
void ai_ballos_f2(Entity *e);
void ai_ballos_f3(Entity *e);

static const int32_t rotator_xy_phase2[256*2];
static const int32_t rotator_xy_phase3[256*2];
static const int32_t platform_xy[256*2];

static void SetEyeStates(uint16_t newstate) {
	eye[0]->state = newstate;
	eye[1]->state = newstate;
}

static void SetRotatorStates(uint16_t newstate) {
	for(uint16_t i = 0; i < 8; i++) {
		rotator(i)->state = newstate;
	}
}

static uint8_t transfer_damage(Entity *src, Entity *tgt) {
	if(src->health < 1000) {
		uint16_t dmg = 1000 - src->health;
		src->health = 1000;
		if(tgt->health > dmg) {
			tgt->health -= dmg;
			return FALSE;
		} else {
			tgt->health = 0;
			return TRUE;
		}
	}
	return FALSE;
}

// left and right maximums during form 1
static const int32_t F1_LEFT = pixel_to_sub(88);
static const int32_t F1_RIGHT = pixel_to_sub(552);

void onspawn_ballos(Entity *e) {
	e->alwaysActive = TRUE;
	e->flags = (NPC_SOLID | NPC_SHOWDAMAGE | NPC_EVENTONDEATH);
	
	e->x = block_to_sub(g_stage.pxm.width >> 1);
	e->y = -pixel_to_sub(64);
	
	e->attack = 0;
	e->health = 800;
	
	e->hidden = TRUE;
	
	e->hit_box = (bounding_box) {{ 32, 48, 32, 48 }};
	
	// create body (the big rock)
	body = entity_create(0, 0, OBJ_BALLOS_BODY, 0);
	body->health = 1000;	// not his real HP, we're using damage transfer
	body->flags = (NPC_SOLID | NPC_SHOOTABLE | NPC_INVINCIBLE);
	body->hit_box = (bounding_box) {{ 48, 20, 48, 36 }};
	body->display_box = (bounding_box) {{ 60, 60, 60, 60 }};
	
	// create eyes (open/close animations)
	for(uint8_t i=0;i<2;i++) {
		eye[i] = entity_create(0, 0, OBJ_BALLOS_EYE, 0);
		eye[i]->health = 1000;
		eye[i]->hit_box = (bounding_box) {{ 12, 8, 12, 8 }};
		eye[i]->display_box = (bounding_box) {{ 12, 8, 12, 8 }};
	}
	eye[1]->flags |= NPC_OPTION2;
	
	// create a top shield to cover eyes from above
	shield = entity_create(0, 0, OBJ_BALLOS_SHIELD, 0);
	//shield->sprite = SPR_BBOX_PUPPET_1;
	shield->hidden = TRUE;
	shield->health = 1000;
	shield->flags = (NPC_SPECIALSOLID | NPC_SHOOTABLE | NPC_INVINCIBLE);
	shield->hit_box = (bounding_box) {{ 32, 6, 32, 4 }};
}

void ondeath_ballos(Entity *e) {
	// as soon as one of his forms is defeated make him non-killable
	// until the init for the next form runs and makes him killable again.
	// intended to fix the extremely rare possibility of killing him completely
	// after his 1st form instead of moving on to the spiky rotators like he should.
	e->health = 0xFFFF;
	tsc_call_event(1000); // defeated script (has a flagjump in it to handle each form)
}

// 1st form as a stageboss.
// the one where he jumps around as a rock.
void ai_ballos_f1(Entity *e) {
	if(transfer_damage(eye[0], e) || transfer_damage(eye[1], e)) {
		ondeath_ballos(e);
		return;
	}
	switch(e->state) {
		case AS_COME_DOWN:
		{
			e->x = player.x;
			e->y = -pixel_to_sub(64);
			e->frame = 0;
			
			// create the targeter
			// setting dir to right tells it don't spawn any lightning
			entity_create(e->x, FLOOR_Y, OBJ_BALLOS_TARGET, 0)->dir = 1;
			e->timer = 0;
			
			if (e->x < F1_LEFT) e->x = F1_LEFT;
			if (e->x > F1_RIGHT) e->x = F1_RIGHT;
			
			e->state++;
		} /* fallthrough */
		case AS_COME_DOWN+1:
		{
			if (++e->timer > TIME_8(30)) e->state++;
		}
		break;
		
		// falling
		case AS_COME_DOWN+2:
		{
			e->y_speed += SPEED_8(0x40);
			LIMIT_Y(SPEED_12(0xC00));
			
			if(e->y + e->y_speed > CRASH_Y) {
				e->y_speed = 0;
				e->timer = 0;
				e->state++;
				// player smush damage
				if(!playerIFrames && PLAYER_DIST_X(e, pixel_to_sub(48)) && PLAYER_DIST_Y2(e, 0, pixel_to_sub(64))) {
					player_inflict_damage(16);
				}
				camera_shake(30);
				SMOKE_AREA((e->x >> CSF) - 40, (e->y >> CSF) + 40, 80, 16, 6);
				if(player.grounded) player.y_speed = -SPEED_10(0x200);
			}
		}
		break;
		
		case AS_COME_DOWN+3:
		{
			if (++e->timer > 31) {
				SetEyeStates(EYE_OPENING);
				e->state++;
			}
		}
		break;
		case AS_BEGIN_FIGHT:
		{
			// can be damaged between eyes opening and boss bar appearing,
			// but it is not counted.
			e->health = 800;
			e->state = AS_PREPARE_JUMP;
		} /* fallthrough */
		case AS_PREPARE_JUMP:	// delay, then jump at player
		{
			e->x_speed = 0;
			e->attack = 0;
			e->state++;
			// he makes two jumps then a pause,
			// after that it's three jumps before pausing.
			// this corresponds to:
			if(++e->timer2 >= 3) {
				e->timer2 = 0;
				e->timer = TIME_8(150);
			} else {
				e->timer = TIME_8(50);
			}
		} /* fallthrough */
		case AS_PREPARE_JUMP+1:
		{
			if(e->timer == 0) {
				e->y_speed = -SPEED_12(0xC00);
				e->x_speed = (e->x < player.x) ? SPEED_10(0x200) : -SPEED_10(0x200);
				e->state = AS_JUMPING;
			} else e->timer--;
		}
		break;
		
		case AS_JUMPING:
		{
			if(e->y_speed < SPEED_12(0xBB0)) e->y_speed += SPEED_8(0x55);
			
			if(e->x + e->x_speed < F1_LEFT) e->x_speed = SPEED_10(0x200);
			if(e->x + e->x_speed > F1_RIGHT) e->x_speed = -SPEED_10(0x200);
			
			if(e->y + e->y_speed > CRASH_Y) {
				// player smush damage
				if(!playerIFrames && PLAYER_DIST_X(e, pixel_to_sub(48)) && PLAYER_DIST_Y2(e, 0, pixel_to_sub(64))) {
					player_inflict_damage(16);
				}
				// player hopping from the vibration
				if(player.grounded) player.y_speed = -SPEED_10(0x200);
				camera_shake(30);
				
				entity_create(e->x - pixel_to_sub(12), e->y + pixel_to_sub(52), OBJ_BALLOS_BONE_SPAWNER, 0)->dir = 0;
				entity_create(e->x + pixel_to_sub(12), e->y + pixel_to_sub(52), OBJ_BALLOS_BONE_SPAWNER, 0)->dir = 1;
				//SmokeXY(e->x, e->y + pixel_to_sub(40), 16, 40, 0);
				
				e->y_speed = 0;
				e->state = AS_PREPARE_JUMP;
			}
		}
		break;
		
		// 1st form defeated
		case AS_DEFEATED:
		{
			SetEyeStates(EYE_CLOSING);
			e->health = 1200;
			e->state++;
			e->x_speed = 0;
			e->damage_time = 0;
		} /* fallthrough */
		case AS_DEFEATED+1:
		{
			if(e->y_speed < SPEED_12(0xBC0)) e->y_speed += SPEED_8(0x40);
			
			if(e->y + e->y_speed > CRASH_Y) {
				e->y_speed = 0;
				e->state++;
				camera_shake(30);
				//SmokeXY(e->x, e->y + 0x5000, 16, 40, 0);
				if (player.grounded) player.y_speed = -SPEED_10(0x200);
				// ... and wait for script to trigger form 2
			}
		}
		break;
		case BS_ENTER_FORM:
		{
			e->type = OBJ_BALLOS_FORM2;
			e->onFrame = &ai_ballos_f2;
		}
		break;
	}
	
	e->x += e->x_speed;
	e->y += e->y_speed;
	// place eyes
	eye[0]->x = e->x - pixel_to_sub(24);
	eye[1]->x = e->x + pixel_to_sub(24);
	eye[0]->y = eye[1]->y = e->y - pixel_to_sub(28);
	// place body
	body->x = e->x;
	body->y = e->y;
	//body->x_speed = e->x_speed;
	//body->y_speed = e->y_speed;
	// place shield
	shield->x = e->x;
	shield->y = e->y - pixel_to_sub(44);
	//shield->x_speed = e->x_speed;
	//shield->y_speed = e->y_speed;
	// The speeds are set above, because the player platform code uses it to move
	// the player along the object they are standing on. It is not used by the object itself
}

// 2nd form as a stageboss.
// the one where he spawns spiky rotators and circles around the room.
void ai_ballos_f2(Entity *e) {
	static const int16_t BS_SPEED = 0x3AA;
	static const int32_t ARENA_LEFT = pixel_to_sub(119);
	static const int32_t ARENA_TOP = pixel_to_sub(119);
	static const int32_t ARENA_RIGHT = pixel_to_sub(521);
	static const int32_t ARENA_BOTTOM = pixel_to_sub(233);
	
	if(transfer_damage(eye[0], e) || transfer_damage(eye[1], e)) {
		ondeath_ballos(e);
		return;
	}
	
	switch(e->state) {
		// enter 2nd form (script-triggered)
		case BS_ENTER_FORM:
		{
			e->timer = 0;
			e->state++;
			rotators_left = 0;
			for(uint16_t angle=0;angle<0x100;angle+=0x20) {
				rotator(rotators_left) = entity_create(e->x, e->y, OBJ_BALLOS_ROTATOR, 0);
				rotator(rotators_left)->angle = angle;
				rotator(rotators_left)->dir = (rotators_left & 1) ? 1 : 0;
				rotators_left++;
			}
		}
		break;
		
		case BS_FIGHT_BEGIN:	// script-triggered
		{
			SetRotatorStates(10);	// spin CCW, work as treads
			e->state = BS_LEFT;
			e->timer = 0;
		} /* fallthrough */
		case BS_LEFT:		// left on floor
		{
			e->x_speed = -SPEED_10(BS_SPEED);
			e->y_speed = 0;
			if(e->x + e->x_speed < ARENA_LEFT) e->state = BS_UP;
		}
		break;
		// up on wall
		case BS_UP:
		{
			e->x_speed = 0;
			e->y_speed = -SPEED_10(BS_SPEED);
			if(e->y + e->y_speed < ARENA_TOP) e->state = BS_RIGHT;
		}
		break;
		// right on ceiling
		case BS_RIGHT:
		{
			e->x_speed = SPEED_10(BS_SPEED);
			e->y_speed = 0;
			// all rotators destroyed?
			if (rotators_left <= 0 && ++e->timer > 3) {
				// center of room
				if (e->x >= pixel_to_sub(312) && e->x <= pixel_to_sub(344)) {
					e->state = CS_ENTER_FORM;
					e->type = OBJ_BALLOS_FORM3;
					e->onFrame = &ai_ballos_f3;
				}
			}
			
			if(e->x + e->x_speed > ARENA_RIGHT) e->state = BS_DOWN;
		}
		break;
		// down on wall
		case BS_DOWN:
		{
			e->x_speed = 0;
			e->y_speed = SPEED_10(BS_SPEED);
			if(e->y + e->y_speed > ARENA_BOTTOM) e->state = BS_LEFT;
		}
		break;
	}
	
	e->x += e->x_speed;
	e->y += e->y_speed;
	// place eyes
	eye[0]->x = e->x - pixel_to_sub(24);
	eye[1]->x = e->x + pixel_to_sub(24);
	eye[0]->y = eye[1]->y = e->y - pixel_to_sub(28);
	// place body
	body->flags ^= NPC_SHOOTABLE;
	body->x = e->x;
	body->y = e->y;
	//body->x_speed = e->x_speed;
	//body->y_speed = e->y_speed;
	// place shield
	shield->flags ^= NPC_SHOOTABLE;
	shield->x = e->x;
	shield->y = e->y - pixel_to_sub(44);
	//shield->x_speed = e->x_speed;
	//shield->y_speed = e->y_speed;
}

// form 3 as a stageboss, the final form.
// he reaches the center of the room, platforms come out, spikes go up,
// and he explodes into a really bloody version of himself.
//
// then the platforms spin in various speeds and directions while he
// spawns red butes from the sides and his top.
void ai_ballos_f3(Entity *e) {
	static const int32_t YPOSITION = pixel_to_sub(167);
	// platform spin speeds and how long they travel at each speed.
	// it's a repeating pattern.
	static const struct {
		uint16_t length;
		int16_t speed;
	} platform_pattern[] = {
		{ 500, 2, },
		{ 200, 1, },
		{ 20,  0, },
		{ 200, -1, },
		{ 500, -2, },
		{ 200, -1, },
		{ 20,  0, },
		{ 200, 1, },
		{ 0,   0, },
	};
	
	/*transfer_damage(body, e) || */
	if(transfer_damage(eye[0], e) || transfer_damage(eye[1], e)) {
		ondeath_ballos(e);
		return;
	}
	
	switch(e->state) {
		// enter form 3
		case CS_ENTER_FORM:
		{
			e->timer = 0;
			e->x_speed = 0;
			e->y_speed = 0;
			e->state++;
			
			entities_clear_by_type(OBJ_GREEN_DEVIL_SPAWNER);
			SetRotatorStates(20);	// fast spin CCW
		} /* fallthrough */
		case CS_ENTER_FORM+1:
		{
			// come down into center of room
			e->y += (YPOSITION - e->y) >> 3;
			e->timer++;
			
			if (e->timer == TIME_8(50)) {
				// create platforms
				platform_speed = 0;
				
				for(uint16_t angle=0x10;angle<0x100;angle+=0x20) {
					Entity *p = entity_create(e->x, e->y, OBJ_BALLOS_PLATFORM, 0);
					p->angle = angle;
				}
			}
			
			if (e->timer > TIME_8(100)) {
				platform_speed = -1;
				
				e->state = CS_SPAWN_SPIKES;
				e->timer = 0;
			}
		}
		break;
		
		case CS_SPAWN_SPIKES:
		{
			entities_clear_by_type(OBJ_SPIKE_SMALL);
			e->timer = 0;
			e->x_mark = 0;
			e->state++;
		} /* fallthrough */
		case CS_SPAWN_SPIKES+1:
		{
			e->timer++;
			
			if ((e->timer & 7) == 0) sound_play(SND_QUAKE, 2);
			
			if ((e->timer & 31) == 1) {
				e->x_mark += 2;
				entity_create(block_to_sub(e->x_mark), FLOOR_Y + pixel_to_sub(48), OBJ_BALLOS_SPIKES, 0);
				
				if (e->x_mark == 38) e->state = CS_EXPLODE_BLOODY;
			}
		}
		break;
		
		// explode into all bloody
		case CS_EXPLODE_BLOODY:
		{
			SetEyeStates(EYE_INVISIBLE);
			SetRotatorStates(30);			// slow spin CW, alternate open/closed
			
			//SmokeClouds(o, 256, 60, 60);	// ka boom!
			sound_play(SND_EXPLOSION1, 5);
			camera_shake(30);
			
			body->frame = 1;		// go all bloody
			body->flags &= ~NPC_INVINCIBLE;
			//shield->eflags &= ~NPC_INVINCIBLE;
			
			e->state = CS_SPIN_PLATFORMS;
		} /* fallthrough */
		// fall-through
		case CS_SPIN_PLATFORMS:
		{
			e->state++;
			e->timer = 0;
			e->timer2 = 0;
			e->timer3 = 0;
			
			platform_speed = platform_pattern[e->timer2].speed;
		} /* fallthrough */
		case CS_SPIN_PLATFORMS+1:
		{
			// spin platforms
			if (++e->timer3 > platform_pattern[e->timer2].length) {
				e->timer3 = 0;
				e->timer2++;
				
				if (!platform_pattern[e->timer2].length) e->timer2 = 0;
				
				platform_speed = platform_pattern[e->timer2].speed;
			}
			
			// spawn butes
			if(entity_active_count < 28) {
				switch(++e->timer) {
					case 270:	// spawn swordsmen from face
					case 281:
					case 292:
					{
						effect_create(EFF_SMOKE, e->x >> CSF, (e->y >> CSF) - 52, FALSE);
						entity_create(e->x, e->y - pixel_to_sub(52), OBJ_BUTE_SWORD_RED, 0);
						sound_play(SND_EM_FIRE, 5);
					}
					break;
					
					case 303:	// spawn archers on side
					case 314:
					{
						// direction butes will be facing, not side of screen
						uint16_t flags = (player.x > e->x) ? 0 : NPC_OPTION2;
						for(uint16_t i = 0; i < 2; i++) {
							// give some granularity to the coords,
							// so that they can't overlap too closely.
							int32_t x = (rand() & 3) << (CSF+2);
							int32_t y = (6 + (rand() & 63)) << (CSF+2);
							if(!flags) x += block_to_sub(g_stage.pxm.width - 1);
							entity_create(x, y, OBJ_BUTE_ARCHER_RED, flags);
						}
					}
					break;
					case 320:
					{
						if(use_pal_speed) e->timer = 0;
					}
					break;
					case 390:
					{
						e->timer = 0;
					}
					break;
				}
			}

			// spawn blood
			//int prob = (e->hp <= 500) ? 4 : 10;
			//if (!rand(0, prob)) {
			//	CreateEntity(e->x + rand(-40<<CSF, 40<<CSF),
			//				 e->y + rand(0, 40<<CSF),
			//				 OBJ_RED_ENERGY)->angle = DOWN;
			//}
		}
		break;
		// Defeated
		case 1000:
		{
			e->state = 1001;
			e->timer = 0;

			SetEyeStates(EYE_EXPLODING);	// blow out eyes
			SetRotatorStates(1000);			// explode rotators
			
			uint16_t mask = ~(NPC_SPECIALSOLID | NPC_SOLID | NPC_SHOOTABLE | NPC_INVINCIBLE);
			e->flags &= mask;
			body->flags &= mask;
			shield->flags &= mask;
			eye[0]->flags &= mask;
			eye[1]->flags &= mask;
		} /* fallthrough */
		case 1001:
		{
			int16_t x = (e->x >> CSF) - 0x3F + (rand() & 0x7F);
			int16_t y = (e->y >> CSF) - 0x3F + (rand() & 0x7F);
			effect_create(EFF_SMOKE, x, y, FALSE);
			effect_create(x, y, EFF_DISSIPATE, FALSE);
			
			e->timer++;
			
			if((e->timer & 15) == 0)
				sound_play(SND_MISSILE_HIT, 5);
			
			if(e->timer > TIME_8(150)) {
				e->timer = 0;
				e->state = 1002;
				
				SCREEN_FLASH(4);
				sound_play(SND_EXPLOSION1, 5);
			}
		}
		break;
		
		case 1002:
		{
			camera_shake(40);
			
			if(++e->timer >= TIME_8(50)) {
				tsc_hide_boss_health(); // Health bar sticks around if we don't do this
				entities_clear_by_type(OBJ_BUTE_ARCHER_RED);
				entities_clear_by_type(OBJ_BUTE_SWORD_RED);
				entities_clear_by_type(OBJ_BALLOS_SPIKES);
				
				body->hidden = TRUE;
				eye[0]->hidden = TRUE;
				eye[1]->hidden = TRUE;
				bossEntity = NULL;
				e->state = 1003;
			}
		}
		break;
	}
	
	e->x += e->x_speed;
	e->y += e->y_speed;
	// place eyes
	eye[0]->x = e->x - pixel_to_sub(24);
	eye[1]->x = e->x + pixel_to_sub(24);
	eye[0]->y = eye[1]->y = e->y - pixel_to_sub(28);
	// place body
	body->flags ^= NPC_SHOOTABLE;
	body->x = e->x;
	body->y = e->y;
	// place shield
	shield->flags ^= NPC_SHOOTABLE;
	shield->x = e->x;
	shield->y = e->y - pixel_to_sub(44);
}

// Handles his eyes.
//
// When closed, the eyes are like "overlay" objects that replace the open eyes
// drawn on the body, and allow animating them seperately from the body.
//
// When open, the eyes turn invisible and are used as shoot-points to detect shots
// hitting the eyes drawn on the body.
void ai_ballos_eye(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->frame = (e->flags & NPC_OPTION2) ? 4 : 0;
			e->flags |= (NPC_SHOOTABLE | NPC_INVINCIBLE);
			e->state = 1;
		}
		break;
		
		// open eyes
		case EYE_OPENING:
		{
			e->frame = (e->flags & NPC_OPTION2) ? 4 : 0;
			e->animtime = 0;
			e->state++;
		} /* fallthrough */
		case EYE_OPENING+1:
		{
			if (++e->animtime > 4) {
				e->animtime = 0;
				e->frame++;
				if ((e->frame & 3) == 3) {
					e->flags &= ~NPC_INVINCIBLE;
					e->hidden = TRUE;
					e->state++;
				}
			}
		}
		break;
		case EYE_OPENING+2:
		{
			// Flash main body when hit
			if(!bossEntity) break;
			Entity *ballos = bossEntity->linkedEntity;
			if(e->damage_time & 1) {
				for(uint16_t i = 0; i < ballos->sprite_count; i++) {
					sprite_pal(&ballos->sprite[i], PAL2);
				}
			} else {
				for(uint16_t i = 0; i < ballos->sprite_count; i++) {
					sprite_pal(&ballos->sprite[i], PAL0);
				}
			}
		}
		break;
		
		// close eyes
		case EYE_CLOSING:
		{
			e->frame = (e->flags & NPC_OPTION2) ? 6 : 2;
			e->hidden = FALSE;
			e->flags |= NPC_INVINCIBLE;
			// Stop flashing
			if(bossEntity) {
				Entity *ballos = bossEntity->linkedEntity;
				for(uint16_t i = 0; i < ballos->sprite_count; i++) {
					sprite_pal(&ballos->sprite[i], PAL0);
				}
			}

			e->animtime = 0;
			e->state++;
		} /* fallthrough */
		case EYE_CLOSING+1:
		{
			if (++e->animtime > 4) {
				e->animtime = 0;
				e->frame--;
				if ((e->frame & 3) == 0) {
					e->state++;
				}
			}
		}
		break;
		
		// invisible (the underlying eyes drawn on the body are what are seen)
		case EYE_INVISIBLE:
		{
			e->flags &= ~NPC_INVINCIBLE;
			e->hidden = TRUE;
			e->state++;
		}
		break;
		case EYE_INVISIBLE+1:
		{
			// Flash main body when hit
			if(!bossEntity) break;
			Entity *ballos = bossEntity->linkedEntity;
			if(e->damage_time & 1) {
				for(uint16_t i = 0; i < ballos->sprite_count; i++) {
					sprite_pal(&ballos->sprite[i], PAL2);
				}
			} else {
				for(uint16_t i = 0; i < ballos->sprite_count; i++) {
					sprite_pal(&ballos->sprite[i], PAL0);
				}
			}
		}
		break;
		
		// explode eyes (final defeat sequence)
		case EYE_EXPLODING:
		{
			e->frame = (e->flags & NPC_OPTION2) ? 7 : 3;	// empty eyes
			e->hidden = FALSE;
			
			e->flags &= ~(NPC_SHOOTABLE | NPC_INVINCIBLE);
			e->state++;

			// Stop flashing
			if(bossEntity) {
				Entity *ballos = bossEntity->linkedEntity;
				for(uint16_t i = 0; i < ballos->sprite_count; i++) {
					sprite_pal(&ballos->sprite[i], PAL0);
				}
			}

			//if (e->dir == LEFT)
			//	SmokeXY(e->x - (4<<CSF), e->y, 10, 4, 4);
			//else
			//	SmokeXY(e->x + (4<<CSF), e->y, 10, 4, 4);
		}
		break;
	}
}

void ai_ballos_rotator(Entity *e) {
	switch(e->state) {
		case 0:		// just spawned
		{
			e->frame = 0;
			e->state = 1;
			e->timer2 = e->angle << 1;
			
			e->timer3 = 0xC0;
			e->attack = 14;
		} /* fallthrough */
		case 1:		// expanding outward; overshoot a bit
		{
			if (e->timer3 < 0x140)
				e->timer3 += 0x08;
			else
				e->state = 2;
		}
		break;
		case 2:		// come back in to correct distance
		{
			if (e->timer3 > 0x130)
				e->timer3 -= 0x04;
			else
				e->state = 3;
		}
		break;
		
		// spinning CCW during form 2 (working like treads)
		case 10:
		{
			e->state = 11;
			
			if(e->dir) e->flags |= NPC_SHOOTABLE; // Interlace shootable state
			e->flags &= ~NPC_INVINCIBLE;
			e->health = 1000;
		} /* fallthrough */
		case 11:		// spinning during phase 2, alive
		{
			e->flags ^= NPC_SHOOTABLE;

			if (e->timer2 <= 1) e->timer2 += 0x200;
			e->timer2 -= 2;
			
			if (e->frame != 1)		// still undestroyed?
			{
				//e->frame = (e->damage_time & 2) ? 1 : 0;
				
				if (e->health <= (1000 - 100)) {
					e->flags &= ~NPC_SHOOTABLE;
					e->frame = 1;	// close eye
					
					//SmokeClouds(o, 32, 16, 16);
					sound_play(SND_LITTLE_CRASH, 5);

					// Stop flashing
					sprite_pal(e->sprite, PAL0);
					
					rotators_left--;
				} else {
					// Flash when hit
					if(e->damage_time & 1) {
						sprite_pal(e->sprite, PAL2);
					} else {
						sprite_pal(e->sprite, PAL0);
					}
				}
			}
			
			//spawn_impact_puffs(o);
		}
		break;
		
		case 20:	// spinning fast CCW while spikes come up
		{
			e->flags &= ~NPC_SHOOTABLE;

			e->frame = 1;
			
			if (e->timer2 <= 3) e->timer2 += 0x200;
			e->timer2 -= 4;
		}
		break;
		
		case 30:	// beginning form 3
		{
			e->state = 31;
			e->health = 1000;
			e->attack = 10;
			
			// this dir was set when they were created and
			// alternates left/right around the circle
			if(!e->dir) {
				//e->flags |= NPC_SHOOTABLE; // Interlace shootable state
				e->frame = 0;
			} else {
				e->flags |= (NPC_SHOOTABLE | NPC_INVINCIBLE);
				e->frame = 1;
			}
		} /* fallthrough */
		case 31:		// form 3 CW slow spin
		{
			// come in closer to main object
			if(e->timer3 > 0x100)
				e->timer3--;
			
			// spin CW
			if(++e->timer2 > 0x200)
				e->timer2 -= 0x200;
			
			e->flags ^= NPC_SHOOTABLE;
			if(!(e->flags & NPC_INVINCIBLE)) {
				if (e->health < (1000 - 100)) {
					e->x_speed = 0;
					e->y_speed = 0;
					
					e->flags &= ~NPC_SHOOTABLE;
					//SmokeClouds(o, 32, 16, 16);
					sound_play(SND_LITTLE_CRASH, 5);
					
					// Stop flashing
					sprite_pal(e->sprite, PAL0);
					
					e->frame = 1;
					e->state = 40;
					e->attack = 5;
					
					// blow up immediately if Ballos is defeated
					e->timer2 = 0;
				} else {
					// Flash when hit
					if(e->damage_time & 1) {
						sprite_pal(e->sprite, PAL2);
					} else {
						sprite_pal(e->sprite, PAL0);
					}
				}
			}
		}
		break;
		
		case 40:	// destroyed during phase 3, bouncing
		{
			if(e->y_speed < SPEED_12(0x5E0)) e->y_speed += SPEED_8(0x20);
			
			if(blk(e->x, -16, e->y, 0) == 0x41) e->x_speed = SPEED_8(0xFF);
			if(blk(e->x, 16, e->y, 0) == 0x41) e->x_speed = -SPEED_8(0xFF);
			
			if(e->y_speed >= 0 && blk(e->x, 0, e->y, 16) == 0x41) {
				// first time they hit they head toward player, after that
				// they keep going in same direction until hit wall
				if(e->x_speed == 0) {
					e->x_speed = (e->x < player.x) ? SPEED_8(0xFF) : -SPEED_8(0xFF);
				}
				e->y_speed = -SPEED_12(0x800);
				sound_play(SND_QUAKE, 5);
			}

			e->x += e->x_speed;
			e->y += e->y_speed;
		}
		break;
		
		case 1000:		// Ballos was defeated
		{
			e->state = 1001;
			e->x_speed = 0;
			e->y_speed = 0;
			
			e->frame = 1;
			e->flags &= ~(NPC_SHOOTABLE | NPC_IGNORESOLID);
			e->attack = 0;
			
			e->timer2 >>= 2;
		} /* fallthrough */
		case 1001:
		{
			// explode one by one going clockwise
			if (e->timer2 <= 0) {
				//SmokeClouds(o, 32, 16, 16);
				sound_play(SND_LITTLE_CRASH, 5);
				//effect(e->x, e->y, EFFECT_BOOMFLASH);
				e->state = STATE_DELETE;
			} else {
				e->timer2--;
				//e->frame = (e->timer2 & 2) ? 1 : 0;
			}
		}
		break;
	}
	
	if (e->state < 40) {
		Entity *ballos = bossEntity;
		if (!ballos) return;
		
		uint8_t angle = e->timer2 >> 1;
		if(e->state < 3) {
			uint16_t dist = e->timer3 >> 2;
			e->x = ballos->x + 0x800 + (((int32_t)cos[angle]) * dist);
			e->y = ballos->y + 0x800 + (((int32_t)sin[angle]) * dist);
		} else if(e->state < 30) {
			e->x = ballos->x + 0x800 + rotator_xy_phase2[angle*2]; //(((int32_t)cos[angle]) << 6) + (((int32_t)cos[angle]) << 4);
			e->y = ballos->y + 0x800 + rotator_xy_phase2[angle*2+1]; //(((int32_t)sin[angle]) << 6) + (((int32_t)sin[angle]) << 4);
		} else {
			e->x = ballos->x + 0x800 + rotator_xy_phase3[angle*2]; //(((int32_t)cos[angle]) << 6);
			e->y = ballos->y + 0x800 + rotator_xy_phase3[angle*2+1]; //(((int32_t)sin[angle]) << 6);
		}
	}
}

void ai_ballos_platform(Entity *e) {
	Entity *ballos = bossEntity;
	if (!ballos) return;
	
	if (ballos->state >= 1000) {		// defeated
		if (e->state < 1000) e->state = 1000;
	}
	
	switch(e->state) {
		case 0:		// just spawned
		{
			e->timer2 = (e->angle << 2);
			e->timer3 = 0xC0;
			e->state = 1;
		} /* fallthrough */
		case 1:		// expanding outward
		{
			if (e->timer3 < 0x1C0) {
				e->timer3 += 8;
			} else {
				e->state = 2;
			}
		}
		break;
		
		// running - the direction/speed is set by the global variable,
		// controlled by Ballos.
		case 2:
		{
			e->timer2 = (e->timer2 + platform_speed) & 0x3FF;
		}
		break;
		
		case 1000:	// ballos defeated!
		{
			e->state = 1001;
			e->x_speed = 0;
			e->y_speed = 0;
			e->flags &= ~NPC_SPECIALSOLID;
		} /* fallthrough */
		case 1001:
		{
			e->y_speed += SPEED_8(0x40);
			e->x += e->x_speed;
			e->y += e->y_speed;
			if (e->y > block_to_sub(g_stage.pxm.height + 1)) {
				e->state = STATE_DELETE;
			}
		}
		return;
	}
	
	// let player jump up through platforms, but be solid when he is standing on them
	if (player.y_speed < 0 || player.y > e->y - 0x800 /*pixel_to_sub(e->hit_box.top)*/) {
		e->flags &= ~NPC_SPECIALSOLID;
	} else {
		e->flags |= NPC_SPECIALSOLID;
	}
	
	// spin
	const uint8_t angle = e->timer2 >> 2;
	int32_t xoff, yoff;
	if(e->state < 2) {
		// While expanding out use the extra cpu cycles to make it smooth
		xoff = ((int32_t)cos[angle]) * e->timer3;
		yoff = ((int32_t)sin[angle]) * e->timer3;
	} else {
		// After expanding use the table
		xoff = platform_xy[(angle * 2)];
		yoff = platform_xy[(angle * 2) + 1];
	}
	
	e->x_next = (xoff >> 2) + ballos->x;
	e->y_next = (yoff >> 2) + pixel_to_sub(12) + ballos->y;
	
	e->x_speed = (e->x_next - e->x) / 2;
	e->y_speed = (e->y_next - e->y) / 2;

	e->x += e->x_speed;
	e->y += e->y_speed;
}


/*
// spawns impact smokeclouds/skulls as the rotators hit the ground/walls
static void spawn_impact_puffs(Entity *e)
{
	Entity *ballos = game.stageboss.object;
	
	#define SHORT		(8<<CSF)
	#define LONG		(12<<CSF)
	#define HITANGLE	12
	static const struct
	{
		int wallangle;
		int xoffs1, xoffs2;
		int yoffs1, yoffs2;
	}
	hitdata[] =
	{
		0x180, SHORT, -SHORT, -LONG, -LONG,		// RIGHT on ceiling
		0x80,  SHORT, -SHORT, LONG, LONG,		// LEFT on floor
		0x100, -LONG, -LONG, SHORT, -SHORT,		// UP left wall
		0x00,  LONG, LONG,   SHORT, -SHORT		// DOWN right wall
	};
	
	int bd = ballos->dirparam;
	if (e->timer2 == hitdata[bd].wallangle + HITANGLE)
	{
		make_puff(e->x + hitdata[bd].xoffs1, e->y + hitdata[bd].yoffs1, bd);
		make_puff(e->x + hitdata[bd].xoffs2, e->y + hitdata[bd].yoffs2, bd);
		sound_play(SND_QUAKE, 5);
		
		if (bd == RIGHT)		// on ceiling
		{
			CreateEntity(e->x - SHORT, e->y - LONG, OBJ_BALLOS_SKULL);
		}
	}
}

static void make_puff(int x, int y, int bd)
{
	Entity *e = SmokePuff(x, y);
	
	// make sure the smoke puff is traveling away from floor/wall
	switch(bd)
	{
		case LEFT:	e->y_speed = -abs(e->y_speed); break;
		case UP:	e->x_speed = abs(e->x_speed); break;
		case RIGHT: e->y_speed = abs(e->y_speed); break;
		case DOWN:	e->x_speed = -abs(e->x_speed); break;
	}
}
*/

static const int32_t rotator_xy_phase3[256*2] = {
        0x00000000,0xFFFF8000, 0x00000324,0xFFFF800A, 0x00000647,0xFFFF8028, 0x0000096A,0xFFFF8059,
        0x00000C8B,0xFFFF809E, 0x00000FAB,0xFFFF80F7, 0x000012C8,0xFFFF8163, 0x000015E2,0xFFFF81E3,
        0x000018F8,0xFFFF8276, 0x00001C0B,0xFFFF831D, 0x00001F19,0xFFFF83D7, 0x00002223,0xFFFF84A3,
        0x00002528,0xFFFF8583, 0x00002826,0xFFFF8676, 0x00002B1F,0xFFFF877C, 0x00002E11,0xFFFF8894,
        0x000030FB,0xFFFF89BF, 0x000033DE,0xFFFF8AFC, 0x000036BA,0xFFFF8C4B, 0x0000398C,0xFFFF8DAB,
        0x00003C56,0xFFFF8F1E, 0x00003F17,0xFFFF90A1, 0x000041CE,0xFFFF9236, 0x0000447A,0xFFFF93DC,
        0x0000471C,0xFFFF9593, 0x000049B4,0xFFFF975A, 0x00004C3F,0xFFFF9931, 0x00004EBF,0xFFFF9B18,
        0x00005133,0xFFFF9D0E, 0x0000539B,0xFFFF9F14, 0x000055F5,0xFFFFA129, 0x00005842,0xFFFFA34C,
        0x00005A82,0xFFFFA57E, 0x00005CB4,0xFFFFA7BE, 0x00005ED7,0xFFFFAA0B, 0x000060EC,0xFFFFAC65,
        0x000062F2,0xFFFFAECD, 0x000064E8,0xFFFFB141, 0x000066CF,0xFFFFB3C1, 0x000068A6,0xFFFFB64C,
        0x00006A6D,0xFFFFB8E4, 0x00006C24,0xFFFFBB86, 0x00006DCA,0xFFFFBE32, 0x00006F5F,0xFFFFC0E9,
        0x000070E2,0xFFFFC3AA, 0x00007255,0xFFFFC674, 0x000073B5,0xFFFFC946, 0x00007504,0xFFFFCC22,
        0x00007641,0xFFFFCF05, 0x0000776C,0xFFFFD1EF, 0x00007884,0xFFFFD4E1, 0x0000798A,0xFFFFD7DA,
        0x00007A7D,0xFFFFDAD8, 0x00007B5D,0xFFFFDDDD, 0x00007C29,0xFFFFE0E7, 0x00007CE3,0xFFFFE3F5,
        0x00007D8A,0xFFFFE708, 0x00007E1D,0xFFFFEA1E, 0x00007E9D,0xFFFFED38, 0x00007F09,0xFFFFF055,
        0x00007F62,0xFFFFF375, 0x00007FA7,0xFFFFF696, 0x00007FD8,0xFFFFF9B9, 0x00007FF6,0xFFFFFCDC,
        0x00008000,0x00000000, 0x00007FF6,0x00000324, 0x00007FD8,0x00000647, 0x00007FA7,0x0000096A,
        0x00007F62,0x00000C8B, 0x00007F09,0x00000FAB, 0x00007E9D,0x000012C8, 0x00007E1D,0x000015E2,
        0x00007D8A,0x000018F8, 0x00007CE3,0x00001C0B, 0x00007C29,0x00001F19, 0x00007B5D,0x00002223,
        0x00007A7D,0x00002528, 0x0000798A,0x00002826, 0x00007884,0x00002B1F, 0x0000776C,0x00002E11,
        0x00007641,0x000030FB, 0x00007504,0x000033DE, 0x000073B5,0x000036BA, 0x00007255,0x0000398C,
        0x000070E2,0x00003C56, 0x00006F5F,0x00003F17, 0x00006DCA,0x000041CE, 0x00006C24,0x0000447A,
        0x00006A6D,0x0000471C, 0x000068A6,0x000049B4, 0x000066CF,0x00004C3F, 0x000064E8,0x00004EBF,
        0x000062F2,0x00005133, 0x000060EC,0x0000539B, 0x00005ED7,0x000055F5, 0x00005CB4,0x00005842,
        0x00005A82,0x00005A82, 0x00005842,0x00005CB4, 0x000055F5,0x00005ED7, 0x0000539B,0x000060EC,
        0x00005133,0x000062F2, 0x00004EBF,0x000064E8, 0x00004C3F,0x000066CF, 0x000049B4,0x000068A6,
        0x0000471C,0x00006A6D, 0x0000447A,0x00006C24, 0x000041CE,0x00006DCA, 0x00003F17,0x00006F5F,
        0x00003C56,0x000070E2, 0x0000398C,0x00007255, 0x000036BA,0x000073B5, 0x000033DE,0x00007504,
        0x000030FB,0x00007641, 0x00002E11,0x0000776C, 0x00002B1F,0x00007884, 0x00002826,0x0000798A,
        0x00002528,0x00007A7D, 0x00002223,0x00007B5D, 0x00001F19,0x00007C29, 0x00001C0B,0x00007CE3,
        0x000018F8,0x00007D8A, 0x000015E2,0x00007E1D, 0x000012C8,0x00007E9D, 0x00000FAB,0x00007F09,
        0x00000C8B,0x00007F62, 0x0000096A,0x00007FA7, 0x00000647,0x00007FD8, 0x00000324,0x00007FF6,
        0x00000000,0x00008000, 0xFFFFFCDC,0x00007FF6, 0xFFFFF9B9,0x00007FD8, 0xFFFFF696,0x00007FA7,
        0xFFFFF375,0x00007F62, 0xFFFFF055,0x00007F09, 0xFFFFED38,0x00007E9D, 0xFFFFEA1E,0x00007E1D,
        0xFFFFE708,0x00007D8A, 0xFFFFE3F5,0x00007CE3, 0xFFFFE0E7,0x00007C29, 0xFFFFDDDD,0x00007B5D,
        0xFFFFDAD8,0x00007A7D, 0xFFFFD7DA,0x0000798A, 0xFFFFD4E1,0x00007884, 0xFFFFD1EF,0x0000776C,
        0xFFFFCF05,0x00007641, 0xFFFFCC22,0x00007504, 0xFFFFC946,0x000073B5, 0xFFFFC674,0x00007255,
        0xFFFFC3AA,0x000070E2, 0xFFFFC0E9,0x00006F5F, 0xFFFFBE32,0x00006DCA, 0xFFFFBB86,0x00006C24,
        0xFFFFB8E4,0x00006A6D, 0xFFFFB64C,0x000068A6, 0xFFFFB3C1,0x000066CF, 0xFFFFB141,0x000064E8,
        0xFFFFAECD,0x000062F2, 0xFFFFAC65,0x000060EC, 0xFFFFAA0B,0x00005ED7, 0xFFFFA7BE,0x00005CB4,
        0xFFFFA57E,0x00005A82, 0xFFFFA34C,0x00005842, 0xFFFFA129,0x000055F5, 0xFFFF9F14,0x0000539B,
        0xFFFF9D0E,0x00005133, 0xFFFF9B18,0x00004EBF, 0xFFFF9931,0x00004C3F, 0xFFFF975A,0x000049B4,
        0xFFFF9593,0x0000471C, 0xFFFF93DC,0x0000447A, 0xFFFF9236,0x000041CE, 0xFFFF90A1,0x00003F17,
        0xFFFF8F1E,0x00003C56, 0xFFFF8DAB,0x0000398C, 0xFFFF8C4B,0x000036BA, 0xFFFF8AFC,0x000033DE,
        0xFFFF89BF,0x000030FB, 0xFFFF8894,0x00002E11, 0xFFFF877C,0x00002B1F, 0xFFFF8676,0x00002826,
        0xFFFF8583,0x00002528, 0xFFFF84A3,0x00002223, 0xFFFF83D7,0x00001F19, 0xFFFF831D,0x00001C0B,
        0xFFFF8276,0x000018F8, 0xFFFF81E3,0x000015E2, 0xFFFF8163,0x000012C8, 0xFFFF80F7,0x00000FAB,
        0xFFFF809E,0x00000C8B, 0xFFFF8059,0x0000096A, 0xFFFF8028,0x00000647, 0xFFFF800A,0x00000324,
        0xFFFF8000,0x00000000, 0xFFFF800A,0xFFFFFCDC, 0xFFFF8028,0xFFFFF9B9, 0xFFFF8059,0xFFFFF696,
        0xFFFF809E,0xFFFFF375, 0xFFFF80F7,0xFFFFF055, 0xFFFF8163,0xFFFFED38, 0xFFFF81E3,0xFFFFEA1E,
        0xFFFF8276,0xFFFFE708, 0xFFFF831D,0xFFFFE3F5, 0xFFFF83D7,0xFFFFE0E7, 0xFFFF84A3,0xFFFFDDDD,
        0xFFFF8583,0xFFFFDAD8, 0xFFFF8676,0xFFFFD7DA, 0xFFFF877C,0xFFFFD4E1, 0xFFFF8894,0xFFFFD1EF,
        0xFFFF89BF,0xFFFFCF05, 0xFFFF8AFC,0xFFFFCC22, 0xFFFF8C4B,0xFFFFC946, 0xFFFF8DAB,0xFFFFC674,
        0xFFFF8F1E,0xFFFFC3AA, 0xFFFF90A1,0xFFFFC0E9, 0xFFFF9236,0xFFFFBE32, 0xFFFF93DC,0xFFFFBB86,
        0xFFFF9593,0xFFFFB8E4, 0xFFFF975A,0xFFFFB64C, 0xFFFF9931,0xFFFFB3C1, 0xFFFF9B18,0xFFFFB141,
        0xFFFF9D0E,0xFFFFAECD, 0xFFFF9F14,0xFFFFAC65, 0xFFFFA129,0xFFFFAA0B, 0xFFFFA34C,0xFFFFA7BE,
        0xFFFFA57E,0xFFFFA57E, 0xFFFFA7BE,0xFFFFA34C, 0xFFFFAA0B,0xFFFFA129, 0xFFFFAC65,0xFFFF9F14,
        0xFFFFAECD,0xFFFF9D0E, 0xFFFFB141,0xFFFF9B18, 0xFFFFB3C1,0xFFFF9931, 0xFFFFB64C,0xFFFF975A,
        0xFFFFB8E4,0xFFFF9593, 0xFFFFBB86,0xFFFF93DC, 0xFFFFBE32,0xFFFF9236, 0xFFFFC0E9,0xFFFF90A1,
        0xFFFFC3AA,0xFFFF8F1E, 0xFFFFC674,0xFFFF8DAB, 0xFFFFC946,0xFFFF8C4B, 0xFFFFCC22,0xFFFF8AFC,
        0xFFFFCF05,0xFFFF89BF, 0xFFFFD1EF,0xFFFF8894, 0xFFFFD4E1,0xFFFF877C, 0xFFFFD7DA,0xFFFF8676,
        0xFFFFDAD8,0xFFFF8583, 0xFFFFDDDD,0xFFFF84A3, 0xFFFFE0E7,0xFFFF83D7, 0xFFFFE3F5,0xFFFF831D,
        0xFFFFE708,0xFFFF8276, 0xFFFFEA1E,0xFFFF81E3, 0xFFFFED38,0xFFFF8163, 0xFFFFF055,0xFFFF80F7,
        0xFFFFF375,0xFFFF809E, 0xFFFFF696,0xFFFF8059, 0xFFFFF9B9,0xFFFF8028, 0xFFFFFCDC,0xFFFF800A,
};
static const int32_t rotator_xy_phase2[256*2] = {
        0x00000000,0xFFFF6000, 0x000003ED,0xFFFF600D, 0x000007D9,0xFFFF6032, 0x00000BC5,0xFFFF606F,
        0x00000FAE,0xFFFF60C6, 0x00001395,0xFFFF6135, 0x0000177A,0xFFFF61BC, 0x00001B5A,0xFFFF625C,
        0x00001F36,0xFFFF6314, 0x0000230E,0xFFFF63E4, 0x000026E0,0xFFFF64CC, 0x00002AAC,0xFFFF65CC,
        0x00002E72,0xFFFF66E4, 0x00003230,0xFFFF6814, 0x000035E7,0xFFFF695B, 0x00003995,0xFFFF6AB9,
        0x00003D3A,0xFFFF6C2E, 0x000040D6,0xFFFF6DBA, 0x00004468,0xFFFF6F5D, 0x000047F0,0xFFFF7116,
        0x00004B6C,0xFFFF72E5, 0x00004EDD,0xFFFF74CA, 0x00005241,0xFFFF76C4, 0x00005599,0xFFFF78D3,
        0x000058E4,0xFFFF7AF8, 0x00005C21,0xFFFF7D30, 0x00005F4F,0xFFFF7F7D, 0x0000626F,0xFFFF81DE,
        0x00006580,0xFFFF8452, 0x00006881,0xFFFF86D9, 0x00006B73,0xFFFF8973, 0x00006E53,0xFFFF8C1F,
        0x00007123,0xFFFF8EDD, 0x000073E1,0xFFFF91AD, 0x0000768D,0xFFFF948D, 0x00007927,0xFFFF977F,
        0x00007BAE,0xFFFF9A80, 0x00007E22,0xFFFF9D91, 0x00008083,0xFFFFA0B1, 0x000082D0,0xFFFFA3DF,
        0x00008508,0xFFFFA71C, 0x0000872D,0xFFFFAA67, 0x0000893C,0xFFFFADBF, 0x00008B36,0xFFFFB123,
        0x00008D1B,0xFFFFB494, 0x00008EEA,0xFFFFB810, 0x000090A3,0xFFFFBB98, 0x00009246,0xFFFFBF2A,
        0x000093D2,0xFFFFC2C6, 0x00009547,0xFFFFC66B, 0x000096A5,0xFFFFCA19, 0x000097EC,0xFFFFCDD0,
        0x0000991C,0xFFFFD18E, 0x00009A34,0xFFFFD554, 0x00009B34,0xFFFFD920, 0x00009C1C,0xFFFFDCF2,
        0x00009CEC,0xFFFFE0CA, 0x00009DA4,0xFFFFE4A6, 0x00009E44,0xFFFFE886, 0x00009ECB,0xFFFFEC6B,
        0x00009F3A,0xFFFFF052, 0x00009F91,0xFFFFF43B, 0x00009FCE,0xFFFFF827, 0x00009FF3,0xFFFFFC13,
        0x0000A000,0x00000000, 0x00009FF3,0x000003ED, 0x00009FCE,0x000007D9, 0x00009F91,0x00000BC5,
        0x00009F3A,0x00000FAE, 0x00009ECB,0x00001395, 0x00009E44,0x0000177A, 0x00009DA4,0x00001B5A,
        0x00009CEC,0x00001F36, 0x00009C1C,0x0000230E, 0x00009B34,0x000026E0, 0x00009A34,0x00002AAC,
        0x0000991C,0x00002E72, 0x000097EC,0x00003230, 0x000096A5,0x000035E7, 0x00009547,0x00003995,
        0x000093D2,0x00003D3A, 0x00009246,0x000040D6, 0x000090A3,0x00004468, 0x00008EEA,0x000047F0,
        0x00008D1B,0x00004B6C, 0x00008B36,0x00004EDD, 0x0000893C,0x00005241, 0x0000872D,0x00005599,
        0x00008508,0x000058E4, 0x000082D0,0x00005C21, 0x00008083,0x00005F4F, 0x00007E22,0x0000626F,
        0x00007BAE,0x00006580, 0x00007927,0x00006881, 0x0000768D,0x00006B73, 0x000073E1,0x00006E53,
        0x00007123,0x00007123, 0x00006E53,0x000073E1, 0x00006B73,0x0000768D, 0x00006881,0x00007927,
        0x00006580,0x00007BAE, 0x0000626F,0x00007E22, 0x00005F4F,0x00008083, 0x00005C21,0x000082D0,
        0x000058E4,0x00008508, 0x00005599,0x0000872D, 0x00005241,0x0000893C, 0x00004EDD,0x00008B36,
        0x00004B6C,0x00008D1B, 0x000047F0,0x00008EEA, 0x00004468,0x000090A3, 0x000040D6,0x00009246,
        0x00003D3A,0x000093D2, 0x00003995,0x00009547, 0x000035E7,0x000096A5, 0x00003230,0x000097EC,
        0x00002E72,0x0000991C, 0x00002AAC,0x00009A34, 0x000026E0,0x00009B34, 0x0000230E,0x00009C1C,
        0x00001F36,0x00009CEC, 0x00001B5A,0x00009DA4, 0x0000177A,0x00009E44, 0x00001395,0x00009ECB,
        0x00000FAE,0x00009F3A, 0x00000BC5,0x00009F91, 0x000007D9,0x00009FCE, 0x000003ED,0x00009FF3,
        0x00000000,0x0000A000, 0xFFFFFC13,0x00009FF3, 0xFFFFF827,0x00009FCE, 0xFFFFF43B,0x00009F91,
        0xFFFFF052,0x00009F3A, 0xFFFFEC6B,0x00009ECB, 0xFFFFE886,0x00009E44, 0xFFFFE4A6,0x00009DA4,
        0xFFFFE0CA,0x00009CEC, 0xFFFFDCF2,0x00009C1C, 0xFFFFD920,0x00009B34, 0xFFFFD554,0x00009A34,
        0xFFFFD18E,0x0000991C, 0xFFFFCDD0,0x000097EC, 0xFFFFCA19,0x000096A5, 0xFFFFC66B,0x00009547,
        0xFFFFC2C6,0x000093D2, 0xFFFFBF2A,0x00009246, 0xFFFFBB98,0x000090A3, 0xFFFFB810,0x00008EEA,
        0xFFFFB494,0x00008D1B, 0xFFFFB123,0x00008B36, 0xFFFFADBF,0x0000893C, 0xFFFFAA67,0x0000872D,
        0xFFFFA71C,0x00008508, 0xFFFFA3DF,0x000082D0, 0xFFFFA0B1,0x00008083, 0xFFFF9D91,0x00007E22,
        0xFFFF9A80,0x00007BAE, 0xFFFF977F,0x00007927, 0xFFFF948D,0x0000768D, 0xFFFF91AD,0x000073E1,
        0xFFFF8EDD,0x00007123, 0xFFFF8C1F,0x00006E53, 0xFFFF8973,0x00006B73, 0xFFFF86D9,0x00006881,
        0xFFFF8452,0x00006580, 0xFFFF81DE,0x0000626F, 0xFFFF7F7D,0x00005F4F, 0xFFFF7D30,0x00005C21,
        0xFFFF7AF8,0x000058E4, 0xFFFF78D3,0x00005599, 0xFFFF76C4,0x00005241, 0xFFFF74CA,0x00004EDD,
        0xFFFF72E5,0x00004B6C, 0xFFFF7116,0x000047F0, 0xFFFF6F5D,0x00004468, 0xFFFF6DBA,0x000040D6,
        0xFFFF6C2E,0x00003D3A, 0xFFFF6AB9,0x00003995, 0xFFFF695B,0x000035E7, 0xFFFF6814,0x00003230,
        0xFFFF66E4,0x00002E72, 0xFFFF65CC,0x00002AAC, 0xFFFF64CC,0x000026E0, 0xFFFF63E4,0x0000230E,
        0xFFFF6314,0x00001F36, 0xFFFF625C,0x00001B5A, 0xFFFF61BC,0x0000177A, 0xFFFF6135,0x00001395,
        0xFFFF60C6,0x00000FAE, 0xFFFF606F,0x00000BC5, 0xFFFF6032,0x000007D9, 0xFFFF600D,0x000003ED,
        0xFFFF6000,0x00000000, 0xFFFF600D,0xFFFFFC13, 0xFFFF6032,0xFFFFF827, 0xFFFF606F,0xFFFFF43B,
        0xFFFF60C6,0xFFFFF052, 0xFFFF6135,0xFFFFEC6B, 0xFFFF61BC,0xFFFFE886, 0xFFFF625C,0xFFFFE4A6,
        0xFFFF6314,0xFFFFE0CA, 0xFFFF63E4,0xFFFFDCF2, 0xFFFF64CC,0xFFFFD920, 0xFFFF65CC,0xFFFFD554,
        0xFFFF66E4,0xFFFFD18E, 0xFFFF6814,0xFFFFCDD0, 0xFFFF695B,0xFFFFCA19, 0xFFFF6AB9,0xFFFFC66B,
        0xFFFF6C2E,0xFFFFC2C6, 0xFFFF6DBA,0xFFFFBF2A, 0xFFFF6F5D,0xFFFFBB98, 0xFFFF7116,0xFFFFB810,
        0xFFFF72E5,0xFFFFB494, 0xFFFF74CA,0xFFFFB123, 0xFFFF76C4,0xFFFFADBF, 0xFFFF78D3,0xFFFFAA67,
        0xFFFF7AF8,0xFFFFA71C, 0xFFFF7D30,0xFFFFA3DF, 0xFFFF7F7D,0xFFFFA0B1, 0xFFFF81DE,0xFFFF9D91,
        0xFFFF8452,0xFFFF9A80, 0xFFFF86D9,0xFFFF977F, 0xFFFF8973,0xFFFF948D, 0xFFFF8C1F,0xFFFF91AD,
        0xFFFF8EDD,0xFFFF8EDD, 0xFFFF91AD,0xFFFF8C1F, 0xFFFF948D,0xFFFF8973, 0xFFFF977F,0xFFFF86D9,
        0xFFFF9A80,0xFFFF8452, 0xFFFF9D91,0xFFFF81DE, 0xFFFFA0B1,0xFFFF7F7D, 0xFFFFA3DF,0xFFFF7D30,
        0xFFFFA71C,0xFFFF7AF8, 0xFFFFAA67,0xFFFF78D3, 0xFFFFADBF,0xFFFF76C4, 0xFFFFB123,0xFFFF74CA,
        0xFFFFB494,0xFFFF72E5, 0xFFFFB810,0xFFFF7116, 0xFFFFBB98,0xFFFF6F5D, 0xFFFFBF2A,0xFFFF6DBA,
        0xFFFFC2C6,0xFFFF6C2E, 0xFFFFC66B,0xFFFF6AB9, 0xFFFFCA19,0xFFFF695B, 0xFFFFCDD0,0xFFFF6814,
        0xFFFFD18E,0xFFFF66E4, 0xFFFFD554,0xFFFF65CC, 0xFFFFD920,0xFFFF64CC, 0xFFFFDCF2,0xFFFF63E4,
        0xFFFFE0CA,0xFFFF6314, 0xFFFFE4A6,0xFFFF625C, 0xFFFFE886,0xFFFF61BC, 0xFFFFEC6B,0xFFFF6135,
        0xFFFFF052,0xFFFF60C6, 0xFFFFF43B,0xFFFF606F, 0xFFFFF827,0xFFFF6032, 0xFFFFFC13,0xFFFF600D,
};
static const int32_t platform_xy[256*2] = {
        0x00038000,0x00000000, 0x00037FBA,0x000015FD, 0x00037EEB,0x00002BF6, 0x00037D92,0x000041E9,
        0x00037BAF,0x000057D2, 0x00037942,0x00006DAE, 0x0003764D,0x00008378, 0x000372CF,0x0000992E,
        0x00036EC8,0x0000AECD, 0x00036A3A,0x0000C450, 0x00036525,0x0000D9B5, 0x00035F8B,0x0000EEF9,
        0x0003596B,0x00010418, 0x000352C6,0x0001190F, 0x00034B9F,0x00012DDA, 0x000343F6,0x00014277,
        0x00033BCB,0x000156E2, 0x00033321,0x00016B18, 0x000329F9,0x00017F16, 0x00032054,0x000192DA,
        0x00031633,0x0001A65F, 0x00030B99,0x0001B9A3, 0x00030086,0x0001CCA2, 0x0002F4FD,0x0001DF5B,
        0x0002E8FF,0x0001F1CA, 0x0002DC8E,0x000203EC, 0x0002CFAC,0x000215BF, 0x0002C25B,0x0002273F,
        0x0002B49E,0x0002386A, 0x0002A675,0x0002493E, 0x000297E4,0x000259B7, 0x000288EC,0x000269D4,
        0x00027991,0x00027991, 0x000269D4,0x000288EC, 0x000259B7,0x000297E4, 0x0002493E,0x0002A675,
        0x0002386A,0x0002B49E, 0x0002273F,0x0002C25B, 0x000215BF,0x0002CFAC, 0x000203EC,0x0002DC8E,
        0x0001F1CA,0x0002E8FF, 0x0001DF5B,0x0002F4FD, 0x0001CCA2,0x00030086, 0x0001B9A3,0x00030B99,
        0x0001A65F,0x00031633, 0x000192DA,0x00032054, 0x00017F16,0x000329F9, 0x00016B18,0x00033321,
        0x000156E2,0x00033BCB, 0x00014277,0x000343F6, 0x00012DDA,0x00034B9F, 0x0001190F,0x000352C6,
        0x00010418,0x0003596B, 0x0000EEF9,0x00035F8B, 0x0000D9B5,0x00036525, 0x0000C450,0x00036A3A,
        0x0000AECD,0x00036EC8, 0x0000992E,0x000372CF, 0x00008378,0x0003764D, 0x00006DAE,0x00037942,
        0x000057D2,0x00037BAF, 0x000041E9,0x00037D92, 0x00002BF6,0x00037EEB, 0x000015FD,0x00037FBA,
        0x00000000,0x00038000, 0xFFFFEA03,0x00037FBA, 0xFFFFD40A,0x00037EEB, 0xFFFFBE17,0x00037D92,
        0xFFFFA82E,0x00037BAF, 0xFFFF9252,0x00037942, 0xFFFF7C88,0x0003764D, 0xFFFF66D2,0x000372CF,
        0xFFFF5133,0x00036EC8, 0xFFFF3BB0,0x00036A3A, 0xFFFF264B,0x00036525, 0xFFFF1107,0x00035F8B,
        0xFFFEFBE8,0x0003596B, 0xFFFEE6F1,0x000352C6, 0xFFFED226,0x00034B9F, 0xFFFEBD89,0x000343F6,
        0xFFFEA91E,0x00033BCB, 0xFFFE94E8,0x00033321, 0xFFFE80EA,0x000329F9, 0xFFFE6D26,0x00032054,
        0xFFFE59A1,0x00031633, 0xFFFE465D,0x00030B99, 0xFFFE335E,0x00030086, 0xFFFE20A5,0x0002F4FD,
        0xFFFE0E36,0x0002E8FF, 0xFFFDFC14,0x0002DC8E, 0xFFFDEA41,0x0002CFAC, 0xFFFDD8C1,0x0002C25B,
        0xFFFDC796,0x0002B49E, 0xFFFDB6C2,0x0002A675, 0xFFFDA649,0x000297E4, 0xFFFD962C,0x000288EC,
        0xFFFD866F,0x00027991, 0xFFFD7714,0x000269D4, 0xFFFD681C,0x000259B7, 0xFFFD598B,0x0002493E,
        0xFFFD4B62,0x0002386A, 0xFFFD3DA5,0x0002273F, 0xFFFD3054,0x000215BF, 0xFFFD2372,0x000203EC,
        0xFFFD1701,0x0001F1CA, 0xFFFD0B03,0x0001DF5B, 0xFFFCFF7A,0x0001CCA2, 0xFFFCF467,0x0001B9A3,
        0xFFFCE9CD,0x0001A65F, 0xFFFCDFAC,0x000192DA, 0xFFFCD607,0x00017F16, 0xFFFCCCDF,0x00016B18,
        0xFFFCC435,0x000156E2, 0xFFFCBC0A,0x00014277, 0xFFFCB461,0x00012DDA, 0xFFFCAD3A,0x0001190F,
        0xFFFCA695,0x00010418, 0xFFFCA075,0x0000EEF9, 0xFFFC9ADB,0x0000D9B5, 0xFFFC95C6,0x0000C450,
        0xFFFC9138,0x0000AECD, 0xFFFC8D31,0x0000992E, 0xFFFC89B3,0x00008378, 0xFFFC86BE,0x00006DAE,
        0xFFFC8451,0x000057D2, 0xFFFC826E,0x000041E9, 0xFFFC8115,0x00002BF6, 0xFFFC8046,0x000015FD,
        0xFFFC8000,0x00000000, 0xFFFC8046,0xFFFFEA03, 0xFFFC8115,0xFFFFD40A, 0xFFFC826E,0xFFFFBE17,
        0xFFFC8451,0xFFFFA82E, 0xFFFC86BE,0xFFFF9252, 0xFFFC89B3,0xFFFF7C88, 0xFFFC8D31,0xFFFF66D2,
        0xFFFC9138,0xFFFF5133, 0xFFFC95C6,0xFFFF3BB0, 0xFFFC9ADB,0xFFFF264B, 0xFFFCA075,0xFFFF1107,
        0xFFFCA695,0xFFFEFBE8, 0xFFFCAD3A,0xFFFEE6F1, 0xFFFCB461,0xFFFED226, 0xFFFCBC0A,0xFFFEBD89,
        0xFFFCC435,0xFFFEA91E, 0xFFFCCCDF,0xFFFE94E8, 0xFFFCD607,0xFFFE80EA, 0xFFFCDFAC,0xFFFE6D26,
        0xFFFCE9CD,0xFFFE59A1, 0xFFFCF467,0xFFFE465D, 0xFFFCFF7A,0xFFFE335E, 0xFFFD0B03,0xFFFE20A5,
        0xFFFD1701,0xFFFE0E36, 0xFFFD2372,0xFFFDFC14, 0xFFFD3054,0xFFFDEA41, 0xFFFD3DA5,0xFFFDD8C1,
        0xFFFD4B62,0xFFFDC796, 0xFFFD598B,0xFFFDB6C2, 0xFFFD681C,0xFFFDA649, 0xFFFD7714,0xFFFD962C,
        0xFFFD866F,0xFFFD866F, 0xFFFD962C,0xFFFD7714, 0xFFFDA649,0xFFFD681C, 0xFFFDB6C2,0xFFFD598B,
        0xFFFDC796,0xFFFD4B62, 0xFFFDD8C1,0xFFFD3DA5, 0xFFFDEA41,0xFFFD3054, 0xFFFDFC14,0xFFFD2372,
        0xFFFE0E36,0xFFFD1701, 0xFFFE20A5,0xFFFD0B03, 0xFFFE335E,0xFFFCFF7A, 0xFFFE465D,0xFFFCF467,
        0xFFFE59A1,0xFFFCE9CD, 0xFFFE6D26,0xFFFCDFAC, 0xFFFE80EA,0xFFFCD607, 0xFFFE94E8,0xFFFCCCDF,
        0xFFFEA91E,0xFFFCC435, 0xFFFEBD89,0xFFFCBC0A, 0xFFFED226,0xFFFCB461, 0xFFFEE6F1,0xFFFCAD3A,
        0xFFFEFBE8,0xFFFCA695, 0xFFFF1107,0xFFFCA075, 0xFFFF264B,0xFFFC9ADB, 0xFFFF3BB0,0xFFFC95C6,
        0xFFFF5133,0xFFFC9138, 0xFFFF66D2,0xFFFC8D31, 0xFFFF7C88,0xFFFC89B3, 0xFFFF9252,0xFFFC86BE,
        0xFFFFA82E,0xFFFC8451, 0xFFFFBE17,0xFFFC826E, 0xFFFFD40A,0xFFFC8115, 0xFFFFEA03,0xFFFC8046,
        0x00000000,0xFFFC8000, 0x000015FD,0xFFFC8046, 0x00002BF6,0xFFFC8115, 0x000041E9,0xFFFC826E,
        0x000057D2,0xFFFC8451, 0x00006DAE,0xFFFC86BE, 0x00008378,0xFFFC89B3, 0x0000992E,0xFFFC8D31,
        0x0000AECD,0xFFFC9138, 0x0000C450,0xFFFC95C6, 0x0000D9B5,0xFFFC9ADB, 0x0000EEF9,0xFFFCA075,
        0x00010418,0xFFFCA695, 0x0001190F,0xFFFCAD3A, 0x00012DDA,0xFFFCB461, 0x00014277,0xFFFCBC0A,
        0x000156E2,0xFFFCC435, 0x00016B18,0xFFFCCCDF, 0x00017F16,0xFFFCD607, 0x000192DA,0xFFFCDFAC,
        0x0001A65F,0xFFFCE9CD, 0x0001B9A3,0xFFFCF467, 0x0001CCA2,0xFFFCFF7A, 0x0001DF5B,0xFFFD0B03,
        0x0001F1CA,0xFFFD1701, 0x000203EC,0xFFFD2372, 0x000215BF,0xFFFD3054, 0x0002273F,0xFFFD3DA5,
        0x0002386A,0xFFFD4B62, 0x0002493E,0xFFFD598B, 0x000259B7,0xFFFD681C, 0x000269D4,0xFFFD7714,
        0x00027991,0xFFFD866F, 0x000288EC,0xFFFD962C, 0x000297E4,0xFFFDA649, 0x0002A675,0xFFFDB6C2,
        0x0002B49E,0xFFFDC796, 0x0002C25B,0xFFFDD8C1, 0x0002CFAC,0xFFFDEA41, 0x0002DC8E,0xFFFDFC14,
        0x0002E8FF,0xFFFE0E36, 0x0002F4FD,0xFFFE20A5, 0x00030086,0xFFFE335E, 0x00030B99,0xFFFE465D,
        0x00031633,0xFFFE59A1, 0x00032054,0xFFFE6D26, 0x000329F9,0xFFFE80EA, 0x00033321,0xFFFE94E8,
        0x00033BCB,0xFFFEA91E, 0x000343F6,0xFFFEBD89, 0x00034B9F,0xFFFED226, 0x000352C6,0xFFFEE6F1,
        0x0003596B,0xFFFEFBE8, 0x00035F8B,0xFFFF1107, 0x00036525,0xFFFF264B, 0x00036A3A,0xFFFF3BB0,
        0x00036EC8,0xFFFF5133, 0x000372CF,0xFFFF66D2, 0x0003764D,0xFFFF7C88, 0x00037942,0xFFFF9252,
        0x00037BAF,0xFFFFA82E, 0x00037D92,0xFFFFBE17, 0x00037EEB,0xFFFFD40A, 0x00037FBA,0xFFFFEA03,
};
