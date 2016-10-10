#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "effect.h"
#include "resources.h"
#include "camera.h"

#define CAI_INIT			20			// ANP'd to this by the entry script in Lab M
#define CAI_START			21			// ANP'd to this by Almond script
#define CAI_RUNNING			22
#define CAI_KNOCKEDOUT		40			// knocked out at beginning of Almond battle
#define CAI_ACTIVE			99

enum { STAND, WALK1, WALK2, LOOKUP, UPWALK1, UPWALK2, LOOKDN, JUMPDN };

u8 curly_mgun = 0;
u8 curly_watershield = 0;
u8 curly_impjump = 0;
u8 curly_reachptimer = 0;
u8 curly_blockedtime = 0;
s8 curly_impjumptime = 0;
u8 curly_tryjumptime = 0;
u8 curly_look = 0;

static void CaiJUMP(Entity *e) {
	if (e->grounded) {
		e->y_speed = -SPEED(0x300) - random(SPEED(0x300));
		e->grounded = FALSE;
		e->frame = WALK2;
		sound_play(SND_PLAYER_JUMP, 5);
	}
}

const char porn[1] = {""};

// curly that fights beside you
void ai_curly_ai(Entity *e) {
	s32 xdist, ydist;
	s32 xlimit;
	u8 reached_p;
	u16 otiley;
	u8 seeking_player = 0;
	u8 wantdir;

	// put these here so she'll spawn the shield immediately, even while she's still
	// knocked out. otherwise she wouldn't have it turned on in the cutscene if the
	// player defeats the core before she gets up. I know that's unlikely but still.
	if (!curly_watershield) {
		Entity *shield = entity_create(e->x, e->y, OBJ_CAI_WATERSHIELD, 0);
		shield->alwaysActive = TRUE;
		shield->linkedEntity = e;
		
		//e->BringToFront();				// curly in front of monsters,
		//shield->BringToFront();			// and shield in front of curly
		
		curly_watershield = 1;
	}
	
	switch(e->state) {
		case 0:
		{
			e->alwaysActive = TRUE;
			e->x_speed = 0;
			e->y_speed += SPEED(0x20);
		}
		break;
		case CAI_INIT:			// set to this by an ANP in Maze M
		{
			e->alwaysActive = TRUE;
			e->x = player.x;
			e->y = player.y;
		}
		/* no break */
		case CAI_START:			// set here after she stops being knocked out in Almond
		{
			e->alwaysActive = TRUE;
			e->x_mark = e->x;
			e->y_mark = e->y;
			e->dir = player.dir;
			e->state = CAI_ACTIVE;
			e->timer = 0;
			// If we traded Curly for machine gun she uses the polar star
			curly_mgun = !player_has_weapon(WEAPON_MACHINEGUN);
			// spawn her gun
			Entity *gun = entity_create(e->x, e->y, OBJ_CAI_GUN + curly_mgun, 0);
			gun->alwaysActive = TRUE;
			gun->linkedEntity = e;
			//gun->PushBehind(o);
		}
		break;
		case CAI_KNOCKEDOUT:
		{
			e->timer = 0;
			e->state = CAI_KNOCKEDOUT+1;
			e->frame = 9;
		}
		/* no break */
		case CAI_KNOCKEDOUT+1:
		{
			if (++e->timer > TIME(1000)) {	// start fighting
				e->state = CAI_START;
			} else if (e->timer > TIME(750))
			{	// stand up
				e->eflags &= ~NPC_INTERACTIVE;
				e->frame = 0;
			}
		}
		break;
	}
	// Check collision up front and remember the result
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	/* u8 blocku = (unused) */collide_stage_ceiling(e);
	if(e->grounded) e->grounded = collide_stage_floor_grounded(e);
	else e->grounded = collide_stage_floor(e);
	u8 blockl = collide_stage_leftwall(e);
	u8 blockr = collide_stage_rightwall(e);
	// Handle underwater
	if((stage_get_block_type(sub_to_block(e->x), sub_to_block(e->y)) & BLOCK_WATER) ||
			(water_entity && e->y > water_entity->y)) {
		e->underwater = TRUE;
	} else {
		e->underwater = FALSE;
	}
	// Inactive, just apply basic physics and gtfo
	if (e->state != CAI_ACTIVE) {
		e->x = e->x_next;
		e->y = e->y_next;
		return;
	}
	
	// first figure out where our target is
	
	// hack in case player REALLY leaves her behind. this works because of the way
	// the level is in a Z shape. first we check to see if the player is on the level below ours.
	if ((player.y > e->y_next && ((player.y - e->y_next) > 160<<9)) || e->state==999) {
		// if we're on the top section, head all the way to right, else if we're on the
		// middle section, head for the trap door that was destroyed by the button
		otiley = (e->y_next >> 9) / 16;
		
		curly_target_time = 0;
		
		if (otiley < 22) {
			e->x_mark = ((126 * 16) + 8) << 9;		// center of big chute on right top
		} else if (otiley > 36 && otiley < 47) {	
			// fell down chute in center of middle section
			// continue down chute, don't get hung up on sides
			e->x_mark = (26 * 16) << 9;
		} else if (otiley >= 47) {	
			// bottom section - head for exit door
			// (this shouldn't ever execute, though, because player can't be lower than this)
			e->x_mark = (81 * 16) << 9;
			seeking_player = 1;		// stop when reach exit door
		} else {	
			// on middle section
			e->x_mark = ((7 * 16) + 8) << 9;		// trap door which was destroyed by switch
		}
		e->y_mark = e->y_next;
	} else {
		// if we get real far away from the player leave the enemies alone and come find him
		if (!PLAYER_DIST_X(160<<9)) curly_target_time = 0;
		
		// if we're attacking an enemy head towards the enemy else return to the player
		if (curly_target_time) {
			e->x_mark = curly_target_x;
			e->y_mark = curly_target_y;
			
			curly_target_time--;
			if (curly_target_time==60 && !(random() % 2)) CaiJUMP(e);
		} else {
			e->x_mark = player.x;
			e->y_mark = player.y;
			seeking_player = 1;
		}
	}
	
	// do not fall off the middle railing in Almond
	if (stageID == 0x2F) {
		#define END_OF_RAILING		(((72*16)-8)<<9)
		if (e->x_mark > END_OF_RAILING) {
			e->x_mark = END_OF_RAILING;
		}
	}
	
	// calculate distance to target
	xdist = abs(e->x_next - e->x_mark);
	ydist = abs(e->y_next - e->y_mark);
	
	// face target. I used two seperate IF statements so she doesn't freak out at start point
	// when her x == xmark.
	if (e->x_next < e->x_mark) wantdir = 1;
	if (e->x_next > e->x_mark) wantdir = 0;
	if (wantdir != e->dir) {
		if (++e->timer2 > 4) {
			e->timer2 = 0;
			e->dir = wantdir;
		}
	} else e->timer2 = 0;
	
	// if trying to return to the player then go into a rest state when we've reached him
	reached_p = 0;
	if (seeking_player && xdist < (32<<9) && ydist < (64<<9)) {
		if (++curly_reachptimer > 80) {
			e->x_speed *= 7;
			e->x_speed /= 8;
			e->frame = 0;
			reached_p = 1;
		}
	}
	else curly_reachptimer = 0;
	
	if (!reached_p)	{	// if not at rest walk towards target
		//e->frame = ANIM_WALKING;
		
		// walk towards target
		if (e->x_next > e->x_mark) e->x_speed -= SPEED(0x20);
		if (e->x_next < e->x_mark) e->x_speed += SPEED(0x20);
		
		// jump if we hit a wall
		if (blockr || blockl) {
			if (++curly_blockedtime > 8) {
				CaiJUMP(e);
			}
		}
		else curly_blockedtime = 0;
		
		// if our target gets really far away (like p is leaving us behind) and
		// the above jumping isn't getting us anywhere, activate the Improbable Jump
		if ((blockl || blockr) && xdist > (80<<9)) {
			if (++curly_impjumptime > 60) {
				if (e->grounded) {
					CaiJUMP(e);
					curly_impjumptime = -100;
					curly_impjump = 1;
				}
			}
		}
		else curly_impjumptime = 0;
		
		// if we're below the target try jumping around randomly
		if (e->y_next > e->y_mark && (e->y_next - e->y_mark) > (16<<9)) {
			if (++curly_tryjumptime > 20) {
				curly_tryjumptime = 0;
				if (random() & 1) CaiJUMP(e);
			}
		}
		else curly_tryjumptime = 0;
	}
	
	// the improbable jump - when AI gets confused, just cheat!
	// jump REALLY high by reducing gravity until we clear the wall
	if (curly_impjump > 0) {
		e->y_speed += SPEED(0x10);
		// deactivate Improbable Jump once we clear the wall or hit the ground
		if (e->dir==0 && blockl) curly_impjump = 0;
		if (e->dir==1 && blockr) curly_impjump = 0;
		if (e->grounded) curly_impjump = 0;
	}
	else e->y_speed += SPEED(0x33);
	
	// slow down when we hit bricks
	if (blockl || blockr) {
		// full stop if on ground, partial stop if in air
		xlimit = e->grounded ? 0x000 : 0x180;
		
		if (blockl) {
			if (e->x_speed < -xlimit) e->x_speed = -xlimit;
		}
		else if (e->x_speed > xlimit) {		
			// we don't have to test blockr because we already know one or the other is set 
			// and that it's not blockl
			e->x_speed = xlimit;
		}
	}
	
	// look up/down at target
	curly_look = 0;
	if (!reached_p || abs(e->y_next - player.y) > (48<<9)) {
		if (e->y_next > e->y_mark && ydist >= (12<<9) && (!seeking_player || ydist >= (80<<9))) curly_look = DIR_UP;
		else if (e->y_next < e->y_mark && !e->grounded && ydist >= (80<<9)) curly_look = DIR_DOWN;
	}
	
	// Sprite Animation
	if(e->grounded) {
		if(curly_look == DIR_UP) {
			if(e->x_speed != 0) {
				ANIMATE(e, 8, UPWALK1,LOOKUP,UPWALK2,LOOKUP);
			} else {
				e->frame = LOOKUP;
			}
		} else if(e->x_speed != 0) {
			ANIMATE(e, 8, WALK1,STAND,WALK2,STAND);
		} else if(curly_look == DIR_DOWN) {
			e->frame = LOOKDN;
		} else {
			e->frame = STAND;
		}
	} else {
		if(curly_look == DIR_UP) {
			e->frame = UPWALK2;
		} else if(curly_look == DIR_DOWN) {
			e->frame = JUMPDN;
		} else {
			e->frame = WALK2;
		}
	}
	
	e->x = e->x_next;
	e->y = e->y_next;
	
	LIMIT_X(SPEED(0x300));
	LIMIT_Y(SPEED(0x5ff));
}

