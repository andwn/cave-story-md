#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "effect.h"

void ai_curly_onUpdate(Entity *e) {
	if(!e->grounded) e->y_speed += GRAVITY_JUMP;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	// Don't test ceiling, only test sticking to ground while moving
	if(e->x_speed < 0) {
		collide_stage_leftwall(e);
	} else if(e->x_speed > 0) {
		collide_stage_rightwall(e);
	}
	if(e->grounded) {
		if(e->state != 11 && e->x_speed != 0) e->grounded = collide_stage_floor_grounded(e);
	} else if(e->y_speed > 0) {
		e->grounded = collide_stage_floor(e);
		if(e->grounded && e->state == 30) ENTITY_SET_STATE(e, 31, 0);
	}
	if(e->state == 10 && (e->direction ?
		(e->x > player.x - block_to_sub(2)) : (e->x < player.x + block_to_sub(2)))) {
		ENTITY_SET_STATE(e, 0, 0);
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

void ai_curly_onState(Entity *e) {
	switch(e->state) {
		case 0: // Stand
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 0);
		break;
		case 3:
		case 4: // Walk
		e->x_speed = pixel_to_sub(e->direction ? 1 : -1);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 1);
		break;
		case 5:
		effect_create_smoke(0, sub_to_block(e->x), sub_to_block(e->y));
		/* no break */
		case 6: // Defeated
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 9);
		break;
		case 10: // Walk towards player until 1 block away
		FACE_PLAYER(e);
		e->x_speed = pixel_to_sub(e->direction ? 1 : -1);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 1);
		break;
		case 11: // Walk with no gravity
		e->x_speed = pixel_to_sub(e->direction ? 1 : -1);
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 1);
		break;
		case 20: // Back turned
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 4);
		break;
		case 21: // Look up
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 2);
		break;
		case 30: // Knocked over
		SPR_SAFEANIM(e->sprite, 10);
		e->x_speed = -0x150 + 0x300 * e->direction;
		e->y_speed = -0x150;
		e->grounded = false;
		break;
		case 31:
		case 32: // Fell over
		e->x_speed = 0;
		SPR_SAFEHFLIP(e->sprite, e->direction);
		SPR_SAFEANIM(e->sprite, 9);
		break;
		case 70:
		case 71: // Walk backwards
		e->x_speed = pixel_to_sub(e->direction ? 1 : -1);
		SPR_SAFEHFLIP(e->sprite, !e->direction);
		SPR_SAFEANIM(e->sprite, 1);
		break;
	}
}

#define CURLYB_FIGHT_START		10
#define CURLYB_WAIT				11
#define CURLYB_WALK_PLAYER		13
#define CURLYB_WALKING_PLAYER	14
#define CURLYB_CHARGE_GUN		20
#define CURLYB_FIRE_GUN			21
#define CURLYB_SHIELD			30

static void curlyboss_fire(Entity *e, u8 dir) {
	Entity *shot = entity_create(0, 0, 0, 0, OBJ_CURLYBOSS_SHOT, 0, dir & 1);
	shot->attack = 6;
	switch(dir) {
		case 0:
			shot->x = e->x - pixel_to_sub(12);
			shot->y = e->y + pixel_to_sub(2);
			shot->x_speed = -4096;
		break;
		case 1:
			shot->x = e->x + pixel_to_sub(12);
			shot->y = e->y + pixel_to_sub(2);
			shot->x_speed = 4096;
		break;
		case 2:
			shot->x = e->x;
			shot->y = e->y - pixel_to_sub(10);
			shot->y_speed = -4096;
			SPR_SAFEANIM(shot->sprite, 1);
		break;
	}
	sound_play(SND_POLAR_STAR_L1_2, 4);
}

