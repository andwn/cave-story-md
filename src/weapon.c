#include "common.h"

#include "audio.h"
#include "camera.h"
#include "dma.h"
#include "effect.h"
#include "entity.h"
#include "input.h"
#include "joy.h"
#include "player.h"
#include "resources.h"
#include "sheet.h"
#include "sprite.h"
#include "stage.h"
#include "tables.h"
#include "tools.h"
#include "vdp.h"
#include "vdp_tile.h"

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
	&bullet_update_none,
	&bullet_update_nemesis,
	&bullet_update_spur
};

static void bullet_destroy_block(uint16_t x, uint16_t y);
static void create_blade_slash(Bullet *b, uint8_t burst);

void weapon_fire_none(Weapon *w) { (void)(w); }

void weapon_fire_snake(Weapon *w) {
	weapon_fire_polarstar(w);
}

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
	b->type = w->type;
	b->level = w->level;
	b->sprite = (VDPSprite) { .size = SPRITE_SIZE(2, 2) };
	b->damage = w->level + (w->level == 3 ? 1 : 0); // 1, 2, 4
	b->ttl = 20 + w->level * 5;
	b->sheet = w->sheet;
	b->hit_box = (bounding_box) { 4, 1 + w->level, 4, 1 + w->level };
	
	b->dir = FIREDIR;
	if(b->dir == UP) {
		b->sprite.attribut = TILE_ATTR_FULL(PAL0,0,0,0,sheets[w->sheet].index+4);
		b->x = player.x;
		b->y = player.y - pixel_to_sub(12);
		b->x_speed = 0;
		b->y_speed = -pixel_to_sub(4);
	} else if(b->dir == DOWN) {
		b->sprite.attribut = TILE_ATTR_FULL(PAL0,0,0,0,sheets[w->sheet].index+4);
		b->x = player.x;
		b->y = player.y + pixel_to_sub(12);
		b->x_speed = 0;
		b->y_speed = pixel_to_sub(4);
	} else {
		b->sprite.attribut = TILE_ATTR_FULL(PAL0,0,0,0,sheets[w->sheet].index);
		b->x = player.x + (b->dir ? pixel_to_sub(12) : -pixel_to_sub(12));
		b->y = player.y + pixel_to_sub(3);
		b->x_speed = (b->dir ? pixel_to_sub(4) : -pixel_to_sub(4));
		b->y_speed = 0;
	}
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
		.attribut = TILE_ATTR_FULL(PAL1,0,0,0,sheets[w->sheet].index)
	};
	b->damage = 2 * w->level; // 2, 4, 6
	b->ttl = 50 + w->level * 10;
	b->sheet = w->sheet;
	b->hit_box = (bounding_box) { 4, 4, 4, 4 };
	
	b->dir = FIREDIR;
	if(b->dir == UP) {
		b->x = player.x;
		b->y = player.y - pixel_to_sub(12);
		b->x_speed = player.dir ? 0x300 : -0x300; // 1.5
		b->y_speed = -pixel_to_sub(3);
	} else if(b->dir == DOWN) {
		b->x = player.x;
		b->y = player.y + pixel_to_sub(12);
		b->x_speed = player.dir ? 0x300 : -0x300; // 1.5
		b->y_speed = pixel_to_sub(2);
	} else {
		b->x = player.x + (b->dir ? pixel_to_sub(8) : -pixel_to_sub(8));
		b->y = player.y + pixel_to_sub(4);
		b->x_speed = b->dir ? 0x500 : -0x500; // 2.5
		b->y_speed = 0;
	}
}

