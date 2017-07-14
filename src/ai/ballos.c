#include "ai_common.h"

//static int platform_speed;
//static int rotators_left;

#define platform_speed	curly_target_x
#define rotators_left	curly_target_y

#define FLOOR_Y			0x26000						// Y coord of floor
#define CRASH_Y			(FLOOR_Y - (40 << CSF))		// Y coord of main when body hits floor

enum EYE_STATES
{
	EYE_OPENING		= 10,
	EYE_CLOSING		= 20,
	EYE_INVISIBLE	= 30,
	EYE_EXPLODING	= 40
};

enum BS_STATES
{
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
void BallosBoss::OnMapEntry(void)
{
	// create (invisible) main controller object
	main = CreateObject(0, 0, OBJ_BALLOS_MAIN);
	game.stageboss.object = main;
	
	main->id2 = 1000;	// defeated script (has a flagjump in it to handle each form)
	main->flags = (FLAG_SHOW_FLOATTEXT | FLAG_SCRIPTONDEATH | \
				   FLAG_SOLID_BRICK | FLAG_IGNORE_SOLID);
	
	main->x = ((map.xsize / 2) * TILE_W) << CSF;
	main->y = -(64 << CSF);
	
	main->damage = 0;
	main->hp = 800;
	
	objprop[main->type].hurt_sound = SND_ENEMY_HURT_COOL;
	main->invisible = true;
	
	// create body (the big rock)
	body = CreateObject(0, 0, OBJ_BALLOS_BODY);
	body->hp = 1000;	// not his real HP, we're using damage transfer
	body->flags = (FLAG_SOLID_MUSHY | FLAG_SHOOTABLE | FLAG_INVULNERABLE | FLAG_IGNORE_SOLID);
	
	// create eyes (open/close animations)
	for(int i=0;i<NUM_EYES;i++)
	{
		eye[i] = CreateObject(0, 0, OBJ_BALLOS_EYE);
		eye[i]->dir = i;
		eye[i]->hp = 1000;
	}
	
	// create a top shield to cover eyes from above
	shield = CreateObject(0, 0, OBJ_BBOX_PUPPET);
	shield->sprite = SPR_BBOX_PUPPET_1;
	shield->invisible = true;
	shield->hp = 1000;
	shield->flags = (FLAG_SOLID_MUSHY | FLAG_SHOOTABLE | FLAG_INVULNERABLE | FLAG_IGNORE_SOLID);
	
	// initilize bboxes
	sprites[body->sprite].bbox.set(-48, -24, 48, 32);
	sprites[shield->sprite].bbox.set(-32, -8, 32, 8);
	sprites[main->sprite].bbox.set(-32, -48, 32, 48);
	
	sprites[main->sprite].solidbox = sprites[main->sprite].bbox;
	sprites[body->sprite].solidbox = sprites[body->sprite].bbox;
	sprites[shield->sprite].solidbox = sprites[shield->sprite].bbox;
	
	// body and eyes are both directly shootable during one form or another
	// but should not shake as their damage is to be transferred to main object.
	objprop[OBJ_BALLOS_MAIN].shaketime = 8;
	objprop[OBJ_BALLOS_BODY].shaketime = 0;
	objprop[OBJ_BALLOS_EYE].shaketime = 0;
	
	// initilize parameters
	NX_LOG("BallosBoss::OnMapEntry()\n");
}

void BallosBoss::Run()
{
	if (!main) return;
	//AIDEBUG;
	
	transfer_damage(body, main);
	transfer_damage(eye[LEFT], main);
	transfer_damage(eye[RIGHT], main);
	transfer_damage(shield, main);
	
	RunForm1(main);
	RunForm2(main);
	RunForm3(main);
	RunDefeated(main);
	
	run_eye(LEFT);
	run_eye(RIGHT);
	
	// flash red when hurt
	if (main->shaketime & 2)
		body->frame |= 1;
	else
		body->frame &= ~1;
}


void BallosBoss::RunAftermove()
{
	if (!main)
		return;
	
	// place eyes
	place_eye(LEFT);
	place_eye(RIGHT);
	
	// place body
	body->x = main->x;
	body->y = main->y;
	
	// place shield
	shield->x = main->x;
	shield->y = main->y - (44 << CSF);
	
	// riding on platform by eye? Player can sort of stay on this platform
	// when he jumps. We don't do this for the shield up top though, in order that
	// he gets kind of slid off--what happens is he'll fall through the shield
	// onto the main body (a SOLID_BRICK), and then now that he's embedded in
	// the shield (a SOLID_MUSHY), it'll repel him to the side.
	if (player->riding == body)
	{
		player->apply_xinertia(main->xinertia);
		player->apply_yinertia(main->yinertia);
	}
}

// left and right maximums during form 1
static const int F1_LEFT = (88 << CSF);
static const int F1_RIGHT = (552 << CSF);

// runs arrival of first form as a stage-boss
void BallosBoss::RunComeDown(Object *o)
{
	switch(o->state)
	{
		case AS_COME_DOWN:
		{
			o->savedhp = o->hp;
			
			o->x = player->CenterX();
			o->y = -(64 << CSF);
			o->frame = 0;
			
			// create the targeter
			// setting dir to right tells it don't spawn any lightning
			CreateObject(o->x, FLOOR_Y, OBJ_BALLOS_TARGET)->dir = RIGHT;
			o->timer = 0;
			
			if (o->x < F1_LEFT) o->x = F1_LEFT;
			if (o->x > F1_RIGHT) o->x = F1_RIGHT;
			
			o->state++;
		}
		case AS_COME_DOWN+1:
		{
			if (++o->timer > 30)
				o->state++;
		}
		break;
		
		// falling
		case AS_COME_DOWN+2:
		{
			o->yinertia += 0x40;
			LIMITY(0xC00);
			
			if (passed_ycoord(GREATER_THAN, CRASH_Y))
			{
				o->yinertia = 0;
				o->timer = 0;
				o->state++;
				
				megaquake(30, SND_MISSILE_HIT);
				
				// player smush damage
				// (he could only get that low if he had been pushed into the floor)
				if (player->y > (o->y + (48<<CSF)))
					hurtplayer(16);
				
				SmokeXY(o->x, o->y + (40<<CSF), 16, 40, 0);
				
				if (player->blockd)
					player->yinertia = -0x200;
			}
		}
		break;
		
		case AS_COME_DOWN+3:
		{
			if (++o->timer > 31)
			{
				SetEyeStates(EYE_OPENING);
				o->state++;
			}
		}
		break;
	}
}


// 1st form as a stageboss.
// the one where he jumps around as a rock.
void BallosBoss::RunForm1(Object *o)
{
	RunComeDown(o);
	
	switch(o->state)
	{
		case AS_BEGIN_FIGHT:
		{
			// can be damaged between eyes opening and boss bar appearing,
			// but it is not counted.
			o->hp = o->savedhp;
			o->state = AS_PREPARE_JUMP;
		}
		case AS_PREPARE_JUMP:	// delay, then jump at player
		{
			o->xinertia = 0;
			o->damage = 0;
			o->state++;
			
			// he makes two jumps then a pause,
			// after that it's three jumps before pausing.
			// this corresponds to:
			if ((++o->timer2 % 3) == 0)
				o->timer = 150;
			else
				o->timer = 50;
		}
		case AS_PREPARE_JUMP+1:
		{
			if (--o->timer <= 0)
			{
				o->yinertia = -0xC00;
				o->xinertia = (o->x < player->x) ? 0x200 : -0x200;
				o->state = AS_JUMPING;
			}
		}
		break;
		
		case AS_JUMPING:
		{
			o->yinertia += 0x55;
			LIMITY(0xC00);
			
			if (passed_xcoord(LESS_THAN, F1_LEFT)) o->xinertia = 0x200;
			if (passed_xcoord(GREATER_THAN, F1_RIGHT)) o->xinertia = -0x200;
			
			if (passed_ycoord(GREATER_THAN, CRASH_Y))
			{
				// player smush damage
				if (player->y > (o->y + (48<<CSF)))
					hurtplayer(16);
				
				// player hopping from the vibration
				if (player->blockd)
					player->yinertia = -0x200;
				
				megaquake(30, SND_MISSILE_HIT);
				
				CreateObject(o->x - (12<<CSF), o->y + (52<<CSF), OBJ_BALLOS_BONE_SPAWNER)->dir = LEFT;
				CreateObject(o->x + (12<<CSF), o->y + (52<<CSF), OBJ_BALLOS_BONE_SPAWNER)->dir = RIGHT;
				
				SmokeXY(o->x, o->y + (40<<CSF), 16, 40, 0);
				
				o->yinertia = 0;
				o->state = AS_PREPARE_JUMP;
			}
		}
		break;
		
		// 1st form defeated
		case AS_DEFEATED:
		{
			SetEyeStates(EYE_CLOSING);
			game.bossbar.defeated = true;
			o->hp = 1200;
			
			o->state++;
			
			o->xinertia = 0;
			o->shaketime = 0;
		}
		case AS_DEFEATED+1:
		{
			o->yinertia += 0x40;
			LIMITY(0xC00);
			
			if (passed_ycoord(GREATER_THAN, CRASH_Y))
			{
				o->yinertia = 0;
				o->state++;
				
				megaquake(30, SND_MISSILE_HIT);
				SmokeXY(o->x, o->y + 0x5000, 16, 40, 0);
				
				if (player->blockd)
					player->yinertia = -0x200;
				
				// ... and wait for script to trigger form 2
			}
		}
		break;
	}
}


// 2nd form as a stageboss.
// the one where he spawns spiky rotators and circles around the room.
void BallosBoss::RunForm2(Object *o)
{
	static const int BS_SPEED = 0x3AA;
	static const int ARENA_LEFT = (119 << CSF);
	static const int ARENA_TOP = (119 << CSF);
	static const int ARENA_RIGHT = (521 << CSF);
	static const int ARENA_BOTTOM = (233 << CSF);
	
	switch(o->state)
	{
		// enter 2nd form (script-triggered)
		case BS_ENTER_FORM:
		{
			o->timer = 0;
			o->state++;
			
			rotators_left = 0;
			for(int angle=0;angle<=0x100;angle+=0x20)
			{
				Object *r = CreateObject(o->x, o->y, OBJ_BALLOS_ROTATOR);
				r->angle = angle;
				r->dir = (rotators_left & 1) ? RIGHT : LEFT;
				
				rotators_left++;
			}
		}
		case BS_ENTER_FORM+1:
		{
			o->y += (ARENA_BOTTOM - o->y) / 8;
			
			if (passed_xcoord(LESS_THAN, ARENA_LEFT, false))
				o->x += (ARENA_LEFT - o->x) / 8;
			
			if (passed_xcoord(GREATER_THAN, ARENA_RIGHT, false))
				o->x += (ARENA_RIGHT - o->x) / 8;
		}
		break;
		
		case BS_FIGHT_BEGIN:	// script-triggered
		{
			SetRotatorStates(10);	// spin CCW, work as treads
			o->state = BS_LEFT;
			o->timer = 0;
		}
		case BS_LEFT:		// left on floor
		{
			o->xinertia = -BS_SPEED;
			o->yinertia = 0;
			o->dirparam = LEFT;
			
			if (passed_xcoord(LESS_THAN, ARENA_LEFT))
				o->state = BS_UP;
		}
		break;
		
		// up on wall
		case BS_UP:
		{
			o->xinertia = 0;
			o->yinertia = -BS_SPEED;
			o->dirparam = UP;
			
			if (passed_ycoord(LESS_THAN, ARENA_TOP))
				o->state = BS_RIGHT;
		}
		break;
		
		// right on ceiling
		case BS_RIGHT:
		{
			o->xinertia = BS_SPEED;
			o->yinertia = 0;
			o->dirparam = RIGHT;
			
			// all rotators destroyed?
			if (rotators_left <= 0 && ++o->timer > 3)
			{
				// center of room
				if (o->x >= (312<<CSF) && o->x <= (344<<CSF))
				{
					o->state = CS_ENTER_FORM;
				}
			}
			
			if (passed_xcoord(GREATER_THAN, ARENA_RIGHT))
				o->state = BS_DOWN;
		}
		break;
		
		// down on wall
		case BS_DOWN:
		{
			o->xinertia = 0;
			o->yinertia = BS_SPEED;
			o->dirparam = DOWN;
			
			if (passed_ycoord(GREATER_THAN, ARENA_BOTTOM))
			{
				o->state = BS_LEFT;
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
void BallosBoss::RunForm3(Object *o)
{
	static const int YPOSITION = (167 << CSF);
	
	// platform spin speeds and how long they travel at each speed.
	// it's a repeating pattern.
	static const struct
	{
		int length, speed;
	}
	platform_pattern[] =
	{
		500, 2,
		200, 1,
		20,  0,
		200, -1,
		500, -2,
		200, -1,
		20,  0,
		200, 1,
		0,   0
	};
	
	switch(o->state)
	{
		// enter form 3
		case CS_ENTER_FORM:
		{
			o->timer = 0;
			o->xinertia = 0;
			o->yinertia = 0;
			o->state++;
			
			DeleteObjectsOfType(OBJ_GREEN_DEVIL_SPAWNER);
			SetRotatorStates(20);	// fast spin CCW
		}
		case CS_ENTER_FORM+1:
		{
			// come down into center of room
			o->y += (YPOSITION - o->y) / 8;
			o->timer++;
			
			if (o->timer == 50)
			{
				// create platforms
				platform_speed = 0;
				
				for(int angle=0;angle<0x100;angle+=0x20)
				{
					Object *p = CreateObject(o->x, o->y, OBJ_BALLOS_PLATFORM);
					p->dirparam = angle;
				}
			}
			
			if (o->timer > 100)
			{
				platform_speed = -1;
				
				o->state = CS_SPAWN_SPIKES;
				o->timer = 0;
			}
		}
		break;
		
		case CS_SPAWN_SPIKES:
		{
			o->timer = 0;
			o->xmark = 0;
			o->state++;
		}
		case CS_SPAWN_SPIKES+1:
		{
			o->timer++;
			
			if ((o->timer % 3) == 0)
				sound(SND_QUAKE);
			
			if ((o->timer % 30) == 1)
			{
				o->xmark += 2;
				CreateObject((o->xmark * TILE_W) << CSF, \
							 FLOOR_Y + (48 << CSF), OBJ_BALLOS_SPIKES);
				
				if (o->xmark == 38)
					o->state = CS_EXPLODE_BLOODY;
			}
		}
		break;
		
		// explode into all bloody
		case CS_EXPLODE_BLOODY:
		{
			SetEyeStates(EYE_INVISIBLE);
			SetRotatorStates(30);			// slow spin CW, alternate open/closed
			
			SmokeClouds(o, 256, 60, 60);	// ka boom!
			sound(SND_EXPLOSION1);
			megaquake(30);
			
			body->frame |= 2;		// go all bloody
			body->flags &= ~FLAG_INVULNERABLE;
			shield->flags &= ~FLAG_INVULNERABLE;
			
			o->state = CS_SPIN_PLATFORMS;
		}
		// fall-through
		case CS_SPIN_PLATFORMS:
		{
			o->state++;
			o->timer = 0;
			o->timer2 = 0;
			o->timer3 = 0;
			
			platform_speed = platform_pattern[o->timer2].speed;
		}
		case CS_SPIN_PLATFORMS+1:
		{
			// spin platforms
			if (++o->timer3 > platform_pattern[o->timer2].length)
			{
				o->timer3 = 0;
				o->timer2++;
				
				if (!platform_pattern[o->timer2].length)
					o->timer2 = 0;
				
				platform_speed = platform_pattern[o->timer2].speed;
			}
			
			// spawn butes
			switch(++o->timer)
			{
				case 270:	// spawn swordsmen from face
				case 280:
				case 290:
				{
					SmokeXY(o->x, o->y - (52<<CSF), 4);
					CreateObject(o->x, o->y - (52<<CSF), OBJ_BUTE_SWORD_RED)->dir = UP;
					sound(SND_EM_FIRE);
				}
				break;
				
				case 300:	// spawn archers on side
				{
					o->timer = 0;
					// direction butes will be facing, not side of screen
					int dir = (player->CenterX() > o->x) ? LEFT : RIGHT;
					
					for(int i=0;i<8;i++)
					{
						// give some granularity to the coords,
						// so that they can't overlap too closely.
						int x = (random(-TILE_W, TILE_W) & ~3) << CSF;
						int y = (random(2 * TILE_H, 17 * TILE_H) & ~3) << CSF;
						if (dir == LEFT) x += MAPX(map.xsize - 1);
						
						CreateObject(x, y, OBJ_BUTE_ARCHER_RED)->dir = dir;
					}
				}
				break;
			}
			
			// spawn blood
			int prob = (o->hp <= 500) ? 4 : 10;
			if (!random(0, prob))
			{
				CreateObject(o->x + random(-40<<CSF, 40<<CSF), \
							 o->y + random(0, 40<<CSF),
							 OBJ_RED_ENERGY)->angle = DOWN;
			}
		}
		break;
	}
	
}


void BallosBoss::RunDefeated(Object *o)
{
	switch(o->state)
	{
		case 1000:
		{
			o->state = 1001;
			o->timer = 0;
			
			SetEyeStates(EYE_EXPLODING);	// blow out eyes
			SetRotatorStates(1000);			// explode rotators
			
			uint32_t mask = ~(FLAG_SOLID_BRICK | FLAG_SOLID_MUSHY | \
							  FLAG_SHOOTABLE | FLAG_INVULNERABLE);
			main->flags &= mask;
			body->flags &= mask;
			shield->flags &= mask;
			eye[LEFT]->flags &= mask;
			eye[RIGHT]->flags &= mask;
		}
		case 1001:
		{
			int x = o->x + random(-60<<CSF, 60<<CSF);
			int y = o->y + random(-60<<CSF, 60<<CSF);
			SmokePuff(x, y);
			effect(x, y, EFFECT_BOOMFLASH);
			
			o->timer++;
			
			if ((o->timer % 12) == 0)
				sound(SND_MISSILE_HIT);
			
			if (o->timer > 150)
			{
				o->timer = 0;
				o->state = 1002;
				
				starflash.Start(o->x, o->y);
				sound(SND_EXPLOSION1);
			}
		}
		break;
		
		case 1002:
		{
			megaquake(40);
			
			if (++o->timer >= 50)
			{
				KillObjectsOfType(OBJ_BUTE_ARCHER_RED);
				KillObjectsOfType(OBJ_BALLOS_SPIKES);
				
				body->invisible = true;
				eye[LEFT]->invisible = true;
				eye[RIGHT]->invisible = true;
				o->state = 1003;
			}
		}
		break;
	}
}

void ondeath_ballos(Object *o)
{
	// as soon as one of his forms is defeated make him non-killable
	// until the init for the next form runs and makes him killable again.
	// intended to fix the extremely rare possibility of killing him completely
	// after his 1st form instead of moving on to the spiky rotators like he should.
	o->hp = 999999;
}

// Handles his eyes.
//
// When closed, the eyes are like "overlay" objects that replace the open eyes
// drawn on the body, and allow animating them seperately from the body.
//
// When open, the eyes turn invisible and are used as shoot-points to detect shots
// hitting the eyes drawn on the body.
void BallosBoss::run_eye(int index)
{
	Object *o = eye[index];
	
	switch(o->state)
	{
		case 0:
		{
			o->flags = (FLAG_SHOOTABLE | FLAG_INVULNERABLE);
			o->state = 1;
		}
		break;
		
		// open eyes
		case EYE_OPENING:
		{
			o->frame = 0;
			o->animtimer = 0;
			o->state++;
		}
		case EYE_OPENING+1:
		{
			if (++o->animtimer > 2)
			{
				o->animtimer = 0;
				if (++o->frame >= 3)
				{
					o->flags &= ~FLAG_INVULNERABLE;
					o->invisible = true;
					o->state++;
				}
			}
		}
		break;
		
		// close eyes
		case EYE_CLOSING:
		{
			o->frame = 3;
			o->invisible = false;
			o->flags |= FLAG_INVULNERABLE;
			
			o->animtimer = 0;
			o->state++;
		}
		case EYE_CLOSING+1:
		{
			if (++o->animtimer > 2)
			{
				o->animtimer = 0;
				if (--o->frame <= 0)
				{
					o->frame = 0;
					o->state++;
				}
			}
		}
		break;
		
		// invisible (the underlying eyes drawn on the body are what are seen)
		case EYE_INVISIBLE:
		{
			o->flags &= ~FLAG_INVULNERABLE;
			o->invisible = true;
			o->state++;
		}
		break;
		
		// explode eyes (final defeat sequence)
		case EYE_EXPLODING:
		{
			o->frame = 4;	// empty eyes
			o->invisible = false;
			
			o->flags &= ~(FLAG_SHOOTABLE | FLAG_INVULNERABLE);
			o->state++;
			
			if (o->dir == LEFT)
				SmokeXY(o->x - (4<<CSF), o->y, 10, 4, 4);
			else
				SmokeXY(o->x + (4<<CSF), o->y, 10, 4, 4);
		}
		break;
	}
	
}

void BallosBoss::place_eye(int index)
{
	Object *o = eye[index];
	
	if (o->dir == LEFT)
		o->x = main->x - (24 << CSF);
	else
		o->x = main->x + (24 << CSF);
	
	o->y = main->y - (36 << CSF);
}

void BallosBoss::SetEyeStates(int newstate)
{
	eye[LEFT]->state = newstate;
	eye[RIGHT]->state = newstate;
}

void ai_ballos_rotator(Object *o)
{
	switch(o->state)
	{
		case 0:		// just spawned
		{
			o->state = 1;
			o->timer2 = o->angle * 2;
			
			o->timer3 = 0xC0;
			o->damage = 14;
		}
		case 1:		// expanding outward; overshoot a bit
		{
			if (o->timer3 < 0x140)
				o->timer3 += 0x08;
			else
				o->state = 2;
		}
		break;
		case 2:		// come back in to correct distance
		{
			if (o->timer3 > 0x130)
				o->timer3 -= 0x04;
			else
				o->state = 3;
		}
		break;
		
		// spinning CCW during form 2 (working like treads)
		case 10:
		{
			o->state = 11;
			
			o->flags |= FLAG_SHOOTABLE;
			o->flags &= ~FLAG_INVULNERABLE;
			o->hp = 1000;
		}
		case 11:		// spinning during phase 2, alive
		{
			o->timer2 -= 2;
			if (o->timer2 < 0) o->timer2 += 0x200;
			
			if (o->frame != 2)		// still undestroyed?
			{
				o->frame = (o->shaketime & 2) ? 1 : 0;
				
				if (o->hp <= (1000 - 100))
				{
					o->flags &= ~FLAG_SHOOTABLE;
					o->frame = 2;	// close eye
					
					SmokeClouds(o, 32, 16, 16);
					sound(SND_LITTLE_CRASH);
					
					rotators_left--;
				}
			}
			
			spawn_impact_puffs(o);
		}
		break;
		
		case 20:	// spinning fast CCW while spikes come up
		{
			o->frame = 2;
			
			o->timer2 -= 4;
			if (o->timer2 < 0) o->timer2 += 0x200;
		}
		break;
		
		case 30:	// beginning form 3
		{
			o->state = 31;
			o->hp = 1000;
			o->damage = 10;
			
			// this dir was set when they were created and
			// alternates left/right around the circle
			if (o->dir == LEFT)
			{
				o->flags |= FLAG_SHOOTABLE;
				o->frame = 0;
			}
			else
			{
				o->flags |= FLAG_INVULNERABLE;
				o->frame = 2;
			}
		}
		case 31:		// form 3 CW slow spin
		{
			// come in closer to main object
			if (o->timer3 > 0x100)
				o->timer3--;
			
			// spin CW
			if (++o->timer2 > 0x200)
				o->timer2 -= 0x200;
			
			if (o->flags & FLAG_SHOOTABLE)
			{
				o->frame = (o->shaketime & 2) ? 1 : 0;
				
				if (o->hp < (1000 - 100))
				{
					o->xinertia = 0;
					o->yinertia = 0;
					
					o->flags &= ~(FLAG_SHOOTABLE | FLAG_IGNORE_SOLID);
					SmokeClouds(o, 32, 16, 16);
					sound(SND_LITTLE_CRASH);
					
					o->frame = 2;
					o->state = 40;
					o->damage = 5;
					
					// blow up immediately if Ballos is defeated
					o->timer2 = 0;
				}
			}
		}
		break;
		
		case 40:	// destroyed during phase 3, bouncing
		{
			o->yinertia += 0x20;
			LIMITY(0x5ff);
			
			if (o->blockl) o->xinertia = 0x100;
			if (o->blockr) o->xinertia = -0x100;
			
			if (o->blockd && o->yinertia >= 0)
			{
				// first time they hit they head toward player, after that
				// they keep going in same direction until hit wall
				if (o->xinertia == 0)
					o->xinertia = (o->CenterX() < player->CenterX()) ? 0x100 : -0x100;
				
				o->yinertia = -0x800;
				sound(SND_QUAKE);
			}
		}
		break;
		
		case 1000:		// Ballos was defeated
		{
			o->state = 1001;
			o->xinertia = 0;
			o->yinertia = 0;
			
			o->frame = 2;
			o->flags &= ~(FLAG_SHOOTABLE | FLAG_IGNORE_SOLID);
			o->damage = 0;
			
			o->timer2 /= 4;
		}
		case 1001:
		{
			// explode one by one going clockwise
			if (o->timer2 <= 0)
			{
				SmokeClouds(o, 32, 16, 16);
				sound(SND_LITTLE_CRASH);
				effect(o->CenterX(), o->CenterY(), EFFECT_BOOMFLASH);
				o->Delete();
			}
			else
			{
				o->timer2--;
				o->frame = (o->timer2 & 2) ? 1 : 0;
			}
		}
		break;
	}
}

void aftermove_ballos_rotator(Object *o)
{
	if (o->state < 40)
	{
		Object *ballos = game.stageboss.object;
		if (!ballos) return;
		
		uint8_t angle = (o->timer2 / 2);
		int dist = (o->timer3 / 4) << CSF;
		
		o->x = ballos->x + (xinertia_from_angle(angle, dist));
		o->y = ballos->y + (yinertia_from_angle(angle, dist));
	}
}


static void SetRotatorStates(int newstate)
{
	Object *o;
	FOREACH_OBJECT(o)
	{
		if (o->type == OBJ_BALLOS_ROTATOR)
			o->state = newstate;
	}
}


// spawns impact smokeclouds/skulls as the rotators hit the ground/walls
static void spawn_impact_puffs(Object *o)
{
	Object *ballos = game.stageboss.object;
	
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
	if (o->timer2 == hitdata[bd].wallangle + HITANGLE)
	{
		make_puff(o->x + hitdata[bd].xoffs1, o->y + hitdata[bd].yoffs1, bd);
		make_puff(o->x + hitdata[bd].xoffs2, o->y + hitdata[bd].yoffs2, bd);
		sound(SND_QUAKE);
		
		if (bd == RIGHT)		// on ceiling
		{
			CreateObject(o->x - SHORT, o->y - LONG, OBJ_BALLOS_SKULL);
		}
	}
}

static void make_puff(int x, int y, int bd)
{
	Object *o = SmokePuff(x, y);
	
	// make sure the smoke puff is traveling away from floor/wall
	switch(bd)
	{
		case LEFT:	o->yinertia = -abs(o->yinertia); break;
		case UP:	o->xinertia = abs(o->xinertia); break;
		case RIGHT: o->yinertia = abs(o->yinertia); break;
		case DOWN:	o->xinertia = -abs(o->xinertia); break;
	}
}

void ai_ballos_platform(Object *o)
{
	Object *ballos = game.stageboss.object;
	if (!ballos) return;
	
	if (ballos->state >= 1000)		// defeated
	{
		if (o->state < 1000)
			o->state = 1000;
	}
	
	switch(o->state)
	{
		case 0:		// just spawned
		{
			o->timer2 = (o->dirparam * 4);
			o->timer3 = 0xC0;
			o->state = 1;
		}
		case 1:		// expanding outward
		{
			if (o->timer3 < 0x1C0)
				o->timer3 += 8;
			else
				o->state = 2;
		}
		break;
		
		// running - the direction/speed is set by the global variable,
		// controlled by Ballos.
		case 2:
		{
			o->timer2 += platform_speed;
			if (o->timer2 < 0) o->timer2 += 0x400;
			if (o->timer2 >= 0x400) o->timer2 -= 0x400;
		}
		break;
		
		case 1000:	// ballos defeated!
		{
			o->state = 1001;
			o->xinertia = 0;
			o->yinertia = 0;
			o->flags &= ~FLAG_SOLID_BRICK;
		}
		case 1001:
		{
			o->yinertia += 0x40;
			
			if (o->Top() > (map.ysize * TILE_H) << CSF)
				o->Delete();
		}
		break;
	}
	
	if (o->state >= 1000)
		return;
	
	// let player jump up through platforms, but be solid when he is standing on them
	if (player->yinertia < 0 || player->CenterY() > o->Top())
	{
		o->flags &= ~FLAG_SOLID_BRICK;
	}
	else
	{
		o->flags |= FLAG_SOLID_BRICK;
	}
	
	// spin
	uint8_t angle = o->timer2 / 4;
	int xoff, yoff;
	
	xoff = xinertia_from_angle(angle, o->timer3 << CSF);
	yoff = yinertia_from_angle(angle, o->timer3 << CSF);
	
	o->xmark = (xoff / 4) + ballos->x;
	o->ymark = ((yoff / 4) + (16 << CSF)) + ballos->y;
	
	switch(abs(platform_speed))
	{
		case 1:
			if ((o->timer2 % 4) == 0)
			{
				o->speed = (o->ymark - o->y) / 4;
			}
		break;
		
		case 2:
			if ((o->timer2 & 2) == 0)
			{
				o->speed = (o->ymark - o->y) / 2;
			}
		break;
		
		default:
			o->speed = (o->ymark - o->y);
		break;
	}
	
	o->xinertia = (o->xmark - o->x);
	o->yinertia = o->speed;
}

bool BallosBoss::passed_xcoord(bool ltgt, int xcoord, bool reset)
{
	int next_x = main->x + main->xinertia;
	bool result;
	
	if (ltgt == LESS_THAN)
		result = (next_x <= xcoord);
	else
		result = (next_x >= xcoord);
	
	if (result && reset)
	{
		main->x = xcoord;
		main->xinertia = 0;
	}
	
	return result;
}


bool BallosBoss::passed_ycoord(bool ltgt, int ycoord, bool reset)
{
	int next_y = main->y + main->yinertia;
	bool result;
	
	if (ltgt == LESS_THAN)
		result = (next_y <= ycoord);
	else
		result = (next_y >= ycoord);
	
	if (result && reset)
	{
		main->y = ycoord;
		main->yinertia = 0;
	}
	
	return result;
}
*/
