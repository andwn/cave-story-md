#include "common.h"

#include "audio.h"
#include "camera.h"
#include "dma.h"
#include "effect.h"
#include "entity.h"
#include "joy.h"
#include "npc.h"
#include "player.h"
#include "resources.h"
#include "sheet.h"
#include "stage.h"
#include "system.h"
#include "tables.h"
#include "tools.h"
#include "vdp.h"

#include "weapon.h"

#define FIREDIR	(playerMoveMode ? RIGHT : joy_down(BUTTON_UP) ? UP :                           \
				 (!player.grounded && joy_down(BUTTON_DOWN)) ? DOWN : player.dir)

const WeaponFunc weapon_fire_array[WEAPON_COUNT] = {
	&weapon_fire_none,
	&weapon_fire_snake,
	&weapon_fire_polarstar,
	&weapon_fire_fireball,
	&weapon_fire_machinegun,
	&weapon_fire_missile,
	&weapon_fire_none,
	&weapon_fire_bubbler,
	&weapon_fire_none,
	&weapon_fire_blade,
	&weapon_fire_supermissile,
	&weapon_fire_none,
	&weapon_fire_nemesis,
	&weapon_fire_spur
};

const BulletFunc bullet_update_array[WEAPON_COUNT] = {
	&bullet_update_none,
	&bullet_update_snake,
	&bullet_update_polarstar,
	&bullet_update_fireball,
	&bullet_update_machinegun,
	&bullet_update_missile,
	&bullet_update_none,
	&bullet_update_bubbler,
	&bullet_update_blade_slash,
	&bullet_update_blade,
	&bullet_update_supermissile,
	&bullet_update_spur_tail,
	&bullet_update_nemesis,
	&bullet_update_spur
};

// Based on similar table from NXengine
typedef struct {
	int16_t maxspeed;
	int16_t accel;
	uint8_t eff_range;
	uint8_t boom_time;
} MissileSettings;
static const MissileSettings missile_settings[2][6] = {
	{ // NTSC
	//  Level 1-3 regular missile
	//  have on record here for damage 4, 6, 4; check if that's correct
	//    maxspd        accel        range, time
		{ 0xA00*5/6,  0x80*5/6,  16,	10,	},
		{ 0xA00*5/6,  0x100*5/6, 32,	15,	},
		{ 0xA00*5/6,  0x80*5/6,  40,	5,	},
	//  Level 1-3 super missile
	//    maxspd        accel        range, time
		{ 0x1400*5/6, 0x200*5/6, 16,	10,	},
		{ 0x1400*5/6, 0x200*5/6, 32,	16,	},
		{ 0x1400*5/6, 0x200*5/6, 40,	4, },
	}, { // PAL
	//  Level 1-3 regular missile
	//  have on record here for damage 4, 6, 4; check if that's correct
	//    maxspd  accel  range, time
		{ 0xA00,  0x80,  16,	10,	},
		{ 0xA00,  0x100, 32,	15,	},
		{ 0xA00,  0x80,  40,	5,	},
	//  Level 1-3 super missile
	//    maxspd  accel  range, time
		{ 0x1400, 0x200, 16,	10,	},
		{ 0x1400, 0x200, 32,	16,	},
		{ 0x1400, 0x200, 40,	4, },
	}
};

Weapon playerWeapon[MAX_WEAPONS] = {};
Bullet playerBullet[MAX_BULLETS] = {};

static void bullet_destroy_block(uint16_t x, uint16_t y);
static void create_blade_slash(Bullet *b, uint8_t burst);

static inline void set_extent_box(Bullet *b) {
	b->extent = (extent_box) {
		.x1 = (b->x >> CSF) - (b->hit_box.left),
		.y1 = (b->y >> CSF) - (b->hit_box.top),
		.x2 = (b->x >> CSF) + (b->hit_box.right),
		.y2 = (b->y >> CSF) + (b->hit_box.bottom),
	};
}

void weapon_fire_none(Weapon *w) { (void)(w); }

static const uint8_t snake_ttl[4] = { 0, 20, 23, 30 };
static const uint16_t snake_speed[4] = { 0, 0x600, 0x200, 0x200 };
static uint8_t wave_dir = 0;

void weapon_fire_snake(Weapon *w) {
	Bullet *b = NULL;
	uint8_t i;
	for(i = 2 + (3 - w->level); i < 6; i++) {
		if(playerBullet[i].ttl > 0) continue;
		b = &playerBullet[i];
		break;
	}
	if(b == NULL) return;
	sound_play(SND_SNAKE_FIRE, 5);
	b->type = w->type;
	b->level = w->level;
	b->sheet = w->sheet;
	b->sprite = (VDPSprite) {
		.size = SPRITE_SIZE(2, 2),
		.attr = TILE_ATTR(PAL1,0,0,0,sheets[b->sheet].index)
	};
	b->damage = 2 + (b->level << 1); // 4, 6, 8
	b->ttl = TIME_8(snake_ttl[b->level]);
	b->hit_box = (bounding_box) { 4, 4, 4, 4 };
	
	b->dir = FIREDIR;
	if(b->dir == UP) {
		b->x = player.x;
		b->y = player.y - pixel_to_sub(10);
		b->x_speed = 0;
		b->y_speed = -SPEED_12(snake_speed[b->level]);
	} else if(b->dir == DOWN) {
		b->x = player.x;
		b->y = player.y + pixel_to_sub(10);
		b->x_speed = 0;
		b->y_speed = SPEED_12(snake_speed[b->level]);
	} else {
		b->x = player.x + (b->dir ? pixel_to_sub(8) : -pixel_to_sub(8));
		b->y = player.y + pixel_to_sub(2);
		b->x_speed = b->dir ? SPEED_12(snake_speed[b->level]) : -SPEED_12(snake_speed[b->level]);
		b->y_speed = 0;
	}
	if(b->level > 1) {
		// start moving off at an angle to our direction.
		// whether we start off going up or down alternates with each shot.
		int16_t wavespeed = (wave_dir & 1) ? -SPEED_10(0x3FF) : SPEED_10(0x3FF);
		wave_dir ^= 1;
		if(b->dir == LEFT || b->dir == RIGHT) {
			b->y_speed = wavespeed;
		} else {
			b->x_speed = wavespeed;
		}
		b->state = 2;
	}
	set_extent_box(b);
}

static const uint8_t pstar_ttl[4] = { 0, 8, 12, 16 };