void weapon_fire_machinegun(Weapon *w) {
	// Use all 10 slots for machine gun
	Bullet *b = NULL;
	for(uint8_t i = 0; i < 10; i++) {
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
	b->damage = w->level * 2; // 2, 4, 6
	b->ttl = TIME(26);
	b->sheet = w->sheet;
	b->hit_box = (bounding_box) { 4, 1 + w->level, 4, 1 + w->level };
	// check up/down first
	b->dir = FIREDIR;
	if(b->dir == UP) {
		b->sprite.attribut = TILE_ATTR_FULL(PAL0,0,0,0,sheets[w->sheet].index+8);
		if(!player.grounded && w->level == 3) {
			player.y_speed += SPEED(0x100);
			if(player.y_speed > SPEED(0x5FF)) player.y_speed = SPEED(0x5FF);
		}
		b->x = player.x;
		b->y = player.y - pixel_to_sub(12);
		b->x_speed = -SPEED(0x50) + (random() % 0xA0);
		b->y_speed = -SPEED(0xC00);
	} else if(b->dir == DOWN) {
		b->sprite.attribut = TILE_ATTR_FULL(PAL0,0,1,0,sheets[w->sheet].index+8);
		if(w->level == 3) {
			if(joy_down(BUTTON_C) || player.y_speed > 0) {
				player.y_speed -= SPEED(0x400);
				if (player.y_speed < -SPEED(0x5FF)) player.y_speed = -SPEED(0x5FF);
			} else {
				player.y_speed -= SPEED(0x380);
				if (player.y_speed < -SPEED(0x4FF)) player.y_speed = -SPEED(0x4FF);
			}
		}
		b->x = player.x;
		b->y = player.y + pixel_to_sub(12);
		b->x_speed = -SPEED(0x50) + (random() % 0xA0);
		b->y_speed = SPEED(0xC00);
	} else {
		//b->level++; // Wonky use of this var, so the trail knows whether to be H/V
		b->sprite.attribut = TILE_ATTR_FULL(PAL0,0,0,b->dir,sheets[w->sheet].index);
		b->x = player.x + (b->dir ? pixel_to_sub(10) : -pixel_to_sub(10));
		b->y = player.y + pixel_to_sub(3);
		b->x_speed = (b->dir ? SPEED(0xC00) : -SPEED(0xC00));
		b->y_speed = -SPEED(0x50) + (random() % 0xA0);
	}
}

void weapon_fire_missile(Weapon *w) {
	// Use end of array for missiles
	// Max bullets for lv 1,2,3: 1,2,6
	Bullet *b = NULL;
	for(uint8_t i = 4 + (w->level == 1 ? 5 : w->level == 2 ? 4 : 0); i < 10; i++) {
		if(playerBullet[i].ttl > 0) continue;
		b = &playerBullet[i];
		break;
	}
	if(!b) return;
	if(w->ammo) w->ammo--;
	else if(w->maxammo) return;
	b->type = WEAPON_MISSILE; //w->type;
	b->level = w->level;
	b->sprite = (VDPSprite) { .size = SPRITE_SIZE(2, 2), };
	b->damage = 0; // 0 damage because an explosion object causes damage instead
	b->ttl = 120;
	b->sheet = w->sheet;
	b->hit_box = (bounding_box) { 4, 4, 4, 4 };
	
	b->dir = FIREDIR;
	if(b->dir == UP) {
		b->sprite.attribut = TILE_ATTR_FULL(PAL0,0,0,player.dir,sheets[w->sheet].index+4);
		b->x = player.x;
		b->y = player.y - pixel_to_sub(12);
		b->x_speed = 0;
		b->y_speed = -pixel_to_sub(3);
	} else if(b->dir == DOWN) {
		b->sprite.attribut = TILE_ATTR_FULL(PAL0,1,0,player.dir,sheets[w->sheet].index+4);
		b->x = player.x;
		b->y = player.y + pixel_to_sub(12);
		b->x_speed = 0;
		b->y_speed = pixel_to_sub(3);
	} else {
		b->sprite.attribut = TILE_ATTR_FULL(PAL0,0,0,b->dir,sheets[w->sheet].index);
		b->x = player.x + (b->dir ? pixel_to_sub(12) : -pixel_to_sub(12));
		b->y = player.y + pixel_to_sub(2);
		b->x_speed = (b->dir ? pixel_to_sub(3) : -pixel_to_sub(3));
		b->y_speed = 0;
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
	if(w->ammo) w->ammo--;
	else if(w->maxammo) return;
	b->type = w->type;
	b->level = w->level;
	b->sheet = w->sheet;
	b->hit_box = (bounding_box) { 3, 3, 3, 3 };
	b->sprite = (VDPSprite) { 
		.size = SPRITE_SIZE(1, 1),
		.attribut = TILE_ATTR_FULL(PAL0,0,0,0,sheets[w->sheet].index)
	};
	sound_play(SND_BUBBLER_FIRE, 5);
	// Starting velocity based on level
	int16_t fwdspeed = 0, sidespeed = 0;
	switch(b->level) {
		case 1:
			b->damage = 1;
			b->ttl = TIME(40);
			fwdspeed = SPEED(0x400); 
			break;
		case 2: 
			b->damage = 2;
			b->ttl = TIME(60);
			fwdspeed = SPEED(0x600);
			sidespeed = -SPEED(0x100) + (random() % SPEED(0x200));
			break;
		case 3:
			b->damage = 5;
			b->ttl = TIME(100);
			fwdspeed = SPEED(0x200) + (random() % SPEED(0x200));
			sidespeed = (random() % SPEED(0x800)) - SPEED(0x400);
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
}

void weapon_fire_blade(Weapon *w) {
	// Blade is only 1 bullet at a time for every level
	Bullet *b = &playerBullet[0];
	if(b->ttl) return;
	b->type = w->type;
	b->level = w->level;
	b->sheet = w->sheet;
	b->hits = 0;
	switch(b->level) {
		case 1: // Small 16x16 Blade
			b->sprite = (VDPSprite) { .size = SPRITE_SIZE(2, 2) };
			b->damage = 15;
			b->ttl = TIME(30);
			b->hit_box = (bounding_box) { 6, 6, 6, 6 };
			break;
		case 2: // Large 24x24 Blade, hits 6 times
			b->sprite = (VDPSprite) { .size = SPRITE_SIZE(3, 3) };
			b->damage = 18;
			b->ttl = TIME(18);
			b->hit_box = (bounding_box) { 10, 10, 10, 10 };
			break;
		case 3: // King's Ghost, AOE on hit
			b->sprite = (VDPSprite) { .size = SPRITE_SIZE(3, 3) };
			b->damage = 1;
			b->ttl = TIME(30);
			b->hit_box = (bounding_box) { 8, 8, 8, 8 };
			break;
	}
	sound_play(SND_FIREBALL, 5);
	b->x = player.x;
	b->y = player.y;
	b->sprite.attribut = TILE_ATTR_FULL(PAL0,0,0,0,sheets[w->sheet].index);
	b->dir = FIREDIR;
	if(b->dir == UP) {
		if(b->level == 3) TILES_QUEUE(SPR_TILES(&SPR_BladeB3k, 0, 1), b->sprite.attribut, 9);
		b->x_speed = 0;
		b->y_speed = -SPEED(0x800);
	} else if(b->dir == DOWN) {
		if(b->level == 3) TILES_QUEUE(SPR_TILES(&SPR_BladeB3k, 0, 2), b->sprite.attribut, 9);
		b->x_speed = 0;
		b->y_speed = SPEED(0x800);
	} else {
		if(b->level == 3) {
			TILES_QUEUE(SPR_TILES(&SPR_BladeB3k, 0, 0), b->sprite.attribut, 9);
			if(b->dir == RIGHT) sprite_hflip(b->sprite, 1);
		}
		b->x_speed = (b->dir ? SPEED(0x800) : -SPEED(0x800));
		b->y_speed = 0;
	}
}

void weapon_fire_supermissile(Weapon *w) {
	weapon_fire_missile(w);
}

void weapon_fire_nemesis(Weapon *w) {
	weapon_fire_polarstar(w);
}

void weapon_fire_spur(Weapon *w) {
	weapon_fire_polarstar(w);
}

void bullet_update_none(Bullet *b) { (void)(b); }

void bullet_update_snake(Bullet *b) {
	bullet_update_polarstar(b);
}

void bullet_update_polarstar(Bullet *b) {
	b->x += b->x_speed;
	b->y += b->y_speed;
	uint8_t block = stage_get_block_type(sub_to_block(b->x), sub_to_block(b->y));
	// Check if bullet is colliding with a breakable block
	if(block == 0x43) {
		b->ttl = 0;
		bullet_destroy_block(sub_to_block(b->x), sub_to_block(b->y));
		effect_create_smoke(sub_to_pixel(b->x), sub_to_pixel(b->y));
		sound_play(SND_BLOCK_DESTROY, 5);
	} else if(block == 0x41) { // Bullet hit a wall
		b->ttl = 0;
		sound_play(SND_SHOT_HIT, 3);
		// TODO: Add the sprite and effect for hitting a wall
	} else {
		sprite_pos(b->sprite, 
			sub_to_pixel(b->x - camera.x) + SCREEN_HALF_W - 8,
			sub_to_pixel(b->y - camera.y) + SCREEN_HALF_H - 8);
		sprite_add(b->sprite);
		b->ttl--;
	}
}

void bullet_update_fireball(Bullet *b) {
	// Check below / above first
	uint8_t block_below = stage_get_block_type(sub_to_block(b->x), sub_to_block(b->y + 0x800));
	uint8_t block_above = b->y_speed < 0
				? stage_get_block_type(sub_to_block(b->x), sub_to_block(b->y - 0x800)) : 0;
	if(block_below == 0x41 || block_below == 0x43) {
		b->y -= sub_to_pixel(b->y + 0x800) % 16;
		b->y_speed = -b->y_speed >> 1;
		if(b->y_speed > -0x300) b->y_speed = -0x300;
	} else if(block_below & BLOCK_SLOPE) {
		uint8_t index = block_below & 0xF;
		if(index >= 4) {
			uint16_t xx = sub_to_pixel(b->x);
			uint16_t yy = sub_to_pixel(b->y + 0x800);
			int8_t overlap = (yy % 16) - heightmap[index % 4][xx % 16];
			if(overlap >= 0) {
				b->y -= overlap;
				b->y_speed = -b->y_speed;
				if(b->y_speed > -0x300) b->y_speed = -0x300;
			}
		}
	} else if(block_above == 0x41 || block_above == 0x43) {
		b->y_speed = -b->y_speed >> 1;
		if(b->y_speed < 0x200) b->y_speed = 0x200;
	} else {
		b->y_speed += SPEED(0x50);
		if(b->y_speed > 0x600) b->y_speed = 0x600;
	}
	// Check in front
	uint8_t block_front = stage_get_block_type(
			sub_to_block(b->x + (b->x_speed > 0 ? 0x800 : -0x800)),
			sub_to_block(b->y - 0x100));
	if(block_front == 0x41 || block_front == 0x43) { // Bullet hit a wall
		b->x_speed = -b->x_speed;
	}
	b->x += b->x_speed;
	b->y += b->y_speed;
	sprite_pos(b->sprite, 
		sub_to_pixel(b->x - camera.x) + SCREEN_HALF_W - 8,
		sub_to_pixel(b->y - camera.y) + SCREEN_HALF_H - 8);
	if(!(b->ttl % 4)) {
		sprite_index(b->sprite, sheets[b->sheet].index + ((uint16_t)(80 - b->ttl) % 12));
	}
	sprite_add(b->sprite);
	b->ttl--;
}

void bullet_update_machinegun(Bullet *b) {
	b->x += b->x_speed;
	b->y += b->y_speed;
	uint8_t block = stage_get_block_type(sub_to_block(b->x), sub_to_block(b->y));
	if(block == 0x43) {
		b->ttl = 0;
		bullet_destroy_block(sub_to_block(b->x), sub_to_block(b->y));
		effect_create_smoke(sub_to_pixel(b->x), sub_to_pixel(b->y));
		sound_play(SND_BLOCK_DESTROY, 5);
	} else if(block == 0x41) { // Bullet hit a wall
		b->ttl = 0;
		sound_play(SND_SHOT_HIT, 3);
		// TODO: Add the sprite and effect for hitting a wall
	} else {
		sprite_pos(b->sprite,
			sub_to_pixel(b->x - camera.x) + SCREEN_HALF_W - 8,
			sub_to_pixel(b->y - camera.y) + SCREEN_HALF_H - 8);
		// Expand sprite of level 3 after a couple frames
		if(b->level == 3) {
			if(b->ttl == TIME(24)) {
				if(b->dir == UP || b->dir == DOWN) {
					b->sprite.size = SPRITE_SIZE(2, 4);
					b->sprite.attribut += 4;
					if(b->sprite.attribut & (1<<12)) b->sprite.y -= 16;
				} else {
					b->sprite.size = SPRITE_SIZE(4, 2);
					if(b->sprite.attribut & (1<<11)) b->sprite.x -= 16;
				}
			} else if(b->ttl < TIME(24)) {
				if(b->sprite.attribut & (1<<12)) b->sprite.y -= 16;
				if(b->sprite.attribut & (1<<11)) b->sprite.x -= 16;
			}
		}
		sprite_add(b->sprite);
		b->ttl--;
	}
}

void bullet_update_missile(Bullet *b) {
	b->ttl--;
	if(b->x_speed | b->y_speed) {
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
		sprite_add(b->sprite);
	}
}

void bullet_update_bubbler(Bullet *b) {
	if(b->level < 3) { // Level 1 and 2 just move forward a bit
		uint8_t decel = SPEED(0x10);
		if(b->level == 1) decel += SPEED(0x0A);
		switch(b->dir) {
			case LEFT: b->x_speed += decel; break;
			case RIGHT: b->x_speed -= decel; break;
			case UP: b->y_speed += decel; break;
			case DOWN: b->y_speed -= decel; break;
		}
		uint8_t block = stage_get_block_type(sub_to_block(b->x), sub_to_block(b->y));
		if((block & 0x41) == 0x41) { // Bullet hit a wall
			b->ttl = 0;
			return;
		}
		// Half assed animation
		uint8_t livetime = b->level == 1 ? TIME(40) : TIME(60);
		uint8_t frame = (livetime - b->ttl) >> 2;
		if(frame > 3) frame = 3;
		sprite_index(b->sprite, sheets[b->sheet].index + frame);
	} else if(b->ttl >= TIME(30)) { // Level 3 orbits around player
		if(!joy_down(BUTTON_B) || b->ttl == TIME(30)) {
			b->ttl = TIME(30);
			sound_play(SND_BUBBLER_LAUNCH, 5);
			b->dir = FIREDIR;
			switch(b->dir) {
				case LEFT: 
					b->x_speed = -SPEED(0xC00);
					b->y_speed = 0;
					sprite_index(b->sprite, sheets[b->sheet].index + 2);
					break;
				case RIGHT: 
					b->x_speed = SPEED(0xC00);
					b->y_speed = 0;
					sprite_index(b->sprite, sheets[b->sheet].index + 2);
					break;
				case UP: 
					b->y_speed = -SPEED(0xC00);
					b->x_speed = 0;
					sprite_index(b->sprite, sheets[b->sheet].index + 3);
					break;
				case DOWN: 
					b->y_speed = SPEED(0xC00);
					b->x_speed = 0;
					sprite_index(b->sprite, sheets[b->sheet].index + 3);
					break;
			}
		} else {
			// Just 2 frames for this part, when launched use 3rd or 4th based on dir
			if(b->ttl == TIME(98)) sprite_index(b->sprite, sheets[b->sheet].index + 1);
			// Follow player
			if (b->x > player.x) {
				b->x_speed -= SPEED(0x20);
				if(b->x_speed < -SPEED(0x5FF)) b->x_speed = -SPEED(0x5FF);
			} else if (b->x < player.x) {
				b->x_speed += SPEED(0x20);
				if(b->x_speed > SPEED(0x5FF)) b->x_speed = SPEED(0x5FF);
			}
			if (b->y > player.y) {
				b->y_speed -= SPEED(0x20);
				if(b->y_speed < -SPEED(0x5FF)) b->y_speed = -SPEED(0x5FF);
			} else if (b->y < player.y) {
				b->y_speed += SPEED(0x20);
				if(b->y_speed > SPEED(0x5FF)) b->y_speed = SPEED(0x5FF);
			}
			// Bounce off walls
			if (b->x_speed < 0 && ((blk(b->x,-4, b->y,0) & 0x41) == 0x41)) 
				b->x_speed = SPEED(0x400);
			else if ((blk(b->x,4, b->y,0) & 0x41) == 0x41) 
				b->x_speed = -SPEED(0x400);
			if (b->y_speed < 0 && ((blk(b->x,0, b->y,-4) & 0x41) == 0x41)) 
				b->y_speed = SPEED(0x400);
			else if ((blk(b->x,0, b->y,4) & 0x41) == 0x41) 
				b->y_speed = -SPEED(0x400);
		}
	} else { // Level 3 being launched
		uint8_t block = stage_get_block_type(sub_to_block(b->x), sub_to_block(b->y));
		if(block == 0x41) { // Bullet hit a wall
			b->ttl = 0;
			sound_play(SND_SHOT_HIT, 3);
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
	sprite_add(b->sprite);
	b->ttl--;
}

void bullet_update_blade(Bullet *b) {
	b->ttl--;
	if(b->level == 3) {
		if(b->x_speed | b->y_speed) {
			uint8_t block = stage_get_block_type(sub_to_block(b->x), sub_to_block(b->y));
			if(b->hits) { // Hit something, stop moving
				b->ttl = TIME(50);
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
		if(anim == 6)      b->sprite.attribut |= (1<<12);
		else if(anim == 4) b->sprite.attribut |= (1<<11);
		else if(anim == 2) b->sprite.attribut &= ~(1<<12);
		else if(anim == 0) b->sprite.attribut &= ~(1<<11);
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
	sprite_add(b->sprite);
}

// Here be dragons
void bullet_update_blade_slash(Bullet *b) {
	b->ttl--;
	// Animate sprite
	if(b->ttl == 10) b->sprite.attribut += 4;
	
	b->x += b->x_speed;
	b->y += b->y_speed;
	sprite_pos(b->sprite, 
		sub_to_pixel(b->x - camera.x) + SCREEN_HALF_W - 8,
		sub_to_pixel(b->y - camera.y) + SCREEN_HALF_H - 8);
	sprite_add(b->sprite);
}

void bullet_update_supermissile(Bullet *b) {
	bullet_update_missile(b);
}

void bullet_update_nemesis(Bullet *b) {
	bullet_update_polarstar(b);
}

void bullet_update_spur(Bullet *b) {
	bullet_update_polarstar(b);
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
	b->x_speed = 0;
	b->y_speed = 0;
	b->ttl = 8;
	b->damage = 1 + b->level;
	b->hit_box = (bounding_box) { 12, 12, 12, 12 };
	for(uint8_t i =  b->level; i < 4; i++) {
		effect_create_smoke(sub_to_pixel(b->x) - 10 + (random() % 20), 
							sub_to_pixel(b->y) - 10 + (random() % 20));
	}
}

uint8_t bullet_missile_is_exploding() {
	for(uint8_t i = 4; i < 10; i++) {
		if(playerBullet[i].ttl > 0 &&
				playerBullet[i].x_speed == 0 && playerBullet[i].y_speed == 0) {
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
		} else if((b->ttl & 15) == 4) {
			slash = &playerBullet[2];
			slash->dir = RIGHT;
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
		} else if((b->ttl & 15) == 8) {
			slash = &playerBullet[2];
			slash->dir = b->dir;
			slash->dir |= 2;
		}
	}
	if(!slash) return;
	slash->x_speed = (slash->dir & 1) ? SPEED(0x400) : -SPEED(0x400);
	slash->y_speed = (slash->dir & 2) ? -SPEED(0x400) : SPEED(0x400);
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
		.attribut = TILE_ATTR_FULL(PAL0,0,(slash->dir&2)>0,(slash->dir&1), TILE_SLASHINDEX)
	};
}
