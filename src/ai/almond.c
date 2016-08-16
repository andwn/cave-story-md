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

/*
void ai_waterlevel(Entity *e)
{
	if (map.wlforcestate)
	{
		//NX_LOG("Forced WL state to %d\n", map.wlforcestate);
		e->state = map.wlforcestate;
		map.wlforcestate = 0;
	}
	
	switch(e->state)
	{
		case 0:
			map.waterlevelobject = e;
			e->state = WL_CALM;
			e->y += (8<<9);
			e->y_mark = e->y;
			e->y_speed = 0x200;
		case WL_CALM:	// calm waves around set point
			e->y_speed += (e->y < e->y_mark) ? 4 : -4;
			LIMITY(0x100);
		break;
		
		case WL_CYCLE:			// wait 1000 ticks, then rise all the way to top come down and repeat
			e->state = WL_DOWN; e->state_time = 0;
		case WL_DOWN:
			e->y_speed += (e->y < e->y_mark) ? 4 : -4;
			LIMITY(0x200);
			if (++e->state_time > 1000)
			{
				e->state = WL_UP;
			}
		break;
		case WL_UP:			// rise all the way to top then come back down
			e->y_speed += (e->y > 0) ? -4 : 4;
			LIMITY(0x200);
			
			// when we reach the top return to normal level
			if (e->y < (64<<9))
			{
				e->state = WL_CYCLE;
			}
		break;
		
		case WL_STAY_UP:	// rise quickly all the way to top and stay there
			e->y_speed += (e->y > 0) ? -4 : 4;
			if (e->y_speed < -0x200) e->y_speed = -0x200;
			if (e->y_speed > 0x100) e->y_speed = 0x100;
		break;
	}
	
	map.wlstate = e->state;
}
*/

/// common code to both Shutter AND Lift
void ai_shutter(Entity *e)
{
	if (e->state == 10)
	{
		// allow hitting the stuck shutter no. 4
		e->eflags &= ~(NPC_SHOOTABLE | NPC_INVINCIBLE);
		
		switch(e->direction)
		{
			case DIR_LEFT:  e->x -= 0x80; break;
			case DIR_RIGHT: e->x += 0x80; break;
			case DIR_UP:    e->y -= 0x80; break;
			case DIR_DOWN:  e->y += 0x80; break;
		}
		
		// animate Almond_Lift
		if (e->type==OBJ_ALMOND_LIFT)
		{
			//ai_animate3(e);
		}
		else if (e->type==OBJ_SHUTTER_BIG)
		{
			if (!e->state_time)
			{
				camera_shake(20);
				sound_play(SND_QUAKE, 5);
				
				e->state_time = 6;
			} else e->state_time--;
		}
	}
	//else if (e->state == 20)	// tripped by script when Shutter_Big closes fully
	//{
		//SmokeSide(o, 4, DOWN);
		//e->state = 21;
	//}
	
	//if (e->type == OBJ_SHUTTER_BIG)
	//{
	//	ANIMATE(10, 0, 3);
	//}
}

void ai_shutter_stuck(Entity *e)
{
	// when you shoot shutter 4, you're actually shooting us, but we want them
	// to think they're shooting the regular shutter object, so go invisible
	//e->invisible = 1;
	SPR_SAFEVISIBILITY(e->sprite, HIDDEN);
}

/*
void c------------------------------() {}
*/

// the damaged robot which wakes up right before the Almond battle
void ai_almond_robot(Entity *e)
{
	switch(e->state)
	{
		case 0:
			//e->frame = 0;
		break;
		
		case 10:	// blows up
			sound_play(SND_BIG_CRASH, 5);
			//SmokeClouds(o, 8, 3, 3);
			//e->Delete();
			e->state = STATE_DELETE;
		break;
		
		case 20:	// flashes
			//ANIMATE(10, 0, 1);
		break;
	}
}
