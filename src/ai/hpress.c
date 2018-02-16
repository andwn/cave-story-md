#include "ai_common.h"

// position in Hell (note this is the center position, because there is a draw point)
#define HELL_X			(168<<CSF)
#define HELL_Y			(80<<CSF)
#define HELL_FLOOR		(168<<CSF)
#define HELL_BOTTOM		(488<<CSF)

// positions of stuff for Passageway scene
#define PWAY_X			(168<<CSF)		// X position of corridor
#define PWAY_TOP		(70<<CSF)		// starting position for falling scene
#define PWAY_BOTTOM		(420<<CSF)		// resting position after fall

// Some aliases
#define uncover_left	8
#define uncover_right	12
#define uncover_y		curly_target_time
//#define shield_left		pieces[0]
//#define shield_right	pieces[1]

static void run_defeated(Entity *e) {
	switch(e->state) {
		// defeated (set by ondeath script, after a brief pause)
		case 500:
		{
			e->nflags &= ~NPC_SHOOTABLE;
			e->state = 501;
			e->timer = 0;
			e->frame = 0;
			
			//if (shield_left) { shield_left->state = STATE_DELETE; shield_left = NULL; }
			//if (shield_right) { shield_right->state = STATE_DELETE; shield_right = NULL; }
			//sprites[e->sprite].bbox = fullwidth_bbox;
			
			// get rid of enemies--the butes can stay, though.
			entities_clear_by_type(OBJ_HP_LIGHTNING);
			entities_clear_by_type(OBJ_ROLLING);
		} /* fallthrough */
		case 501:
		{
			e->timer++;
			if ((e->timer % 16) == 0) {
				sound_play(SND_BLOCK_DESTROY, 5);
				//SmokePuff(e->x + random(-60<<CSF, 60<<CSF),
				//		  e->y + random(-40<<CSF, 40<<CSF));
			}
			
			// eye opens
			switch(e->timer)
			{
				case 95: e->frame = 1; break;	// partially open
				case 98: e->frame = 2; break;	// fully open
				case 101:
				{
					e->state = 502;		// fall
					e->attack = 127;
					e->eflags &= ~NPC_SPECIALSOLID;
				}
				break;
			}
		}
		break;
		
		// falling (haven't hit floor yet)
		case 502:
		{
			// hit floor: bounce. The floor tiles are not
			// solid to NPC's so we can't use blockd.
			if (e->y > HELL_FLOOR) {
				e->state = 503;
				e->y_speed = -SPEED_10(0x200);
				
				// kill floor
				uint16_t y = sub_to_block(HELL_FLOOR) + 4;
				for(uint16_t x=uncover_left-1;x<=uncover_right+1;x++) {
					stage_replace_block(x, y, 0);
				}
				sound_play(SND_BLOCK_DESTROY, 5);
				
				if (e->y > HELL_BOTTOM) e->state = 0;
			}
		} /* fallthrough */
		case 503:	// falling, and hit floor already
		{
			e->y_speed += SPEED_8(0x40);
			e->y += e->y_speed;
		}
		break;
	}
}


// states for when it's in the Passageway
static void run_passageway(Entity *e) {
	switch(e->state) {
		// "dead"/smoking on Passageway floor
		// note they may also be coming back from Statue Room.
		// script-triggered.
		case 20:
		{
			e->state = 21;
			e->x = PWAY_X;
			e->y = PWAY_BOTTOM;
			e->nflags &= ~(NPC_SHOOTABLE | NPC_SPECIALSOLID);
			e->attack = 0;
		} /* fallthrough */
		case 21:
		{
			if ((++e->timer % 16) == 0)
			{
				//int x = random(e->Left(), e->Right());
				//int y = random(e->Top(), e->Bottom());
				//SmokePuff(x, y);
				//effect(x, y, EFFECT_BOOMFLASH);
			}
		}
		break;
		
		// falling through Passageway when you jump down hole in post-defeat scene.
		// script-triggered.
		case 30:
		{
			e->state = 31;
			e->frame = 2;	// eye open
			//COPY_PFBOX;		// ensure that we are using the full-width bbox
			
			e->x = PWAY_X;
			e->y = PWAY_TOP;
		} /* fallthrough */
		case 31:
		{
			e->y += SPEED_12(0x800);
			
			if (e->y >= PWAY_BOTTOM) {
				e->y = PWAY_BOTTOM;
				sound_play(SND_MISSILE_HIT, 5);
				
				e->frame = 0;
				//for(int i=0;i<5;i++)
				//	SmokePuff(random(e->Left(), e->Right()), e->Bottom());
				
				e->state = 20;	// continue smoking
			}
		}
		break;
	}
}

