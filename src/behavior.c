#include "behavior.h"

#include <genesis.h>
#include "player.h"
#include "tables.h"
#include "audio.h"

void ai_update_toroko(Entity *e) {
	if(bullet_colliding(e)) {
		sound_play(e->hurtSound, 10); // Squeak
		// TODO: Change animation to falling on ground
		e->attack = 0; // Don't hurt the player anymore
		e->flags |= NPC_INTERACTIVE; // Enable interaction
	}
}

void ai_activate_spike() {

}
