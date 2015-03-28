#include "ai_balrog.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "sprite.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"


// 12 - Balrog (Cutscene)
void ai_update_balrog_scene(Entity *e) {
	if(e->state == 30) {
		if(e->state_time > 0) {
			e->state_time--;
		} else {
			e->set_state(e, 0);
		}
	}
	e->x += e->x_speed;
	e->y += e->y_speed;
}

bool ai_setstate_balrog_scene(Entity *e, u16 state) {
	e->state = state;
	switch(state) {
	case 0: // Standing around
		sprite_set_animation(e->sprite, 0);
		break;
	case 10: // Going up!
	case 11:
		sprite_set_animation(e->sprite, 3);
		e->y_speed = pixel_to_sub(-2);
		break;
	case 20: // Smoking, going up!
	case 21:
		sprite_set_animation(e->sprite, 5);
		e->y_speed = pixel_to_sub(-2);
		break;
	case 30: // Smile
		sprite_set_animation(e->sprite, 6);
		e->state_time = 60;
		break;
	case 70: // Vanish
	case 71:
		return true;
	}
	return false;
}

// 68 - Boss: Balrog (Mimiga Village)
void ai_update_balrog_boss1(Entity *e) {

}

bool ai_setstate_balrog_boss1(Entity *e, u16 state) {
	e->state = state;
	if(state == STATE_DEFEATED) {
		e->state += 1;
		tsc_call_event(e->event); // Boss defeated event.. maybe
	}
	return false;
}