void weapon_fire_polarstar(Weapon *w) {
	// Use first 3 slots for polar star
	// Max bullets for lv 1,2,3: 3,3,2
	Bullet *b = NULL;
	for(uint8_t i = 0 + (w->level == 3 ? 1 : 0); i < 3; i++) {
		if(playerBullet[i].ttl > 0) continue;
		b = &playerBullet[i];
		break;
	}
	if(!b) return;
	if(w->level == 3) sound_play(SND_POLAR_STAR_L3, 5);
	else sound_play(SND_POLAR_STAR_L1_2, 5);
	b->type = WEAPON_POLARSTAR;
	b->level = w->level;
	b->sprite = (VDPSprite) { .size = SPRITE_SIZE(2, 2) };
	b->damage = w->level + (w->level == 3 ? 1 : 0); // 1, 2, 4
	b->ttl = TIME_8(pstar_ttl[b->level]);
	// Polar Star or Spur?
	if(w->type == WEAPON_POLARSTAR) {
		b->sheet = w->sheet;
	} else {
		SHEET_FIND(b->sheet, SHEET_PSTAR);
	}
	
	b->dir = FIREDIR;
	if(b->dir == UP) {
		b->sprite.attr = TILE_ATTR(PAL0,0,0,0,sheets[b->sheet].index+4);
		b->x = player.x;
		b->y = player.y - pixel_to_sub(8);
		b->x_speed = 0;
		b->y_speed = -SPEED_12(0xFFF);
		b->hit_box = (bounding_box) { 1 + b->level, 6, 1 + b->level, 6 };
	} else if(b->dir == DOWN) {
		b->sprite.attr = TILE_ATTR(PAL0,0,0,0,sheets[b->sheet].index+4);
		b->x = player.x;
		b->y = player.y + pixel_to_sub(8);
		b->x_speed = 0;
		b->y_speed = SPEED_12(0xFFF);
		b->hit_box = (bounding_box) { 1 + b->level, 6, 1 + b->level, 6 };
	} else {
		b->sprite.attr = TILE_ATTR(PAL0,0,0,0,sheets[b->sheet].index);
		b->x = player.x + (b->dir ? pixel_to_sub(8) : -pixel_to_sub(8));
		b->y = player.y + pixel_to_sub(3);
		b->x_speed = (b->dir ? SPEED_12(0xFFF) : -SPEED_12(0xFFF));
		b->y_speed = 0;
		b->hit_box = (bounding_box) { 6, 1 + b->level, 6, 1 + b->level };
	}
	set_extent_box(b);
}

void weapon_fire_fireball(Weapon *w) {
	// The real cave story stores bullets for weapons in separate lists most likely,
	// since you can switch back and forth to get 3 polar star and 3 fireball bullets.
	// I try to simulate this by storing fireball bullets in a different part of the array
	// Max bullets for lv 1,2,3: 2,3,4
	Bullet *b = NULL;
	for(uint8_t i = 2 + (3 - w->level); i < 6; i++) {
		if(playerBullet[i].ttl > 0) continue;
		b = &playerBullet[i];
		break;
	}
	if(b == NULL) return;
	sound_play(SND_FIREBALL, 5);
	b->type = w->type;
	b->level = w->level;
	b->sprite = (VDPSprite) {
		.size = SPRITE_SIZE(2, 2),
		.attr = TILE_ATTR(PAL1,0,0,0,sheets[w->sheet].index)
	};
	b->damage = b->level << 1; // 2, 4, 6
	b->ttl = TIME_8(100);
	b->sheet = w->sheet;
	b->hit_box = (bounding_box) { 4, 4, 4, 4 };
	
	b->dir = FIREDIR;
	if(b->dir == UP) {
		b->x = player.x;
		b->y = player.y - pixel_to_sub(12);
		b->x_speed = player.x_speed + (player.dir ? SPEED_8(0x80) : -SPEED_8(0x80));
		b->dir = player.dir;
		b->y_speed = -SPEED_12(0x5FF);
	} else if(b->dir == DOWN) {
		b->x = player.x;
		b->y = player.y + pixel_to_sub(12);
		b->x_speed = player.x_speed + (player.dir ? SPEED_8(0x80) : -SPEED_8(0x80));
		b->dir = player.dir;
		b->y_speed = SPEED_12(0x5FF);
	} else {
		b->x = player.x + (b->dir ? pixel_to_sub(8) : -pixel_to_sub(8));
		b->y = player.y + pixel_to_sub(4);
		b->x_speed = b->dir ? SPEED_12(0x400) : -SPEED_12(0x400);
		b->y_speed = 0;
	}
	set_extent_box(b);
}

void weapon_fire_machinegun(Weapon *w) {
	// Use all 10 slots for machine gun
	Bullet *b = NULL;
	for(uint8_t i = 0; i < MAX_BULLETS; i++) {
		if(playerBullet[i].ttl > 0) continue;
		b = &playerBullet[i];
		break;
	}
	if(!b) return;
	if(w->level == 3) sound_play(SND_POLAR_STAR_L3, 5);
	else sound_play(SND_POLAR_STAR_L1_2, 5);
	b->type = w->type;
	b->level = w->level;
	b->sprite = (VDPSprite) { .size = SPRITE_SIZE(2, 2), };
	b->damage = b->level << 1; // 2, 4, 6
	b->ttl = TIME_8(20);
	b->sheet = w->sheet;
	b->hit_box = (bounding_box) { 4, 1 + w->level, 4, 1 + w->level };
	// check up/down first
	b->dir = FIREDIR;
	if(b->dir == UP) {
		b->sprite.attr = TILE_ATTR(PAL0,0,0,0,sheets[w->sheet].index+8);
		if(!player.grounded && w->level == 3) {
			player.y_speed += SPEED_10(0x240);
			if(player.y_speed > SPEED_12(0x600)) player.y_speed = SPEED_12(0x600);
		}
		b->x = player.x;
		b->y = player.y - pixel_to_sub(12);
		b->x_speed = -0x7F + (random() & 0xFF);
		b->y_speed = -SPEED_12(0xFFF);
	} else if(b->dir == DOWN) {
		b->sprite.attr = TILE_ATTR(PAL0,0,1,0,sheets[w->sheet].index+8);
		if(w->level == 3) {
			if(joy_down(btn[cfg_btn_jump]) || player.y_speed > 0) {
				player.y_speed -= SPEED_10(0x3FF);
				if(player.y_speed < -SPEED_12(0x5FF)) player.y_speed = -SPEED_12(0x5FF);
			} else {
				player.y_speed -= SPEED_10(0x380);
				if(player.y_speed < -SPEED_12(0x4FF)) player.y_speed = -SPEED_12(0x4FF);
			}
		}
		b->x = player.x;
		b->y = player.y + pixel_to_sub(12);
		b->x_speed = -0x7F + (random() & 0xFF);
		b->y_speed = SPEED_12(0xFFF);
	} else {
		//b->level++; // Wonky use of this var, so the trail knows whether to be H/V
		b->sprite.attr = TILE_ATTR(PAL0,0,0,b->dir,sheets[w->sheet].index);
		b->x = player.x + (b->dir ? pixel_to_sub(10) : -pixel_to_sub(10));
		b->y = player.y + pixel_to_sub(3);
		b->x_speed = (b->dir ? SPEED_12(0xFFF) : -SPEED_12(0xFFF));
		b->y_speed = -0x7F + (random() & 0xFF);
	}
	set_extent_box(b);
}

