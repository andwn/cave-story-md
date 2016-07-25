#include "weapon.h"

#include "player.h"
#include "stage.h"
#include "camera.h"
#include "audio.h"
#include "resources.h"
#include "tables.h"
#include "effect.h"
#include "entity.h"

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
	&bullet_update_none,
	&bullet_update_blade,
	&bullet_update_supermissile,
	&bullet_update_none,
	&bullet_update_nemesis,
	&bullet_update_spur
};

void weapon_fire_none(Weapon *w) { }

void weapon_fire_snake(Weapon *w) {
	
}

void weapon_fire_polarstar(Weapon *w) {
	// Use first 3 slots for polar star
	// Max bullets for lv 1,2,3: 3,3,2
	Bullet *b = NULL;
	for(u8 i = 0 + (w->level == 3 ? 1 : 0); i < 3; i++) {
		if(playerBullet[i].ttl > 0) continue;
		b = &playerBullet[i];
		break;
	}
	if(b == NULL) return;
	if(w->level == 3) sound_play(SND_POLAR_STAR_L3, 5);
	else sound_play(SND_POLAR_STAR_L1_2, 5);
	b->type = w->type;
	b->level = w->level;
	b->sprite = SPR_addSprite(w->level == 1 ? &SPR_PolarB1 : w->level == 2 
				? &SPR_PolarB2 : &SPR_PolarB3, 0, 0, TILE_ATTR(PAL0, 0, 0, 0));
	b->damage = w->level + (w->level == 3 ? 1 : 0); // 1, 2, 4
	b->ttl = 20 + w->level * 5;
	b->hit_box = (bounding_box) { 4, 1 + w->level, 4, 1 + w->level };
	if(player.controller[0]&BUTTON_UP) {
		SPR_SAFEANIM(b->sprite, 1);
		b->x = player.x;
		b->y = player.y - pixel_to_sub(12);
		b->x_speed = 0;
		b->y_speed = pixel_to_sub(-4);
	} else if(!player.grounded && (player.controller[0]&BUTTON_DOWN)) {
		SPR_SAFEANIM(b->sprite, 1);
		b->x = player.x;
		b->y = player.y + pixel_to_sub(12);
		b->x_speed = 0;
		b->y_speed = pixel_to_sub(4);
	} else {
		b->x = player.x + (player.direction ? pixel_to_sub(12) : pixel_to_sub(-12));
		b->y = player.y + pixel_to_sub(4);
		b->x_speed = (player.direction ? pixel_to_sub(4) : pixel_to_sub(-4));
		b->y_speed = 0;
	}
}

void weapon_fire_fireball(Weapon *w) {
	// The real cave story stores bullets for weapons in separate lists most likely,
	// since you can switch back and forth to get 3 polar star and 3 fireball bullets.
	// I try to simulate this by storing fireball bullets in a different part of the array
	// Max bullets for lv 1,2,3: 2,3,4
	Bullet *b = NULL;
	for(u8 i = 2 + (3 - w->level); i < 6; i++) {
		if(playerBullet[i].ttl > 0) continue;
		b = &playerBullet[i];
		break;
	}
	if(b == NULL) return;
	sound_play(SND_FIREBALL, 5);
	b->type = w->type;
	b->level = w->level;
	b->sprite = SPR_addSprite(&SPR_FirebB1, 0, 0, TILE_ATTR(PAL0, 0, 0, 0));
	b->damage = 2 * w->level; // 2, 4, 6
	b->ttl = 50 + w->level * 10;
	b->hit_box = (bounding_box) { 4, 4, 4, 4 };
	if(player.controller[0]&BUTTON_UP) {
		b->x = player.x;
		b->y = player.y - pixel_to_sub(12);
		b->x_speed = player.direction ? 0x300 : -0x300; // 1.5
		b->y_speed = pixel_to_sub(-3);
	} else if(!player.grounded && (player.controller[0]&BUTTON_DOWN)) {
		b->x = player.x;
		b->y = player.y + pixel_to_sub(12);
		b->x_speed = player.direction ? 0x300 : -0x300; // 1.5
		b->y_speed = pixel_to_sub(2);
	} else {
		b->x = player.x + (player.direction ? pixel_to_sub(12) : pixel_to_sub(-12));
		b->y = player.y + pixel_to_sub(4);
		b->x_speed = player.direction ? 0x500 : -0x500; // 2.5
		b->y_speed = 0;
	}
}

