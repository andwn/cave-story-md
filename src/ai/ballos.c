#include "ai_common.h"

//static int platform_speed;
//static int rotators_left;

#define platform_speed	curly_target_x
#define rotators_left	curly_target_y

#define angle		jump_time
#define timer3		id

#define eye			pieces
#define body		e->linkedEntity
#define shield		body->linkedEntity
#define rotator(i)	pieces[2+(i)]

#define FLOOR_Y			0x26000						// Y coord of floor
#define CRASH_Y			(FLOOR_Y - (40 << CSF))		// Y coord of main when body hits floor

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
static const int F1_LEFT = (88 << CSF);
static const int F1_RIGHT = (552 << CSF);

// runs arrival of first form as a stage-boss
static void RunComeDown(Entity *e) {
	switch(e->state) {
		case AS_COME_DOWN:
		{
			//e->savedhp = e->health;
			
			e->x = player.x;
			e->y = -(64 << CSF);
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
			if (++e->timer > TIME(30)) e->state++;
		}
		break;
		
		// falling
		case AS_COME_DOWN+2:
		{
			e->y_speed += SPEED(0x40);
			LIMIT_Y(SPEED(0xC00));
			
			if (passed_ycoord(e, GREATER_THAN, CRASH_Y, FALSE)) {
				e->y_speed = 0;
				e->timer = 0;
				e->state++;
				
				camera_shake(30);
				
				// player smush damage
				// (he could only get that low if he had been pushed into the floor)
				if (player.y > (e->y + (48<<CSF))) player_inflict_damage(16);
				
				//SmokeXY(e->x, e->y + (40<<CSF), 16, 40, 0);
				
				if (player.grounded) player.y_speed = -SPEED(0x200);
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
	}
}

// 1st form as a stageboss.
// the one where he jumps around as a rock.
static void RunForm1(Entity *e) {
	RunComeDown(e);
	
	switch(e->state) {
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
			if ((++e->timer2 % 3) == 0)
				e->timer = TIME(150);
			else
				e->timer = TIME(50);
		} /* fallthrough */
		case AS_PREPARE_JUMP+1:
		{
			if (e->timer-- == 0) {
				e->y_speed = -SPEED(0xC00);
				e->x_speed = (e->x < player.x) ? SPEED(0x200) : -SPEED(0x200);
				e->state = AS_JUMPING;
			}
		}
		break;
		
		case AS_JUMPING:
		{
			e->y_speed += SPEED(0x55);
			LIMIT_Y(SPEED(0xC00));
			
			if (passed_xcoord(e, LESS_THAN, F1_LEFT, FALSE)) e->x_speed = SPEED(0x200);
			if (passed_xcoord(e, GREATER_THAN, F1_RIGHT, FALSE)) e->x_speed = -SPEED(0x200);
			
			if (passed_ycoord(e, GREATER_THAN, CRASH_Y, FALSE)) {
				// player smush damage
				if (player.y > (e->y + (48<<CSF))) player_inflict_damage(16);
				
				// player hopping from the vibration
				if (player.grounded) player.y_speed = -SPEED(0x200);
				
				camera_shake(30);
				
				entity_create(e->x - (12<<CSF), e->y + (52<<CSF), OBJ_BALLOS_BONE_SPAWNER, 0)->dir = 0;
				entity_create(e->x + (12<<CSF), e->y + (52<<CSF), OBJ_BALLOS_BONE_SPAWNER, 0)->dir = 1;
				
				//SmokeXY(e->x, e->y + (40<<CSF), 16, 40, 0);
				
				e->y_speed = 0;
				e->state = AS_PREPARE_JUMP;
			}
		}
		break;
		
		// 1st form defeated
		case AS_DEFEATED:
		{
			SetEyeStates(EYE_CLOSING);
			//game.bossbar.defeated = true;
			e->health = 1200;
			
			e->state++;
			
			e->x_speed = 0;
			e->damage_time = 0;
		} /* fallthrough */
		case AS_DEFEATED+1:
		{
			e->y_speed += SPEED(0x40);
			LIMIT_Y(SPEED(0xC00));
			
			if (passed_ycoord(e, GREATER_THAN, CRASH_Y, FALSE)) {
				e->y_speed = 0;
				e->state++;
				
				camera_shake(30);
				//SmokeXY(e->x, e->y + 0x5000, 16, 40, 0);
				
				if (player.grounded) player.y_speed = -SPEED(0x200);
				
				// ... and wait for script to trigger form 2
			}
		}
		break;
	}
}

// 2nd form as a stageboss.
// the one where he spawns spiky rotators and circles around the room.
static void RunForm2(Entity *e) {
	static const uint16_t BS_SPEED = 0x3AA;
	static const uint32_t ARENA_LEFT = (119 << CSF);
	static const uint32_t ARENA_TOP = (119 << CSF);
	static const uint32_t ARENA_RIGHT = (521 << CSF);
	static const uint32_t ARENA_BOTTOM = (233 << CSF);
	
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
		} /* fallthrough */
		case BS_ENTER_FORM+1:
		{
			//e->y += (ARENA_BOTTOM - e->y) / 8;
			
			//if (passed_xcoord(e, LESS_THAN, ARENA_LEFT, FALSE))
			//	e->x += (ARENA_LEFT - e->x) / 8;
			
			//if (passed_xcoord(e, GREATER_THAN, ARENA_RIGHT, FALSE))
			//	e->x += (ARENA_RIGHT - e->x) / 8;
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
			//e->dirparam = LEFT;
			
			if (passed_xcoord(e, LESS_THAN, ARENA_LEFT, FALSE))
				e->state = BS_UP;
		}
		break;
		
		// up on wall
		case BS_UP:
		{
			e->x_speed = 0;
			e->y_speed = -SPEED_10(BS_SPEED);
			//e->dirparam = UP;
			
			if (passed_ycoord(e, LESS_THAN, ARENA_TOP, FALSE))
				e->state = BS_RIGHT;
		}
		break;
		
		// right on ceiling
		case BS_RIGHT:
		{
			e->x_speed = SPEED_10(BS_SPEED);
			e->y_speed = 0;
			//e->dirparam = RIGHT;
			
			// all rotators destroyed?
			if (rotators_left <= 0 && ++e->timer > 3) {
				// center of room
				if (e->x >= (312<<CSF) && e->x <= (344<<CSF)) {
					e->state = CS_ENTER_FORM;
				}
			}
			
			if (passed_xcoord(e, GREATER_THAN, ARENA_RIGHT, FALSE))
				e->state = BS_DOWN;
		}
		break;
		
		// down on wall
		case BS_DOWN:
		{
			e->x_speed = 0;
			e->y_speed = SPEED_10(BS_SPEED);
			//e->dirparam = DOWN;
			
			if (passed_ycoord(e, GREATER_THAN, ARENA_BOTTOM, FALSE)) {
				e->state = BS_LEFT;
			}
		}
		break;
	}
}

