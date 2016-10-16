#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "effect.h"
#include "camera.h"

void onspawn_sunstone(Entity *e) {
	e->x += pixel_to_sub(8);
	e->y += pixel_to_sub(8);
}

void onspawn_puppy(Entity *e) {
	e->eflags |= NPC_INTERACTIVE; // Yeah..
}

void ai_puppy(Entity *e) {
	
}

void onspawn_puppyCarry(Entity *e) {
	e->alwaysActive = TRUE;
	// One's all you can manage. One's all you can manage. One's all you can manage.
	e->eflags &= ~NPC_INTERACTIVE;
	e->nflags &= ~NPC_INTERACTIVE;
}

void ai_puppyCarry(Entity *e) {
	e->dir = player.dir;
	e->x = player.x + pixel_to_sub(e->dir ? -4 : 4);
	e->y = player.y - pixel_to_sub(5);
}

void onspawn_jenka(Entity *e) {
	if(e->type == OBJ_JENKA_COLLAPSED) {
		e->frame = 2;;
	}
}

void ai_polish(Entity *e) {
	#define POLISH_ACCEL	0x20
	#define POLISH_SPEED	0x200
	#define POLISH_BOUNCE	0x100
	
	#define POLISH_CCW_LEFT		1
	#define POLISH_CCW_UP		2
	#define POLISH_CCW_RIGHT	3
	#define POLISH_CCW_DOWN		4
	
	#define POLISH_CW_LEFT		5
	#define POLISH_CW_UP		6
	#define POLISH_CW_RIGHT		7
	#define POLISH_CW_DOWN		8
	
	// Split after 20 damage
	if(e->health <= 100) {
		entity_create(e->x - (8<<CSF), e->y, OBJ_POLISHBABY, 0)->dir = 0;
		entity_create(e->x + (8<<CSF), e->y, OBJ_POLISHBABY, 0)->dir = 1;
		e->state = STATE_DELETE;
		effect_create_smoke(sub_to_pixel(e->x), sub_to_pixel(e->y));
		sound_play(e->deathSound, 5);
		return;
	}
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	u8 blockl = collide_stage_leftwall(e);
	u8 blockr = collide_stage_rightwall(e);
	u8 blocku = collide_stage_ceiling(e);
	u8 blockd = collide_stage_floor(e);

	switch(e->state) {
		case 0:		// initilization
			if(e->eflags & NPC_OPTION2) {
				e->state = POLISH_CW_RIGHT;
			} else {
				e->state = POLISH_CCW_LEFT;
			}
		break;
		
		// -------------- Traveling around counter-clockwise --------------
		
		case POLISH_CCW_LEFT:	// traveling left on ceiling
			e->y_speed -= POLISH_ACCEL;
			if (blocku)
			{
				e->y_speed = POLISH_BOUNCE;
				e->x_speed -= POLISH_BOUNCE;
			}
			
			if (blockl) e->state = POLISH_CCW_DOWN;
		break;
		
		case POLISH_CCW_UP:	// traveling up right wall
		{
			e->x_speed += POLISH_ACCEL;
			if (blockr)
			{
				e->x_speed = -POLISH_BOUNCE;
				e->y_speed -= POLISH_BOUNCE;
			}
			
			if (blocku) e->state = POLISH_CCW_LEFT;
		}
		break;
		
		case POLISH_CCW_RIGHT:	// traveling right on floor
		{
			e->y_speed += POLISH_ACCEL;
			if (blockd)
			{
				e->y_speed = -POLISH_BOUNCE;
				e->x_speed += POLISH_BOUNCE;
			}
			
			if (blockr) e->state = POLISH_CCW_UP;
		}
		break;
		
		case POLISH_CCW_DOWN:	// traveling down left wall
		{
			e->x_speed -= POLISH_ACCEL;
			if (blockl)
			{
				e->x_speed = POLISH_BOUNCE;
				e->y_speed += POLISH_BOUNCE;
			}
			
			if (blockd) e->state = POLISH_CCW_RIGHT;
		}
		break;
		
		// -------------- Traveling around clockwise --------------
		
		case POLISH_CW_LEFT:		// traveling left on floor
		{
			e->y_speed += POLISH_ACCEL;
			if (blockd)
			{
				e->y_speed = -POLISH_BOUNCE;
				e->x_speed -= POLISH_BOUNCE;
			}
			
			if (blockl) e->state = POLISH_CW_UP;
		}
		break;
		
		case POLISH_CW_UP:		// traveling up left wall
		{
			e->x_speed -= POLISH_ACCEL;
			if (blockl)
			{
				e->x_speed = POLISH_BOUNCE;
				e->y_speed -= POLISH_BOUNCE;
			}
			
			if (blocku) e->state = POLISH_CW_RIGHT;
		}
		break;
		
		case POLISH_CW_RIGHT:		// traveling right on ceiling
		{
			e->y_speed -= POLISH_ACCEL;
			if (blocku)
			{
				e->y_speed = POLISH_BOUNCE;
				e->x_speed += POLISH_BOUNCE;
			}
			
			if (blockr) e->state = POLISH_CW_DOWN;
		}
		break;
		
		case POLISH_CW_DOWN:		// traveling down right wall
		{
			e->x_speed += POLISH_ACCEL;
			if (blockr)
			{
				e->x_speed = -POLISH_BOUNCE;
				e->y_speed += POLISH_BOUNCE;
			}
			
			if (blockd) e->state = POLISH_CW_LEFT;
		}
		break;
	}
	
	LIMIT_X(POLISH_SPEED);
	LIMIT_Y(POLISH_SPEED);
	
	e->x = e->x_next;
	e->y = e->y_next;
	
	ANIMATE(e, 4, 0,1);
	e->dir = 0;
}

