#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"
#include "input.h"
#include "system.h"
#include "resources.h"
#include "effect.h"

void ai_energy_onCreate(Entity *e) {
	if(!(e->eflags & NPC_OPTION2)) {
		e->display_box.left -= 4;
		e->display_box.right -= 4;
	}
	e->x_speed = 0x200 - (random() % 0x400);
	// Sync to global timer
	e->alwaysActive = true;
	energyCount++;
	e->state_time = (system_get_frame() + energyCount * 2) % 4;
}

void ai_energy_onUpdate(Entity *e) {
	if(entity_overlapping(&player, e)) {
		Weapon *w = &playerWeapon[currentWeapon];
		w->energy += e->experience;
		if(w->level < 3 && w->energy >= weapon_info[w->type].experience[w->level-1]) {
			sound_play(SND_LEVEL_UP, 5);
			w->energy -= weapon_info[w->type].experience[w->level-1];
			w->level++;
		} else {
			sound_play(SND_GET_XP, 5);
			player.damage_time = 30;
			player.damage_value += e->experience;
		}
		e->state = STATE_DELETE;
		energyCount--;
	} else {
		e->state_time++;
		if(e->state_time > 10 * 60) {
			e->state = STATE_DELETE;
			energyCount--;
			return;
		} else if(energyCount > 8 || e->state_time > 7 * 60) {
			SPR_SAFEVISIBILITY(e->sprite, (e->state_time & 3) > 1 ? AUTO_FAST : HIDDEN);
		} else if(!entity_on_screen(e)) {
			SPR_SAFERELEASE(e->sprite);
		} else if(e->sprite == NULL) {
			entity_sprite_create(e);
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
			sound_play(SND_XP_BOUNCE, 0);
		}
		e->x = e->x_next;
		e->y = e->y_next;
	}
}

void ai_missile_onUpdate(Entity *e) {
	// Hide the sprite when under a breakable block
	// Reduces unnecessary lag in sand zone
	if(stage_get_block_type(sub_to_block(e->x), sub_to_block(e->y)) == 0x43) {
		SPR_SAFERELEASE(e->sprite);
		return;
	} else if(e->sprite == NULL) {
		entity_sprite_create(e);
	}
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
		sound_play(SND_GET_MISSILE, 5);
		e->state = STATE_DELETE;
	} else if(e->eflags & NPC_OPTION1) {
		e->state_time++;
		if(e->state_time > 10 * 60) {
			e->state = STATE_DELETE;
			return;
		} else if(e->state_time > 7 * 60) {
			SPR_SAFEVISIBILITY(e->sprite, (e->state_time & 3) > 1 ? AUTO_FAST : HIDDEN);
		}
	}
}

void ai_heart_onUpdate(Entity *e) {
	// Hide the sprite when under a breakable block
	// Reduces unnecessary lag in sand zone
	if(stage_get_block_type(sub_to_block(e->x), sub_to_block(e->y)) == 0x43) {
		SPR_SAFERELEASE(e->sprite);
		return;
	} else if(e->sprite == NULL) {
		entity_sprite_create(e);
	}
	// Increases health, plays sound and deletes itself
	if(entity_overlapping(&player, e)) {
		player.health += e->health;
		// Don't go over max health
		if(player.health >= playerMaxHealth) player.health = playerMaxHealth;
		sound_play(SND_HEALTH_REFILL, 5);
		e->state = STATE_DELETE;
	} else if(e->eflags & NPC_OPTION1) {
		e->state_time++;
		if(e->state_time > 10 * 60) {
			e->state = STATE_DELETE;
			return;
		} else if(e->state_time > 7 * 60) {
			SPR_SAFEVISIBILITY(e->sprite, (e->state_time & 3) > 1 ? AUTO_FAST : HIDDEN);
		}
	}
}

void ai_hiddenPowerup_onCreate(Entity *e) {
	e->eflags |= NPC_SHOOTABLE;
}

void ai_hiddenPowerup_onUpdate(Entity *e) {
	if(e->health < 990) {
		effect_create_smoke(0, sub_to_pixel(e->x), sub_to_pixel(e->y));
		sound_play(SND_EXPL_SMALL, 5);
		if(e->eflags & NPC_OPTION2) {
			e->type = OBJ_MISSILE;
			SPR_SAFEADD(e->sprite, &SPR_MisslP, sub_to_pixel(e->x), sub_to_pixel(e->y),
				TILE_ATTR(PAL1, 0, 0, 0), 4);
			e->eflags &= ~NPC_OPTION2;
		} else {
			e->type = OBJ_HEART;
			SPR_SAFEADD(e->sprite, &SPR_Heart, sub_to_pixel(e->x), sub_to_pixel(e->y),
				TILE_ATTR(PAL1, 0, 0, 0), 4);
			e->health = 2;
		}
		e->eflags &= ~NPC_SHOOTABLE;
		e->nflags &= ~NPC_SHOOTABLE;
	}
}