void weapon_fire_missile(Weapon *w) {
	// Launch 3 for level 3 missile
	for(uint8_t count = 0; count < (w->level == 3 ? 3 : 1); count++) {
		// Use end of array for missiles
		// Max bullets for lv 1,2,3: 1,2,6
		Bullet *b = NULL;
		for(uint8_t i = 4 + (w->level == 1 ? 5 : w->level == 2 ? 4 : 0); i < 10; i++) {
			if(playerBullet[i].ttl > 0) continue;
			b = &playerBullet[i];
			break;
		}
		if(!b) return;
		// Only consume 1 ammo for Lv 3
		if(count == 0) {
			if(w->ammo) w->ammo--;
			else if(w->maxammo) { // Out of ammo
				sound_play(SND_GUN_CLICK, 5);
				if(!missileEmptyFlag) {
					missileEmptyFlag = TRUE;
					entity_create(player.x, player.y, cfg_language == LANG_JA ? OBJ_EMPTY_JA : OBJ_EMPTY, 0);
				}
				return;
			}
		}
		missileEmptyFlag = FALSE;

		b->type = w->type;
		b->level = w->level;
		b->sprite = (VDPSprite) { .size = SPRITE_SIZE(2, 2), };
		b->hit_box = (bounding_box) { 4, 4, 4, 4 };
		b->sheet = w->sheet;
		b->damage = (b->type == WEAPON_SUPERMISSILE) ? 2 : 1;
		b->hits = 0;
		b->state = 0;
		b->ttl = TIME_8(100);
		b->x_speed = 0;
		b->y_speed = 0;
		
		b->dir = FIREDIR;
		if(b->dir == UP) {
			b->sprite.attr = TILE_ATTR(PAL0,0,0,player.dir,sheets[w->sheet].index+4);
			b->x = player.x;
			b->y = player.y - pixel_to_sub(12);
			if(b->level == 3) {
				b->x_speed = (random() & 0xFFF) - 0x7FF;
				b->y_speed = (random() & 0x3FF) - 0x1FF;
			}
		} else if(b->dir == DOWN) {
			b->sprite.attr = TILE_ATTR(PAL0,1,0,player.dir,sheets[w->sheet].index+4);
			b->x = player.x;
			b->y = player.y + pixel_to_sub(12);
			if(b->level == 3) {
				b->x_speed = (random() & 0xFFF) - 0x7FF;
				b->y_speed = (random() & 0x3FF) - 0x1FF;
			}
		} else {
			b->sprite.attr = TILE_ATTR(PAL0,0,0,b->dir,sheets[w->sheet].index);
			b->x = player.x + (b->dir ? pixel_to_sub(12) : -pixel_to_sub(12));
			b->y = player.y + pixel_to_sub(2);
			if(b->level == 3) {
				b->y_speed = (random() & 0xFFF) - 0x7FF;
				b->x_speed = (random() & 0x3FF) - 0x1FF;
			}
		}
		set_extent_box(b);
	}
}

void weapon_fire_bubbler(Weapon *w) {
	// Limit level 1 to 4 bubbles, others use whole array
	uint8_t limit = w->level == 1 ? 4 : 10;
	Bullet *b = NULL;
	for(uint8_t i = 0; i < limit; i++) {
		if(playerBullet[i].ttl > 0) continue;
		b = &playerBullet[i];
		break;
	}
	if(!b) return;
	if(w->ammo) {
		missileEmptyFlag = FALSE;
		w->ammo--;
	} else if(w->maxammo) {
		if(!missileEmptyFlag) {
			missileEmptyFlag = TRUE;
			entity_create(player.x, player.y, cfg_language == LANG_JA ? OBJ_EMPTY_JA : OBJ_EMPTY, 0);
		}
		sound_play(SND_GUN_CLICK, 5);
		return;
	}
	b->type = w->type;
	b->level = w->level;
	b->sheet = w->sheet;
	b->hit_box = (bounding_box) { 3, 3, 3, 3 };
	b->sprite = (VDPSprite) { 
		.size = SPRITE_SIZE(1, 1),
		.attr = TILE_ATTR(PAL0,0,0,0,sheets[w->sheet].index)
	};
	sound_play(SND_BUBBLER_FIRE, 5);
	// Starting velocity based on level
	int16_t fwdspeed = 0, sidespeed = 0;
	switch(b->level) {
		case 1:
			b->damage = 1;
			b->ttl = TIME_8(40);
			fwdspeed = SPEED_10(0x3FF); 
			break;
		case 2: 
			b->damage = 2;
			b->ttl = TIME_8(60);
			fwdspeed = SPEED_12(0x600);
			sidespeed = -0xFF + (random() & 0x1FF);
			break;
		case 3:
			b->damage = 5;
			b->ttl = TIME_8(100);
			fwdspeed = 0x1FF + (random() & 0x1FF);
			sidespeed = (random() & 0x7FF) - 0x3FF;
			break;
	}
	b->dir = FIREDIR;
	switch(b->dir) {
		case LEFT:
			b->x = player.x - (8 << CSF);
			b->y = player.y + (3 << CSF);
			b->x_speed = -fwdspeed;
			b->y_speed = sidespeed;
			break;
		case RIGHT:
			b->x = player.x + (8 << CSF);
			b->y = player.y + (3 << CSF);
			b->x_speed = fwdspeed;
			b->y_speed = sidespeed;
			break;
		case UP:
			b->x = player.x;
			b->y = player.y - (10 << CSF);
			b->x_speed = sidespeed;
			b->y_speed = -fwdspeed;
			break;
		case DOWN:
			b->x = player.x;
			b->y = player.y + (10 << CSF);
			b->x_speed = sidespeed;
			b->y_speed = fwdspeed;
			break;
	}
	set_extent_box(b);
}

void weapon_fire_blade(Weapon *w) {
	// Blade is only 1 bullet at a time for every level
	Bullet *b = &playerBullet[0];
	if(b->ttl) return;
	b->type = w->type;
	b->level = w->level;
	b->sheet = w->sheet;
	b->hits = 0;
	sound_play(SND_FIREBALL, 5);
	switch(b->level) {
		case 1: // Small 16x16 Blade
			b->sprite = (VDPSprite) { .size = SPRITE_SIZE(2, 2) };
			b->damage = 15;
			b->ttl = TIME_8(30);
			b->hit_box = (bounding_box) { 6, 6, 6, 6 };
			break;
		case 2: // Large 24x24 Blade, hits 6 times
			b->sprite = (VDPSprite) { .size = SPRITE_SIZE(3, 3) };
			b->damage = 18;
			b->ttl = TIME_8(18);
			b->hit_box = (bounding_box) { 10, 10, 10, 10 };
			break;
		case 3: // King's Ghost, AOE on hit
			b->sprite = (VDPSprite) { .size = SPRITE_SIZE(3, 3) };
			b->damage = 1;
			b->ttl = TIME_8(30);
			b->hit_box = (bounding_box) { 8, 8, 8, 8 };
			break;
	}
	b->x = player.x;
	b->y = player.y - (3 << CSF);
	b->sprite.attr = TILE_ATTR(PAL0,0,0,0,sheets[w->sheet].index);
	b->dir = FIREDIR;
	if(b->dir == UP) {
		if(b->level == 3) TILES_QUEUE(SPR_TILES(&SPR_BladeB3k, 0, 1), b->sprite.attr, 9);
		b->x_speed = 0;
		b->y_speed = -SPEED_12(0x800);
	} else if(b->dir == DOWN) {
		if(b->level == 3) TILES_QUEUE(SPR_TILES(&SPR_BladeB3k, 0, 2), b->sprite.attr, 9);
		b->x_speed = 0;
		b->y_speed = SPEED_12(0x800);
	} else {
		if(b->level == 3) {
			TILES_QUEUE(SPR_TILES(&SPR_BladeB3k, 0, 0), b->sprite.attr, 9);
			if(b->dir == RIGHT) sprite_hflip(b->sprite, 1);
		}
		// Spawn slightly behind the player
		if(b->level == 2) {
			b->x += b->dir ? -(3 << CSF) : (3 << CSF);
		} else {
			b->x += b->dir ? -(6 << CSF) : (6 << CSF);
		}
		b->x_speed = (b->dir ? SPEED_12(0x800) : -SPEED_12(0x800));
		b->y_speed = 0;
	}
	set_extent_box(b);
}