void ai_baby(Entity *e) {
	if(!e->state) {
		e->state = 1;
		if(random() & 1) {
			e->x_speed = (random() % 0x100) + 0x100;
		} else {
			e->x_speed = (random() % 0x100) - 0x300;
		}
		if(random() & 1) {
			e->y_speed = (random() % 0x100) + 0x100;
		} else {
			e->y_speed = (random() % 0x100) - 0x300;
		}
	}
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	
	// Collide functions set speed to 0. Remember using mark vars
	e->x_mark = e->x_speed;
	e->y_mark = e->y_speed;
	if (e->x_speed > 0 && collide_stage_rightwall(e)) e->x_speed = -e->x_mark;
	if (e->x_speed < 0 && collide_stage_leftwall(e)) e->x_speed = -e->x_mark;
	if (e->y_speed > 0 && collide_stage_floor(e)) e->y_speed = -e->y_mark;
	if (e->y_speed < 0 && collide_stage_ceiling(e)) e->y_speed = -e->y_mark;
	
	e->x = e->x_next;
	e->y = e->y_next;
	
	ANIMATE(e, 4, 0,1);
}

void ai_sandcroc(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->timer = 0;
			e->x_mark = e->x;
			e->y_mark = e->y;
			e->eflags &= ~(NPC_SOLID | NPC_SHOOTABLE | NPC_INVINCIBLE | NPC_IGNORESOLID);
			e->nflags &= ~(NPC_SOLID | NPC_SHOOTABLE | NPC_INVINCIBLE | NPC_IGNORESOLID);
		}
		/* no break */
		case 1:
		{
			// track player invisibly underground
			if(e->type != OBJ_SANDCROC_OSIDE) {
				e->x_speed = (e->x < player.x) ? 0x400:-0x400;
				e->x_next = e->x + e->x_speed;
				e->y_next = e->y - 0x200;
				collide_stage_leftwall(e);
				collide_stage_rightwall(e);
				e->x = e->x_next;
			}
			if(PLAYER_DIST_X(pixel_to_sub(19))) {
				// check if bottoms of player and croc are near
				if(player.y < e->y + 0x200 && sub_to_pixel(player.y) + 
					player.hit_box.bottom + 12 > sub_to_pixel(e->y) + e->hit_box.bottom) {
					// attack!!
					e->x_speed = 0;
					e->state = 2;
					e->timer = 0;
					sound_play(SND_JAWS, 5);
					e->hidden = FALSE;
				}
			} else {
				e->hidden = TRUE;
			}
		}
		break;
		case 2:		// attacking
		{
			e->timer++;
			if(e->timer == 6) {
				e->frame = 1;
			} else if(e->timer == 12) {
				e->attack = (e->type == OBJ_SANDCROC_OSIDE) ? 15 : 10;
				e->frame = 2;
			} else if(e->timer==16) {
				e->eflags |= NPC_SHOOTABLE;
				e->eflags |= NPC_SOLID;
				e->attack = 0;
				e->state = 3;
				e->timer = 0;
				e->frame = 3;
			}
		}
		break;
		case 3:
		{
			e->timer++;
			if(e->damage_time) {
				e->state = 4;
				e->timer = 0;
				e->y_speed = 0;
				e->damage_time += 25;		// delay floattext until after we're underground
			}
		}
		break;
		case 4:		// retreat
		{
			e->y += 0x280;
			e->eflags &= ~(NPC_SOLID);
			
			if (++e->timer == 30) {
				e->eflags &= ~(NPC_SHOOTABLE);
				e->hidden = TRUE;
				e->state = 5;
				e->timer = 0;
			}
		}
		break;
		case 5:
		{
			e->frame = 0;
			e->y = e->y_mark;
			
			if(e->timer < 100) {
				e->timer++;
				// have to wait before moving: till floattext goes away
				// else they can see us jump
				if(e->timer==98) {
					e->x_speed = player.x - e->x;
				} else {
					e->x_speed = 0;
				}
			} else {
				e->state = 0;
			}
		}
		break;
	}
}

