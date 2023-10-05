#include "ai_common.h"

// position in Hell (note this is the center position, because there is a draw point)
#define HELL_X			pixel_to_sub(168)
#define HELL_Y			pixel_to_sub(80)
#define HELL_FLOOR		pixel_to_sub(168)
#define HELL_BOTTOM		pixel_to_sub(488)

// positions of stuff for Passageway scene
#define PWAY_X			pixel_to_sub(168)		// X position of corridor
#define PWAY_TOP		pixel_to_sub(70)		// starting position for falling scene
#define PWAY_BOTTOM		pixel_to_sub(420)		// resting position after fall

// Some aliases
#define uncover_left	8
#define uncover_right	12
#define uncover_y		curly_target_time
#define shield_left		pieces[0]
#define shield_right	pieces[1]

static void run_defeated(Entity *e) {
	switch(e->state) {
		// defeated (set by ondeath script, after a brief pause)
		case 500:
		{
			e->flags &= ~NPC_SHOOTABLE;
			e->state = 501;
			e->timer = 0;
			e->frame = 0;
			
			if(shield_left) { shield_left->state = STATE_DELETE; shield_left = NULL; }
			if(shield_right) { shield_right->state = STATE_DELETE; shield_right = NULL; }
			
			// get rid of enemies--the butes can stay, though.
			entities_clear_by_type(OBJ_HP_LIGHTNING);
			entities_clear_by_type(OBJ_ROLLING);
		} /* fallthrough */
		case 501:
		{
			if ((++e->timer & 15) == 0) {
				sound_play(SND_BLOCK_DESTROY, 5);
				effect_create_smoke((e->x - 0x7FFF + (rand() & 0xFFFF)) >> CSF,
									(e->y - 0x7FFF + (rand() & 0xFFFF)) >> CSF);
			}
			// eye opens
			if(e->timer == TIME_8(90)) e->frame = 1;
			else if(e->timer == TIME_8(100)) e->frame = 2;
			else if(e->timer >= TIME_8(105)) {
				e->state = 502;		// fall
				e->attack = 127;
				e->flags &= ~NPC_SPECIALSOLID;
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
				e->attack = 0;
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
			e->flags &= ~(NPC_SHOOTABLE | NPC_SPECIALSOLID);
			e->attack = 0;
		} /* fallthrough */
		case 21:
		{
			if ((++e->timer & 15) == 0) {
				effect_create_smoke((e->x - 0x7FFF + (rand() & 0xFFFF)) >> CSF,
									(e->y - 0x7FFF + (rand() & 0xFFFF)) >> CSF);
			}
		}
		break;
		
		// falling through Passageway when you jump down hole in post-defeat scene.
		// script-triggered.
		case 30:
		{
			e->state = 31;
			e->frame = 2;	// eye open
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
				//	SmokePuff(rand(e->Left(), e->Right()), e->Bottom());
				
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
	shield_left = NULL;
	shield_right = NULL;
	uncover_y = 11;
	
	e->hurtSound = SND_ENEMY_HURT_COOL;
	
	e->flags = (NPC_SHOWDAMAGE | NPC_EVENTONDEATH | NPC_SPECIALSOLID | NPC_IGNORESOLID);
	
	e->attack = 10;
	e->health = 700;
	e->event = 1000;	// defeated script
	
	e->hit_box =     (bounding_box) {{ 36, 56, 36, 56 }};
	e->display_box = (bounding_box) {{ 40, 60, 40, 60 }};
}

void ai_heavypress(Entity *e) {
	if(stageID == STAGE_HELL_PASSAGEWAY_2) run_passageway(e);
	else if(e->state >= 500) run_defeated(e);
	else {
		switch(e->state) {
			// fight begin (script-triggered)
			case 100:
			{
				// create shielding objects for invincibility on either side
				shield_left = entity_create(e->x, e->y, OBJ_HEAVY_PRESS_SHIELD, 0);
				shield_right = entity_create(e->x, e->y, OBJ_HEAVY_PRESS_SHIELD, NPC_OPTION2);
				
				e->flags |= NPC_SHOOTABLE;
				e->flags &= ~NPC_INVINCIBLE;
				
				e->state = 101;
				e->timer = 0;	// pause a moment before Butes come
			} /* fallthrough */
			case 101:
			{	// fire lightning
				entity_create(e->x, e->y + 0x7800, OBJ_HP_LIGHTNING, 0);
				e->state = 102;
			} /* fallthrough */
			case 102:
			{
				// spawn butes on alternating sides
				e->timer++;
				if(e->timer == TIME_8(100) || e->timer == TIME_10(260)) {
					entity_create(block_to_sub(17), block_to_sub(15), OBJ_BUTE_FALLING, NPC_OPTION1);
				} else if(e->timer == TIME_8(180) || e->timer == TIME_10(340)) {
					entity_create(block_to_sub(3), block_to_sub(15), OBJ_BUTE_FALLING, NPC_OPTION1);
				} else if(e->timer >= TIME_10(400)) {
					// fire lightning next frame
					e->state = 101;
					e->timer = TIME_8(90);
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
				dma_queue(DmaCRAM, (uint32_t) bright_sym, 22<<1, 4, 2);
			} else if((e->damage_time & 3) == 1) {
				// Flash normal
				dma_queue(DmaCRAM, (uint32_t) &PAL_Sym[6], 22<<1, 4, 2);
			}
		}
	}
}

void ondeath_heavypress(Entity *e) {
	tsc_call_event(e->event);
	e->flags &= ~NPC_SHOOTABLE;
}

void onspawn_hp_lightning(Entity *e) {
	e->alwaysActive = TRUE;
	sound_play(SND_TELEPORT, 5);
}

void ai_hp_lightning(Entity *e) {
	if((++e->timer & 3) == 0 && ++e->frame > 2) e->frame = 0;
	
	if(e->timer > TIME_8(50)) {
		sound_play(SND_LIGHTNING_STRIKE, 5);
		entity_create(e->x + pixel_to_sub(32), e->y + pixel_to_sub(72), OBJ_LIGHTNING, 0);
		e->state = STATE_DELETE;
		// smoke on floor where it struck
		//SmokeXY(e->x, e->Bottom() - (7<<CSF), 3, 0, 0);
	}
}

void onspawn_hpress_shield(Entity *e) {
	e->alwaysActive = TRUE;
	if(e->flags & NPC_OPTION2) {
		e->x = bossEntity->x + pixel_to_sub(28);
	} else {
		e->x = bossEntity->x - pixel_to_sub(28);
	}
	e->y = bossEntity->y + pixel_to_sub(40);
	e->hit_box = (bounding_box) {{ 16, 16, 16, 16 }};
	e->flags |= NPC_SHOOTABLE | NPC_INVINCIBLE;
	e->health = 1000;
}