void onspawn_heavypress(Entity *e) {
	e->alwaysActive = TRUE;
	e->x = HELL_X;
	e->y = HELL_Y;
	//shield_left = shield_right = NULL;
	uncover_y = 11;
	
	e->hurtSound = SND_ENEMY_HURT_COOL;
	//e->damage_time = 8;
	
	e->nflags = (NPC_SHOWDAMAGE | NPC_EVENTONDEATH | NPC_SPECIALSOLID | NPC_IGNORESOLID);
	
	e->attack = 10;
	e->health = 700;
	e->event = 1000;	// defeated script
	
	e->hit_box =     (bounding_box) { 36, 56, 36, 56 };
	e->display_box = (bounding_box) { 40, 60, 40, 60 };
	
	entities_clear_by_type(OBJ_ROLLING);
	
	// setup bboxes
	//center_bbox = sprites[e->sprite].frame[0].dir[0].pf_bbox;
	//fullwidth_bbox = sprites[e->sprite].frame[2].dir[0].pf_bbox;
	
	//sprites[e->sprite].bbox = fullwidth_bbox;
}

void ai_heavypress(Entity *e) {
	run_defeated(e);
	run_passageway(e);
	
	switch(e->state) {
		// fight begin (script-triggered)
		case 100:
		{
			// create shielding objects for invincibility on either side
			// don't use puppet 1 because Deleet's use that when they explode.
			//shield_left = entity_create(e->x, e->y, OBJ_HEAVY_PRESS_SHIELD, 0);
			//shield_left->sprite = SPR_BBOX_PUPPET_2;
			//sprites[shield_left->sprite].bbox = fullwidth_bbox;
			//sprites[shield_left->sprite].bbox.x2 = center_bbox.x1 - 1;
			
			//shield_right = entity_create(e->x, e->y, OBJ_HEAVY_PRESS_SHIELD, 0);
			//shield_right->sprite = SPR_BBOX_PUPPET_3;
			//sprites[shield_right->sprite].bbox = fullwidth_bbox;
			//sprites[shield_right->sprite].bbox.x1 = center_bbox.x2 + 1;
			
			// then switch to small pfbox where we're only hittable in the center
			e->frame = 0;
			//sprites[e->sprite].bbox = center_bbox;
			
			e->nflags |= NPC_SHOOTABLE;
			e->nflags &= ~NPC_INVINCIBLE;
			
			e->state = 101;
			e->timer = 0;	// pause a moment before Butes come
		} /* fallthrough */
		case 101:
		{	// fire lightning
			//entity_create(e->x, e->y+0x7800, OBJ_HP_LIGHTNING, 0);
			e->state = 102;
		} /* fallthrough */
		case 102:
		{
			// spawn butes on alternating sides
			switch(e->timer++)
			{
				case 100:
				case 260:
					//entity_create(block_to_sub(17), block_to_sub(15), OBJ_BUTE_FALLING, 0);
				break;
				
				case 180:
				case 340:
					//entity_create(block_to_sub(3), block_to_sub(15), OBJ_BUTE_FALLING, 0)->dir = 1;
				break;
				
				case 398:
					// fire lightning next frame
					e->state = 101;
					e->timer = 100;
				break;
			}
			
			// uncover as it's damaged
			if (e->health < (uncover_y * 70) && uncover_y > 1) {
				uncover_y--;
				sound_play(SND_BLOCK_DESTROY, 5);
				
				for(uint16_t x=uncover_left;x<=uncover_right;x++)
					stage_replace_block(x, uncover_y, 0);
			}
		}
		break;
	}
	
	// Flicker the gray part of PAL_Sym to make the body flash.
	// This will cause some other things to flash, but whatever.
	static const uint16_t bright_sym[4] = { 0x888, 0xAAA, 0xCCC, 0xEEE };
	if(e->damage_time) {
		if((e->damage_time & 3) == 3) {
			// Flash bright
			DMA_queueDma(DMA_CRAM, (uint32_t) bright_sym, 22*2, 4, 2);
		} else if((e->damage_time & 3) == 1) {
			// Flash normal
			DMA_queueDma(DMA_CRAM, (uint32_t) &PAL_Sym.data[6], 22*2, 4, 2);
		}
	}
}

void ondeath_heavypress(Entity *e) {
	tsc_call_event(e->event);
	e->nflags &= ~NPC_SHOOTABLE;
}

void ai_hp_lightning(Entity *e) {
	switch(e->state) {
		case 0:
		{
			sound_play(SND_TELEPORT, 5);
			//e->sprite = SPR_HP_CHARGE;
			e->state = 1;
		} /* fallthrough */
		case 1:
		{
			ANIMATE(e, 4, 0,1,2);
			
			if (++e->timer > TIME(50)) {
				e->state = 10;
				e->animtime = 0;
				e->frame = 3;
				e->attack = 10;
				e->timer = 0;
				//e->sprite = SPR_HP_LIGHTNING;
				sound_play(SND_LIGHTNING_STRIKE, 5);
				
				// smoke on floor where it struck
				//SmokeXY(e->x, e->Bottom() - (7<<CSF), 3, 0, 0);
			}
		}
		break;
		
		case 10:
		{
			ANIMATE(e, 4, 3,4,5,6);
			if (++e->timer > 16) e->state = STATE_DELETE;
		}
		break;
	}
	
}