void weapon_fire_supermissile(Weapon *w) {
	weapon_fire_missile(w);
}

void weapon_fire_nemesis(Weapon *w) {
	// Use last 2 slots, less likely to conflict with other weapons
	Bullet *b = NULL;
	for(uint8_t i = 8; i < 10; i++) {
		if(playerBullet[i].ttl > 0) continue;
		b = &playerBullet[i];
		break;
	}
	if(!b) return;
	b->type = w->type;
	b->level = w->level;
	b->sheet = w->sheet;
	b->ttl = TIME_8(20);
	b->state = 0;
	uint16_t speed = 0;
	switch(b->level) {
		case 1:
		b->damage = 12;
		speed = SPEED_12(0xFFF);
		sound_play(SND_NEMESIS_FIRE, 5);
		break;
		case 2:
		b->damage = 6;
		speed = SPEED_12(0xFFF);
		sound_play(SND_POLAR_STAR_L3, 5);
		break;
		case 3:
		b->damage = 1;
		speed = SPEED_12(0x555);
		break;
	}
	b->dir = FIREDIR;
	switch(b->dir) {
		case LEFT:
		case RIGHT:
		b->sprite.size = SPRITE_SIZE(3, 2);
		b->sprite.attr = TILE_ATTR(PAL0,0,0,(b->dir&1),sheets[b->sheet].index);
		b->x = player.x + ((b->dir&1) ? pixel_to_sub(12) : -pixel_to_sub(12));
		b->y = player.y + pixel_to_sub(1);
		b->x_speed = ((b->dir&1) ? speed : -speed);
		b->y_speed = 0;
		b->hit_box = (bounding_box) { 10, 6, 10, 6 };
		break;
		case UP:
		case DOWN:
		b->sprite.size = SPRITE_SIZE(2, 3);
		b->sprite.attr = TILE_ATTR(PAL0,0,(b->dir&1),0,TILE_NEMINDEX);
		b->x = player.x - (4<<CSF);
		b->y = player.y + ((b->dir&1) ? pixel_to_sub(12) : -pixel_to_sub(12));
		b->x_speed = 0;
		b->y_speed = ((b->dir&1) ? speed : -speed);
		b->hit_box = (bounding_box) { 6, 10, 6, 10 };
		break;
	}
	b->last_hit[0] = NULL;
    b->last_hit[1] = NULL;
	set_extent_box(b);
}

static int16_t spur_xmark;
static int16_t spur_ymark;

void weapon_fire_spur(Weapon *w) {
	// Uncharged shot (Polar Star Lv3)
	if(w->level == 1) {
		uint8_t lv = w->level;
		w->level = 3;
		weapon_fire_polarstar(w);
		w->level = lv;
		return;
	}
	// Charged shot
	Bullet *b = NULL;
	for(uint8_t i = 0; i < MAX_BULLETS; i++) {
		if(playerBullet[i].ttl > 0) continue;
		b = &playerBullet[i];
		break;
	}
	if(!b) return;
	// Reload the sprite sheet (bullet looks different depending on charge)
	w->level--;
	sheets_refresh_weapon(w);
	// Since Lv1 is a special case for the polar star shot, and Lv4 is a special
	// case for max charge, subtract 1 to get the "real" level
	b->level = w->level;
	w->level++;
	
	b->type = w->type;
	b->sprite = (VDPSprite) { .size = SPRITE_SIZE(2, 2) };
	b->sheet = w->sheet;
	//b->hit_box = (bounding_box) { 4, 4, 4, 4 };
	b->ttl = TIME_8(30);
	b->damage = b->level << 2; // 4, 8, 12
	b->hits = 0;
	b->state = 0;
	sound_play(SND_SPUR_FIRE_1 + b->level - 1, 5);
	b->dir = FIREDIR;
	if(b->dir == UP) {
		b->sprite.attr = TILE_ATTR(PAL0,0,0,0,sheets[b->sheet].index+8);
		b->x = player.x;
		b->y = player.y - pixel_to_sub(8);
		b->x_speed = 0;
		b->y_speed = -SPEED_12(0xFFF);
		b->hit_box = (bounding_box) { 2 + b->level, 6, 2 + b->level, 6 };
	} else if(b->dir == DOWN) {
		b->sprite.attr = TILE_ATTR(PAL0,0,0,0,sheets[b->sheet].index+8);
		b->x = player.x;
		b->y = player.y + pixel_to_sub(8);
		b->x_speed = 0;
		b->y_speed = SPEED_12(0xFFF);
		b->hit_box = (bounding_box) { 2 + b->level, 6, 2 + b->level, 6 };
	} else {
		b->sprite.attr = TILE_ATTR(PAL0,0,0,0,sheets[b->sheet].index);
		b->x = player.x + (b->dir ? pixel_to_sub(8) : -pixel_to_sub(8));
		b->y = player.y + pixel_to_sub(2);
		b->x_speed = (b->dir ? SPEED_12(0xFFF) : -SPEED_12(0xFFF));
		b->y_speed = 0;
		b->hit_box = (bounding_box) { 6, 2 + b->level, 6, 2 + b->level };
	}
	spur_xmark = b->x >> CSF;
	spur_ymark = b->y >> CSF;
	set_extent_box(b);
}

void bullet_update_none(Bullet *b) { (void)(b); }