void ai_curlyBoss_onUpdate(Entity *e) {
	switch(e->state) {
		case CURLYB_FIGHT_START:
		{
			e->state = CURLYB_WAIT;
			e->state_time = (random() % 50) + 50;
			SPR_SAFEANIM(e->sprite, 0);
			e->direction = (e->x <= player.x);
			SPR_SAFEHFLIP(e->sprite, e->direction);
			e->eflags |= NPC_SHOOTABLE;
			e->eflags &= ~NPC_INVINCIBLE;
			e->nflags &= ~NPC_INVINCIBLE;
		}
		/* no break */
		case CURLYB_WAIT:
		{
			if(!e->state_time--) e->state = CURLYB_WALK_PLAYER;
		}
		break;
		case CURLYB_WALK_PLAYER:
		{
			e->state = CURLYB_WALKING_PLAYER;
			SPR_SAFEANIM(e->sprite, 1);
			e->state_time = (random() % 50) + 50;
			e->direction = (e->x <= player.x);
			SPR_SAFEHFLIP(e->sprite, e->direction);
		}
		/* no break */
		case CURLYB_WALKING_PLAYER:
			ACCEL_X(SPEED(0x40));
			if (e->state_time) {
				e->state_time--;
			} else {
				e->eflags |= NPC_SHOOTABLE;
				e->state = CURLYB_CHARGE_GUN;
				e->state_time = 0;
				sound_play(SND_CHARGE_GUN, 5);
			}
		break;
		case CURLYB_CHARGE_GUN:
		{
			FACE_PLAYER(e);
			e->x_speed -= e->x_speed >> 3;
			SPR_SAFEANIM(e->sprite, 0);
			if (++e->state_time > TIME(50)) {
				e->state = CURLYB_FIRE_GUN;
				e->x_speed = 0;
				e->state_time = 0;
			}
		}
		break;
		case CURLYB_FIRE_GUN:
		{
#ifdef PAL
			if (e->state_time % 4 == 0) {	// time to fire
#else
			if (e->state_time % 5 == 0) {	// time to fire
#endif
				// check if player is trying to jump over
				if (abs(e->x - player.x) < pixel_to_sub(32) && player.y + pixel_to_sub(10) < e->y) {
					// shoot up instead
					SPR_SAFEANIM(e->sprite, 2);
					curlyboss_fire(e, 2);
				} else {
					SPR_SAFEANIM(e->sprite, 0);
					curlyboss_fire(e, e->direction);
				}
			}
			if (++e->state_time > TIME(30)) e->state = CURLYB_FIGHT_START;
		}
		break;
		case CURLYB_SHIELD:
		{
			e->x_speed = 0;
			if (++e->state_time > TIME(30)) e->state = CURLYB_FIGHT_START;
		}
		break;
	}

	if (e->state > CURLYB_FIGHT_START && e->state < CURLYB_SHIELD) {
		// curly activates her shield anytime a missile's explosion goes off,
		// even if it's nowhere near her at all
		if(bullet_missile_is_exploding()) {
			e->state_time = 0;
			e->state = CURLYB_SHIELD;
			SPR_SAFEANIM(e->sprite, 3);
			e->eflags &= ~NPC_SHOOTABLE;
			e->eflags |= NPC_INVINCIBLE;
			e->x_speed = 0;
		}
	}
	
	if (e->x_speed > SPEED(0x200)) e->x_speed = SPEED(0x200);
	if (e->x_speed < -SPEED(0x200)) e->x_speed = -SPEED(0x200);

	e->x_next = e->x + e->x_speed;

	collide_stage_leftwall(e);
	collide_stage_rightwall(e);

	e->x = e->x_next;
}

void ai_curlyBoss_onState(Entity *e) {
	if(e->state == STATE_DEFEATED) {
		SPR_SAFERELEASE(e->sprite);
		entity_default(e, OBJ_CURLY, 0);
		entity_sprite_create(e);
		e->x -= 8;
		e->eflags &= ~NPC_SHOOTABLE;
		e->state = 0;
		entities_clear_by_type(OBJ_CURLYBOSS_SHOT);
		tsc_call_event(e->event);
	}
}

void ai_curlyBossShot_onUpdate(Entity *e)
{
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	if(collide_stage_leftwall(e) || collide_stage_rightwall(e) || collide_stage_ceiling(e)) {
		e->state = STATE_DELETE;
	} else if(!player_invincible() && entity_overlapping(e, &player)) {
		player_inflict_damage(e->attack);
		e->state = STATE_DELETE;
	} else {
		e->x = e->x_next;
		e->y = e->y_next;
	}
}

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
		e->y_speed = random(-0x600, -0x300);
		e->grounded = false;
		//e->frame = 3;
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
	if (!curly_watershield)
	{
		Entity *shield = entity_create(0, 0, 0, 0, OBJ_CAI_WATERSHIELD, 0, 0);
		shield->alwaysActive = true;
		//shield->sprite = SPR_WATER_SHIELD;
		shield->linkedEntity = e;
		
		//e->BringToFront();				// curly in front of monsters,
		//shield->BringToFront();			// and shield in front of curly
		
		curly_watershield = 1;
	}
	
	switch(e->state)
	{
		case 0:
		{
			e->alwaysActive = true;
			e->x_speed = 0;
			e->y_speed += 0x20;
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
			//e->invisible = 0;
			e->x_mark = e->x;
			e->y_mark = e->y;
			e->direction = player.direction;
			e->state = CAI_ACTIVE;
			e->state_time = 0;
			
			// spawn her gun
			Entity *gun = entity_create(0, 0, 0, 0, OBJ_CAI_GUN, 0, 0);
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
			//e->frame = 15;
		}
		/* no break */
		case CAI_KNOCKEDOUT+1:
		{
			if (++e->state_time > 1000)
			{	// start fighting
				e->state = CAI_START;
			}
			else if (e->state_time > 750)
			{	// stand up
				e->eflags &= ~NPC_INTERACTIVE;
				//e->frame = 0;
			}
		}
		break;
	}
	
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	bool blocku = collide_stage_ceiling(e);
	if(e->grounded) e->grounded = collide_stage_floor_grounded(e);
	else e->grounded = collide_stage_floor(e);
	bool blockl = collide_stage_leftwall(e);
	bool blockr = collide_stage_rightwall(e);
	
	if (e->state != CAI_ACTIVE) {
		e->x = e->x_next;
		e->y = e->y_next;
		return;
	}
	
	// first figure out where our target is
	
	// hack in case player REALLY leaves her behind. this works because of the way
	// the level is in a Z shape. first we check to see if the player is on the level below ours.
	if ((player.y > e->y && ((player.y - e->y) > 160<<9)) || e->state==999)
	{
		// if we're on the top section, head all the way to right, else if we're on the
		// middle section, head for the trap door that was destroyed by the button
		otiley = (e->y >> 9) / 16;
		
		curly_target_time = 0;
		
		if (otiley < 22)
		{
			e->x_mark = ((126 * 16) + 8) << 9;		// center of big chute on right top
		}
		else if (otiley > 36 && otiley < 47)
		{	// fell down chute in center of middle section
			// continue down chute, don't get hung up on sides
			e->x_mark = (26 * 16) << 9;
		}
		else if (otiley >= 47)
		{	// bottom section - head for exit door
			// (this shouldn't ever execute, though, because player can't be lower than this)
			e->x_mark = (81 * 16) << 9;
			seeking_player = 1;		// stop when reach exit door
		}
		else
		{	// on middle section
			e->x_mark = ((7 * 16) + 8) << 9;		// trap door which was destroyed by switch
		}
		
		e->y_mark = e->y;
	}
	else
	{
		// if we get real far away from the player leave the enemies alone and come find him
		if (!PLAYER_DIST_X(160<<9)) curly_target_time = 0;
		
		// if we're attacking an enemy head towards the enemy else return to the player
		if (curly_target_time)
		{
			e->x_mark = curly_target_x;
			e->y_mark = curly_target_y;
			
			curly_target_time--;
			if (curly_target_time==60 && !(random() % 2)) CaiJUMP(e);
		}
		else
		{
			e->x_mark = player.x;
			e->y_mark = player.y;
			seeking_player = 1;
		}
	}
	
	// do not fall off the middle railing in Almond
	if (stageID == 0x2F)
	{
		#define END_OF_RAILING		(((72*16)-8)<<9)
		if (e->x_mark > END_OF_RAILING)
		{
			e->x_mark = END_OF_RAILING;
		}
	}
	
	// calculate distance to target
	xdist = abs(e->x - e->x_mark);
	ydist = abs(e->y - e->y_mark);
	
	// face target. I used two seperate IF statements so she doesn't freak out at start point
	// when her x == xmark.
	if (e->x < e->x_mark) wantdir = 1;
	if (e->x > e->x_mark) wantdir = 0;
	if (wantdir != e->direction)
	{
		if (++e->state_time2 > 4)
		{
			e->state_time2 = 0;
			e->direction = wantdir;
		}
	}
	else e->state_time2 = 0;
	
	// if trying to return to the player then go into a rest state when we've reached him
	reached_p = 0;
	if (seeking_player && xdist < (32<<9) && ydist < (64<<9))
	{
		if (++curly_reachptimer > 80)
		{
			e->x_speed *= 7;
			e->x_speed /= 8;
			//e->frame = 0;
			reached_p = 1;
		}
	}
	else curly_reachptimer = 0;
	
	if (!reached_p)		// if not at rest walk towards target
	{
		// walking animation
		//if (++e->animtimer > 4)
		//{
		//	e->animtimer = 0;
		//	if (++e->animframe > 3) e->animframe = 0;
		//}
		
		// walk towards target
		if (e->x > e->x_mark) e->x_speed -= 0x20;
		if (e->x < e->x_mark) e->x_speed += 0x20;
		//e->frame = e->animframe;
		
		// jump if we hit a wall
		if ((e->x_speed > 0 && blockr) || 
			(e->x_speed < 0 && blockl))
		{
			if (++curly_blockedtime > 8)
			{
				CaiJUMP(e);
			}
		}
		else curly_blockedtime = 0;
		
		// if our target gets really far away (like p is leaving us behind) and
		// the above jumping isn't getting us anywhere, activate the Improbable Jump
		if ((blockl || blockr) && xdist > (80<<9))
		{
			if (++curly_impjumptime > 60)
			{
				if (e->grounded)
				{
					CaiJUMP(e);
					curly_impjumptime = -100;
					curly_impjump = 1;
				}
			}
		}
		else curly_impjumptime = 0;
		
		// if we're below the target try jumping around randomly
		if (e->y > e->y_mark && (e->y - e->y_mark) > (16<<9))
		{
			if (++curly_tryjumptime > 20)
			{
				curly_tryjumptime = 0;
				if (random() & 1) CaiJUMP(e);
			}
		}
		else curly_tryjumptime = 0;
	}
	//else e->animtimer = e->animframe = 0;		// reset walk anim
	
	// force jump/fall frames
	//if (e->y_speed < 0) e->frame = 3;
	//else if (!e->blockd) e->frame = 1;
	//else if (e->x==e->x_mark) e->frame = 0;
	
	// the improbable jump - when AI gets confused, just cheat!
	// jump REALLY high by reducing gravity until we clear the wall
	if (curly_impjump > 0)
	{
		e->y_speed += 0x10;
		// deactivate Improbable Jump once we clear the wall or hit the ground
		if (e->direction==0 && blockl) curly_impjump = 0;
		if (e->direction==1 && blockr) curly_impjump = 0;
		if (e->y_speed > 0 && e->grounded) curly_impjump = 0;
	}
	else e->y_speed += 0x33;
	
	// slow down when we hit bricks
	if (blockl || blockr)
	{
		// full stop if on ground, partial stop if in air
		xlimit = e->grounded ? 0x000:0x180;
		
		if (blockl)
		{
			if (e->x_speed < -xlimit) e->x_speed = -xlimit;
		}
		else if (e->x_speed > xlimit)		// we don't have to test blockr because we already know one or the other is set and that it's not blockl
		{
			e->x_speed = xlimit;
		}
	}
	
	// look up/down at target
	curly_look = 0;
	if (!reached_p || abs(e->y - player.y) > (48<<9))
	{
		if (e->y > e->y_mark && ydist >= (12<<9) && (!seeking_player || ydist >= (80<<9))) curly_look = DIR_UP;
		else if (e->y < e->y_mark && !e->grounded && ydist >= (80<<9)) curly_look = DIR_DOWN;
	}
	
	//if (e->curly.look == UP) e->frame += 4;
	//else if (e->curly.look == DOWN) e->frame += 8;
	
	e->x = e->x_next;
	e->y = e->y_next;
	
	LIMIT_X(0x300);
	LIMIT_Y(0x5ff);
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
	
	//e->frame = 0;
	if (curly_look)
	{
		//e->sprite = (curly->curly.gunsprite + 1);
		//if (curly->curly.look==DOWN) e->frame = 1;
	}
	else
	{
		//e->sprite = curly->curly.gunsprite;
	}
	
	if (curly_target_time)
	{
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

void aftermove_cai_gun(Entity *e)
{
	Entity *curly = e->linkedEntity;
	if (curly)
	{
		e->x = curly->x;
		e->y = curly->y;
		e->direction = curly->direction;
	}
}


// curly's air bubble when she goes underwater
void aftermove_cai_watershield(Entity *e)
{
	Entity *curly = e->linkedEntity;
	if (curly == NULL)
	{
		e->state = STATE_DELETE;
		return;
	}
	
	//static const Point cwp = { 8, 5 };
	if(curly->underwater)
	{
		//e->invisible = false;
		e->x = curly->x;
		e->y = curly->y;
		
		//e->frame = (++e->state_time & 2) ? 1 : 0;
	}
	else
	{
		//e->invisible = true;
		//e->state_time = e->frame = 0;
		e->state_time = 0;
	}
}
