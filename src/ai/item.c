#include "ai_common.h"

#define left_gravity	underwater

void onspawn_energy(Entity *e) {
	if(!(e->flags & NPC_OPTION2)) {
		// Small energy
		e->display_box = (bounding_box) {{ 4,4,4,4 }};
	} else {
		// Big energy
		SHEET_FIND(e->sheet, SHEET_ENERGYL);
		e->vramindex = sheets[e->sheet].index;
		e->framesize = 4;
		e->sprite[0].size = SPRITE_SIZE(2, 2);
		e->display_box = (bounding_box) {{ 8,8,8,8 }};
	}
	e->left_gravity = (g_stage.id == STAGE_WATERWAY_BOSS || g_stage.id == STAGE_OUTER_WALL);
	e->x_speed = 0x1FF - (rand() & 0x3FF);
	e->alwaysActive = TRUE;
}

static uint8_t collide_player(Entity *e) {
	//uint16_t px = player.x >> CSF;
	//uint16_t py = player.y >> CSF;
	//uint16_t ex = e->x >> CSF;
	//uint16_t ey = e->y >> CSF;
	int32_t box = 7 + e->display_box.left;
	box <<= CSF;
	//return px > ex - box && px < ex + box && py > ey - box && py < ey + box;
	return player.x > e->x - box && player.x < e->x + box && player.y > e->y - box && player.y < e->y + box;
}

