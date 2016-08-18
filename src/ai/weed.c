#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "effect.h"
#include "camera.h"
#include "system.h"

void ai_jelly_onUpdate(Entity *e) {
	switch(e->state) {
		case 0:
			e->state_time = random() % 20;
			e->x_mark = e->x;
			e->y_mark = e->y;
			if(e->eflags & NPC_OPTION2) e->direction = 1;
			e->x_speed = e->direction ? 0x200 : -0x200;
			e->state = 1;
		case 1:
			if(e->state_time == 0) {
				e->state = 10;
			} else {
				e->state_time--;
				break;
			}
		case 10:
			if(++e->state_time > 10) {
				e->state_time = 0;
				e->state = 11;
			}
		break;
		
		case 11:
			if(++e->state_time == 12) {
				e->x_speed += e->direction ? 0x100 : -0x100;
				e->y_speed -= 0x200;
			} else if(e->state_time > 16) {
				e->state = 12;
			}
		break;
		
		case 12:
			e->state_time++;
			if(e->y > e->y_mark && e->state_time > 10) {
				e->state_time = 0;
				e->state = 10;
			}
		break;
	}
	e->direction = (e->x < e->x_mark);
	if(e->y <= e->y_mark) e->y_speed += 0x20;
	LIMIT_X(0x100);
	LIMIT_Y(0x200);
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(collide_stage_leftwall(e)) e->direction = 1;
	if(collide_stage_rightwall(e)) e->direction = 0;
	if(collide_stage_floor(e)) e->y_speed = -0x200;
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_kulala_onUpdate(Entity *e) {
	switch(e->state) {
		case 0:		// frozen/in stasis. waiting for player to shoot.
			SPR_SAFEANIM(e->sprite, 4);
			if(e->damage_time) {
				camera_shake(30);
				e->state = 10;
				SPR_SAFEANIM(e->sprite, 0);
				e->state_time = 0;
			}
		break;
		
		case 10:	// falling
			//e->eflags |= FLAG_SHOOTABLE;
			e->eflags &= ~NPC_INVINCIBLE;
			
			if(++e->state_time > 40) {
				e->state_time = 0;
				e->state = 11;
			}
		break;
		
		case 11:	// animate thrust
			e->state_time++;
			if(e->state_time % 5 == 0) {
				u8 frame = e->sprite->animInd;
				SPR_SAFEANIM(e->sprite, ++frame);
				if(frame >= 3) {
					e->state = 12;
					e->state_time = 0;
				}
			}
		break;
		
		case 12:	// thrusting upwards
			e->y_speed = -0x155;
			if(++e->state_time > 20) {
				e->state = 10;
				SPR_SAFEANIM(e->sprite, 0);
				e->state_time = 0;
			}
		break;
		
		case 20:	// shot/freeze over/go invulnerable
			SPR_SAFEANIM(e->sprite, 4);
			e->x_speed >>= 1;
			e->y_speed += 0x20;
			
			if(!e->damage_time) {
				e->state = 10;
				SPR_SAFEANIM(e->sprite, 0);
				e->state_time = 30;
			}
		break;
	}
	
	if(e->damage_time) {
		// x_mark unused so use it as a second timer
		if(++e->x_mark > 12) {
			e->state = 20;
			SPR_SAFEANIM(e->sprite, 4);
			//e->eflags &= ~NPC_SHOOTABLE;
			e->eflags |= NPC_INVINCIBLE;
		}
	} else {
		e->x_mark = 0;
	}
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	if(e->state >= 10) {
		e->y_speed += 0x10;
		if(collide_stage_floor(e)) e->y_speed = -0x300;
		
		// Unused y_mark for third timer
		if(collide_stage_leftwall(e)) { e->y_mark = 50; e->direction = 1; }
		if(collide_stage_rightwall(e)) { e->y_mark = 50; e->direction = 0; }
		
		if(e->y_mark > 0) {
			e->y_mark--;
			e->x_speed += e->direction ? 0x80 : -0x80;
		} else {
			e->y_mark = 50;
			FACE_PLAYER(e);
		}
	}
	
	LIMIT_X(0x100);
	LIMIT_Y(0x300);
	
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_mannan_onUpdate(Entity *e) {
	if(e->state < 3 && e->health < 90) {
		sound_play(e->deathSound, 5);
		effect_create_smoke(0, sub_to_pixel(e->x), sub_to_pixel(e->y));
		entity_drop_powerup(e);
		// Face sprite remains after defeated
		e->eflags &= ~NPC_SHOOTABLE;
		e->nflags &= ~NPC_SHOOTABLE;
		SPR_SAFEANIM(e->sprite, 2);
		e->attack = 0;
		e->state = 3;
		return;
	} else if(e->state == 1 && ++e->state_time > 24) {
		ENTITY_SET_STATE(e, 0, 0);
	}
}

void ai_mannan_onState(Entity *e) {
	switch(e->state) {
		case 0:
		SPR_SAFEANIM(e->sprite, 0);
		break;
		case 1: {
			SPR_SAFEANIM(e->sprite, 1);
			Entity *shot = entity_create(sub_to_block(e->x), sub_to_block(e->y), 
				0, 0, 0x67, 0, e->direction);
			shot->direction = e->direction;
			// We want the bullet to delete itself offscreen, it can't do this while inactive
			shot->alwaysActive = true;
		}
		break;
	}
}

void ai_mannan_onHurt(Entity *e) {
	// Fire projectile
	if(e->state == 0) ENTITY_SET_STATE(e, 1, 0);
}

void ai_mannanShot_onUpdate(Entity *e) {
	e->x_speed += e->direction ? 0x1D : -0x1D;
	if((e->state_time % 8) == 1) {
		sound_play(SND_IRONH_SHOT_FLY, 2);
	}
	if(++e->state_time > 120) e->state = STATE_DELETE;
	e->x += e->x_speed;
}

void ai_malco_onUpdate(Entity *e) {
	switch(e->state) {
		case 10:
		if(++e->state_time < 100) {
			if((e->state_time % 4) == 0) {
				sound_play(SND_COMPUTER_BEEP, 5);
			}
		} else if(e->state_time > 150) {
			ENTITY_SET_STATE(e, 15, 0);
		}
		break;
		case 15:		// shaking
		if(e->state_time % 4 == 0) {
			e->x += sub_to_pixel(1);
			sound_play(SND_DOOR, 5);
		} else if(e->state_time % 4 == 2) {
			e->x -= sub_to_pixel(1);
		}
		if(++e->state_time > 50) ENTITY_SET_STATE(e, 16, 0);
		break;
		case 16:		// stand up
		if(++e->state_time > 150) ENTITY_SET_STATE(e, 18, 0);
		break;
		case 18:		// gawking/bobbing up and down
		if(++e->state_time % 8 == 0) sound_play(SND_DOOR, 5);
		if(e->state_time > 100) ENTITY_SET_STATE(e, 20, 0);
		break;
	}
}

void ai_malco_onState(Entity *e) {
	switch(e->state) {
		case 0:
		SPR_SAFEANIM(e->sprite, 0);
		break;
		case 10:
		effect_create_smoke(0, sub_to_pixel(e->x), sub_to_pixel(e->y));
		break;
		case 16:
		SPR_SAFEANIM(e->sprite, 2);
		sound_play(SND_BLOCK_DESTROY, 5);
		effect_create_smoke(0, sub_to_pixel(e->x), sub_to_pixel(e->y));
		break;
		case 18:
		SPR_SAFEANIM(e->sprite, 3);
		break;
		case 20: 
		SPR_SAFEANIM(e->sprite, 4);
		effect_create_smoke(0, sub_to_pixel(e->x), sub_to_pixel(e->y));
		break;
		case 21:	// got smushed!
		SPR_SAFEANIM(e->sprite, 5);
		sound_play(SND_ENEMY_HURT, 5);
		break;
	}
}

void ai_malcoBroken_onCreate(Entity *e) {
	e->spriteAnim = 6;
}

void ai_malcoBroken_onState(Entity *e) {
	switch(e->state) {
		case 0:
		SPR_SAFEANIM(e->sprite, 0);
		FACE_PLAYER(e);
		break;
		case 10:	// set when pulled out of ground
		sound_play(SND_BLOCK_DESTROY, 5);
		effect_create_smoke(0, sub_to_pixel(e->x), sub_to_pixel(e->y));
		e->state = 0;
		break;
	}
}

void ai_powerc_onCreate(Entity *e) {
	e->y += pixel_to_sub(8);
}

void ai_press_onUpdate(Entity *e) {
	switch(e->state) {
		case 0:
			e->x_next = e->x;
			e->y_next = e->y + 0x200;
			e->grounded = collide_stage_floor(e);
			if(!e->grounded) {
				e->state = 10;
				e->state_time = 0;
				SPR_SAFEFRAME(e->sprite, 1);
			}
		break;
		case 10:		// fall
			e->state_time++;
			if(e->state_time == 4) {
				SPR_SAFEFRAME(e->sprite, 2);
			}
			e->y_speed += 0x20;
			if(e->y_speed > 0x5FF) e->y_speed = 0x5FF;
			e->y_next = e->y + e->y_speed;
			if(e->y < player.y) {
				e->eflags &= ~NPC_SOLID;
				e->attack = 127;
			} else {
				e->eflags |= NPC_SOLID;
				e->attack = 0;
			}
			e->grounded = collide_stage_floor(e);
			if(e->grounded) {
				//SmokeSide(o, 4, DOWN);
				camera_shake(10);
				e->state = 11;
				SPR_SAFEFRAME(e->sprite, 0);
				e->attack = 0;
				e->eflags |= NPC_SOLID;
			}
			e->y = e->y_next;
		break;
	}
}

void ai_frog_onUpdate(Entity *e) {
	if(!e->grounded) e->y_speed += 0x80;

	if(e->y_speed > 0x5ff) e->y_speed = 0x5ff;
	if(e->y_speed < -0x5ff) e->y_speed = -0x5ff;

	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;

	switch(e->state) {
		case 0:
			e->state_time = 0;
			e->x_speed = 0;
			e->y_speed = 0;

			// Balfrog sets OPTION1
			if(e->eflags & NPC_OPTION1) {
				e->direction = random() & 1;
				e->eflags |= NPC_IGNORESOLID;

				e->state = 3;
				SPR_SAFEANIM(e->sprite, 1);
			} else {
				e->grounded = true;
				e->eflags &= ~NPC_IGNORESOLID;
				e->state = 1;
			}
		case 1:		// standing
		case 2:
			e->state_time++;
		break;
		case 3:		// falling out of ceiling during balfrog fight
			if(++e->state_time > 40) {
				e->eflags &= ~NPC_IGNORESOLID;

				if((e->grounded = collide_stage_floor(e))) {
					e->state = 0;
					SPR_SAFEANIM(e->sprite, 0);
					e->state_time = 0;
				}
			}
		break;

		case 10:	// jumping
		case 11:
			if (e->x_speed < 0 && collide_stage_leftwall(e)) {
				e->direction = 1;
				e->x_speed = -e->x_speed;
			}
			if (e->x_speed > 0 && collide_stage_rightwall(e)) {
				e->direction = 0;
				e->x_speed = -e->x_speed;
			}
			if (e->y_speed >= 0 && (e->grounded = collide_stage_floor(e))) {
				e->state = 0;
				SPR_SAFEANIM(e->sprite, 0);
				e->state_time = 0;
			}
		break;
	}

	// random jumping, and jump when shot
	if (e->state < 3 && e->state_time > 10) {
		bool dojump = false;

		if(e->damage_time) {
			dojump = true;
		} else if(PLAYER_DIST_X(0x14000) && PLAYER_DIST_Y(0x8000)) {
			if((random() % 50) == 0) {
				dojump = true;
			}
		}

		if (dojump) {
			FACE_PLAYER(e);
			SPR_SAFEHFLIP(e->sprite, e->direction);
			e->state = 10;
			SPR_SAFEANIM(e->sprite, 1);
			e->y_speed = -0x5ff;
			e->grounded = false;

			// no jumping sound in cutscenes at ending
			//if (!player->inputs_locked && !player->disabled)
			//	sound(SND_ENEMY_JUMP);

			e->x_speed = e->direction ? 0x200 : -0x200;
		}
	}

	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_hey_onUpdate(Entity *e) {
	switch(e->state) {
	case 0:
		e->y -= 16;
		e->x += 8;
		e->state = 1;
		e->state_time = 0;
	case 1:
		if(++e->state_time >= 60) {
			e->y += 8;
			e->x += 8;
			e->state = 2;
			e->state_time = 0;
		}
	break;
	case 2:
		if(++e->state_time >= 60) {
			e->y -= 8;
			e->x += 8;
			e->state = 1;
			e->state_time = 0;
		}
	break;
	}
}

void ai_motorbike_onUpdate(Entity *e) {

	switch(e->state) {
		case 0:		// parked
		break;
		
		case 10:	// kazuma and booster mounted
			e->alwaysActive = true;
			e->y -= 0x400;
			//SPR_SAFEADD(e->sprite, SPR_Buggy3, 0, 0, TILE_ATTR(PAL3, 0, 0, e->direction), 5);
			e->state++;
		break;
		
		case 20:	// kazuma and booster start the engine
			e->state = 21;
			e->state_time = 1;
			e->x_mark = e->x;
			e->y_mark = e->y;
		case 21:
			e->x = e->x_mark + 0x200 - (random() % 0x400);
			e->y = e->y_mark + 0x200 - (random() % 0x400);
			if(++e->state_time > 30) {
				e->state = 30;
			}
		break;
		
		case 30:	// kazuma and booster take off
			e->state = 31;
			e->state_time = 1;
			e->x_speed = -0x800;
			e->y_mark = e->y;
			sound_play(SND_MISSILE_HIT, 5);
		case 31:
			e->x_speed += 0x20;
			e->state_time++;
			e->y = e->y_mark + 0x200 - (random() % 0x400);
			if (e->state_time > 10)  e->direction = 1;
			if (e->state_time > 200) e->state = 40;
		break;
		
		case 40:		// flying away (fast out-of-control)
			e->state = 41;
			e->state_time = 2;
			e->direction = 0;
			e->y -= pixel_to_sub(48);		// move up...
			e->x_speed = -0x1000;		// ...and fly fast
		case 41:
			e->state_time += 2;	// makes exhaust sound go faster
			if(e->state_time > 1200) e->state = STATE_DELETE;
		break;
	}
	
	if(e->state >= 20 && (e->state_time & 3) == 0) {
		sound_play(SND_FIREBALL, 5);
		// make exhaust puffs, and make them go out horizontal
		// instead of straight up as this effect usually does
		//Caret *puff = effect(o->ActionPointX(), o->ActionPointY(), EFFECT_SMOKETRAIL_SLOW);
		//puff->yinertia = 0;
		//puff->xinertia = (o->dir == LEFT) ? 0x280 : -0x280;
	}
}