void weapon_fire_machinegun(Weapon *w) {
	
}

void weapon_fire_missile(Weapon *w) {
	// Use end of array for missiles
	// Max bullets for lv 1,2,3: 1,2,6
	Bullet *b = NULL;
	for(u8 i = 4 + (w->level == 1 ? 5 : w->level == 2 ? 4 : 0); i < 10; i++) {
		if(playerBullet[i].ttl > 0) continue;
		b = &playerBullet[i];
		break;
	}
	if(b == NULL || w->ammo == 0) return;
	w->ammo--;
	b->type = w->type;
	b->level = w->level;
	b->sprite = SPR_addSprite(w->level == 2 ? &SPR_MisslB2 : &SPR_MisslB1, 
		0, 0, TILE_ATTR(PAL0, 0, 0, 0));
	b->damage = 0; // 0 damage because an explosion object causes damage instead
	b->ttl = 120;
	b->hit_box = (bounding_box) { 4, 4, 4, 4 };
	if(player.controller[0]&BUTTON_UP) {
		SPR_SAFEANIM(b->sprite, 1);
		b->x = player.x;
		b->y = player.y - pixel_to_sub(12);
		b->x_speed = 0;
		b->y_speed = pixel_to_sub(-3);
	} else if(!player.grounded && (player.controller[0]&BUTTON_DOWN)) {
		SPR_SAFEANIM(b->sprite, 2);
		b->x = player.x;
		b->y = player.y + pixel_to_sub(12);
		b->x_speed = 0;
		b->y_speed = pixel_to_sub(3);
	} else {
		b->x = player.x + (player.direction ? pixel_to_sub(12) : pixel_to_sub(-12));
		b->y = player.y + pixel_to_sub(2);
		b->x_speed = (player.direction ? pixel_to_sub(3) : pixel_to_sub(-3));
		b->y_speed = 0;
	}
}

void weapon_fire_bubbler(Weapon *w) {
	
}

void weapon_fire_blade(Weapon *w) {
	
}

void weapon_fire_supermissile(Weapon *w) {
	
}

void weapon_fire_nemesis(Weapon *w) {
	
}

void weapon_fire_spur(Weapon *w) {
	
}

void bullet_update_none(Bullet *b) { }

void bullet_update_snake(Bullet *b) {
	
}

void bullet_update_polarstar(Bullet *b) {
	b->x += b->x_speed;
	b->y += b->y_speed;
	u8 block = stage_get_block_type(sub_to_block(b->x), sub_to_block(b->y));
	// Check if bullet is colliding with a breakable block
	if(block == 0x43) {
		b->ttl = 0;
		SPR_SAFERELEASE(b->sprite);
		stage_replace_block(sub_to_block(b->x), sub_to_block(b->y), 0);
		effect_create_smoke(1, sub_to_pixel(b->x), sub_to_pixel(b->y));
		sound_play(SND_BLOCK_DESTROY, 5);
	} else if(block == 0x41) { // Bullet hit a wall
		b->ttl = 0;
		SPR_SAFERELEASE(b->sprite);
		sound_play(SND_TINK, 3);
		// TODO: Add the sprite and effect for hitting a wall
	} else if(--b->ttl == 0) { // Bullet time to live expired
		SPR_SAFERELEASE(b->sprite);
	} else {
		SPR_SAFEMOVE(b->sprite, 
			sub_to_pixel(b->x - camera.x) + SCREEN_HALF_W - 8,
			sub_to_pixel(b->y - camera.y) + SCREEN_HALF_H - 8);
	}
}

