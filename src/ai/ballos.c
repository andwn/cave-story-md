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
/*
enum { LESS_THAN, GREATER_THAN };

static uint8_t passed_xcoord(Entity *e, uint8_t ltgt, int32_t xcoord, uint8_t reset) {
	int32_t next_x = e->x + e->x_speed;
	uint8_t result;
	
	if (ltgt == LESS_THAN)
		result = (next_x <= xcoord);
	else
		result = (next_x >= xcoord);
	
	if (result && reset) {
		e->x = xcoord;
		e->x_speed = 0;
	}
	
	return result;
}


static uint8_t passed_ycoord(Entity *e, uint8_t ltgt, int32_t ycoord, uint8_t reset) {
	int32_t next_y = e->y + e->y_speed;
	uint8_t result;
	
	if (ltgt == LESS_THAN)
		result = (next_y <= ycoord);
	else
		result = (next_y >= ycoord);
	
	if (result && reset) {
		e->y = ycoord;
		e->y_speed = 0;
	}
	
	return result;
}
*/
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
	
	e->x = block_to_sub(stageWidth >> 1);
	e->y = -pixel_to_sub(64);
	
	e->attack = 0;
	e->health = 800;
	
	e->hidden = TRUE;
	
	e->hit_box = (bounding_box) {{ 32, 48, 32, 48 }};
	
	//objprop[main->type].hurt_sound = SND_ENEMY_HURT_COOL;
	//main->invisible = true;
	
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
	shield->flags = (NPC_SOLID | NPC_SHOOTABLE | NPC_INVINCIBLE);
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
	/*transfer_damage(body, e) || */
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
	// place shield
	shield->x = e->x;
	shield->y = e->y - pixel_to_sub(44);
	
	// riding on platform by eye? Player can sort of stay on this platform
	// when he jumps. We don't do this for the shield up top though, in order that
	// he gets kind of slid off--what happens is he'll fall through the shield
	// onto the main body (a SPECIALSOLID), and then now that he's embedded in
	// the shield (a SOLID), it'll repel him to the side.
	//if (playerPlatform == body) {
	//	player.apply_x_speed(e->x_speed);
	//	player.apply_y_speed(e->y_speed);
	//}
}

