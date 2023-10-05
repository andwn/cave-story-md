#include "ai_common.h"

#define CFRONT				5
#define CBACK				6
#define CFACE				7
#define CSUE				8
#define CMISERY				9

#define savedhp		id
#define angle		jump_time

enum CORE_STATES {
	CR_FightBegin		= 20,		// scripted
	CR_FaceClosed		= 200,
	CR_FaceSkull		= 210,
	CR_FaceTeeth		= 220,
	CR_FaceDoom			= 230,
	
	CR_Defeated			= 500,		// scripted
	CR_Exploding		= 1000
};

enum FACE_STATES {
	FC_Closed		= 0,
	FC_Skull		= 1,
	FC_Teeth		= 2,
	FC_Mouth		= 3
};

enum ROTR_STATES {
	RT_Spin_Closed			= 10,
	RT_Spin_Open			= 20,
	RT_Spin_Slow_Closed		= 30,
	RT_Spin_Fast_Closed		= 40
};

// Minicore frame indeces
static const uint16_t mframeindex[4] = {
	TILE_BACKINDEX, 		// Back - dark
	TILE_BACKINDEX + 12,	// Back - bright
	TILE_BACKINDEX + 24,	// Bottom - dark
	TILE_BACKINDEX + 28,	// Bottom - bright
};

// Prototypes
static void SpawnPellet(uint8_t angle);
static void RunHurtFlash(uint16_t timer);
static uint8_t RunDefeated(Entity *e);
static void DrawMinicoreBack(Entity *e);
static void FadeMinicore(Entity *e);

/*
	Main core body:
		non-shootable when closed
		invulnerable when open
		starsolid's appear just behind the little dots
		
		when you shoot the face the starsolid hits at just past his nose.
		
		when face is open it does block the face from shooting below,
		but there is still a spot that you can hurt it from below.
		
		four always-dark minicores that spin around it, they also seem
		to be switching their z-order.
*/

void onspawn_undead_core(Entity *e) {
	// Flag 1340 is set after the Undead Core is defeated. Then there is a scene with
	// Momo and Itoh at the helicopter, and this stage gets reloaded again.
	// So anyways we need this here, otherwise there is gonna be an Unundead Core
	if(system_get_flag(1340)) {
		e->state = STATE_DELETE;
		return;
	}
	e->alwaysActive = TRUE;
	e->hurtSound = SND_CORE_HURT;
	e->health = 700;
	e->x = pixel_to_sub(624);
	e->y = pixel_to_sub(128);
	e->event = 1000;	// defeated script
	
	// create rear rotators
	//rotator[2] = create_rotator(0, 1);
	//rotator[3] = create_rotator(0x80, 1);
	
	// create front & back
	pieces[CFRONT] = entity_create(0, 0, OBJ_UDCORE_FRONT, 0);
	pieces[CFRONT]->display_box = (bounding_box) {{ 32, 48, 32, 48 }};
	
	pieces[CBACK] = entity_create(0, 0, OBJ_UDCORE_BACK, 0);
	pieces[CBACK]->display_box = (bounding_box) {{ 40, 48, 40, 48 }};
	
	// create face
	pieces[CFACE] = entity_create(0, 0, OBJ_UDCORE_FACE, 0);
	pieces[CFACE]->state = FC_Closed;
	pieces[CFACE]->hit_box = (bounding_box) {{ 24, 14, 24, 14 }};
	pieces[CFACE]->display_box = (bounding_box) {{ 28, 16, 28, 16 }};
	pieces[CFACE]->health = 1000;

	// create front rotators
	//rotator[0] = create_rotator(0, 0);
	//rotator[1] = create_rotator(0x80, 0);
	
	// Upload some tile data for the minicore sprites into the background section
	
	SHEET_LOAD(&SPR_MUCoreBack, 2, 12, mframeindex[0], 1, 0,0, 0,1);
	SHEET_LOAD(&SPR_MUCoreBottom, 2, 4, mframeindex[2], 1, 0,0, 0,1);
}