void bullet_update_snake(Bullet *b) {
	b->ttl--;
	uint16_t index = 0;
	if(b->level > 1) {
		// accelerate the shot
		switch(b->dir) {
			case LEFT:  b->x_speed -= SPEED_8(0x80); break;
			case UP:    b->y_speed -= SPEED_8(0x80); break;
			case RIGHT: b->x_speed += SPEED_8(0x80); break;
			case DOWN:  b->y_speed += SPEED_8(0x80); break;
		}
		// periodically abruptly change the wave's direction
		// use "state" as a timer
		if((++b->state > TIME_8(5))) {
			b->state = 0;
			if(b->dir == LEFT || b->dir == RIGHT) {
				b->y_speed = -b->y_speed;
			} else {
				b->x_speed = -b->x_speed;
			}
		}
	}
	// spin in shot direction
	if(b->dir == LEFT) {
		index = 12 - ((b->ttl & 3) << 2);
	} else {
		index = (b->ttl & 3) << 2;
	}
	if(index >= 12) index = 0;

	b->x += b->x_speed;
	b->y += b->y_speed;
	sprite_pos(b->sprite,
		sub_to_pixel(b->x - camera.x) + SCREEN_HALF_W - 8,
		sub_to_pixel(b->y - camera.y) + SCREEN_HALF_H - 8);
	sprite_index(b->sprite, sheets[b->sheet].index + index);
	vdp_sprite_add(&b->sprite);
    if(b->level > 1 && (b->ttl & 3) == 0) {
        effect_create_misc(EFF_SNAKETRAIL, b->x >> CSF, b->y >> CSF, FALSE);
    };
}

void bullet_update_polarstar(Bullet *b) {
	b->x += b->x_speed;
	b->y += b->y_speed;
	uint16_t bx = sub_to_pixel(b->x);
    uint16_t by = sub_to_pixel(b->y);
	uint8_t block = stage_get_block_type(pixel_to_block(bx), pixel_to_block(by));
	// Check if bullet is colliding with a breakable block
	if(block == 0x43) {
		b->ttl = 0;
		bullet_destroy_block(pixel_to_block(bx), pixel_to_block(by));
		effect_create_smoke(bx, by);
		sound_play(SND_BLOCK_DESTROY, 5);
	} else if(block == 0x41) { // Bullet hit a wall
		b->ttl = 0;
		sound_play(SND_SHOT_HIT, 3);
        effect_create_misc(EFF_PSTAR_HIT, bx, by, FALSE);
	} else if(block & BLOCK_SLOPE) {
        int8_t height = heightmap[block & 3][bx & 15];
        if(block & 4) height = -height;
        int8_t overlap = (by & 15) + height;
        if(overlap > 0) {
            b->ttl = 0;
            sound_play(SND_SHOT_HIT, 3);
            effect_create_misc(EFF_PSTAR_HIT, bx, by, FALSE);
        }
    } else {
		sprite_pos(b->sprite, 
			sub_to_pixel(b->x - camera.x) + SCREEN_HALF_W - 8,
			sub_to_pixel(b->y - camera.y) + SCREEN_HALF_H - 8);
		vdp_sprite_add(&b->sprite);
		if(!--b->ttl) {
		    effect_create_misc(EFF_PSTAR_HIT, bx, by, FALSE);
		}
	}
}

void bullet_update_fireball(Bullet *b) {
	// Check below / above first
	uint8_t block_below = blk(b->x, 0, b->y, 6);
	uint8_t block_above = b->y_speed < 0 ? blk(b->x, 0, b->y, -6) : 0;
	if(block_below == 0x41 || block_below == 0x43) {
		//b->y -= sub_to_pixel(b->y + 0x800) & 15;
		b->y_speed = -b->y_speed >> 1;
		if(b->y_speed > -SPEED_10(0x3FF)) b->y_speed = -SPEED_10(0x3FF);
	} else if(block_below & BLOCK_SLOPE) {
		uint8_t index = block_below & 0xF;
		if(index >= 4) {
			uint16_t xx = sub_to_pixel(b->x);
			uint16_t yy = sub_to_pixel(b->y + 0xA00);
			int8_t overlap = (yy & 15) - heightmap[index & 3][xx & 15];
			if(overlap >= 0) {
				b->y -= overlap;
				b->y_speed = -b->y_speed;
				//if(b->y_speed > -SPEED_10(0x3FF)) 
				b->y_speed = -SPEED_10(0x3FF);
			}
		}
	} else if(block_above == 0x41 || block_above == 0x43) {
		b->y_speed = -b->y_speed >> 1;
		if(b->y_speed < SPEED_8(0xFF)) b->y_speed = SPEED_8(0xFF);
	} else {
		if(b->y_speed < SPEED_12(0x3C0)) b->y_speed += SPEED_8(0x55);
	}
	// Check in front
	uint8_t block_front = blk(b->x, b->x_speed > 0 ? 6 : -6, b->y, -1);
	if(block_front == 0x41 || block_front == 0x43) { // Bullet hit a wall
		b->x_speed = -b->x_speed;
	}
	b->x += b->x_speed;
	b->y += b->y_speed;
	sprite_pos(b->sprite, 
		sub_to_pixel(b->x - camera.x) + SCREEN_HALF_W - 8,
		sub_to_pixel(b->y - camera.y) + SCREEN_HALF_H - 8);
	uint16_t index = (b->ttl & 3) << 2;
	sprite_index(b->sprite, sheets[b->sheet].index + (index < 12 ? index : 0));
	vdp_sprite_add(&b->sprite);
	b->ttl--;
	if(b->level > 1 && (b->ttl & 3) == 0) {
	    effect_create_misc(EFF_FIRETRAIL, b->x >> CSF, b->y >> CSF, FALSE);
	};
}

void bullet_update_machinegun(Bullet *b) {
	b->x += b->x_speed;
	b->y += b->y_speed;
	uint8_t block = blk(b->x, 0, b->y, 0);
	if(block == 0x43) {
		b->ttl = 0;
		bullet_destroy_block(sub_to_block(b->x), sub_to_block(b->y));
		effect_create_smoke(sub_to_pixel(b->x), sub_to_pixel(b->y));
		sound_play(SND_BLOCK_DESTROY, 5);
	} else if(block == 0x41) { // Bullet hit a wall
		b->ttl = 0;
		sound_play(SND_SHOT_HIT, 3);
        //effect_create_misc(EFF_MGUN_HIT, (b->x >> CSF) - 4, (b->y >> CSF) - 4, FALSE);
	} else {
		sprite_pos(b->sprite,
			sub_to_pixel(b->x - camera.x) + SCREEN_HALF_W - 8,
			sub_to_pixel(b->y - camera.y) + SCREEN_HALF_H - 8);
		// Expand sprite of level 3 after a couple frames
		if(b->level == 3) {
			if(b->ttl == TIME_8(18)) {
				if(b->dir == UP || b->dir == DOWN) {
					b->sprite.size = SPRITE_SIZE(2, 4);
					b->sprite.attr += 4;
					if(b->sprite.attr & (1<<12)) b->sprite.y -= 16;
				} else {
					b->sprite.size = SPRITE_SIZE(4, 2);
					if(b->sprite.attr & (1<<11)) b->sprite.x -= 16;
				}
			} else if(b->ttl < TIME_8(18)) {
				if(b->sprite.attr & (1<<12)) b->sprite.y -= 16;
				if(b->sprite.attr & (1<<11)) b->sprite.x -= 16;
			}
		}
		vdp_sprite_add(&b->sprite);
		b->ttl--;
	}
}

