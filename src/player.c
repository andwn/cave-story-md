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

#define WEAPON_NONE 0
#define WEAPON_SNAKE 1
#define WEAPON_POLARSTAR 2
#define WEAPON_FIREBALL 3
#define WEAPON_MACHINEGUN 4
#define WEAPON_MISSILES 5
#define WEAPON_BUBBLER 7
#define WEAPON_BLADE 9
#define WEAPON_SUPERMISSILES 10
#define WEAPON_NEMESIS 12
#define WEAPON_SPUR 13

#define INVINCIBILITY_FRAMES 60

#define MAX_AIR_NTSC (30 * 100)
#define MAX_AIR_PAL (25 * 100)

u16 dummyController[2] = { 0, 0 };
u8 playerIFrames = 0;
bool playerDead = false;
u8 playerWeaponCount = 0;

void player_update_entity_collision();
void player_update_bounds();
void player_update_shooting();
void player_update_bullets();
void player_update_interaction();

// Default values for player
void player_init() {
	controlsLocked = false;
	player_reset_sprites();
	player.direction = 0;
	player.eflags = NPC_IGNORE44|NPC_SHOWDAMAGE;
	player.controller = &joystate;
	playerMaxHealth = 3;
	player.health = 3;
	playerMaxAir = MAX_AIR_NTSC; playerAir = playerMaxAir;
	player.x = block_to_sub(10) + pixel_to_sub(8);
	player.y = block_to_sub(8) + pixel_to_sub(8);
	player.x_next = player.x;
	player.y_next = player.y;
	player.x_speed = 0;
	player.y_speed = 0;
	player.damage_time = 0;
	player.damage_value = 0;
	// Actually 6,6,5,8 but need to get directional hitboxes working first
	player.hit_box = (bounding_box){ 6, 6, 6, 8 };
	playerEquipment = 0; // Nothing equipped
	for(u8 i = 0; i < 32; i++) playerInventory[i] = 0; // Empty inventory
	for(u8 i = 0; i < 8; i++) playerWeapon[i].type = 0; // No playerWeapons
	for(u8 i = 0; i < 3; i++) playerBullet[i].ttl = 0; // No bullets
	playerDead = false;
	currentWeapon = 0;
}

void player_reset_sprites() {
	player.sprite = sprite_create(&SPR_Quote, PAL0, false);
	sprite_set_attr(player.sprite, TILE_ATTR(PAL0, false, false, player.direction));
	if(playerWeapon[currentWeapon].type > 0) {
		playerWeapon[currentWeapon].sprite = sprite_create(
			weapon_info[playerWeapon[currentWeapon].type].sprite, PAL1, false);
	}
}