// form 3 as a stageboss, the final form.
// he reaches the center of the room, platforms come out, spikes go up,
// and he explodes into a really bloody version of himself.
//
// then the platforms spin in various speeds and directions while he
// spawns red butes from the sides and his top.
static void RunForm3(Entity *e) {
	static const int YPOSITION = (167 << CSF);
	
	// platform spin speeds and how long they travel at each speed.
	// it's a repeating pattern.
	static const struct {
		int16_t length, speed;
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
	
	switch(e->state)
	{
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
			e->y += (YPOSITION - e->y) / 8;
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
				entity_create(block_to_sub(e->x_mark), FLOOR_Y + (48 << CSF), OBJ_BALLOS_SPIKES, 0);
				
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
			body->eflags &= ~NPC_INVINCIBLE;
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
			switch(++e->timer) {
				case 270:	// spawn swordsmen from face
				case 280:
				case 290:
				{
					//SmokeXY(e->x, e->y - (52<<CSF), 4);
					//entity_create(e->x, e->y - (52<<CSF), OBJ_BUTE_SWORD_RED, 0)->dir = UP;
					//sound_play(SND_EM_FIRE, 5);
				}
				break;
				
				case 300:	// spawn archers on side
				{
					e->timer = 0;
					// direction butes will be facing, not side of screen
					//int dir = (player.x > e->x) ? 0 : 1;
					
					//for(uint8_t i=0;i<8;i++) {
						// give some granularity to the coords,
						// so that they can't overlap too closely.
						//int32_t x = ((random() & 3) << (CSF+2));
						//int32_t y = ((random() & 63) << (CSF+2)) + 4;
						//if (!dir) x += block_to_sub(stageWidth - 1);
						
						//entity_create(x, y, OBJ_BUTE_ARCHER_RED, 0)->dir = dir;
					//}
				}
				break;
			}
			
			// spawn blood
			//int prob = (e->hp <= 500) ? 4 : 10;
			//if (!random(0, prob)) {
			//	CreateEntity(e->x + random(-40<<CSF, 40<<CSF),
			//				 e->y + random(0, 40<<CSF),
			//				 OBJ_RED_ENERGY)->angle = DOWN;
			//}
		}
		break;
	}
}