void ai_sunstone(Entity *e) {
	switch(e->state) {
		case 0:
		{
			// Don't allow bullets through
			e->eflags |= (NPC_SHOOTABLE | NPC_INVINCIBLE);
			e->state = 1;
		}
		break;
		case 10:	// triggered to move by hvtrigger script
		{
			// Always face left, don't flip the sprite while moving
			MOVE_X(SPEED(0x80));
			e->dir = 0;
			e->timer = 0;
			e->state++;
		}
		/* no break */
		case 11:
		{
			e->x += e->x_speed;
			if((e->timer & 7) == 0) sound_play(SND_QUAKE, 5);
			e->timer++;
			camera_shake(20);
		}
		break;
	}
}

void ai_armadillo(Entity *e) {
	ANIMATE(e, 16, 0,1);
	if(!e->grounded) e->y_speed += SPEED(0x40);
	LIMIT_Y(SPEED(0x5ff));
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		case 0:
		{
			FACE_PLAYER(e);
			e->state = 1;
		}
		/* no break */
		case 1:
		{
			if((!e->dir && collide_stage_leftwall(e)) ||
				(e->dir && collide_stage_rightwall(e))) {
				TURN_AROUND(e);
			}
			MOVE_X(SPEED(0x100));
		}
		break;
	}
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_crow(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		case 0:
		{
			u16 angle = random() % 1024;
			e->x_speed = sintab32[angle] >> 1;
			e->y_speed = sintab32[(angle + 256) % 1024] >> 1;
			e->x_mark = e->x + (e->x_speed * 8);
			e->y_mark = e->y + (e->y_speed * 8);
			e->state = 1;
		}
		/* no break */
		case 1:
		case 101:
		{
			if 		(e->x > e->x_mark) e->x_speed -= SPEED(16);
			else if (e->x < e->x_mark) e->x_speed += SPEED(16);
			if 		(e->y > e->y_mark) e->y_speed -= SPEED(16);
			else if (e->y < e->y_mark) e->y_speed += SPEED(16);
			
			FACE_PLAYER(e);
			LIMIT_X(SPEED(0x200));
			LIMIT_Y(SPEED(0x200));
			ANIMATE(e, 8, 0,1);
			
			if (e->damage_time) {
				e->state++;		// state 2/102
				e->frame = 2;
				e->timer = 0;
				e->y_speed = 0;
			}
		}
		break;
		case 2:
		case 102:
		{
			FACE_PLAYER(e);
			if (e->damage_time) {
				e->frame = 2;
				// fall while hurt
				e->y_speed += SPEED(0x20);
				e->x_speed = 0;
				e->timer = 0;
			} else {
				ANIMATE(e, 8, 0,1);
				// move towards player
				if(e->x < player.x) e->x_speed += SPEED(0x10);
				else e->x_speed -= SPEED(0x10);
				if(e->y < player.y) e->y_speed += SPEED(0x10);
				else e->y_speed -= SPEED(0x10);
				if(!e->timer) e->timer++;
			}
			// bounce off walls
			if (e->x_speed < 0 && collide_stage_leftwall(e)) e->x_speed = SPEED(0x200);
			if (e->x_speed > 0 && collide_stage_rightwall(e)) e->x_speed = SPEED(-0x200);
			if (e->y_speed < 0 && collide_stage_ceiling(e)) e->y_speed = SPEED(0x200);
			if (e->y_speed > 0 && collide_stage_floor(e)) e->y_speed = SPEED(-0x200);
			
			LIMIT_X(SPEED(0x5ff));
			LIMIT_Y(SPEED(0x5ff));
		}
		break;
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_skullhead(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		case 0:
		{
			e->state = 1;
			e->timer = random() % 5;
		}
		/* no break */
		case 1:
		{
			if(++e->timer > 8) {
				e->y_speed = -0x350;
				e->state = 2;
				e->frame = 2;
				MOVE_X(0x100);
			} else {
				break;
			}
		}
		/* no break */
		case 2:
		{
			e->y_speed += 0x40;
			LIMIT_Y(0x5ff);
			if(e->y_speed > 0) {
				if(collide_stage_floor(e)) {
					e->x_speed = 0;
					e->y_speed = 0;
					e->state = 1;
					e->timer = 0;
					e->frame = 0;
				} else {
					e->frame = 1;
				}
			} else {
				collide_stage_ceiling(e);
			}
		}
		break;
	}
	if(e->x_speed) {
		if (collide_stage_leftwall(e)) { e->dir = 1; e->x_speed = 0x100; }
		if (collide_stage_rightwall(e)) { e->dir = 0; e->x_speed = -0x100; }
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_skelShot(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	// bounce off walls
	if((e->x_speed < 0 && collide_stage_leftwall(e)) || 
		(e->x_speed > 0 && collide_stage_rightwall(e))) {
		e->x_speed = -e->x_speed;
		e->timer += 5;
	}
	
	// bounce off ceiling
	if(e->y_speed < 0 && collide_stage_ceiling(e)) {
		e->y_speed = -e->y_speed;
		e->timer += 5;
	}
	
	// if hit floor bounce along it...
	if (collide_stage_floor(e)) {
		e->y_speed = -0x180;
		e->state = 1;	// begin falling
		e->timer += 4;
	}
	
	if(e->state == 1) {
		e->y_speed += 0x10;
		LIMIT_Y(0x5ff);
	}
	
	if(e->timer >= 10) {
		e->state = STATE_DELETE;
	}
	
	e->x = e->x_next;
	e->y = e->y_next;
}

// curly's mimiga's
void ai_curlys_mimigas(Entity *e) {
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	switch(e->state) {
		case 0:		// init/set initial anim state
		{
			if(e->type == OBJ_MIMIGAC1) {
				e->state = 100;
			} else { // OBJ_MIMIGAC2 & OBJ_MIMIGA_ENEMY
				e->state = 2;			// stand and blink
				if(e->type == OBJ_MIMIGAC2) {
					if(e->eflags & NPC_OPTION2) e->state = 110; // sleeping
				}
			}
		}
		break;
		case 2:		// init stand and blink
		{
			e->state = 3;
		}
		/* no break */
		case 3:		// stand and blink
		{
			e->frame = 0;
			RANDBLINK(e, 3, 200);
		}
		break;
		// sitting mimiga (when facing right)
		// facing away mimiga (when facing left)
		case 100:
		{
			e->frame = 4;
		}
		break;
		case 110:	// sleeping facing left mimiga
		{
			e->frame = 7;
			//ai_zzzz_spawner(o);
		}
		break;
		/// ******************** Fighting Mimiga Code ********************
		case 10:
		{
			e->eflags |= NPC_SHOOTABLE;
			e->health = 1000;
			e->state = 11;
			e->timer = random() % 50;
			e->frame = 0;
		}
		/* no break */
		case 11:
			if(e->timer) e->timer--;
			else e->state = 13;
		break;
		case 13:
		{
			e->state = 14;
			e->timer = random() % 50;
			FACE_PLAYER(e);
			e->frame = 1;
		}
		/* no break */
		case 14:
		{
			ANIMATE(e, 8, 1,0,2,0);
			if(e->dir) e->x_speed += 0x40;
			else e->x_speed -= 0x40;
			
			if(e->timer) {
				e->timer--;
			} else {	
				// enter hop state
				e->state = 15;
				e->y_speed = -0x200;
				e->attack = 2;
				e->frame = 2;
			}
		}
		break;
		case 15:	// hopping
		{
			if(e->grounded) {	
				// landed
				e->x_speed = 0;
				e->state = 10;
				e->attack = 0;
			}
		}
		break;
		case 20:	// aiiie! got shot!!
		{
			if(e->grounded) {
				e->x_speed = 0;
				e->state = 21;
				e->frame += 2;
				e->timer = 300 + (random() % 100);
			}
		}
		break;
		case 21:	// lying on ground knocked out
		{
			if (e->timer) {
				e->timer--;
			} else {
				e->eflags |= NPC_SHOOTABLE;
				e->health = 1000;
				e->state = 11;
				e->timer = random() % 50;
				e->frame = 0;
			}
		}
		break;
	}
	if (e->state > 10 && e->state < 20 && e->health != 1000) {
		// got shot by player
		e->state = 20;
		e->y_speed = -0x200;
		e->frame = (random() & 1) + 5;
		e->attack = 0;
		e->eflags &= ~NPC_SHOOTABLE;
	}
	
	if(!e->grounded) e->y_speed += SPEED(0x20);
	LIMIT_Y(SPEED(0x5ff));
	LIMIT_X(SPEED(0x1ff));
	if(e->grounded) e->grounded = collide_stage_floor_grounded(e);
	else e->grounded = collide_stage_floor(e);
	
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_skeleton_shot(Entity *e) {
	ANIMATE(e, 8, 0,1,2,3);
	
	if ((e->x_speed < 0 && blk(e->x, -6, e->y, 0) == 0x41) || 
			(e->x_speed > 0 && blk(e->x, 6, e->y, 0) == 0x41)) {
		e->x_speed = -e->x_speed;
		e->timer += 5;
	}
	
	if ((e->y_speed < 0 && blk(e->x, 0, e->y, -6) == 0x41)) {
		e->y_speed = -e->y_speed;
		e->timer += 5;
	}
	
	// Have to collide with slopes, so use the normal method
	e->x_next = e->x;
	e->y_next = e->y;
	if (collide_stage_floor(e)) {
		e->y_speed = -SPEED(0x180);
		e->state = 1;	// begin falling
		e->timer += 4;
	}
	
	if (e->state == 1) {
		e->y_speed += SPEED(0x10);
		LIMIT_Y(SPEED(0x5ff));
	}
	
	e->x += e->x_speed;
	e->y += e->y_speed;
	
	if (e->timer >= 10) {
		//effect(o->CenterX(), o->CenterY(), EFFECT_FISHY);
		e->state = STATE_DELETE;
	}
}

#define SKNEAR_BELOW	(160<<CSF)
#define SKNEAR_ABOVE	(64<<CSF)

void ai_skeleton(Entity *e) {
	u8 pnear = PLAYER_DIST_Y2(SKNEAR_ABOVE, SKNEAR_BELOW);
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(!e->grounded) e->grounded = collide_stage_floor(e);
	else e->grounded = collide_stage_floor_grounded(e);
	
	switch(e->state) {
		case 0:
		{
			e->frame = e->grounded;
			e->x_speed = 0;
			
			// if player comes near, enter attack state
			// also if he shoots us while we're inactive, don't just stand there
			// like a ninny, jump around!
			if (e->damage_time) e->state = 20;
			if (pnear) e->state = 10;
		}
		break;
		
		case 10:	// prepare for jump
			e->timer = e->timer2 = e->frame = e->x_speed = 0;
			e->state = 11;
		case 11:
			if (++e->timer > 5 && e->grounded) {
				// jump if player near, else go to idle
				e->state = pnear ? 20:0;
			}
		if (e->state != 20) break;
		
		case 20:
		{
			e->state = 21;
			e->frame = 1;
			e->timer2 = 0;
			e->y_speed = -(SPEED(0x200) + (random() % SPEED(0x400)));
			e->grounded = FALSE;
			
			// jump towards player, unless we've been hurt; in that case jump away
			if (!e->damage_time) {
				e->x_speed += (e->x_next > player.x) ? -SPEED(0x100):SPEED(0x100);
			} else {
				e->x_speed += (e->x_next > player.x) ? SPEED(0x100):-SPEED(0x100);
			}
		}
		case 21:
		{
			if (e->y_speed > 0) {
				if (e->timer2 == 0) {
					e->timer2++;
					
					FIRE_ANGLED_SHOT(OBJ_SKELETON_SHOT, e->x, e->y, 
							e->dir ? A_RIGHT+1:A_LEFT-1, 0x300);
					sound_play(SND_EM_FIRE, 3);
				}
				
				
			} else collide_stage_ceiling(e);
			if (e->grounded) {
				e->state = 10;
				e->frame = 0;
			}
		}
		break;
	}
	
	if (e->state >= 10) FACE_PLAYER(e);
	if(!e->grounded) e->y_speed += SPEED(0x33);
	LIMIT_X(SPEED(0x5ff));
	LIMIT_Y(SPEED(0x5ff));
	
	e->x = e->x_next;
	e->y = e->y_next;
}