void bullet_update_missile(Bullet *b) {
	b->ttl--;
	uint8_t index = b->level - 1 + (b->type == WEAPON_SUPERMISSILE ? 3 : 0);
	if(!b->state) {
		int16_t accel = missile_settings[pal_mode||cfg_60fps][index].accel;
		int16_t maxspeed = missile_settings[pal_mode||cfg_60fps][index].maxspeed;
		switch(b->dir) {
			case LEFT: 
			if(b->x_speed > -(maxspeed + accel)) b->x_speed -= accel; 
			if(b->ttl & 1) b->y_speed >>= 1;
			break;
			case RIGHT: 
			if(b->x_speed < (maxspeed + accel)) b->x_speed += accel; 
			if(b->ttl & 1) b->y_speed >>= 1;
			break;
			case UP: 
			if(b->y_speed > -(maxspeed + accel)) b->y_speed -= accel; 
			if(b->ttl & 1) b->x_speed >>= 1;
			break;
			case DOWN: 
			if(b->y_speed < (maxspeed + accel)) b->y_speed += accel; 
			if(b->ttl & 1) b->x_speed >>= 1;
			break;
		}
		if((b->ttl & 7) == 0) effect_create_misc(EFF_BOOST2, b->x >> CSF, b->y >> CSF, FALSE);
		b->x += b->x_speed;
		b->y += b->y_speed;
		uint8_t block = stage_get_block_type(sub_to_block(b->x), sub_to_block(b->y));
		if(block == 0x41) { // Explode
			bullet_missile_explode(b);
		} else if(block == 0x43) {
			bullet_missile_explode(b);
			bullet_destroy_block(sub_to_block(b->x), sub_to_block(b->y));
			effect_create_smoke(sub_to_pixel(b->x), sub_to_pixel(b->y));
			sound_play(SND_BLOCK_DESTROY, 5);
		}
		sprite_pos(b->sprite, 
			sub_to_pixel(b->x - camera.x) + SCREEN_HALF_W - 8,
			sub_to_pixel(b->y - camera.y) + SCREEN_HALF_H - 8);
		vdp_sprite_add(&b->sprite);
	} else {
		if(b->ttl & 3) {
			uint16_t range = missile_settings[pal_mode||cfg_60fps][index].eff_range;
			effect_create_smoke((b->x>>CSF) - range + (random() & (range<<1)), 
								(b->y>>CSF) - range + (random() & (range<<1)));
		}
	}
}

void bullet_update_bubbler(Bullet *b) {
	if(b->level < 3) { // Level 1 and 2 just move forward a bit
		uint8_t decel = SPEED_8(0x10);
		if(b->level == 1) decel += SPEED_8(0x0A);
		switch(b->dir) {
			case LEFT: b->x_speed += decel; break;
			case RIGHT: b->x_speed -= decel; break;
			case UP: b->y_speed += decel; break;
			case DOWN: b->y_speed -= decel; break;
		}
		uint8_t block = stage_get_block_type(sub_to_block(b->x), sub_to_block(b->y));
		if(block == 0x41) { // Bullet hit a wall
			b->ttl = 0;
			effect_create_misc(EFF_BUBB_POP, b->x >> CSF, b->y >> CSF, FALSE);
			return;
		}
		// Half assed animation
		uint8_t livetime = b->level == 1 ? TIME_8(40) : TIME_8(60);
		uint8_t frame = (livetime - b->ttl) >> 2;
		if(frame > 3) frame = 3;
		sprite_index(b->sprite, sheets[b->sheet].index + frame);
	} else if(b->ttl >= TIME_8(30)) { // Level 3 orbits around player
		if(!joy_down(btn[cfg_btn_shoot]) || b->ttl == TIME_8(30)) {
			b->ttl = TIME_8(30);
			sound_play(SND_BUBBLER_LAUNCH, 5);
			b->dir = FIREDIR;
			switch(b->dir) {
				case LEFT: 
					b->x_speed = -SPEED_12(0xC00);
					b->y_speed = 0;
					sprite_index(b->sprite, sheets[b->sheet].index + 2);
					break;
				case RIGHT: 
					b->x_speed = SPEED_12(0xC00);
					b->y_speed = 0;
					sprite_index(b->sprite, sheets[b->sheet].index + 2);
					break;
				case UP: 
					b->y_speed = -SPEED_12(0xC00);
					b->x_speed = 0;
					sprite_index(b->sprite, sheets[b->sheet].index + 3);
					break;
				case DOWN: 
					b->y_speed = SPEED_12(0xC00);
					b->x_speed = 0;
					sprite_index(b->sprite, sheets[b->sheet].index + 3);
					break;
			}
		} else {
			// Just 2 frames for this part, when launched use 3rd or 4th based on dir
			if(b->ttl == TIME_8(98)) sprite_index(b->sprite, sheets[b->sheet].index + 1);
			// Follow player
			if (b->x > player.x) {
				if(b->x_speed > -SPEED_12(0x5E0)) b->x_speed -= SPEED_8(0x20);
			} else if (b->x < player.x) {
				if(b->x_speed < SPEED_12(0x5E0)) b->x_speed += SPEED_8(0x20);
			}
			if (b->y > player.y) {
				if(b->y_speed > -SPEED_12(0x5E0)) b->y_speed -= SPEED_8(0x20);
			} else if (b->y < player.y) {
				if(b->y_speed < SPEED_12(0x5E0)) b->y_speed += SPEED_8(0x20);
			}
			// Bounce off walls
			if (b->x_speed < 0 && ((blk(b->x,-4, b->y,0) & 0x41) == 0x41)) 
				b->x_speed = SPEED_10(0x3FF);
			else if ((blk(b->x,4, b->y,0) & 0x41) == 0x41) 
				b->x_speed = -SPEED_10(0x3FF);
			if (b->y_speed < 0 && ((blk(b->x,0, b->y,-4) & 0x41) == 0x41)) 
				b->y_speed = SPEED_10(0x3FF);
			else if ((blk(b->x,0, b->y,4) & 0x41) == 0x41) 
				b->y_speed = -SPEED_10(0x3FF);
		}
	} else { // Level 3 being launched
		uint8_t block = stage_get_block_type(sub_to_block(b->x), sub_to_block(b->y));
		if(block == 0x41) { // Bullet hit a wall
			b->ttl = 0;
			sound_play(SND_SHOT_HIT, 3);
            effect_create_misc(EFF_BUBB_POP, b->x >> CSF, b->y >> CSF, FALSE);
			return;
		} else if(block == 0x43) { // Breakable block
			b->ttl = 0;
			bullet_destroy_block(sub_to_block(b->x), sub_to_block(b->y));
			effect_create_smoke(sub_to_pixel(b->x), sub_to_pixel(b->y));
			sound_play(SND_BLOCK_DESTROY, 5);
			return;
		}
	}
	b->x += b->x_speed;
	b->y += b->y_speed;
	sprite_pos(b->sprite, 
		sub_to_pixel(b->x - camera.x) + SCREEN_HALF_W - 4,
		sub_to_pixel(b->y - camera.y) + SCREEN_HALF_H - 4);
	vdp_sprite_add(&b->sprite);
	if(!--b->ttl) effect_create_misc(EFF_BUBB_POP, b->x >> CSF, b->y >> CSF, FALSE);
}