void ai_undead_core(Entity *e) {
	if (!e->health && RunDefeated(e)) return;
	
	switch(e->state) {
		// fight begin (scripted)
		case CR_FightBegin:
		{
			e->state = CR_FaceSkull;
			e->dir = 0;
			
			//SetRotatorStates(RT_Spin_Slow_Closed);
			//SpawnFaceSmoke();
		}
		break;
		
		// face closed
		case CR_FaceClosed:
		{
			e->state++;
			e->timer = 0;
			
			pieces[CFACE]->state = FC_Closed;
			pieces[CFRONT]->frame = 0;		// closed
			RunHurtFlash(0); 
			
			//set_bbox_shootable(FALSE);
			//SetRotatorStates(RT_Spin_Closed);
			//SpawnFaceSmoke();
		} /* fallthrough */
		case CR_FaceClosed+1:
		{
			e->timer++;
			
			if (e->dir || e->frame > 0 || e->health < 200) {
				if (e->timer > TIME_8(200)) {
					e->timer2++;
					sound_play(SND_CORE_THRUST, 5);
					
					// select attack mode
					if (e->health < 200) {
						e->state = CR_FaceDoom;
					} else if (e->timer2 > 2) {
						e->state = CR_FaceTeeth;
					} else {
						e->state = CR_FaceSkull;
					}
				}
			}
		}
		break;
		
		// face open/skull
		case CR_FaceSkull:
		{
			e->state++;
			e->timer = 0;
			
			pieces[CFACE]->state = FC_Skull;
			pieces[CFRONT]->frame = 1;
			//SpawnFaceSmoke();
			
			e->savedhp = e->health;
			//set_bbox_shootable(TRUE);
		} /* fallthrough */
		case CR_FaceSkull+1:
		{
			e->timer++;
			RunHurtFlash(e->timer);
			
			if (e->timer < TIME_10(300)) {
				if ((e->timer & 127) == 0) {
					SpawnPellet(1);
				} else if ((e->timer & 127) == 63) {
					SpawnPellet(0);
				}
			}
			
			if (e->timer > TIME_10(400) || (e->savedhp - e->health) > 50) {
				e->state = CR_FaceClosed;
			}
		}
		break;
		
		// face open/teeth
		case CR_FaceTeeth:
		{
			e->state++;
			e->timer = 0;
			
			pieces[CFACE]->state = FC_Teeth;
			pieces[CFRONT]->frame = 1;
			//SpawnFaceSmoke();
			
			//SetRotatorStates(RT_Spin_Open);
			camera_shake(100);
			
			e->savedhp = e->health;
			//set_bbox_shootable(TRUE);
		} /* fallthrough */
		case CR_FaceTeeth+1:
		{
			e->timer++;
			RunHurtFlash(e->timer);
			
			// fire rotators
			if ((e->timer & 63) == 1) {
				//uint8_t i = rand() & 3;
				//int32_t x = rotator[i]->x - (16<<CSF);
				//int32_t y = rotator[i]->y;
				
				sound_play(SND_FUNNY_EXPLODE, 5);
				//CreateSpinner(x, y);
			}
			
			if (e->timer > TIME_10(400) || (e->savedhp - e->health) > 150 || e->health < 200) {
				e->state = CR_FaceClosed;
			}
		}
		break;
		
		// face open/mouth: blasts of doom
		case CR_FaceDoom:
		{
			e->state++;
			e->timer = 0;
			
			pieces[CFACE]->state = FC_Mouth;
			pieces[CFRONT]->frame = 1;
			//SpawnFaceSmoke();
			//SetRotatorStates(RT_Spin_Fast_Closed);
			
			sound_play(SND_FUNNY_EXPLODE, 5);
			
			// spawn a whole bunch of crazy spinners from the face
			//CreateSpinner(pieces[CFACE]->x - (16<<CSF), pieces[CFACE]->y);
			//CreateSpinner(pieces[CFACE]->x, pieces[CFACE]->y - (16<<CSF));
			//CreateSpinner(pieces[CFACE]->x, pieces[CFACE]->y + (16<<CSF));
			
			e->savedhp = e->health;
			//set_bbox_shootable(TRUE);
		} /* fallthrough */
		case CR_FaceDoom+1:
		{
			e->timer++;
			RunHurtFlash(e->timer);
			
			if ((e->timer & 127) == 0) {
				SpawnPellet(1);
			} else if ((e->timer & 127) == 63) {
				SpawnPellet(0);
			}
		}
		break;
	}
	
	// move back and forth
	if (e->state >= CR_FightBegin && e->state < CR_Defeated) {
		if (e->x < block_to_sub(19))
			e->dir = 1;
		
		if (e->x > block_to_sub(stageWidth - 1))
			e->dir = 0;
		
		ACCEL_X(4);
	}
	
	// spawn minicore platforms
	switch(e->state) {
		case CR_FaceClosed+1:
		case CR_FaceTeeth+1:
		case CR_FaceSkull+1:
		case CR_FaceDoom+1:
		{
			e->jump_time++;
			
			// upper platforms
			if (e->jump_time == TIME_8(75)) {
				entity_create(block_to_sub(stageWidth) + 40,
							 block_to_sub(1 + (rand() & 3)), OBJ_UDMINI_PLATFORM, 0);
			}
			
			// lower platforms
			if (e->jump_time == TIME_8(150)) {
				e->jump_time = 0;
				entity_create(block_to_sub(stageWidth) + 40,
							 block_to_sub(11 + (rand() & 3)), OBJ_UDMINI_PLATFORM, 0);
				
				break;
			}
		}
		break;
	}
	
	LIMIT_X(SPEED_8(0x80));
	LIMIT_Y(SPEED_8(0x80));
	
	e->x += e->x_speed;
	e->y += e->y_speed;
}