void fire_mgun(s32 x, s32 y, u8 dir) {
	// Curly shares the bullet array with the player so don't use up the whole thing
	// Use slots 2-4 and 7-8 backwards
	// This leaves room in the worst case for 1 missile, 2 polar star, and 2 fireball
	Bullet *b = NULL;
	for(u8 i = 8; i > 6; i--) {
		if(playerBullet[i].ttl > 0) continue;
		b = &playerBullet[i];
		break;
	}
	if(b == NULL) {
		for(u8 i = 4; i > 1; i--) {
			if(playerBullet[i].ttl > 0) continue;
			b = &playerBullet[i];
			break;
		}
		if(b == NULL) return;
	}
	sound_play(SND_POLAR_STAR_L3, 5);
	b->type = WEAPON_MACHINEGUN;
	b->level = 3;
	// Need to set the position immediately or else the sprite will blink in upper left
	////SPR_SAFEADD(b->sprite, &SPR_MGunB3, (x >> CSF) - (camera.x >> CSF) + SCREEN_HALF_W - 8,
	//		(y >> CSF) - (camera.y >> CSF) + SCREEN_HALF_H - 8, TILE_ATTR(PAL0, 0, 0, 0), 3);
	b->damage = 6;
	b->ttl = 90;
	b->hit_box = (bounding_box) { 4, 4, 4, 4 };
	b->x = x;
	b->y = y;
	if(dir == DIR_UP) {
		////SPR_SAFEANIM(b->sprite, 1);
		b->x_speed = 0;
		b->y_speed = -pixel_to_sub(4);
	} else if(dir == DIR_DOWN) {
		////SPR_SAFEANIM(b->sprite, 1);
		////SPR_SAFEVFLIP(b->sprite, 1);
		b->x_speed = 0;
		b->y_speed = pixel_to_sub(4);
	} else {
		////SPR_SAFEHFLIP(b->sprite, dir > 0);
		b->x_speed = (dir > 0 ? pixel_to_sub(4) : -pixel_to_sub(4));
		b->y_speed = 0;
	}
}