void bullet_update_blade(Bullet *b) {
	b->ttl--;
	if(b->level == 3) {
		if(b->x_speed | b->y_speed) {
			uint8_t block = stage_get_block_type(sub_to_block(b->x), sub_to_block(b->y));
			if(b->hits) { // Hit something, stop moving
				b->ttl = TIME_8(50);
				b->x_speed = 0;
				b->y_speed = 0;
				
				TILES_QUEUE(SPR_TILES(&SPR_BladeB3k, 0, 3), sheets[b->sheet].index, 9);
				
			} else if(block == 0x43) { // Breakable block
				b->ttl = 0;
				bullet_destroy_block(sub_to_block(b->x), sub_to_block(b->y));
				effect_create_smoke(sub_to_pixel(b->x), sub_to_pixel(b->y));
				sound_play(SND_BLOCK_DESTROY, 5);
				return;
			} else if(block == 0x41) {
				b->ttl = 0;
				return;
			} else {
				create_blade_slash(b, FALSE);
			}
		} else {
			create_blade_slash(b, TRUE);
		}
	} else {
		// Level 1 and 2 hit walls, spin
		uint8_t block = stage_get_block_type(sub_to_block(b->x), sub_to_block(b->y));
		if(block == 0x41) { // Hit wall/floor
			b->ttl = 0;
			sound_play(SND_SHOT_HIT, 3);
			return;
		} else if(block == 0x43) { // Hit breakable block
			b->ttl = 0;
			bullet_destroy_block(sub_to_block(b->x), sub_to_block(b->y));
			effect_create_smoke(sub_to_pixel(b->x), sub_to_pixel(b->y));
			sound_play(SND_BLOCK_DESTROY, 5);
			return;
		}
		uint8_t anim = b->ttl & 7;
		if(anim == 6)      b->sprite.attr |= (1<<12);
		else if(anim == 4) b->sprite.attr |= (1<<11);
		else if(anim == 2) b->sprite.attr &= ~(1<<12);
		else if(anim == 0) b->sprite.attr &= ~(1<<11);
	}
	b->x += b->x_speed;
	b->y += b->y_speed;
	sprite_pos(b->sprite, 
		sub_to_pixel(b->x - camera.x) + SCREEN_HALF_W - 8,
		sub_to_pixel(b->y - camera.y) + SCREEN_HALF_H - 8);
	// Level 2 and 3 sprites are a bit larger so adjust the display position
	if(b->level > 1) {
		b->sprite.x -= 4;
		b->sprite.y -= 4;
	}
	vdp_sprite_add(&b->sprite);
}

// Here be dragons
void bullet_update_blade_slash(Bullet *b) {
	b->ttl--;
	// Animate sprite
	if(b->ttl == 10) b->sprite.attr += 4;
	
	b->x += b->x_speed;
	b->y += b->y_speed;
	sprite_pos(b->sprite, 
		sub_to_pixel(b->x - camera.x) + SCREEN_HALF_W - 8,
		sub_to_pixel(b->y - camera.y) + SCREEN_HALF_H - 8);
	vdp_sprite_add(&b->sprite);
}

void bullet_update_supermissile(Bullet *b) {
	bullet_update_missile(b);
}

void bullet_update_nemesis(Bullet *b) {
	uint8_t block = blk(b->x, 0, b->y, 0);
	if(block == 0x41) {
		b->ttl = 0;
	} else if(block == 0x43) {
		bullet_destroy_block(sub_to_block(b->x), sub_to_block(b->y));
		effect_create_smoke(sub_to_pixel(b->x), sub_to_pixel(b->y));
		sound_play(SND_BLOCK_DESTROY, 5);
		b->ttl = 0;
	} else {
		b->ttl--;
		b->state ^= 1;
		b->sprite.attr += b->state ? 6 : -6;
		b->x += b->x_speed;
		b->y += b->y_speed;
		sprite_pos(b->sprite, 
			sub_to_pixel(b->x - camera.x) + SCREEN_HALF_W - (b->hit_box.left + 1),
			sub_to_pixel(b->y - camera.y) + SCREEN_HALF_H - (b->hit_box.top + 1));
		vdp_sprite_add(&b->sprite);
	}
}

void bullet_update_spur(Bullet *b) {
	uint8_t block = blk(b->x, 0, b->y, 0);
	if(block == 0x41) {
		sound_play(SND_SHOT_HIT, 2);
		b->ttl = 0;
	} else if(block == 0x43) {
		bullet_destroy_block(sub_to_block(b->x), sub_to_block(b->y));
		effect_create_smoke(sub_to_pixel(b->x), sub_to_pixel(b->y));
		sound_play(SND_BLOCK_DESTROY, 5);
		b->ttl = 0;
	} else {
		if(++b->state >= TIME_8(5)) {
			// Spawn tail
			Bullet *t = NULL;
			for(uint8_t i = 0; i < MAX_BULLETS; i++) {
				if(playerBullet[i].ttl > 0) continue;
				t = &playerBullet[i];
				break;
			}
			if(t) {
				b->state = 0;
				t->type = WEAPON_SPUR_TAIL;
				t->level = b->level;
				t->sheet = b->sheet;
				t->damage = t->level + 1;
				t->ttl = TIME_8(20);
				t->dir = b->dir;
				t->x = pixel_to_sub(spur_xmark);
				t->y = pixel_to_sub(spur_ymark);
				t->x_speed = b->x_speed;
				t->y_speed = b->y_speed;
				t->hit_box = b->hit_box;
				t->sprite = (VDPSprite) {
					.size = SPRITE_SIZE(2,2),
					.attr = TILE_ATTR(PAL0,0,0,0,sheets[t->sheet].index+(t->dir < 2 ? 0 : 8)),
				};
				//switch(t->dir) {
				//	case LEFT:  t->x += (b->state<<11); break;
				//	case RIGHT: t->x -= (b->state<<11); break;
				//	case UP:    t->y += (b->state<<11); break;
				//	case DOWN:  t->y -= (b->state<<11); break;
				//}
			}
		}
		b->ttl--;
		b->x += b->x_speed;
		b->y += b->y_speed;
		sprite_pos(b->sprite, 
			sub_to_pixel(b->x - camera.x) + SCREEN_HALF_W - 8,
			sub_to_pixel(b->y - camera.y) + SCREEN_HALF_H - 8);
		if(b->ttl == TIME_8(28)) {
			if(b->dir == UP || b->dir == DOWN) {
				b->sprite.size = SPRITE_SIZE(2, 4);
				b->sprite.attr += 4;
				if(b->sprite.attr & (1<<12)) {
					b->sprite.y -= 16;
					b->hit_box.top += 16;
				} else {
					b->hit_box.bottom += 16;
				}
			} else {
				b->sprite.size = SPRITE_SIZE(4, 2);
				if(b->sprite.attr & (1<<11)) {
					b->sprite.x -= 16;
					b->hit_box.right += 16;
				} else {
					b->hit_box.left += 16;
				}
			}
		} else if(b->ttl < TIME_8(28)) {
			if(b->sprite.attr & (1<<12)) b->sprite.y -= 16;
			if(b->sprite.attr & (1<<11)) b->sprite.x -= 16;
		}
		vdp_sprite_add(&b->sprite);
	}
}