// spawn smoke puffs from face that come when face opens/closes
//void UDCoreBoss::SpawnFaceSmoke()
//{
//	camera_shake(20);
//	
//	for(int i=0;i<8;i++)
//	{
//		int x = face->x + rand(-16<<CSF, 32<<CSF);
//		int y = main->CenterY();
//		Entity *s = SmokePuff(x, y);
//		s->x_speed = rand(-0x200, 0x200);
//		s->y_speed = rand(-0x100, 0x100);
//	}
//}

// spit a "pellet" shot out of the face. That's what I'm calling the flaming lava-rock
// type things that are thrown out and trail along the ceiling or floor.
static void SpawnPellet(uint8_t dir) {
	entity_create(bossEntity->x - (32<<CSF), bossEntity->y, OBJ_UD_PELLET, 
				  dir ? NPC_OPTION2 : 0);
}

// Swap palette back and forth between orange and normal blue color
static void RunHurtFlash(uint16_t timer) {
	uint16_t pal = (pieces[CFACE]->damage_time && (timer & 2)) ? PAL3 : PAL2;
	for(uint8_t i = 0; i < pieces[CFRONT]->sprite_count; i++) {
		sprite_pal(&pieces[CFRONT]->sprite[i], pal);
	}
	for(uint8_t i = 0; i < pieces[CBACK]->sprite_count; i++) {
		sprite_pal(&pieces[CBACK]->sprite[i], pal);
	}
}