static void RunDefeated(Entity *e) {
	switch(e->state) {
		case 1000:
		{
			e->state = 1001;
			e->timer = 0;
			
			SetEyeStates(EYE_EXPLODING);	// blow out eyes
			SetRotatorStates(1000);			// explode rotators
			
			uint16_t mask = ~(NPC_SPECIALSOLID | NPC_SOLID | NPC_SHOOTABLE | NPC_INVINCIBLE);
			e->eflags &= mask;
			body->eflags &= mask;
			//shield->eflags &= mask;
			eye[0]->eflags &= mask;
			eye[1]->eflags &= mask;
		} /* fallthrough */
		case 1001:
		{
			//int x = e->x + random(-60<<CSF, 60<<CSF);
			//int y = e->y + random(-60<<CSF, 60<<CSF);
			//SmokePuff(x, y);
			//effect(x, y, EFFECT_BOOMFLASH);
			
			e->timer++;
			
			if ((e->timer % TIME_8(12)) == 0)
				sound_play(SND_MISSILE_HIT, 5);
			
			if (e->timer > TIME_8(150)) {
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
			
			if (++e->timer >= TIME_8(50)) {
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
}

void onspawn_ballos(Entity *e) {
	e->alwaysActive = TRUE;
	// create (invisible) main controller object
	//main = CreateEntity(0, 0, OBJ_BALLOS_MAIN);
	//game.stageboss.object = main;
	
	//e->event = 1000;	
	e->eflags = (NPC_SOLID | NPC_SHOWDAMAGE | NPC_EVENTONDEATH);
	
	e->x = block_to_sub(stageWidth >> 1);
	e->y = -(64 << CSF);
	
	e->attack = 0;
	e->health = 800;
	
	e->hidden = TRUE;
	
	e->hit_box = (bounding_box) { 32, 48, 32, 48 };
	//e->display_box = (bounding_box) { 60, 60, 60, 60 };
	
	//objprop[main->type].hurt_sound = SND_ENEMY_HURT_COOL;
	//main->invisible = true;
	
	// create body (the big rock)
	body = entity_create(0, 0, OBJ_BALLOS_BODY, 0);
	body->health = 1000;	// not his real HP, we're using damage transfer
	body->eflags = (NPC_SOLID | NPC_SHOOTABLE | NPC_INVINCIBLE);
	body->hit_box = (bounding_box) { 48, 20, 48, 36 };
	body->display_box = (bounding_box) { 60, 60, 60, 60 };
	
	// create eyes (open/close animations)
	for(uint8_t i=0;i<2;i++) {
		eye[i] = entity_create(0, 0, OBJ_BALLOS_EYE, 0);
		eye[i]->health = 1000;
		eye[i]->hit_box = (bounding_box) { 12, 8, 12, 8 };
		eye[i]->display_box = (bounding_box) { 12, 8, 12, 8 };
	}
	eye[1]->eflags |= NPC_OPTION2;
	
	// create a top shield to cover eyes from above
	shield = entity_create(0, 0, OBJ_BALLOS_SHIELD, 0);
	//shield->sprite = SPR_BBOX_PUPPET_1;
	shield->hidden = TRUE;
	shield->health = 1000;
	shield->eflags = (NPC_SOLID | NPC_SHOOTABLE | NPC_INVINCIBLE);
	shield->hit_box = (bounding_box) { 32, 6, 32, 4 };
	
	// initilize bboxes
	//sprites[body->sprite].bbox.set(-48, -24, 48, 32);
	//sprites[shield->sprite].bbox.set(-32, -8, 32, 8);
	//sprites[main->sprite].bbox.set(-32, -48, 32, 48);
	
	//sprites[main->sprite].solidbox = sprites[main->sprite].bbox;
	//sprites[body->sprite].solidbox = sprites[body->sprite].bbox;
	//sprites[shield->sprite].solidbox = sprites[shield->sprite].bbox;
	
	// body and eyes are both directly shootable during one form or another
	// but should not shake as their damage is to be transferred to main object.
	//objprop[OBJ_BALLOS_MAIN].damage_time = 8;
	//objprop[OBJ_BALLOS_BODY].damage_time = 0;
	//objprop[OBJ_BALLOS_EYE].damage_time = 0;
	
	// initilize parameters
	//NX_LOG("BallosBoss::OnMapEntry()\n");
}

void ondeath_ballos(Entity *e) {
	// as soon as one of his forms is defeated make him non-killable
	// until the init for the next form runs and makes him killable again.
	// intended to fix the extremely rare possibility of killing him completely
	// after his 1st form instead of moving on to the spiky rotators like he should.
	e->health = 0xFFFF;
	tsc_call_event(1000); // defeated script (has a flagjump in it to handle each form)
}

void ai_ballos(Entity *e) {
	//if (!main) return;
	//AIDEBUG;
	
	if(transfer_damage(body, e) || transfer_damage(eye[0], e) || transfer_damage(eye[1], e)) {
		ondeath_ballos(e);
		return;
	}
	//transfer_damage(shield, e);
	
	RunForm1(e);
	RunForm2(e);
	RunForm3(e);
	RunDefeated(e);
	
	e->x += e->x_speed;
	e->y += e->y_speed;
	
	//run_eye(0);
	//run_eye(1);
	
	// flash red when hurt
	// TODO: palettes again
	//if (e->damage_time & 2)
	//	body->frame |= 1;
	//else
	//	body->frame &= ~1;
	
	// place eyes
	eye[0]->x = e->x - (24 << CSF);
	eye[1]->x = e->x + (24 << CSF);
	eye[0]->y = eye[1]->y = e->y - (28 << CSF);
	
	// place body
	body->x = e->x;
	body->y = e->y;
	
	// place shield
	shield->x = e->x;
	shield->y = e->y - (44 << CSF);
	
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
			e->frame = (e->eflags & NPC_OPTION2) ? 4 : 0;
			e->eflags |= (NPC_SHOOTABLE | NPC_INVINCIBLE);
			e->state = 1;
		}
		break;
		
		// open eyes
		case EYE_OPENING:
		{
			e->frame = (e->eflags & NPC_OPTION2) ? 4 : 0;
			e->animtime = 0;
			e->state++;
		} /* fallthrough */
		case EYE_OPENING+1:
		{
			if (++e->animtime > 4) {
				e->animtime = 0;
				e->frame++;
				if ((e->frame & 3) == 3) {
					e->eflags &= ~NPC_INVINCIBLE;
					e->hidden = TRUE;
					e->state++;
				}
			}
		}
		break;
		
		// close eyes
		case EYE_CLOSING:
		{
			e->frame = (e->eflags & NPC_OPTION2) ? 6 : 2;
			e->hidden = FALSE;
			e->eflags |= NPC_INVINCIBLE;
			
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
			e->eflags &= ~NPC_INVINCIBLE;
			e->hidden = TRUE;
			e->state++;
		}
		break;
		
		// explode eyes (final defeat sequence)
		case EYE_EXPLODING:
		{
			e->frame = (e->eflags & NPC_OPTION2) ? 7 : 3;	// empty eyes
			e->hidden = FALSE;
			
			e->eflags &= ~(NPC_SHOOTABLE | NPC_INVINCIBLE);
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
			e->timer2 = e->angle * 2;
			
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
			
			e->nflags |= NPC_SHOOTABLE;
			e->nflags &= ~NPC_INVINCIBLE;
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
					e->nflags &= ~NPC_SHOOTABLE;
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
			if (!e->dir) {
				e->eflags |= NPC_SHOOTABLE;
				e->frame = 0;
			} else {
				e->eflags |= NPC_INVINCIBLE;
				e->frame = 1;
			}
		} /* fallthrough */
		case 31:		// form 3 CW slow spin
		{
			// come in closer to main object
			if (e->timer3 > 0x100)
				e->timer3--;
			
			// spin CW
			if (++e->timer2 > 0x200)
				e->timer2 -= 0x200;
			
			if (e->eflags & NPC_SHOOTABLE) {
				//e->frame = (e->damage_time & 2) ? 1 : 0;
				
				if (e->health < (1000 - 100)) {
					e->x_speed = 0;
					e->y_speed = 0;
					
					e->eflags &= ~(NPC_SHOOTABLE | NPC_IGNORESOLID);
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
			e->y_speed += SPEED_8(0x20);
			LIMIT_Y(SPEED_12(0x5ff));
			
			if (blk(e->x, -16, e->y, 0) == 0x41) e->x_speed = SPEED_8(0xFF);
			if (blk(e->x, 16, e->y, 0) == 0x41) e->x_speed = -SPEED_8(0xFF);
			
			if (e->y_speed >= 0 && blk(e->x, 0, e->y, 16) == 0x41) {
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
			e->eflags &= ~(NPC_SHOOTABLE | NPC_IGNORESOLID);
			e->attack = 0;
			
			e->timer2 /= 4;
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
			e->x = ballos->x + (4<<CSF) + (cos[angle] * dist);
			e->y = ballos->y + (4<<CSF) + (sin[angle] * dist);
		} else if(e->state < 30) {
			e->x = ballos->x + (4<<CSF) + (cos[angle] << 6) + (cos[angle] << 4);
			e->y = ballos->y + (4<<CSF) + (sin[angle] << 6) + (sin[angle] << 4);
		} else {
			e->x = ballos->x + (4<<CSF) + (cos[angle] << 6);
			e->y = ballos->y + (4<<CSF) + (sin[angle] << 6);
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
			e->eflags &= ~NPC_SPECIALSOLID;
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
	if (player.y_speed < 0 || player.y > e->y - (e->hit_box.top << CSF)) {
		e->nflags &= ~NPC_SPECIALSOLID;
	} else {
		e->nflags |= NPC_SPECIALSOLID;
	}
	
	// spin
	uint8_t angle = e->timer2 >> 2;
	int32_t xoff, yoff;
	if(e->state < 2) {
		// While expanding out use the extra cpu cycles to make it smooth
		xoff = cos[angle] * e->timer3;
		yoff = sin[angle] * e->timer3;
	} else {
		// After expanding the distance will always be 0x1C0, this effectively multiplies by that,
		// with bit shifts instead of multiplying a signed int 16 times per frame
		xoff = (cos[angle] << 9) /* 0x200 */ - (cos[angle] << 6) /* 0x40 */;
		yoff = (sin[angle] << 9) /* 0x200 */ - (sin[angle] << 6) /* 0x40 */;
	}
	
	e->x_mark = (xoff >> 2) + ballos->x;
	e->y_mark = (yoff >> 2) + (8 << CSF) + ballos->y;
	
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