void fire_pstar(s32 x, s32 y, u8 dir) {
	// Use slot 7 and 8, this messes with player's missiles slightly
	Bullet *b = NULL;
	for(u8 i = 7; i < 9; i++) {
		if(playerBullet[i].ttl > 0) continue;
		b = &playerBullet[i];
		break;
	}
	if(b == NULL) return;
	sound_play(SND_POLAR_STAR_L3, 5);
	b->type = WEAPON_POLARSTAR;
	b->level = 3;
	////SPR_SAFEADD(b->sprite, &SPR_PolarB3, (x >> CSF) - (camera.x >> CSF) + SCREEN_HALF_W - 8,
	//		(y >> CSF) - (camera.y >> CSF) + SCREEN_HALF_H - 8, TILE_ATTR(PAL0, 0, 0, 0), 3);
	b->damage = 4;
	b->ttl = 35;
	b->hit_box = (bounding_box) { 4, 4, 4, 4 };
	b->x = x;
	b->y = y;
	if(dir == DIR_UP) {
		////SPR_SAFEANIM(b->sprite, 1);
		b->x_speed = 0;
		b->y_speed = -pixel_to_sub(4);
	} else if(dir == DIR_DOWN) {
		////SPR_SAFEANIM(b->sprite, 1);
		b->x_speed = 0;
		b->y_speed = pixel_to_sub(4);
	} else {
		b->x_speed = (dir > 0 ? pixel_to_sub(4) : -pixel_to_sub(4));
		b->y_speed = 0;
	}
}

