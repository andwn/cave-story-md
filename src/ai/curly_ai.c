#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "effect.h"
#include "resources.h"

#define CAI_INIT			20			// ANP'd to this by the entry script in Lab M
#define CAI_START			21			// ANP'd to this by Almond script
#define CAI_RUNNING			22
#define CAI_KNOCKEDOUT		40			// knocked out at beginning of Almond battle
#define CAI_ACTIVE	99

bool curly_mgun = 0;
bool curly_watershield = 0;
bool curly_impjump = 0;
u8 curly_reachptimer = 0;
u8 curly_blockedtime = 0;
s8 curly_impjumptime = 0;
u8 curly_tryjumptime = 0;
u8 curly_look = 0;

static void CaiJUMP(Entity *e) {
	if (e->grounded) {
		e->y_speed = SPEED(-0x300) - random(SPEED(0x300));
		e->grounded = false;
		SPR_SAFEANIM(e->sprite, ANIM_JUMPING);
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
		Entity *shield = entity_create(sub_to_block(e->x), sub_to_block(e->y), 
				0, 0, OBJ_CAI_WATERSHIELD, 0, 0);
		shield->alwaysActive = true;
		shield->linkedEntity = e;
		
		//e->BringToFront();				// curly in front of monsters,
		//shield->BringToFront();			// and shield in front of curly
		
		curly_watershield = 1;
	}
	
	switch(e->state) {
		case 0:
		{
			e->alwaysActive = true;
			e->x_speed = 0;
			e->y_speed += SPEED(0x20);
		}
		break;
		case CAI_INIT:			// set to this by an ANP in Maze M
		{
			e->alwaysActive = true;
			e->x = player.x;
			e->y = player.y;
		}
		/* no break */
		case CAI_START:			// set here after she stops being knocked out in Almond
		{
			e->alwaysActive = true;
			e->x_mark = e->x;
			e->y_mark = e->y;
			e->direction = player.direction;
			e->state = CAI_ACTIVE;
			e->state_time = 0;
			// spawn her gun
			Entity *gun = entity_create(sub_to_block(e->x), sub_to_block(e->y), 
					0, 0, OBJ_CAI_GUN, 0, 0);
			gun->alwaysActive = true;
			gun->linkedEntity = e;
			//gun->PushBehind(o);
			
			// If we traded Curly for machine gun she uses the polar star
			curly_mgun = !player_has_weapon(WEAPON_MACHINEGUN);
		}
		break;
		case CAI_KNOCKEDOUT:
		{
			e->state_time = 0;
			e->state = CAI_KNOCKEDOUT+1;
			SPR_SAFEANIM(e->sprite, 9);
		}
		/* no break */
		case CAI_KNOCKEDOUT+1:
		{
			if (++e->state_time > TIME(1000)) {	// start fighting
				e->state = CAI_START;
			}
			else if (e->state_time > TIME(750))
			{	// stand up
				e->eflags &= ~NPC_INTERACTIVE;
				SPR_SAFEANIM(e->sprite, ANIM_STANDING);
			}
		}
		break;
	}
	// Check collision up front and remember the result
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	/* bool blocku = (unused) */collide_stage_ceiling(e);
	if(e->grounded) e->grounded = collide_stage_floor_grounded(e);
	else e->grounded = collide_stage_floor(e);
	bool blockl = collide_stage_leftwall(e);
	bool blockr = collide_stage_rightwall(e);
	// Handle underwater
	if((stage_get_block_type(sub_to_block(e->x), sub_to_block(e->y)) & 0x20) ||
			(water_entity != NULL && e->y > water_entity->y)) {
		e->underwater = true;
	} else {
		e->underwater = false;
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
	if (wantdir != e->direction) {
		if (++e->state_time2 > 4) {
			e->state_time2 = 0;
			e->direction = wantdir;
		}
	} else e->state_time2 = 0;
	
	// if trying to return to the player then go into a rest state when we've reached him
	reached_p = 0;
	if (seeking_player && xdist < (32<<9) && ydist < (64<<9)) {
		if (++curly_reachptimer > 80) {
			e->x_speed *= 7;
			e->x_speed /= 8;
			SPR_SAFEANIM(e->sprite, ANIM_STANDING);
			reached_p = 1;
		}
	}
	else curly_reachptimer = 0;
	
	if (!reached_p)	{	// if not at rest walk towards target
		SPR_SAFEANIM(e->sprite, ANIM_WALKING);
		
		// walk towards target
		if (e->x_next > e->x_mark) e->x_speed -= SPEED(0x20);
		if (e->x_next < e->x_mark) e->x_speed += SPEED(0x20);
		
		// jump if we hit a wall
		if ((e->x_speed > 0 && blockr) || 
			(e->x_speed < 0 && blockl)) {
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
		else if(e->x_speed != 0) curly_impjumptime = 0;
		
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
		if (e->direction==0 && blockl) curly_impjump = 0;
		if (e->direction==1 && blockr) curly_impjump = 0;
		if (e->y_speed > 0 && e->grounded) curly_impjump = 0;
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
	u8 anim;
	if(e->grounded) {
		if(curly_look == DIR_UP) {
			if(e->x_speed != 0) {
				anim = ANIM_LOOKUPWALK;
			} else {
				anim = ANIM_LOOKUP;
			}
		} else if(e->x_speed != 0) {
			anim = ANIM_WALKING;
		} else if(curly_look == DIR_DOWN) {
			anim = ANIM_INTERACT;
		} else {
			anim = ANIM_STANDING;
		}
	} else {
		if(curly_look == DIR_UP) {
			anim = ANIM_LOOKUPJUMP;
		} else if(curly_look == DIR_DOWN) {
			anim = ANIM_LOOKDOWNJUMP;
		} else {
			anim = ANIM_JUMPING;
		}
	}
	// Set animation
	SPR_SAFEANIM(e->sprite, anim);
	// Change direction if pressing left or right
	SPR_SAFEHFLIP(e->sprite, e->direction);
	
	e->x = e->x_next;
	e->y = e->y_next;
	
	LIMIT_X(SPEED(0x300));
	LIMIT_Y(SPEED(0x5ff));
}

void ai_cai_gun(Entity *e) {
	Entity *curly = e->linkedEntity;
	//Entity *shot;
	//u8 fire;
	//int x, y;
	//u8 dir;
#define SMALLDIST		(32 << CSF)
#define BIGDIST			(160 << CSF)

	if (curly == NULL) { e->state = STATE_DELETE; return; }
	
	if(!e->state) {
		if(curly_mgun) {
			SPR_SAFEADD(e->sprite, &SPR_MGun, 0, 0, TILE_ATTR(PAL0, 0, 0, 0), 3);
		}
		e->state = 1;
	}
	// Stick to curly
	e->x = curly->x;
	e->y = curly->y;
	e->direction = curly->direction;
	if (curly_look) {
		SPR_SAFEANIM(e->sprite, curly_look == DIR_DOWN ? 2 : 1);
	} else {
		SPR_SAFEANIM(e->sprite, 0);
	}
	SPR_SAFEHFLIP(e->sprite, e->direction);
	
	if (curly_target_time) {
		// fire when we get close to the target
		if (!curly_look)
		{	// firing LR-- fire when lined up vertically and close by horizontally
			//fire = ((abs(e->x - game.curlytarget.x) <= BIGDIST) && (abs(e->y - game.curlytarget.y) <= SMALLDIST));
		}
		else
		{	// firing vertically-- fire when lined up horizontally and close by vertically
			//fire = ((abs(e->x - game.curlytarget.x) <= SMALLDIST) && (abs(e->y - game.curlytarget.y) <= BIGDIST));
		}
		/*
		if (fire)
		{
			// get coordinate of our action point
			x = (e->ActionPointX() - e->DrawPointX());
			y = (e->ActionPointY() - e->DrawPointY());
			dir = curly->curly.look ? curly->curly.look : e->dir;
			
			if (curly->curly.gunsprite==SPR_MGUN)
			{	// she has the Machine Gun
				if (!e->state_time)
				{
					e->state_time2 = random(2, 6);		// no. shots to fire
					e->state_time = random(40, 50);
					e->animtimer = 0;
				}
				
				if (e->state_time2)
				{	// create the MGun blast
					if (!e->animtimer)
					{
						FireLevel23MGun(x, y, 2, dir);
						
						e->animtimer = 5;
						e->state_time2--;
					}
					else e->animtimer--;
				}
			}
			else
			{	// she has the Polar Star
				if (!e->state_time)
				{
					e->state_time = random(4, 16);
					if (random(0, 10)==0) e->state_time += random(20, 30);
					
					// create the shot
					shot = CreateEntity(x, y, OBJ_POLAR_SHOT);
					SetupBullet(shot, x, y, B_PSTAR_L3, dir);
				}
			}
		}
		* */
	}
	
	if (e->state_time) e->state_time--;
}

// curly's air bubble when she goes underwater
void ai_cai_watershield(Entity *e) {
	Entity *curly = e->linkedEntity;
	if (curly == NULL) { e->state = STATE_DELETE; return; }
	
	if(curly->underwater) {
		SPR_SAFEVISIBILITY(e->sprite, AUTO_FAST);
		e->x = curly->x;
		e->y = curly->y;
	} else {
		SPR_SAFEVISIBILITY(e->sprite, HIDDEN);
		e->state_time = 0;
	}
}
