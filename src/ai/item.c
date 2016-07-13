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
	if(entity_overlapping(&player, e)) {
		Weapon *w = &playerWeapon[currentWeapon];
		w->energy += e->experience;
		if(w->level < 3 && w->energy >= weapon_info[w->type].experience[w->level-1]) {
			sound_play(SOUND_LEVELUP, 5);
			w->energy -= weapon_info[w->type].experience[w->level-1];
			w->level++;
		} else {
			sound_play(SOUND_GETEXP, 5);
			player.damage_time = 30;
			player.damage_value += e->experience;
		}
		e->state = STATE_DELETE;
	} else {
		if(++e->state_time > 15 * 60) {
			e->state = STATE_DELETE;
			return;
		} else if(e->state_time > 12 * 60) {
			SPR_SAFEVISIBILITY(e->sprite, (e->state_time & 3) > 1 ? VISIBLE : HIDDEN);
		}
		e->y_speed += GRAVITY;
		e->x_next = e->x + e->x_speed;
		e->y_next = e->y + e->y_speed;
		// Reverse direction when hitting a wall
		s16 xsp = e->x_speed;
		if(e->x_speed < 0) {
			collide_stage_leftwall(e);
		} else {
			collide_stage_rightwall(e);
		}
		if(e->x_speed == 0) {
			e->direction = !e->direction;
			e->x_speed = -xsp;
		}
		// Bounce when hitting the ground
		if(collide_stage_floor(e)) {
			e->y_speed = pixel_to_sub(-2);
			e->grounded = false;
			sound_play(SOUND_EXPBOUNCE, 0);
		}
		e->x = e->x_next;
		e->y = e->y_next;
	}
}

void ai_missile_onUpdate(Entity *e) {
	// Increases missile ammo, plays sound and deletes itself
	if(entity_overlapping(&player, e)) {
		// Find missile or super missile
		Weapon *w = player_find_weapon(WEAPON_MISSILE);
		if(w == NULL) w = player_find_weapon(WEAPON_SUPERMISSILE);
		// If we found either increase ammo
		if(w != NULL) {
			// OPTION2 is large pickup
			w->ammo += (e->eflags & NPC_OPTION2) ? 3 : 1;
			if(w->ammo >= w->maxammo) w->ammo = w->maxammo;
		}
		sound_play(0x2A, 5);
		e->state = STATE_DELETE;
	}
}

void ai_heart_onUpdate(Entity *e) {
	// Increases health, plays sound and deletes itself
	if(entity_overlapping(&player, e)) {
		player.health += e->health;
		// Don't go over max health
		if(player.health >= playerMaxHealth) player.health = playerMaxHealth;
		sound_play(SOUND_REFILL, 5);
		e->state = STATE_DELETE;
	}
}
