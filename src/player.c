#include "player.h"

#include <genesis.h>
#include "input.h"
#include "camera.h"
#include "resources.h"
#include "sprite.h"
#include "tsc.h"
#include "system.h"
#include "sound.h"
#include "audio.h"
#include "effect.h"
#include "stage.h"
#include "tables.h"

#define ANIM_STANDING 0
#define ANIM_WALKING 1
#define ANIM_LOOKUP 2
#define ANIM_LOOKUPWALK 3
#define ANIM_INTERACT 4
#define ANIM_JUMPING 5
#define ANIM_LOOKUPJUMP 6
#define ANIM_LOOKDOWNJUMP 7

#define INVINCIBILITY_FRAMES 60

u16 dummyController[2] = { 0, 0 };
u8 playerIFrames = 0;
bool playerDead = false;
u8 weaponCount = 0;

void player_update_entity_collision();
void player_update_bounds();
void player_update_shooting();
void player_update_bullets();
void player_update_interaction();

// Default values for player
void player_init() {
	controlsLocked = false;
	player_reset_sprites();
	playerFacing = 0;
	player.flags = NPC_IGNORE44|NPC_SHOWDAMAGE;
	player.controller = &joystate;
	playerMaxHealth = 3;
	player.health = 3;
	playerMaxAir = 1800; playerAir = playerMaxAir;
	player.x = block_to_sub(10) + pixel_to_sub(8);
	player.y = block_to_sub(8) + pixel_to_sub(8);
	player.x_next = player.x;
	player.y_next = player.y;
	player.x_speed = 0;
	player.y_speed = 0;
	//player.accel = 0x55 >> 1;
	//player.max_speed = 0x32C >> 1;
	// Actually 6,6,5,8 but need to get directional hitboxes working first
	player.hit_box = (bounding_box){ 6, 6, 6, 8 };
	//player.active = true;
	playerEquipment = 0; // Nothing equipped
	for(u8 i = 0; i < 32; i++) playerInventory[i] = 0; // Empty inventory
	for(u8 i = 0; i < 8; i++) playerWeapon[i].type = 0; // No weapons
	for(u8 i = 0; i < 3; i++) playerBullet[i].ttl = 0; // No bullets
	playerDead = false;
	currentWeapon = 0;
}

void player_reset_sprites() {
	player.sprite = sprite_create(&SPR_Quote, PAL0, false);
	sprite_set_attr(player.sprite, TILE_ATTR(PAL0, false, false, playerFacing));
	if(playerWeapon[currentWeapon].type > 0) {
		playerWeapon[currentWeapon].sprite = sprite_create(
			weapon_info[playerWeapon[currentWeapon].type].sprite, PAL1, false);
	}
}

void player_update() {
	if(playerDead) return;
	entity_update_movement(&player);
	entity_update_collision(&player);
	player_update_entity_collision();
	player_update_bounds();
	player.x = player.x_next;
	player.y = player.y_next;
	if(player.health == 0) {
		playerIFrames = 0;
		sprite_set_visible(player.sprite, false);
		playerDead = true;
		return;
	}
	player_update_shooting();
	player_update_bullets();
	player_update_interaction();
	player_draw();
}

void player_update_shooting() {
	if((player.controller[0]&BUTTON_B) && !(player.controller[1]&BUTTON_B)) {
		Weapon *w = &playerWeapon[currentWeapon];
		if(w->type == 2) { // Polar Star
			Bullet *b = NULL;
			for(u8 i = 0; i < 3; i++) {
				if(playerBullet[i].ttl > 0) continue;
				b = &playerBullet[i];
				break;
			}
			if(b != NULL) {
				if(w->level == 3) sound_play(0x31, 5);
				else sound_play(0x20, 5);
				b->sprite = sprite_create(weapon_info[2].bulletSprite[w->level-1], PAL0, false);
				b->damage = w->level;
				b->ttl = 20 + w->level * 5;
				if(player.controller[0]&BUTTON_UP) {
					sprite_set_animation(b->sprite, 1);
					b->x = player.x;
					b->y = player.y - pixel_to_sub(8);
					b->x_speed = 0;
					b->y_speed = pixel_to_sub(-4);
				} else {
					b->x = player.x - pixel_to_sub(8) + pixel_to_sub(16) * playerFacing;
					b->y = player.y + pixel_to_sub(4);
					b->x_speed = pixel_to_sub(-4) + pixel_to_sub(8) * playerFacing;
					b->y_speed = 0;
				}
			}
		}
	}
}