static uint8_t RunDefeated(Entity *e) {
	switch(e->state) {
		// defeated (descending)
		case CR_Defeated:
		{
			e->state++;
			e->timer = 0;
			e->x_speed = 0;
			e->y_speed = 0;
			
			pieces[CFACE]->state = FC_Closed;
			pieces[CFRONT]->frame = 0;		// pieces[CFRONT] closed
			//SetRotatorStates(RT_Spin_Slow_Closed);
			
			camera_shake(20);
			//SmokeXY(e->x, e->y, 100, 128, 64);
			
			entities_clear_by_type(OBJ_UDMINI_PLATFORM);
			//set_bbox_shootable(FALSE);
		} /* fallthrough */
		case CR_Defeated+1:
		{
			//SmokeXY(e->x, e->y, 1, 64, 32);
			
			e->x_speed = 0x40;
			e->y_speed = 0x80;
			
			if (++e->timer > TIME_8(200)) {
				e->state = CR_Exploding;
				e->x_speed = 0;
				e->y_speed = 0;
				e->timer = 0;
			}
		}
		break;
		
		// defeated (exploding)
		case CR_Exploding:
		{
			camera_shake(100);
			e->timer++;
			
			if ((e->timer & 7) == 0)
				sound_play(SND_MISSILE_HIT, 5);
			
			//int x = e->x + rand(-72<<CSF, 72<<CSF);
			//int y = e->y + rand(-64<<CSF, 64<<CSF);
			//SmokePuff(x, y);
			//effect(x, y, EFFECT_BOOMFLASH);
			
			if (e->timer > TIME_8(100)) {
				sound_play(SND_EXPLOSION1, 5);
				SCREEN_FLASH(3);
				// Delete this stuff now while the screen is white
				entities_clear_by_type(OBJ_MISERY_MISSILE);
				pieces[CFRONT]->state = STATE_DELETE;
				pieces[CBACK]->state = STATE_DELETE;
				pieces[CFACE]->state = STATE_DELETE;
				//for(int i=0;i<NUM_ROTATORS;i++) rotator[i]->Delete();
				//for(int i=0;i<NUM_BBOXES;i++) bbox[i]->Delete();
				
				e->state++;
				e->timer = 0;
			}
		}
		break;
		case CR_Exploding+1:
		{
			camera_shake(40);
			if (++e->timer > TIME_8(5)) {
				e->state = STATE_DELETE;
				bossEntity = NULL;
				
				return 1;
			}
		}
		break;
	}
	
	return 0;
}

void ai_undead_core_face(Entity *e) {
	//e->sprite = SPR_UD_FACES;
	e->hidden = FALSE;
	e->flags = NPC_SHOWDAMAGE;
	
	switch(e->state) {
		// to "show" the closed face, we go invisible and the
		// face area of the main core shows through.
		case FC_Closed:
		{
			e->hidden = TRUE;
		}
		break;
		case FC_Skull:
		{
			e->flags |= NPC_SHOOTABLE;
			e->frame = 0;
		}
		break;
		case FC_Teeth:
		{
			e->flags |= NPC_SHOOTABLE;
			e->frame = 1;
		}
		break;
		
		// mouth blasts of doom. Once started, it's perpetual blasting
		// until told otherwise.
		case FC_Mouth:
		{
			e->state++;
			e->timer = 100;
		} /* fallthrough */
		case FC_Mouth+1:
		{
			e->flags |= NPC_SHOOTABLE;
			
			if (++e->timer > TIME_10(300))
				e->timer = 0;
			
			if (e->timer > TIME_8(250)) {
				if ((e->timer & 31) == 1)
					sound_play(SND_QUAKE, 5);
				
				if ((e->timer & 31) == 7) {
					entity_create(e->x, e->y, OBJ_UD_BLAST, 0);
					sound_play(SND_LIGHTNING_STRIKE, 5);
				}
			}
			
			if (e->timer == TIME_8(200))
				sound_play(SND_CORE_CHARGE, 5);
			
			if (e->timer >= TIME_8(200)) { //&& (e->timer & 1))
				e->frame = 3;	// mouth lit
			} else {
				e->frame = 2;	// mouth norm
			}
		}
		break;
	}
	
	// Transfer damage to main object
	if(e->health < 1000) {
		if(bossEntity->health > 1000 - e->health) {
			bossEntity->health -= 1000 - e->health;
		} else { // ded
			bossEntity->health = 0;
			tsc_call_event(1000);
		}
		e->health = 1000;
	}
	
	e->x = bossEntity->x - pixel_to_sub(28);
	e->y = bossEntity->y + pixel_to_sub(2);
}