#define SMALLDIST		(32 << CSF)
#define BIGDIST			(160 << CSF)

void ai_cai_gun(Entity *e) {
	Entity *curly = e->linkedEntity;
	if (curly == NULL) { e->state = STATE_DELETE; return; }
	
	// Stick to curly
	e->x = curly->x - (4 << CSF);
	e->y = curly->y - (4 << CSF);
	e->dir = curly->dir;
	//if (curly_look) {
	//	e->frame = curly_look == DIR_DOWN ? 2 : 1;
	//} else {
	//	e->frame = 0;
	//}
	////SPR_SAFEHFLIP(e->sprite, e->dir);
	
	if (curly_target_time) {
		// fire when we get close to the target
		u8 fire;
		if (!curly_look) {
			// firing LR-- fire when lined up vertically and close by horizontally
			fire = ((abs(curly->x - curly_target_x) <= BIGDIST) && (abs(curly->y - curly_target_y) <= SMALLDIST));
		} else {
			// firing vertically-- fire when lined up horizontally and close by vertically
			fire = ((abs(curly->x - curly_target_x) <= SMALLDIST) && (abs(curly->y - curly_target_y) <= BIGDIST));
		}
		if (fire) {
			// Get point where bullet will be created based on direction / looking
			if(!curly_look) {
				e->x_mark = curly->x + (curly->dir ? 8 << CSF : -(8 << CSF));
				e->y_mark = curly->y + (1 << CSF);
			} else if(curly_look == DIR_UP) {
				e->x_mark = curly->x;
				e->y_mark = curly->y - (8 << CSF);
			} else if(curly_look == DIR_DOWN) {
				e->x_mark = curly->x;
				e->y_mark = curly->y + (8 << CSF);
			}
			if (curly_mgun) {	// she has the Machine Gun
				if (!e->timer2) {
					e->timer2 = 2 + (random() % 4);		// no. shots to fire
					e->timer = 40 + (random() % 10);
				}
				if (e->timer) {
					e->timer--;
				} else {
					// create the MGun blast
					// curly_look is forward (0), DIR_UP (1), or DIR_DOWN (3)
					// curly->dir is either 0 or 1, where 1 means right but DIR_RIGHT is 2
					// So we do this weird thing to fix it
					fire_mgun(e->x_mark, e->y_mark, curly_look ? curly_look : curly->dir << 1);
					e->timer = 40 + (random() % 10);
					e->timer2--;
				}
			} else {	// she has the Polar Star
				if (!e->timer) {
					e->timer = 4 + (random() % 12);
					if (random() % 10 == 0) e->timer += 20 + random() % 10;
					// create the shot
					fire_pstar(e->x_mark, e->y_mark, curly_look ? curly_look : curly->dir << 1);
				} else {
					e->timer--;
				}
			}
		}
	}
}

// curly's air bubble when she goes underwater
void ai_cai_watershield(Entity *e) {
	Entity *curly = e->linkedEntity;
	if (!curly) { e->state = STATE_DELETE; return; }
	
	if(curly->underwater) {
		e->hidden = FALSE;
		e->x = curly->x;
		e->y = curly->y;
	} else {
		e->hidden = TRUE;
		e->timer = 0;
	}
}