void ai_energy(Entity *e) {
	if(++e->animtime >= 4) {
		e->animtime = 0;
		if(++e->frame > 5) e->frame = 0;
	}
	e->timer++;
	if((e->timer & 1) && collide_player(e)) {
		Weapon *w = &playerWeapon[currentWeapon];
		if(w->type != WEAPON_SPUR && w->level == 3 && 
				w->energy + e->experience > weapon_info[w->type].experience[w->level-1]) {
			w->energy = weapon_info[w->type].experience[w->level-1];
		} else {
			w->energy += e->experience;
		}
		if(w->level < 3 && w->energy >= weapon_info[w->type].experience[w->level-1]) {
			sound_play(SND_LEVEL_UP, 5);
			entity_create(player.x, player.y, OBJ_LEVELUP, 0);
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
		if(e->timer > TIME_10(500)) {
			e->state = STATE_DELETE;
			return;
		} else if(e->timer > TIME_10(350)) {
			e->hidden = (e->timer & 3) > 1;
		}

		e->x += e->x_speed;
		e->y += e->y_speed;
		if(e->left_gravity) {
			e->x_speed -= SPEED_8(8);
			if(blk(e->x, -4, e->y, 0) == 0x41) e->x_speed = SPEED_8(0xFF);
		} else {
			if(e->y_speed < SPEED_10(0x3E0)) e->y_speed += SPEED_8(0x40);
			if(e->x_speed > 0) e->x_speed--;
			if(e->x_speed < 0) e->x_speed++;
			// Check below / above first
			uint8_t block_below = blk(e->x, 0, e->y, e->display_box.top);
			uint8_t block_above = blk(e->x, 0, e->y, -e->display_box.top);
			if(block_below == 0x41 || block_below == 0x43) {
				e->y_speed = -e->y_speed >> 1;
				if(e->y_speed > -SPEED_10(0x3FF)) e->y_speed = -SPEED_10(0x3FF);
				sound_play(SND_XP_BOUNCE, 0);
			} else if(block_below & BLOCK_SLOPE) {
				uint8_t index = block_below & 0xF;
				if(index >= 4) {
					uint16_t xx = sub_to_pixel(e->x);
					uint16_t yy = sub_to_pixel(e->y + 0x800);
					int8_t overlap = (yy & 15) - heightmap[index & 3][xx & 15];
					if(overlap >= 0) {
						e->y -= overlap;
						if(e->y_speed >= SPEED_10(0x200)) sound_play(SND_XP_BOUNCE, 0);
						e->y_speed = -e->y_speed;
						if(e->y_speed > -SPEED_10(0x3FF)) e->y_speed = -SPEED_10(0x3FF);
					}
				}
			} else if(block_above == 0x41 || block_above == 0x43) {
				e->y_speed = -e->y_speed >> 1;
				if(e->y_speed < SPEED_10(0x300)) e->y_speed = SPEED_10(0x300);
			}
			// Check in front
			uint8_t block_front = blk(e->x, e->x_speed > 0 ? 4 : -4, e->y, -1);
			if(block_front == 0x41 || block_front == 0x43) { // hit a wall
				e->x_speed = -e->x_speed;
			}
		}
	}
}

void onspawn_powerup(Entity *e) {
	if(e->flags & NPC_OPTION1) {
		e->alwaysActive = TRUE;
		e->frame = (e->flags & NPC_OPTION2) ? 2 : 0;
	} else {
		e->x_mark = sub_to_block(e->x);
		e->y_mark = sub_to_block(e->y);
		e->hidden = TRUE;
	}
}

void ai_missile(Entity *e) {
	if(e->flags & NPC_OPTION1) {
		if((++e->animtime & 3) == 0) e->frame ^= 1;
		if(g_stage.id == STAGE_WATERWAY_BOSS || g_stage.id == STAGE_OUTER_WALL) {
			e->x_speed -= SPEED_8(6);
			e->x += e->x_speed;
		}
		if(e->timer > TIME_10(500)) {
			e->state = STATE_DELETE;
			return;
		} else if(e->timer > TIME_10(350)) {
			e->hidden = (e->timer & 2);
		}
	} else if(!e->state) {
		// Hide the sprite when under a breakable block
		// Reduces unnecessary lag in sand zone
		if(stage_get_block_type(e->x_mark, e->y_mark) != 0x43) {
			e->hidden = FALSE;
			e->state++;
		}
	}
	// Increases missile ammo, plays sound and deletes itself
	if((++e->timer & 1) && entity_overlapping(&player, e)) {
		// Find missile or super missile
		Weapon *w = player_find_weapon(WEAPON_MISSILE);
		if(!w) w = player_find_weapon(WEAPON_SUPERMISSILE);
		// If we found either increase ammo
		if(w) {
			// OPTION2 is large pickup
			w->ammo += e->experience;
			if(w->ammo >= w->maxammo) w->ammo = w->maxammo;
		}
		sound_play(SND_GET_MISSILE, 5);
		e->state = STATE_DELETE;
	}
}

void ai_heart(Entity *e) {
	if(e->flags & NPC_OPTION1) {
		if((++e->animtime & 3) == 0) e->frame ^= 1;
		if(g_stage.id == STAGE_WATERWAY_BOSS || g_stage.id == STAGE_OUTER_WALL) {
			e->x_speed -= SPEED_8(6);
			e->x += e->x_speed;
		}
		if(e->timer > TIME_10(500)) {
			e->state = STATE_DELETE;
			return;
		} else if(e->timer > TIME_10(350)) {
			e->hidden = (e->timer & 2);
		}
	} else if(!e->state) {
		// Hide the sprite when under a breakable block
		// Reduces unnecessary lag in sand zone
		if(stage_get_block_type(e->x_mark, e->y_mark) != 0x43) {
			e->hidden = FALSE;
			e->state++;
		}
	}
	// Increases health, plays sound and deletes itself
	if((++e->timer & 1) && entity_overlapping(&player, e)) {
        player.health += e->experience;
		// Don't go over max health
		if(player.health >= playerMaxHealth) player.health = playerMaxHealth;
		sound_play(SND_HEALTH_REFILL, 5);
		e->state = STATE_DELETE;
	}
}

void onspawn_hiddenPowerup(Entity *e) {
	e->flags |= NPC_SHOOTABLE;
}

void ai_hiddenPowerup(Entity *e) {
	if(e->health < 990) {
		effect_create_smoke(sub_to_pixel(e->x), sub_to_pixel(e->y));
		sound_play(SND_EXPL_SMALL, 5);
		if(e->flags & NPC_OPTION2) {
			Entity *missile = entity_create(e->x, e->y, OBJ_MISSILE, e->flags & ~(NPC_OPTION2|NPC_SHOOTABLE));
            missile->experience = 2;
		} else {
			Entity *heart = entity_create(e->x, e->y, OBJ_HEART, e->flags & ~(NPC_SHOOTABLE));
            heart->experience = 2;
		}
		e->state = STATE_DELETE;
	}
}