void ai_undead_core_front(Entity *e) {
	e->x = bossEntity->x - (32<<CSF);
	e->y = bossEntity->y;
}

void ai_undead_core_back(Entity *e) {
	e->x = bossEntity->x + (40<<CSF);
	e->y = bossEntity->y;
}

// "front" refers to whether they are doing the pieces[CFRONT] (left) or rear (right)
// half of the arc; the ones marked "pieces[CFRONT]" are actually BEHIND the core.
//Entity *UDCoreBoss::create_rotator(int angle, int front)
//{
//	Entity *e = entity_create(0, 0, OBJ_UDMINI_ROTATOR);
//	e->angle = angle;
//	e->substate = front;
//	
//	return o;
//}
/*
// the rotators are 4 minicores that spin around the main core during the battle
// and have pseudo-3D effects. They also shoot the spinners during the teeth-face phase.
//
// instead of having the cores constantly rearranging their Z-Order as they pass
// in pieces[CFRONT] and behind the core, an optical illusion is used. 2 cores are always
// in pieces[CFRONT] and 2 are always behind. Each set of two cores covers only half the full
// circle. When a core in the pieces[CFRONT] set reaches the top, it warps pieces[CBACK] to the bottom
// just as a core in the pieces[CBACK] set reaches the bottom and warps pieces[CBACK] to the top.
// Thus, they swap places and the core appears to continue around the circle using
// the different z-order of the one that was just swapped-in.
void UDCoreBoss::run_rotator(Entity *e)
{
	//debug("rotr s%d", e->state);
	
	switch(e->state)
	{
		case 0:
		{
			e->sprite = SPR_UD_ROTATOR;
			e->eflags &= ~NPC_SHOOTABLE;
			e->health = 1000;
		}
		break;
		
		case RT_Spin_Closed:
		{
			e->frame = 0;
			e->angle += 2;
		}
		break;
		
		// used when firing spinners in Teeth face
		// (it's easier to coordinate if spinners are actually spawned by core
		// and just positioned next to us)
		case RT_Spin_Open:
		{
			e->frame = 1;
			e->angle += 2;
		}
		break;
		
		case RT_Spin_Slow_Closed:
		{
			e->frame = 0;
			e->angle++;
		}
		break;
		
		case RT_Spin_Fast_Closed:
		{
			e->frame = 0;
			e->angle += 4;
		}
		break;
	}
	
	// each "side" covers half the rotation angle
	int angle = (e->angle / 2);
	
	if (e->substate)
	{	// pieces[CFRONT] (left) half of arc
		angle += 0x40;
	}
	else
	{	// pieces[CBACK] (right) half of arc
		angle += 0xC0;
	}
	
	e->x = (main->x - (8<<CSF)) + x_speed_from_angle(angle, (48<<CSF));
	e->y = main->y + y_speed_from_angle(angle, (80<<CSF));
}

void UDCoreBoss::SetRotatorStates(int newstate)
{
	for(int i=0;i<NUM_ROTATORS;i++)
		rotator[i]->state = newstate;
}
*/
/*
// extra bbox puppets/shoot targets
// only one, located at the face, is shootable, the other 3 are invulnerable shields.
void UDCoreBoss::move_bboxes() {
	for(int i=0;i<NUM_BBOXES;i++) {
		bbox[i]->x = main->x + (core_bboxes[i].offset.x << CSF);
		bbox[i]->y = main->y + (core_bboxes[i].offset.y << CSF);
	}
	
	transfer_damage(bbox[BB_TARGET], main);
}

// sets up bboxes for the Core entering shootable or non-shootable mode.
void UDCoreBoss::set_bbox_shootable(bool enable) {
uint32_t body_flags, target_flags;
int i;

	// in shootable mode target can be hit and shields are up.
	// in non-shootable mode (when face is closed) nothing can be hit.
	if (enable)
	{
		body_flags = NPC_INVULNERABLE;
		target_flags = NPC_SHOOTABLE;
	}
	else
	{
		body_flags = 0;
		target_flags = 0;
	}
	
	for(i=0;i<NUM_BBOXES;i++)
	{
		bbox[i]->flags &= ~(NPC_SHOOTABLE | NPC_INVULNERABLE);
		
		if (i == BB_TARGET)
			bbox[i]->flags |= target_flags;
		else
			bbox[i]->flags |= body_flags;
	}
}
*/

