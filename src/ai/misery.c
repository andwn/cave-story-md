#include "ai_common.h"

void ai_misery_float(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->x_next += (1 << CSF);	// check Undead Core intro to prove this is correct
			e->x_mark = e->x_next;
			e->y_mark = e->y_next;
			e->frame = 0;
			e->timer = 0;
		} /* fallthrough */
		case 1:
		{
			//if (DoTeleportIn(o, 1)) {
				e->state = 10;
			//}
		}
		break;
		case 10:	// floating
		{
			e->state = 11;
			e->timer = 0;
			e->y_speed = SPEED(1 << CSF);
		} /* fallthrough */
		case 11:
		{
			e->frame = 0;
			RANDBLINK(e, 1, 200);
			if (e->y_next > e->y_mark) e->y_speed -= SPEED_8(16);
			if (e->y_next < e->y_mark) e->y_speed += SPEED_8(16);
			if (e->y_speed > SPEED_8(0xFF)) e->y_speed = SPEED_8(0xFF);
			if (e->y_speed < -SPEED_8(0xFF)) e->y_speed = -SPEED_8(0xFF);
		}
		break;
		case 13:	// fall from floaty
			e->frame = 2;
			e->y_speed += SPEED_8(0x40);
			LIMIT_Y(SPEED_12(0x5FF));
			
			if ((e->grounded = collide_stage_floor(e))) {
				sound_play(SND_THUD, 5);
				e->state = 14;
			}
		break;
		case 14: break;			// standing
		// spawn the bubble which picks up Toroko in Shack
		case 15:
		{
			e->frame = 4;
			e->timer = 0;
			e->state = 16;
		} /* fallthrough */
		case 16:
		{
			if (++e->timer >= TIME_8(20)) {
				sound_play(SND_BUBBLE, 5);
				entity_create(e->x, e->y - (16<<CSF), OBJ_MISERYS_BUBBLE, 0);
				e->state = 17;
				e->timer = 0;
			}
		}
		break;
		case 17:
		{
			if (++e->timer >= TIME(50)) e->state = 14;
		} /* fallthrough */
		case 20:	// she flys away
		{
			e->state++;
			e->frame = 0;
			e->y_speed = 0;
			e->timer = (stageID == STAGE_MIMIGA_SHACK) ? TIME_8(10) : 0;
		} /* fallthrough */
		case 21:
		{
			if(e->timer) {
				e->timer--;
				break;
			}
			e->y_speed -= SPEED_8(0x20);
			//if(e->y_speed < -SPEED_10(0x3FF)) e->y_speed = -SPEED_10(0x3FF);
			if (e->y < -0x1000) e->state = STATE_DELETE;
		}
		break;
		case 25:	// big spell
		{
			e->state = 26;
			e->timer = 0;
			e->frame = 4;
		} /* fallthrough */
		case 26:	// she flashes, then a clap of thunder
		{
			ANIMATE(e, 2, 4,5);
			if (++e->timer >= TIME_8(20)) {
				entity_create(e->x, e->y, OBJ_LIGHTNING, 0);
				sound_play(SND_LIGHTNING_STRIKE, 5);
				SCREEN_FLASH(3);
				e->state = 27;
				e->timer = 0;
			}
		}
		break;
		case 27:	// return to standing after lightning strike
		{
			if (++e->timer > TIME_8(16)) e->state = 14;
		}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_misery_stand(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		case 0:
		{
			e->state = 1;
		} /* fallthrough */
		case 1:
		{
			e->frame = 2;
			RANDBLINK(e, 3, 200);
		}
		break;
		case 20:	// she flys away
		{
			e->state++;
			e->frame = 0;
			e->y_speed = 0;
			e->timer = (stageID == STAGE_MIMIGA_SHACK) ? TIME_8(10) : 0;
		} /* fallthrough */
		case 21:
		{
			if(e->timer) {
				e->timer--;
				break;
			}
			e->y_speed -= SPEED_8(0x20);
			//if(e->y_speed < -SPEED_10(0x3FF)) e->y_speed = -SPEED_10(0x3FF);
			if (e->y < -0x1000) e->state = STATE_DELETE;
		}
		break;
		// big spell
		// she flashes, then a clap of thunder,
		// and she teleports away.
		case 25:
		{
			e->state = 26;
			e->timer = 0;
			e->frame = 5;
			e->animtime = 0;
		} /* fallthrough */
		case 26:
		{
			ANIMATE(e, 2, 4, 5);
			if (++e->timer >= TIME_8(20)) {
				entity_create(e->x, e->y, OBJ_LIGHTNING, 0);
				sound_play(SND_LIGHTNING_STRIKE, 5);
				SCREEN_FLASH(3);
				e->state = 27;
				e->timer = 0;
			}
		}
		break;
		case 27:
		{
			if (++e->timer > TIME_8(50)) {	
				// return to standing
				e->state = 0;
			}
		}
		break;
		case 30:	// she throws up her staff like she's summoning something
		{
			e->timer = 0;
			e->state++;
			e->frame = 2;
		} /* fallthrough */
		case 31:
		{
			if (e->timer == TIME_8(10)) e->frame = 4;
			if (e->timer == TIME_8(130)) e->state = 1;
			e->timer++;
		}
		break;
		// fire at DOCTOR_GHOST
		case 40:
		{
			e->state = 41;
			e->timer = 0;
			e->frame = 4;
		} /* fallthrough */
		case 41:
		{
			e->timer++;
			if (e->timer == TIME_8(30) || e->timer == TIME_8(40) || e->timer == TIME_8(50)) {
				Entity *shot = entity_create(e->x+pixel_to_sub(16), e->y, OBJ_IGOR_SHOT, 0);
				shot->x_speed = SPEED_12(0x600);
				shot->y_speed = -SPEED_10(rand() & 0x1FF);
				
				sound_play(SND_SNAKE_FIRE, 5);
			}
			
			if (e->timer > TIME_8(50)) e->state = 0;
		}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_misery_bubble(Entity *e) {
	// find the Toroko object we are to home in on
	Entity *target = entity_find_by_type(0x3C);
	if(!target) {
		e->state = STATE_DELETE;
		return;
	} else if(e->y < 0) {
		e->state = STATE_DELETE;
		entity_delete(target);
		return;
	}
	switch(e->state) {
		case 0:
		{
			// Wait a bit
			if(++e->timer > TIME_8(20)) e->state = 1;
		}
		break;
		case 1:
		{
			// Calculate the speed it will take to reach the target in 1 second
			e->x_speed = (target->x - e->x) / TIME_8(50);
			e->y_speed = (target->y - e->y) / TIME_8(50);
			if(e->x > target->x) e->x_speed = -e->x_speed;
			if(e->y > target->y) e->y_speed = -e->y_speed;
			e->state = 2;
			e->timer = 0;
		} /* fallthrough */
		case 2:
		{
			e->x += e->x_speed;
			e->y += e->y_speed;
			// Did we reach the target?
			if(++e->timer == TIME_8(50)) {
				sound_play(SND_BUBBLE, 5);
				e->state = 3;
				e->x = target->x;
				e->y = target->y;
				e->x_speed = 0;
				e->y_speed = 0;
				target->state = 500;
			}
		}
		break;
		case 3: // Carry Toroko away
		{
			e->y_speed -= SPEED_8(0x1C);
			e->y += e->y_speed;
			target->x = e->x + 0x200;
			target->y = e->y - 0x200;
		}
		break;
	}
}

#define STATE_FIGHTING			100		// script-triggered, must be constant

#define STATE_FLASH_FOR_SPELL	200		// flashes then casts either FIRE_SHOTS or SUMMON_BLOCk
#define STATE_FIRE_SHOTS		210		// fires black shots at player
#define STATE_SUMMON_BLOCK		220		// summons falling block over player's head
#define STATE_SUMMON_BALLS		230		// summons black-lightning balls
#define STATE_TP_AWAY			240		// teleports away then reappears

#define STATE_DEFEATED			1000	// script-triggered, must be constant

#define savedhp		curly_target_time
#define timer3		curly_target_x

static Entity *CreateRing(Entity *e, uint8_t angle) {
	Entity *ring = entity_create(0, 0, OBJ_MISERY_RING, 0);
	ring->jump_time = angle;
	ring->linkedEntity = e;
	
	return ring;
}

void ai_boss_misery(Entity *e) {
	switch(e->state) {
		// Intro states
		case 0:
		{
			// her initial target height when fight begins
			e->y_mark = pixel_to_sub(64);
			
			e->state = 1;
		} /* fallthrough */
		case 1:
		{
			e->frame = 0;
			RANDBLINK(e, 1, 200);
		}
		break;
		
		case 20:		// fall from throne (script-triggered)
		{
			if (blk(e->x, 0, e->y, 8) == 0x41) {
				e->state = 21;
				e->frame = 2;
				e->y_speed = 0;
			} else {
				e->y_speed += SPEED_8(0x40);
			}
		}
		break;
		case 21:		// standing/talking after fall from throne
		{
			e->frame = 2;
			RANDBLINK(e, 3, 200);
		}
		break;

		// fight begin and default/base state
		case STATE_FIGHTING:
		{
			e->flags |= NPC_SHOOTABLE;
			savedhp = e->health;
			
			e->timer = 0;
			e->frame = 0;
			e->x_speed = 0;
			e->state++;
		} /* fallthrough */
		case STATE_FIGHTING+1:
		{
			FACE_PLAYER(e);
			
			e->y_speed += (e->y < e->y_mark) ? SPEED_8(0x20) : -SPEED_8(0x20);
			LIMIT_Y(SPEED_10(0x200));
			
			if (++e->timer > TIME_8(200) || (savedhp - e->health) >= 80) {
				e->state = STATE_FLASH_FOR_SPELL;
				e->timer = 0;
			}
		}
		break;

		// flashes for spell...
		// then either fires shots or casts the falling-block spell
		case STATE_FLASH_FOR_SPELL:
		{
			e->flags &= ~NPC_SHOOTABLE;
			e->x_speed = 0;
			e->y_speed = 0;
			
			e->timer = 0;
			e->state++;
		} /* fallthrough */
		case STATE_FLASH_FOR_SPELL+1:
		{
			e->timer++;
			e->frame = 5 + (e->timer & 1);
			
			if (e->timer > TIME_8(30)) {
				e->timer = 0;
				e->frame = 4;
				
				if (++e->timer2 >= 3) {
					e->state = STATE_SUMMON_BLOCK;
					e->timer2 = 0;
				} else {
					e->state = STATE_FIRE_SHOTS;
				}
			}
		}
		break;
		// fire black shots at player
		case STATE_FIRE_SHOTS:
		{
			if ((++e->timer & 7) == 0) {
				uint8_t angle = get_angle(e->x, e->y, player.x, player.y);
				Entity *shot = entity_create(e->x, e->y, OBJ_MISERY_SHOT, 0);
				angle -= 3;
				angle += rand() & 7;
				shot->x_speed = ((int32_t)cos[angle] * (int32_t)SPEED_12(0x800)) >> CSF;
				shot->y_speed = ((int32_t)sin[angle] * (int32_t)SPEED_12(0x800)) >> CSF;
				sound_play(SND_FIREBALL, 3);
			}
			
			if (e->timer > TIME_8(40)) {
				e->timer = 0;
				e->state = STATE_TP_AWAY;
			}
		}
		break;
		// summon falling block
		case STATE_SUMMON_BLOCK:
		{
			if (++e->timer == TIME_8(10)) {
				entity_create(player.x - pixel_to_sub(8), player.y - pixel_to_sub(64), 
						OBJ_FALLING_BLOCK, NPC_OPTION1);
			}
			
			if (e->timer > TIME_8(30)) {
				e->state = STATE_TP_AWAY;
				e->timer = 0;
			}
		}
		break;
		// summon black balls
		case STATE_SUMMON_BALLS:
		{
			FACE_PLAYER(e);
			e->frame = 4;
			
			e->timer = 0;
			timer3 = 0;
			e->state++;
		} /* fallthrough */
		case STATE_SUMMON_BALLS+1:
		{
			e->y_speed += (e->y < e->y_mark) ? SPEED_8(0x20) : -SPEED_8(0x20);
			LIMIT_Y(SPEED_10(0x200));
			
			if(++timer3 > TIME_8(24)) {
				timer3 = 0;
				entity_create(e->x, e->y+(4<<CSF), OBJ_MISERY_BALL, 0);
				sound_play(SND_FIREBALL, 3);
			}
			
			if(++e->timer > TIME_8(72)) {
				e->state = 100;
				e->timer = 0;
			}
		}
		break;

		// teleport away, then reappear someplace else
		case STATE_TP_AWAY:
		{
			e->state++;
			e->timer = 0;
			e->flags &= ~NPC_SHOOTABLE;
			e->frame = 8;
			sound_play(SND_TELEPORT, 5);
		} /* fallthrough */
		case STATE_TP_AWAY+1:
		{
			ANIMATE(e, 2, 8,9);
			e->timer++;
			if (e->timer == TIME_8(30)) {
				static const uint8_t xpos[32] = {
					 0, 1, 2, 2, 3, 4, 5, 5, 6, 7, 8, 8, 9,10,11,11,
					12,13,13,14,15,15,16,17,17,18,19,19,20,21,21,22
				};
				e->x_mark = e->x = block_to_sub(9 + xpos[rand() & 31]);
				e->y_mark = e->y = tile_to_sub(9 + (rand() & 3));
			} else if (e->timer == TIME_8(50)) {
				// switch back to showing real misery instead of the phase-in effect
				e->flags |= NPC_SHOOTABLE;
				e->frame = 0;
				e->dir = 0;
				// spawn rings
				if (e->health < 340) {
					CreateRing(e, A_RIGHT);
					CreateRing(e, A_LEFT);
					
					if (e->health < 180) {
						CreateRing(e, A_UP);
						CreateRing(e, A_DOWN);
					}
				}
				// after tp we can summon the black balls if the player
				// is far enough away from us that they won't immediately trigger
				if (abs(player.x - e->x) > pixel_to_sub(112)) {
					e->state = STATE_SUMMON_BALLS;
				} else {
					e->state = STATE_FIGHTING;
				}
				// setup sinusoidal hover, both of those possible states
				// are in-air states that do it.
				e->timer = 0;
				e->y_speed = -SPEED_10(0x200);
			}
		}
		break;

		// defeated! "gaah" in air
		case 1000:
		{
			e->flags &= ~NPC_SHOOTABLE;
			entities_clear_by_type(OBJ_MISERY_RING);
			//SmokeClouds(o, 3, 2, 2);
			
			e->x_speed = 0;
			e->y_speed = 0;
			
			e->state = 1001;
			e->timer = 0;
			e->frame = 4;
			e->x_mark = e->x;
		} /* fallthrough */
		case 1001:		// shake until script tells us otherwise
		{
			e->x = e->x_mark;
			if (++e->timer & 2)
				e->x += (1 << CSF);
		}
		break;
		
		case 1010:		// fall to ground and do defeated frame: "ergh"
		{
			e->y_speed += SPEED_8(10);
			if (blk(e->x, 0, e->y, 8) == 0x41) {
				e->frame = 7;
				e->state = 1011;
				e->y_speed = 0;
			}
		}
		break;
	}
	
	LIMIT_X(SPEED_10(0x200));
	LIMIT_Y(SPEED_10(0x3FF));
	
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ondeath_boss_misery(Entity *e) {
	e->flags &= ~NPC_SHOOTABLE;
	e->attack = 0;
	tsc_call_event(e->event);
}

void ai_misery_ring(Entity *e) {
	if (!e->linkedEntity) {
		effect_create_smoke(e->x >> CSF, e->y >> CSF);
		e->state = STATE_DELETE;
		return;
	}
	
	switch(e->state) {
		case 0:
		{
			e->frame = 3;
			e->state = 1;
			e->timer = 0;
			e->flags |= (NPC_SHOOTABLE | NPC_INVINCIBLE); // Rings block bullets
		} /* fallthrough */
		case 1:
		{
			// angle
			e->jump_time += 2;
			// distance from misery
			if (e->timer < 0x40) {
				e->timer++;
				e->x = e->linkedEntity->x + (int32_t)cos2[e->jump_time] * (e->timer >> 1);
				e->y = e->linkedEntity->y + (int32_t)sin2[e->jump_time] * (e->timer >> 1);
			} else {
				e->x = e->linkedEntity->x + (cos2[e->jump_time] << 5);
				e->y = e->linkedEntity->y + (sin2[e->jump_time] << 5);
			}
			
			// turn to bats when misery teleports
			if (e->linkedEntity->state >= STATE_TP_AWAY &&
				e->linkedEntity->state < STATE_TP_AWAY+10) {
				e->state = 10;
			}
		}
		break;
		
		case 10:	// transform to bat
		{
			//e->eflags &= ~NPC_INVINCIBLE;
			e->flags &= ~NPC_INVINCIBLE; // ugh
			
			THROW_AT_TARGET(e, player.x, player.y, SPEED(0x280));
			FACE_PLAYER(e);
			
			// Change to bat sprite
			SHEET_FIND(e->sheet, SHEET_BAT);
			e->vramindex = sheets[e->sheet].index;
			e->frame = 0;
			e->oframe = 255;
			
			e->state = 11;
		} /* fallthrough */
		case 11:
		{
			ANIMATE(e, 4, 0,1,2);
			e->x += e->x_speed;
			e->y += e->y_speed;
			// Disappear when touching with any solid blocks
			if (blk(e->x, 0, e->y, 0) == 0x41) {
				effect_create_smoke(e->x >> CSF, e->y >> CSF);
				e->state = STATE_DELETE;
			}
		}
		break;
	}
}

void ai_misery_ball(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->y_mark = e->y;
			e->x_speed = 0;
			e->y_speed = -0x200;
		} /* fallthrough */
		case 1:
		{
			ANIMATE(e, 2, 0,1);
			
			e->x_speed += (e->x < player.x) ? 0x10 : -0x10;
			e->y_speed += (e->y < e->y_mark) ? 0x20 : -0x20;
			LIMIT_X(0x200);
			LIMIT_Y(0x200);
			
			if (PLAYER_DIST_X(e, 8<<CSF) && player.y > e->y) {
				e->state = 10;
				e->timer = 0;
			} else if(++e->timer > TIME_10(750)) {
				effect_create_smoke(e->x >> CSF, e->y >> CSF);
				e->state = STATE_DELETE;
			}
		}
		break;
		
		case 10:	// black lightning
		{
			if (++e->timer > 10) {
				sound_play(SND_LIGHTNING_STRIKE, 5);
				entity_create(e->x, e->y, OBJ_BLACK_LIGHTNING, 0);
				e->state = STATE_DELETE;
			}
			
			e->frame = (e->timer & 2) ? 2 : 1;
		}
		break;
	}
	
	e->x += e->x_speed;
	e->y += e->y_speed;
}

void ai_black_lightning(Entity *e) {
	ANIMATE(e, 2, 0,1);
	if (blk(e->x, 0, e->y, 15) == 0x41) {
		//effect(e->CenterX(), e->Bottom(), EFFECT_BOOMFLASH);
		effect_create_smoke(e->x >> CSF, e->y >> CSF);
		e->state = STATE_DELETE;
	}
	e->y += SPEED_12(0xFFF);
}