void player_update_bullets() {
	for(u8 i = 0; i < 3; i++) {
		Bullet *b = &playerBullet[i];
		if(b->ttl == 0) continue;
		b->x += b->x_speed;
		b->y += b->y_speed;
		sprite_set_position(b->sprite,
				sub_to_pixel(b->x - camera.x) + SCREEN_HALF_W - 4,
				sub_to_pixel(b->y - camera.y) + SCREEN_HALF_H - 4);
		if(--b->ttl == 0) sprite_delete(b->sprite);
	}
}

void player_update_interaction() {
	// Interaction with entities when pressing down
	if((player.controller[0]&BUTTON_DOWN) && !(player.controller[1]&BUTTON_DOWN)) {
		Entity *e = entityList;
		while(e != NULL) {
			if((e->flags & NPC_INTERACTIVE) && entity_overlapping(&player, e)) {
				player.controller[1] |= BUTTON_DOWN; // To avoid triggering it twice
				if(e->event > 0) tsc_call_event(e->event);
				return;
			}
			e = e->next;
		}
		// TODO: Question mark above head
	}
}

void player_draw() {
	// Sprite Animation
	u8 anim;
	if(player.grounded) {
		if(player.controller[0]&BUTTON_UP) {
			if((player.controller[0]&BUTTON_RIGHT) ||
					(player.controller[0]&BUTTON_LEFT)) {
				anim = ANIM_LOOKUPWALK;
			} else {
				anim = ANIM_LOOKUP;
			}
		} else if((player.controller[0]&BUTTON_RIGHT) ||
				(player.controller[0]&BUTTON_LEFT)) {
			anim = ANIM_WALKING;
		} else if((player.controller[0]&BUTTON_DOWN)) {
			anim = ANIM_INTERACT;
		} else {
			anim = ANIM_STANDING;
		}
	} else {
		if((player.controller[0]&BUTTON_UP)) {
			anim = ANIM_LOOKUPJUMP;
		} else if((player.controller[0]&BUTTON_DOWN)) {
			anim = ANIM_LOOKDOWNJUMP;
		} else {
			anim = ANIM_JUMPING;
		}
	}
	sprite_set_animation(player.sprite, anim);
	if((player.controller[0]&BUTTON_RIGHT) && playerFacing == 0) {
		sprite_set_attr(player.sprite, TILE_ATTR(PAL0, false, false, true));
		playerFacing = 1;
	} else if((player.controller[0]&BUTTON_LEFT) && playerFacing == 1) {
		sprite_set_attr(player.sprite, TILE_ATTR(PAL0, false, false, false));
		playerFacing = 0;
	}
	sprite_set_visible(player.sprite, !((playerIFrames / 2) % 2));
	sprite_set_position(player.sprite,
			sub_to_pixel(player.x) - sub_to_pixel(camera.x) + SCREEN_HALF_W - 8,
			sub_to_pixel(player.y) - sub_to_pixel(camera.y) + SCREEN_HALF_H - 8);
	// Weapon sprite
	if(playerWeapon[currentWeapon].type > 0) {
		bool wanim = 0;
		if(anim==ANIM_LOOKUP || anim==ANIM_LOOKUPWALK || anim==ANIM_LOOKUPJUMP) wanim = 1;
		else if(anim==ANIM_LOOKDOWNJUMP) wanim = 2;
		sprite_set_animation(playerWeapon[currentWeapon].sprite, wanim);
		sprite_set_attr(playerWeapon[currentWeapon].sprite, TILE_ATTR(PAL1, true, false, playerFacing));
		sprite_set_visible(playerWeapon[currentWeapon].sprite, !((playerIFrames / 2) % 2));
		sprite_set_position(playerWeapon[currentWeapon].sprite,
			sub_to_pixel(player.x) - sub_to_pixel(camera.x) + SCREEN_HALF_W - 12,
			sub_to_pixel(player.y) - sub_to_pixel(camera.y) + SCREEN_HALF_H - 8);

	}
}