// minicores by entrance seen before fight
void onspawn_ud_minicore(Entity *e) {
	e->alwaysActive = TRUE;
	e->display_box = (bounding_box) {{ 16, 16, 16, 16 }};
	e->hit_box = (bounding_box) {{ 10, 10, 32, 16 }};
	if (e->flags & NPC_OPTION2) FadeMinicore(e);
}

void ai_udmini_idle(Entity *e) {
	DrawMinicoreBack(e);
}

// these are the ones you can ride
void ai_udmini_platform(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->y_mark = e->y;
			
			e->x_speed = -SPEED_10(0x200);
			e->y_speed = SPEED_10(0x100);
			if (rand() & 1) e->y_speed = -e->y_speed;
		} /* fallthrough */
		case 1:
		{
			if (e->x < -pixel_to_sub(64)) e->state = STATE_DELETE;
			
			if (e->y > e->y_mark) e->y_speed -= 0x10;
			if (e->y < e->y_mark) e->y_speed += 0x10;
			LIMIT_Y(SPEED_8(0xFF));
			
			// when player jumps on them, they open up and start
			// moving their Y to align with the core.
			if (playerPlatform == e) {
				e->y_mark = block_to_sub(9);
				e->frame = 0;
				// Don't squish player into ceiling
				if(e->y <= block_to_sub(6)) {
					playerPlatform = NULL;
					FadeMinicore(e);
				}
			} else if(e->frame == 0) {
				e->frame = 1;
			}
		}
		break;
	}
	
	e->x += e->x_speed;
	e->y += e->y_speed;
	
	DrawMinicoreBack(e);
	
	// Fade & desolidify if we are about to squish the player
	if(e->frame < 3) {
		bounding_box c = entity_react_to_collision(&player, e);
		if(c.top && blk(player.x, 0, player.y, 10) == 0x41)
			FadeMinicore(e);
		else if(c.right && blk(player.x, -8, player.y, 0) == 0x41)
			FadeMinicore(e);
	}
}

// The face of the minicores have 4 different frames for light/dark and faces, but the
// backs just use 2 for dark/light, so we draw them separately from the engine
static void DrawMinicoreBack(Entity *e) {
	e->sprite[1] = (Sprite) { // Back
		.x = (e->x>>CSF) - (camera.x>>CSF) + ScreenHalfW + 16 + 128,
		.y = (e->y>>CSF) - (camera.y>>CSF) + ScreenHalfH - 16 + 128,
		.size = SPRITE_SIZE(3, 4),
		.attr = TILE_ATTR(PAL2,0,0,0,mframeindex[e->frame >> 1])
	};
	e->sprite[2] = (Sprite) { // Bottom
		.x = (e->x>>CSF) - (camera.x>>CSF) + ScreenHalfW - 8 + 128,
		.y = (e->y>>CSF) - (camera.y>>CSF) + ScreenHalfH + 16 + 128,
		.size = SPRITE_SIZE(4, 1),
		.attr = TILE_ATTR(PAL2,0,0,0,mframeindex[2 + (e->frame >> 1)])
	};
}

static void FadeMinicore(Entity *e) {
	e->flags &= ~NPC_SPECIALSOLID;
	e->frame = 3;
}