void player_update() {
	if(playerDead) return;
	if(debuggingEnabled && (joystate&BUTTON_A)) {
		entity_update_float(&player);
		player.x_next = player.x + player.x_speed;
		player.y_next = player.y + player.y_speed;
	} else {
		entity_update_movement(&player);
		entity_update_collision(&player);
		player_update_entity_collision();
		player_update_bounds();
	}
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

Bullet *bullet_colliding(Entity *e) {
	for(u8 i = 0; i < 3; i++) {
		if(playerBullet[i].ttl > 0) {
			Bullet *b = &playerBullet[i];
			s16 bx = sub_to_pixel(b->x), by = sub_to_pixel(b->y);
			if(bx-4 < sub_to_pixel(e->x)+e->hit_box.right &&
					bx+4 > sub_to_pixel(e->x)-e->hit_box.left &&
					by-4 < sub_to_pixel(e->y)+e->hit_box.bottom &&
					by+4 > sub_to_pixel(e->y)-e->hit_box.top) {
				return b;
			}
		}
	}
	return NULL;
}

void player_update_shooting() {
	if((player.controller[0]&BUTTON_Y) && !(player.controller[1]&BUTTON_Y)) {
		for(u8 i = (currentWeapon-1) % 8; i != currentWeapon; i = (i-1) % 8) {
			if(playerWeapon[i].type > 0) {
				sprite_delete(playerWeapon[currentWeapon].sprite);
				currentWeapon = i;
				playerWeapon[i].sprite = sprite_create(
						weapon_info[playerWeapon[i].type].sprite, PAL1, true);
				break;
			}
		}
	} else if((player.controller[0]&BUTTON_Z) && !(player.controller[1]&BUTTON_Z)) {
		for(u8 i = (currentWeapon+1) % 8; i != currentWeapon; i = (i+1) % 8) {
			if(playerWeapon[i].type > 0) {
				sprite_delete(playerWeapon[currentWeapon].sprite);
				currentWeapon = i;
				playerWeapon[i].sprite = sprite_create(
						weapon_info[playerWeapon[i].type].sprite, PAL1, true);
				break;
			}
		}
	} else if((player.controller[0]&BUTTON_B) && !(player.controller[1]&BUTTON_B)) {
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
					sprite_set_frame(b->sprite, 1);
					b->x = player.x;
					b->y = player.y - pixel_to_sub(12);
					b->x_speed = 0;
					b->y_speed = pixel_to_sub(-4);
				} else if(!player.grounded && (player.controller[0]&BUTTON_DOWN)) {
					sprite_set_frame(b->sprite, 1);
					b->x = player.x;
					b->y = player.y + pixel_to_sub(12);
					b->x_speed = 0;
					b->y_speed = pixel_to_sub(4);
				} else {
					b->x = player.x - pixel_to_sub(12) + pixel_to_sub(24) * player.direction;
					b->y = player.y + pixel_to_sub(4);
					b->x_speed = pixel_to_sub(-4) + pixel_to_sub(8) * player.direction;
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
				sub_to_pixel(b->x - camera.x) + SCREEN_HALF_W - 8,
				sub_to_pixel(b->y - camera.y) + SCREEN_HALF_H - 8);
		u16 bx = sub_to_block(b->x), by = sub_to_block(b->y);
		if(stage_get_block_type(bx, by) == 0x43) {
			sprite_delete(b->sprite);
			b->ttl = 0;
			sound_play(SOUND_BREAK, 8);
			stage_replace_block(bx, by, 0);
			continue;
		}
		if(--b->ttl == 0) sprite_delete(b->sprite);
	}
}