void player_lock_controls() {
	controlsLocked = true;
	player.controller = dummyController;
}

void player_unlock_controls() {
	controlsLocked = false;
	player.controller = &joystate;
}

void player_update_entity_collision() {
	if(playerIFrames > 0) playerIFrames--;
	Entity *e = entityList;
	while(e != NULL) {
		if(playerIFrames == 0 && e->attack > 0 && entity_overlapping(&player, e)) {
			// Take health
			if(player.health <= e->attack) {
				player.health = 0;
				sound_play(SOUND_DIE, 15);
				tsc_call_event(PLAYER_DEFEATED_EVENT);
				break;
			}
			player.health -= e->attack;
			sound_play(SOUND_HURT, 5);
			// Show damage numbers
			effect_create_damage_string(-e->attack,
					sub_to_pixel(player.x), sub_to_pixel(player.y), 60);
			playerIFrames = INVINCIBILITY_FRAMES;
			// TODO: Find an accurate value for knock back
			player.y_speed -= pixel_to_sub(2);
		}
		// TODO: Solid Entities
		if(e->flags & NPC_SOLID) {

		}
		e = e->next;
	}
}

void player_update_bounds() {
	if(player.y_next > block_to_sub(stageHeight + 1)) {
		player.health = 0;
		tsc_call_event(PLAYER_OOB_EVENT);
	}
}

Weapon *player_find_weapon(u8 id) {
	for(u8 i = 0; i < weaponCount; i++) {
		if(playerWeapon[i].type == id) return &playerWeapon[i];
	}
	return NULL;
}

void player_give_weapon(u8 id, u8 ammo) {
	Weapon *w = player_find_weapon(id);
	if(w == NULL) {
		w = &playerWeapon[weaponCount];
		w->sprite = sprite_create(weapon_info[id].sprite, PAL1, true);
		w->type = id;
		w->level = 1;
		w->energy = 0;
		w->maxammo = 0;
		w->ammo = 0;
	}
	w->maxammo += ammo;
	w->ammo += ammo;
}

void player_take_weapon(u8 id) {

}

bool player_has_weapon(u8 id) {
	for(u8 i = 0; i < weaponCount; i++) {
		if(playerWeapon[i].type == id) return true;
	}
	return false;
}

void player_trade_weapon(u8 id_take, u8 id_give, u8 ammo) {

}

void player_refill_ammo() {

}

void player_take_allweapons() {

}

void player_heal(u8 health) {
	player.health += health;
	if(player.health > playerMaxHealth) player.health = playerMaxHealth;
}

void player_maxhealth_increase(u8 health) {
	player.health += health;
	playerMaxHealth += health;
}

void player_give_item(u8 id) {
	for(u8 i = 0; i < MAX_ITEMS; i++) {
		if(playerInventory[i] == 0) {
			playerInventory[i] = id;
			break;
		}
	}
}

void player_take_item(u8 id) {
	for(u8 i = 0; i < MAX_ITEMS; i++) {
		if(playerInventory[i] == id) {
			playerInventory[i] = 0;
			break;
		}
	}
}

bool player_has_item(u8 id) {
	for(u8 i = 0; i < MAX_ITEMS; i++) {
		if(playerInventory[i] == id) return true;
	}
	return false;
}
