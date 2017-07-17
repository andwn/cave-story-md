#include "ai_common.h"

#define left_gravity	underwater

void onspawn_energy(Entity *e) {
	if(!(e->eflags & NPC_OPTION2)) {
		// Small energy
		e->display_box.left -= 4;
		e->display_box.right -= 4;
	} else {
		// Big energy
		SHEET_FIND(e->sheet, SHEET_ENERGYL);
		e->vramindex = sheets[e->sheet].index;
		e->framesize = 4;
		e->sprite[0].size = SPRITE_SIZE(2, 2);
	}
	e->left_gravity = (stageID == 0x1F || stageID == 0x35);
	e->x_speed = 0x200 - (random() % 0x400);
	e->alwaysActive = TRUE;
}

void ai_energy(Entity *e) {
	if(++e->animtime >= 4) {
		e->animtime = 0;
		if(++e->frame > 5) e->frame = 0;
	}
	if((e->timer & 1) && entity_overlapping(&player, e)) {
		Weapon *w = &playerWeapon[currentWeapon];
		if(w->level == 3 && w->energy + e->experience > 
				weapon_info[w->type].experience[w->level-1]) {
			w->energy = weapon_info[w->type].experience[w->level-1];
		} else {
			w->energy += e->experience;
		}
		if(w->level < 3 && w->energy >= weapon_info[w->type].experience[w->level-1]) {
			sound_play(SND_LEVEL_UP, 5);
			entity_create(player.x, player.y, 
						cfg_language ? OBJ_LEVELUP_JA : OBJ_LEVELUP, 0);
			w->energy -= weapon_info[w->type].experience[w->level-1];
			w->level++;
			sheets_refresh_weapon(w);
		} else {
			sound_play(SND_GET_XP, 5);
			player.damage_time = 30;
			player.damage_value += e->experience;
		}
		e->state = STATE_DELETE;
	} else {
		e->timer++;
		if(e->timer > 10 * FPS) {
			e->state = STATE_DELETE;
			return;
		} else if(e->timer > 7 * FPS) {
			e->hidden = (e->timer & 3) > 1;
		}
		if(e->left_gravity) {
			e->x_speed -= SPEED(6);
			if(blk(e->x, -4, e->y, 0) == 0x41) e->x_speed = SPEED(0x100);
		} else {
			e->y_speed += SPEED(0x12);
			if(e->x_speed > 0) e->x_speed--;
			if(e->x_speed < 0) e->x_speed++;
			if(e->y_speed > 0x400) e->y_speed = 0x400;
			// Check below / above first
			uint8_t block_below = stage_get_block_type(
					sub_to_block(e->x), sub_to_block(e->y + 0x800));
			uint8_t block_above = stage_get_block_type(
					sub_to_block(e->x), sub_to_block(e->y - 0x800));
			if(block_below == 0x41 || block_below == 0x43) {
				e->y -= sub_to_pixel(e->y + 0x800) % 16;
				e->y_speed = -e->y_speed >> 1;
				if(e->y_speed > -0x400) e->y_speed = -0x400;
				sound_play(SND_XP_BOUNCE, 0);
			} else if(block_below & BLOCK_SLOPE) {
				uint8_t index = block_below & 0xF;
				if(index >= 4) {
					uint16_t xx = sub_to_pixel(e->x);
					uint16_t yy = sub_to_pixel(e->y + 0x800);
					int8_t overlap = (yy % 16) - heightmap[index % 4][xx % 16];
					if(overlap >= 0) {
						e->y -= overlap;
						if(e->y_speed >= 0x200) sound_play(SND_XP_BOUNCE, 0);
						e->y_speed = -e->y_speed;
						if(e->y_speed > -0x400) e->y_speed = -0x400;
					}
				}
			} else if(block_above == 0x41 || block_above == 0x43) {
				e->y_speed = -e->y_speed >> 1;
				if(e->y_speed < 0x300) e->y_speed = 0x300;
			} else {
				e->y_speed += SPEED(0x50);
				//if(e->y_speed > 0x600) e->y_speed = 0x600;
			}
			// Check in front
			uint8_t block_front = stage_get_block_type(
					sub_to_block(e->x + (e->x_speed > 0 ? 0x800 : -0x800)),
					sub_to_block(e->y - 0x100));
			if(block_front == 0x41 || block_front == 0x43) { // hit a wall
				e->x_speed = -e->x_speed;
			}
		}
		e->x += e->x_speed;
		e->y += e->y_speed;
	}
}