void bullet_update_spur_tail(Bullet *b) {
    uint8_t block = blk(b->x, 0, b->y, 0);
    if(block == 0x41) {
        sound_play(SND_SHOT_HIT, 2);
        b->ttl = 0;
    } else if(block == 0x43) {
        bullet_destroy_block(sub_to_block(b->x), sub_to_block(b->y));
        effect_create_smoke(sub_to_pixel(b->x), sub_to_pixel(b->y));
        sound_play(SND_BLOCK_DESTROY, 5);
        b->ttl = 0;
    } else {
        b->ttl--;
        b->x += b->x_speed;
        b->y += b->y_speed;
        sprite_pos(b->sprite,
                   sub_to_pixel(b->x - camera.x) + SCREEN_HALF_W - 8,
                   sub_to_pixel(b->y - camera.y) + SCREEN_HALF_H - 8);
        if (b->ttl == TIME_8(18)) {
            if (b->dir == UP || b->dir == DOWN) {
                b->sprite.size = SPRITE_SIZE(2, 4);
                b->sprite.attr += 4;
                if (b->sprite.attr & (1 << 12)) {
                    b->sprite.y -= 16;
                    b->hit_box.top += 16;
                } else {
                    b->hit_box.bottom += 16;
                }
            } else {
                b->sprite.size = SPRITE_SIZE(4, 2);
                if (b->sprite.attr & (1 << 11)) {
                    b->sprite.x -= 16;
                    b->hit_box.right += 16;
                } else {
                    b->hit_box.left += 16;
                }
            }
        } else if (b->ttl < TIME_8(18)) {
            if (b->sprite.attr & (1 << 12)) b->sprite.y -= 16;
            if (b->sprite.attr & (1 << 11)) b->sprite.x -= 16;
        }
        vdp_sprite_add(&b->sprite);
    }
}

Bullet *bullet_colliding(Entity *e) {
	extent_box ee = (extent_box) {
		.x1 = (e->x >> CSF) - (e->hit_box.left),
		.y1 = (e->y >> CSF) - (e->hit_box.top),
		.x2 = (e->x >> CSF) + (e->hit_box.right),
		.y2 = (e->y >> CSF) + (e->hit_box.bottom),
	};
	for(uint8_t i = 0; i < MAX_BULLETS; i++) {
		if (playerBullet[i].ttl &&
			playerBullet[i].extent.x1 <= ee.x2 &&
			playerBullet[i].extent.x2 >= ee.x1 &&
			playerBullet[i].extent.y1 <= ee.y2 &&
			playerBullet[i].extent.y2 >= ee.y1) return &playerBullet[i];
	}
	return NULL;
}

void bullet_missile_explode(Bullet *b) {
	sound_play(SND_MISSILE_HIT, 5);
	b->state = 1;
	b->x_speed = 0;
	b->y_speed = 0;
	uint8_t index = b->level - 1 + (b->type == WEAPON_SUPERMISSILE ? 3 : 0);
	b->ttl = missile_settings[pal_mode||cfg_60fps][index].boom_time;
	uint8_t size = (b->type == WEAPON_SUPERMISSILE) ? 24 : 32;
	b->hit_box = (bounding_box) { size, size, size, size };
	// TODO: Explosion graphic instead of smoke
	for(uint8_t i = 0; i < 2; i++) {
		effect_create_smoke(sub_to_pixel(b->x) - 15 + (random() & 31), 
							sub_to_pixel(b->y) - 15 + (random() & 31));
	}
}

uint8_t bullet_missile_is_exploding() {
	for(uint8_t i = 4; i < 10; i++) {
		if(playerBullet[i].ttl && (playerBullet[i].type == WEAPON_MISSILE
				|| playerBullet[i].type == WEAPON_SUPERMISSILE) && playerBullet[i].state) {
			return TRUE;
		}
	}
	return FALSE;
}

static void bullet_destroy_block(uint16_t x, uint16_t y) {
	uint8_t ind;
	if(stageTileset == 21) ind = 22; // Balcony
	else if(stageTileset == 13) ind = 0; // Labyrinth
	else ind = stage_get_block(x, y) - 1;
	stage_replace_block(x, y, ind);
}

static void create_blade_slash(Bullet *b, uint8_t burst) {
	Bullet *slash = NULL;
	if(burst) {
		if((b->ttl & 15) == 0) {
			slash = &playerBullet[1];
			slash->dir = LEFT;
			slash->dir &= ~2;
		} else if((b->ttl & 15) == 4) {
			slash = &playerBullet[2];
			slash->dir = RIGHT;
			slash->dir &= ~2;
		} else if((b->ttl & 15) == 8) {
			slash = &playerBullet[3];
			slash->dir = LEFT;
			slash->dir |= 2;
		} else if((b->ttl & 15) == 12) {
			slash = &playerBullet[4];
			slash->dir = RIGHT;
			slash->dir |= 2;
		}
	} else {
		if((b->ttl & 15) == 0) {
			slash = &playerBullet[1];
			slash->dir = b->dir;
			slash->dir &= ~2;
		} else if((b->ttl & 15) == 8) {
			slash = &playerBullet[2];
			slash->dir = b->dir;
			slash->dir |= 2;
		}
	}
	if(!slash) return;
	slash->x_speed = (slash->dir & 1) ? SPEED_10(0x3FF) : -SPEED_10(0x3FF);
	slash->y_speed = (slash->dir & 2) ? -SPEED_10(0x3FF) : SPEED_10(0x3FF);
	slash->damage = 1;
	slash->x = b->x;
	slash->y = b->y + ((slash->dir & 2) ? 0x2000 : -0x2000);
	if(burst) { // Spread them for AOE
		slash->x += -0x2000 + (random() % 0x4000);
		slash->y += -0x2000 + (random() % 0x4000);
	}
	slash->type = WEAPON_BLADE_SLASH;
	slash->ttl = 20;
	slash->hit_box = (bounding_box) { 6, 6, 6, 6 };
	slash->sprite = (VDPSprite) { 
		.size = SPRITE_SIZE(2, 2), 
		.attr = TILE_ATTR(PAL0,0,(slash->dir&2)>0,(slash->dir&1), TILE_SLASHINDEX)
	};
}