void bullet_update_fireball(Bullet *b) {
	if(--b->ttl == 0) { // Bullet time to live expired
		SPR_SAFERELEASE(b->sprite);
		return;
	}
	u8 block_below = b->y_speed > 0 
			? stage_get_block_type(sub_to_block(b->x), sub_to_block(b->y + 0x800)) : 0;
	u8 block_above = b->y_speed < 0
				? stage_get_block_type(sub_to_block(b->x), sub_to_block(b->y - 0x800)) : 0;
	u8 block_front = stage_get_block_type(
			sub_to_block(b->x + (b->x_speed > 0 ? 0x800 : -0x800)), sub_to_block(b->y));
	if(block_front == 0x41 || block_front == 0x43) { // Bullet hit a wall
		b->x_speed = -b->x_speed;
	} else if(block_front & BLOCK_SLOPE) {
		b->y_speed = -b->y_speed >> 1;
		if(b->y_speed > -0x300) b->y_speed = -0x300;
	}
	if(block_below == 0x41 || block_below == 0x43) {
		b->y_speed = -b->y_speed >> 1;
		if(b->y_speed > -0x300) b->y_speed = -0x300;
	} else if(block_below == 0x41 || block_below == 0x43) {
		b->y_speed = -b->y_speed >> 1;
		if(b->y_speed < 0x300) b->y_speed = 0x300;
	} else {
		b->y_speed += GRAVITY;
		if(b->y_speed > 0x600) b->y_speed = 0x600;
	}
	b->x += b->x_speed;
	b->y += b->y_speed;
	SPR_SAFEMOVE(b->sprite, 
		sub_to_pixel(b->x - camera.x) + SCREEN_HALF_W - 8,
		sub_to_pixel(b->y - camera.y) + SCREEN_HALF_H - 8);
}

void bullet_update_machinegun(Bullet *b) {
	
}

void bullet_update_missile(Bullet *b) {
	if(--b->ttl == 0) { // Bullet time to live expired
		SPR_SAFERELEASE(b->sprite);
		return;
	}
	if(b->x_speed != 0 || b->y_speed != 0) {
		b->x += b->x_speed;
		b->y += b->y_speed;
		u8 block = stage_get_block_type(sub_to_block(b->x), sub_to_block(b->y));
		if(block == 0x41) { // Explode
			bullet_missile_explode(b);
		} else if(block == 0x43) {
			bullet_missile_explode(b);
			stage_replace_block(sub_to_block(b->x), sub_to_block(b->y), 0);
			effect_create_smoke(1, sub_to_pixel(b->x), sub_to_pixel(b->y));
			sound_play(SND_BLOCK_DESTROY, 5);
		}
		SPR_SAFEMOVE(b->sprite, 
			sub_to_pixel(b->x - camera.x) + SCREEN_HALF_W - 8,
			sub_to_pixel(b->y - camera.y) + SCREEN_HALF_H - 8);
	}
}

void bullet_update_bubbler(Bullet *b) {
	
}

void bullet_update_blade(Bullet *b) {
	
}

void bullet_update_supermissile(Bullet *b) {
	
}

void bullet_update_nemesis(Bullet *b) {
	
}

void bullet_update_spur(Bullet *b) {
	
}

Bullet *bullet_colliding(Entity *e) {
	for(u8 i = 0; i < MAX_BULLETS; i++) {
		if(playerBullet[i].ttl == 0) continue;
		bounding_box bb = playerBullet[i].hit_box;
		if(sub_to_pixel(playerBullet[i].x) - bb.left >= 
			sub_to_pixel(e->x) + e->hit_box.right) continue;
		if(sub_to_pixel(playerBullet[i].x) + bb.right <= 
			sub_to_pixel(e->x) - e->hit_box.left) continue;
		if(sub_to_pixel(playerBullet[i].y) - bb.top >= 
			sub_to_pixel(e->y) + e->hit_box.bottom) continue;
		if(sub_to_pixel(playerBullet[i].y) + bb.bottom <= 
			sub_to_pixel(e->y) - e->hit_box.top) continue;
		return &playerBullet[i];
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
	SPR_SAFERELEASE(b->sprite);
	for(u8 i =  b->level; i < 4; i++) {
		effect_create_smoke(0, 
			sub_to_pixel(b->x) - 10 + (random() % 20), 
			sub_to_pixel(b->y) - 10 + (random() % 20));
	}
}