void ai_missile(Entity *e) {
	// Hide the sprite when under a breakable block
	// Reduces unnecessary lag in sand zone
	if(stage_get_block_type(sub_to_block(e->x), sub_to_block(e->y)) == 0x43) {
		e->hidden = TRUE;
		return;
	} else {
		e->hidden = FALSE;
	}
	if(e->eflags & NPC_OPTION1) {
		if(e->eflags & NPC_OPTION2) {ANIMATE(e, 4, 2,3);}
		else  {ANIMATE(e, 4, 0,1);}
	} else e->frame = 0;
	// Dropped health/missiles should be deleted after 10 seconds, even if it is offscreen
	if(!e->state) {
		e->alwaysActive = (e->eflags & NPC_OPTION1) > 0;
		e->state = 1;
	}
	// Increases missile ammo, plays sound and deletes itself
	e->timer++;
	if((e->timer & 1) && entity_overlapping(&player, e)) {
		// Find missile or super missile
		Weapon *w = player_find_weapon(WEAPON_MISSILE);
		if(!w) w = player_find_weapon(WEAPON_SUPERMISSILE);
		// If we found either increase ammo
		if(w) {
			// OPTION2 is large pickup
			w->ammo += (e->eflags & NPC_OPTION2) ? 3 : 1;
			if(w->ammo >= w->maxammo) w->ammo = w->maxammo;
		}
		sound_play(SND_GET_MISSILE, 5);
		e->state = STATE_DELETE;
	} else {
		if(stageID == 0x1F) {
			e->x_speed -= SPEED(6);
			e->x += e->x_speed;
		}
		if(e->eflags & NPC_OPTION1) {
			if(e->timer > 10 * FPS) {
				e->state = STATE_DELETE;
				return;
			} else if(e->timer > 7 * FPS) {
				e->hidden ^= 1;
			}
		}
	}
}

void ai_heart(Entity *e) {
	// Hide the sprite when under a breakable block
	// Reduces unnecessary lag in sand zone
	if(stage_get_block_type(sub_to_block(e->x), sub_to_block(e->y)) == 0x43) {
		e->hidden = TRUE;
		return;
	} else {
		e->hidden = FALSE;
	}
	if(e->eflags & NPC_OPTION1) {
		if(e->eflags & NPC_OPTION2) {ANIMATE(e, 4, 2,3);}
		else  {ANIMATE(e, 4, 0,1);}
	} else e->frame = 0;
	// Dropped health/missiles should be deleted after 10 seconds, even if it is offscreen
	if(!e->state) {
		e->alwaysActive = (e->eflags & NPC_OPTION1) > 0;
		e->state = 1;
	}
	// Increases health, plays sound and deletes itself
	e->timer++;
	if((e->timer & 1) && entity_overlapping(&player, e)) {
		if(e->eflags & NPC_OPTION1) {
			player.health += e->health;
		} else if(e->eflags & NPC_OPTION2) {
			player.health += 5;
		} else {
			player.health += 2;
		}
		// Don't go over max health
		if(player.health >= playerMaxHealth) player.health = playerMaxHealth;
		sound_play(SND_HEALTH_REFILL, 5);
		e->state = STATE_DELETE;
	} else {
		if(stageID == 0x1F) {
			e->x_speed -= SPEED(6);
			e->x += e->x_speed;
		}
		if(e->eflags & NPC_OPTION1) {
			if(e->timer > 10 * FPS) {
				e->state = STATE_DELETE;
				return;
			} else if(e->timer > 7 * FPS) {
				e->hidden ^= 1;
			}
		}
	}
}

void onspawn_hiddenPowerup(Entity *e) {
	e->eflags |= NPC_SHOOTABLE;
}

void ai_hiddenPowerup(Entity *e) {
	if(e->health < 990) {
		effect_create_smoke(sub_to_pixel(e->x), sub_to_pixel(e->y));
		sound_play(SND_EXPL_SMALL, 5);
		if(e->eflags & NPC_OPTION2) {
			entity_create(e->x, e->y, OBJ_MISSILE, e->eflags & ~(NPC_OPTION2|NPC_SHOOTABLE));
		} else {
			entity_create(e->x, e->y, OBJ_HEART, e->eflags & ~(NPC_SHOOTABLE))->health = 2;
		}
		e->state = STATE_DELETE;
	}
}