// 2nd form as a stageboss.
// the one where he spawns spiky rotators and circles around the room.
void ai_ballos_f2(Entity *e) {
	static const int16_t BS_SPEED = 0x3AA;
	static const int32_t ARENA_LEFT = pixel_to_sub(119);
	static const int32_t ARENA_TOP = pixel_to_sub(119);
	static const int32_t ARENA_RIGHT = pixel_to_sub(521);
	static const int32_t ARENA_BOTTOM = pixel_to_sub(233);
	
	/*transfer_damage(body, e) || */
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
	body->x = e->x;
	body->y = e->y;
	// place shield
	shield->x = e->x;
	shield->y = e->y - pixel_to_sub(44);
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
						effect_create_smoke(e->x >> CSF, (e->y >> CSF) - 52);
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
							int32_t y = (4 + (rand() & 63)) << (CSF+2);
							if(!flags) x += block_to_sub(stageWidth - 1);
							entity_create(x, y, OBJ_BUTE_ARCHER_RED, flags);
						}
					}
					break;
					case 320:
					{
						if(pal_mode || cfg_60fps) e->timer = 0;
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
			//shield->eflags &= mask;
			eye[0]->flags &= mask;
			eye[1]->flags &= mask;
		} /* fallthrough */
		case 1001:
		{
			//int x = e->x + rand(-60<<CSF, 60<<CSF);
			//int y = e->y + rand(-60<<CSF, 60<<CSF);
			//SmokePuff(x, y);
			//effect(x, y, EFFECT_BOOMFLASH);
			
			e->timer++;
			
			if((e->timer & 15) == 0)
				sound_play(SND_MISSILE_HIT, 5);
			
			if(e->timer > TIME_8(150)) {
				e->timer = 0;
				e->state = 1002;
				
				//starflash.Start(e->x, e->y);
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
	body->x = e->x;
	body->y = e->y;
	// place shield
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
		
		// close eyes
		case EYE_CLOSING:
		{
			e->frame = (e->flags & NPC_OPTION2) ? 6 : 2;
			e->hidden = FALSE;
			e->flags |= NPC_INVINCIBLE;
			
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
		
		// explode eyes (final defeat sequence)
		case EYE_EXPLODING:
		{
			e->frame = (e->flags & NPC_OPTION2) ? 7 : 3;	// empty eyes
			e->hidden = FALSE;
			
			e->flags &= ~(NPC_SHOOTABLE | NPC_INVINCIBLE);
			e->state++;
			
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
			
			e->flags |= NPC_SHOOTABLE;
			e->flags &= ~NPC_INVINCIBLE;
			e->health = 1000;
		} /* fallthrough */
		case 11:		// spinning during phase 2, alive
		{
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
					
					rotators_left--;
				}
			}
			
			//spawn_impact_puffs(o);
		}
		break;
		
		case 20:	// spinning fast CCW while spikes come up
		{
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
				e->flags |= NPC_SHOOTABLE;
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
				//e->frame = (e->damage_time & 2) ? 1 : 0;
				
				if (e->health < (1000 - 100)) {
					e->x_speed = 0;
					e->y_speed = 0;
					
					e->flags &= ~NPC_SHOOTABLE;
					//SmokeClouds(o, 32, 16, 16);
					sound_play(SND_LITTLE_CRASH, 5);
					
					e->frame = 1;
					e->state = 40;
					e->attack = 5;
					
					// blow up immediately if Ballos is defeated
					e->timer2 = 0;
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
			e->x = ballos->x + 0x800 + (((int32_t)cos[angle]) << 6) + (((int32_t)cos[angle]) << 4);
			e->y = ballos->y + 0x800 + (((int32_t)sin[angle]) << 6) + (((int32_t)sin[angle]) << 4);
		} else {
			e->x = ballos->x + 0x800 + (((int32_t)cos[angle]) << 6);
			e->y = ballos->y + 0x800 + (((int32_t)sin[angle]) << 6);
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
			
			if (e->x > block_to_sub(stageHeight + 1)) e->state = STATE_DELETE;
		}
		break;
	}
	
	if (e->state >= 1000) return;
	
	// let player jump up through platforms, but be solid when he is standing on them
	if (player.y_speed < 0 || player.y > e->y - pixel_to_sub(e->hit_box.top)) {
		e->flags &= ~NPC_SPECIALSOLID;
	} else {
		e->flags |= NPC_SPECIALSOLID;
	}
	
	// spin
	uint8_t angle = e->timer2 >> 2;
	int32_t xoff, yoff;
	if(e->state < 2) {
		// While expanding out use the extra cpu cycles to make it smooth
		xoff = ((int32_t)cos[angle]) * e->timer3;
		yoff = ((int32_t)sin[angle]) * e->timer3;
	} else {
		// After expanding the distance will always be 0x1C0, this effectively multiplies by that,
		// with bit shifts instead of multiplying a signed int 16 times per frame
		xoff = (((int32_t)cos[angle]) << 9) /* 0x200 */ - (((int32_t)cos[angle]) << 6) /* 0x40 */;
		yoff = (((int32_t)sin[angle]) << 9) /* 0x200 */ - (((int32_t)sin[angle]) << 6) /* 0x40 */;
	}
	
	e->x_mark = (xoff >> 2) + ballos->x;
	e->y_mark = (yoff >> 2) + pixel_to_sub(12) + ballos->y;
	
	switch(abs(platform_speed)) {
		case 1:
			if ((e->timer2 & 3) == 0) {
				e->y_speed = (e->y_mark - e->y) >> 2;
			}
		break;
		
		case 2:
			if ((e->timer2 & 2) == 0) {
				e->y_speed = (e->y_mark - e->y) >> 1;
			}
		break;
		
		default:
			e->y_speed = (e->y_mark - e->y);
		break;
	}
	
	e->x_speed = (e->x_mark - e->x);

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
