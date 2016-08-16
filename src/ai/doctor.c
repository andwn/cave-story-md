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

void ai_doctor(Entity *e)
{
	switch(e->state)
	{
		case 10:	// he chuckles
			e->state = 11;
			e->state_time2 = 0;
			//e->frame = 1;
			//e->animtimer = 0;
		case 11:
			//ANIMATE(6, 0, 1);
			if (++e->state_time2 > 8*6) { 
				//e->frame = 0; 
				e->state = 1; 
			}
		break;
		
		case 20:	// he rises up and hovers
		{
			e->state = 21;
			e->state_time = 0;
			//e->frame = 2;
			e->y_mark = e->y - (32 << 9);
		}
		case 21:
		{
			e->y_speed += (e->y > e->y_mark) ? -0x20 : 0x20;
			LIMIT_Y(0x200);
		}
		break;
		
		case 30:	// he teleports away
		{
			e->state_time = 0;
			//e->frame = 2;
			e->y_speed = 0;
			e->state++;
		}
		case 31:
		{
			//if (DoTeleportOut(o, 1))
			//	e->Delete();
			e->state = STATE_DELETE;
		}
		break;
		
		case 40:	// he teleports in and hovers
		{
			e->state_time = 0;
			e->state = 41;
			//e->frame = 2;
		}
		case 41:
		{
			//if (DoTeleportIn(o, 1))
			//{
				e->state = 20;
				e->y_speed = -0x200;
			//}
		}
		break;
	}
}
