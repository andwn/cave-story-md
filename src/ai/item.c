#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "input.h"

void ai_energy_onCreate(Entity *e) {
	e->x_speed = 0x200 - (random() % 0x400);
}

void ai_energy_onUpdate(Entity *e) {
	// Bounce when hitting the ground
	if(e->grounded) {
		e->y_speed = pixel_to_sub(-2);
		e->grounded = false;
		sound_play(SOUND_EXPBOUNCE, 0);
	}
	e->y_speed += GRAVITY;
	e->x_next = e->x + e->x_speed;
	e->y_next = e->y + e->y_speed;
	s16 xsp = e->x_speed;
	entity_update_collision(e);
	// Reverse direction when hitting a wall
	if(e->x_speed == 0) {
		e->direction = !e->direction;
		e->x_speed = -xsp;
	}
	e->x = e->x_next;
	e->y = e->y_next;
}