// falling lava-rock thing from Skull face
void ai_ud_pellet(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->x_speed = -SPEED_10(0x200);
			e->state = 1;
		} /* fallthrough */
		case 1:		// falling
		{
			if (e->flags & NPC_OPTION2) {
				e->y_speed -= SPEED_8(0x20);
				LIMIT_Y(SPEED_12(0x5ff));
				
				if (blk(e->x, 0, e->y, -4) == 0x41) e->state = 2;
			} else {
				e->y_speed += SPEED_8(0x20);
				LIMIT_Y(SPEED_12(0x5ff));
				
				if (blk(e->x, 0, e->y, 4) == 0x41) e->state = 2;
			}
			
			ANIMATE(e, 4, 0,1);
		}
		break;
		
		case 2:		// hit ground/ceiling
		{
			sound_play(SND_MISSILE_HIT, 5);
			e->x_speed = (e->x > player.x) ? -SPEED_10(0x3FF) : SPEED_10(0x3FF);
			e->y_speed = 0;
			
			e->state = 3;
			e->timer = 0;
			
			e->x -= pixel_to_sub(4);
			e->y -= pixel_to_sub(4);
		} /* fallthrough */
		case 3:
		{
			ANIMATE(e, 2, 2,3,4);
			
			if ((++e->timer & 3) == 1) {
				SMOKE_AREA((e->x >> CSF) - 8, (e->y >> CSF) - 8, 16, 16, 1);
			}
				
			if (e->x < block_to_sub(1) || e->x > block_to_sub(stageWidth - 1)) {
				e->state = STATE_DELETE;
			}
		}
		break;
	}
	e->x += e->x_speed;
	e->y += e->y_speed;
}

/*
void ai_ud_smoke(Entity *e)
{
	switch(e->state)
	{
		case 0:
		{
			e->x_speed = rand(-4, 4) << CSF;
			e->state = 1;
		}
		case 1:
		{
			e->x_speed *= 20; e->x_speed /= 21;
			e->y_speed *= 20; e->y_speed /= 21;
			
			ANIMATE_FWD(1);
			if (e->frame > sprites[e->sprite].nframes)
				e->Delete();
		}
		break;
	}
}
*/

//static void CreateSpinner(int x, int y)
//{
//	entity_create(x, y, OBJ_UD_SPINNER);
//	entity_create(x, y, OBJ_UD_SPINNER)->angle = 0x80;
//}
/*
// spinny thing shot by rotators during Teeth phase.
// they come in pairs.
void ai_ud_spinner(Entity *e)
{
	if (e->x < 0 || e->x > MAPX(map.xsize))
	{
		effect(e->CenterX(), e->CenterY(), EFFECT_BOOMFLASH);
		e->Delete();
		return;
	}
	
	switch(e->state)
	{
		case 0:
		{
			e->x_mark = e->x;
			e->y_mark = e->y;
			e->state = 1;
		}
		case 1:
		{
			e->angle += 24;
			
			e->speed -= 0x15;
			e->x_mark += e->speed;
			
			e->x = e->x_mark + x_speed_from_angle(e->angle, (4<<CSF));
			e->y = e->y_mark + y_speed_from_angle(e->angle, (6<<CSF));
			
			entity_create(e->x, e->y, OBJ_UD_SPINNER_TRAIL);
			e->BringToFront();
		}
		break;
	}
}

void ai_ud_spinner_trail(Entity *e)
{
	e->frame++;
	if (e->frame > 2)
		e->Delete();
}
*/
void ai_ud_blast(Entity *e) {
	e->x += -SPEED_12(0xFFF);
	//e->frame ^= 1;

	//SmokePuff(e->CenterX() + (rand(0, 16) << CSF),
	//		  e->CenterY() + (rand(-16, 16) << CSF));
	
	if (e->x < -pixel_to_sub(8)) e->state = STATE_DELETE;
}