void player_update_interaction() {
	// Interaction with entities when pressing down
	if((player.controller[0]&BUTTON_DOWN) && !(player.controller[1]&BUTTON_DOWN)) {
		Entity *e = entityList;
		while(e != NULL) {
			if((e->eflags & NPC_INTERACTIVE) && entity_overlapping(&player, e)) {
				player.controller[1] |= BUTTON_DOWN; // To avoid triggering it twice
				if(e->event > 0) {
					tsc_call_event(e->event);
					return;
				}
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
	if((player.controller[0]&BUTTON_RIGHT) && player.direction == 0) {
		sprite_set_attr(player.sprite, TILE_ATTR(PAL0, false, false, true));
		player.direction = 1;
	} else if((player.controller[0]&BUTTON_LEFT) && player.direction == 1) {
		sprite_set_attr(player.sprite, TILE_ATTR(PAL0, false, false, false));
		player.direction = 0;
	}
	sprite_set_visible(player.sprite, !((playerIFrames / 2) % 2));
	sprite_set_position(player.sprite,
			sub_to_pixel(player.x) - sub_to_pixel(camera.x) + SCREEN_HALF_W - 8,
			sub_to_pixel(player.y) - sub_to_pixel(camera.y) + SCREEN_HALF_H - 8);
	// Weapon sprite
	if(playerWeapon[currentWeapon].type > 0) {
		u8 wanim = 0;
		if(anim==ANIM_LOOKUP || anim==ANIM_LOOKUPWALK || anim==ANIM_LOOKUPJUMP) wanim = 1;
		else if(anim==ANIM_LOOKDOWNJUMP) wanim = 2;
		sprite_set_animation(playerWeapon[currentWeapon].sprite, wanim);
		sprite_set_attr(playerWeapon[currentWeapon].sprite, TILE_ATTR(PAL1, false, false, player.direction));
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
	// This "damage_time" block is for energy numbers
	// When the player gets hurt damage numbers are created instantly
	if(player.damage_time > 0) {
		player.damage_time--;
		if(player.damage_time == 0) {
			effect_create_damage_string(player.damage_value,
					sub_to_pixel(player.x) - 8, sub_to_pixel(player.y) - 4, 60);
			player.damage_value = 0;
		}
	}
	Entity *e = entityList;
	while(e != NULL) {
		// Handle special cases
		// TODO: Maybe this could be handled with function pointers instead
		// The "continue" statements here go to the next iteration of the while loop
		switch(e->type) {
		case 1: // Energy
			// Increase weapon level/energy
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
				e = entity_delete(e);
				continue;
			}
			break;
		//case 46: // Trigger
			// Call the trigger's event if an event isn't already running
			// We don't delete the trigger here, the event takes care of it
		//	if(entity_overlapping(&player, e) && !tsc_running()) {
		//		tsc_call_event(e->event);
		//		e = e->next;
		//		continue;
		//	}
		//	break;
		case 87: // Heart
			// Increases health, plays sound and deletes itself
			if(entity_overlapping(&player, e)) {
				player.health += e->health;
				// Don't go over max health
				if(player.health >= playerMaxHealth) player.health = playerMaxHealth;
				sound_play(SOUND_REFILL, 5);
				e = entity_delete(e);
				continue;
			}
			break;
		default:
			break;
		}
		// Solid Entities
		bounding_box collision = { 0, 0, 0, 0 };
		if((e->eflags|e->nflags) & NPC_SOLID) {
			collision = entity_react_to_collision(&player, e);
			if(collision.bottom && ((e->eflags|e->nflags) & NPC_BOUNCYTOP)) {
				player.y_speed = pixel_to_sub(-1);
				player.grounded = false;
			}
		}
		// Enemies
		if(e->attack > 0) {
			u32 collided = *(u32*)&collision; // I do what I want
			if((collided > 0 || entity_overlapping(&player, e)) && playerIFrames == 0) {
				// If the enemy has NPC_FRONTATKONLY, and the player is not colliding
				// with the front of the enemy, the player shouldn't get hurt
				if((e->eflags|e->nflags)&NPC_FRONTATKONLY) {
					if((e->direction == 0 && collision.right == 0) ||
							(e->direction > 0 && collision.left == 0)) {
						e = e->next;
						continue;
					}
				}
				// Take health
				if(player.health <= e->attack) {
					// If health reached 0 we are dead
					player.health = 0;
					effect_create_smoke(2, player.x, player.y);
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
				// Knock back
				player.y_speed = -0x300; // 1.5 pixels per frame
				player.grounded = false;
			}
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
	for(u8 i = 0; i < playerWeaponCount; i++) {
		if(playerWeapon[i].type == id) return &playerWeapon[i];
	}
	return NULL;
}

void player_give_weapon(u8 id, u8 ammo) {
	Weapon *w = player_find_weapon(id);
	if(w == NULL) {
		sprite_delete(playerWeapon[currentWeapon].sprite);
		for(u8 i = 0; i < 8; i++) {
			if(playerWeapon[i].type > 0) continue;
			w = &playerWeapon[i];
			w->sprite = sprite_create(weapon_info[id].sprite, PAL1, true);
			w->type = id;
			w->level = 1;
			w->energy = 0;
			w->maxammo = 0;
			w->ammo = 0;
			break;
		}
	}
	w->maxammo += ammo;
	w->ammo += ammo;
}

void player_take_weapon(u8 id) {
	Weapon *w = player_find_weapon(id);
	if(w != NULL) {
		if(w == &playerWeapon[currentWeapon]) {
			//player_next_weapon();
		}
		w->type = 0;
	}
}

bool player_has_weapon(u8 id) {
	for(u8 i = 0; i < playerWeaponCount; i++) {
		if(playerWeapon[i].type == id) return true;
	}
	return false;
}

void player_trade_weapon(u8 id_take, u8 id_give, u8 ammo) {
	Weapon *w = player_find_weapon(id_take);
	if(w != NULL) {
		w->sprite = sprite_create(weapon_info[id_give].sprite, PAL1, true);
		w->type = id_give;
		w->level = 1;
		w->energy = 0;
		w->maxammo = ammo;
		w->ammo = ammo;
	}
}

void player_refill_ammo() {
	for(u8 i = 0; i < playerWeaponCount; i++) {
		playerWeapon[i].ammo = playerWeapon[i].maxammo;
	}
}

void player_take_allweapons() {
	for(u8 i = 0; i < 8; i++) playerWeapon[i].type = 0;
	playerWeaponCount = 0;
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

void player_equip(u16 id) {
	playerEquipment |= id;
}

void player_unequip(u16 id) {
	playerEquipment &= ~id;
}
