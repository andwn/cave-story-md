#include "ai_common.h"
#include "ai_object.h"
#include "ai_mimiga.h"
#include "ai_balrog.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "sprite.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

void ai_activate_base(Entity *e) {

}
void ai_update_base(Entity *e) {

}
bool ai_setstate_base(Entity *e, u16 state) {
	e->state = state;
	if(state == STATE_DEFEATED) {
		return true;
	}
	return false;
}

void entity_create_special(Entity *e, bool option1, bool option2) {
	switch(e->type) {
	case 1: // Weapon Energy
		e->x_speed = 0x200 - (random() % 0x400);
		e->update = &ai_update_energy;
		break;
	case 12: // Balrog (Cutscene)
		e->update = &ai_update_balrog_scene;
		e->set_state = &ai_setstate_balrog_scene;
		break;
	case 18: // Door
		e->activate = &ai_activate_door;
		e->activate(e);
		break;
	case 46: // Trigger
		e->update = &ai_update_trigger;
		break;
	case 60: // Toroko
		e->update = &ai_update_toroko;
		break;
	case 63: // Toroko attacking with stick
		e->y -= block_to_sub(1);
		e->update = &ai_update_toroko;
		e->state = 3; // Running back and forth
		sprite_set_animation(e->sprite, 2);
		break;
	case 68: // Boss: Balrog (Mimiga Village)
		e->update = &ai_update_balrog_boss1;
		e->set_state = &ai_setstate_balrog_boss1;
		break;
	case 211: // Spikes
		e->activate = &ai_activate_spike;
		if(e->sprite != SPRITE_NONE) e->activate(e);
		break;
	default:
		break;
	}
}
