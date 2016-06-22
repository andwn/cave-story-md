#include "player.h"

#include "input.h"
#include "camera.h"
#include "resources.h"
#include "tsc.h"
#include "system.h"
#include "sound.h"
#include "audio.h"
#include "effect.h"
#include "stage.h"
#include "tables.h"
#include "vdp_ext.h"

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

#define INVINCIBILITY_FRAMES 120

#define MAX_AIR_NTSC (12 * 100)
#define MAX_AIR_PAL (10 * 100)

u16 dummyController[2] = { 0, 0 };
u8 playerIFrames = 0;
bool playerDead = false;
bool playerShow;
u8 playerWeaponCount = 0;
Sprite *weaponSprite = NULL;
Sprite *airSprite = NULL;
Sprite *mapNameSprite = NULL;
u16 mapNameTTL = 0;
u8 airTime = 0;

void player_update_entity_collision();
void player_update_bounds();
void player_update_shooting();
void player_update_bullets();
void player_update_interaction();
void player_update_air_display();

// Default values for player
void player_init() {
	controlsLocked = false;
	playerShow = true;
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
	player.direction = 1;
	player.spriteAnim = 0;
	player_reset_sprites();
	// Actually 6,6,5,8 but need to get directional hitboxes working first
	player.hit_box = (bounding_box){ 6, 6, 5, 8 };
	playerEquipment = 0; // Nothing equipped
	for(u8 i = 0; i < MAX_ITEMS; i++) playerInventory[i] = 0; // Empty inventory
	for(u8 i = 0; i < MAX_WEAPONS; i++) playerWeapon[i].type = 0; // No Weapons
	playerDead = false;
	currentWeapon = 0;
}

void player_reset_sprites() {
	// Manual visibility
	player.sprite = SPR_addSprite(&SPR_Quote, 
		sub_to_pixel(player.x) - sub_to_pixel(camera.x) + SCREEN_HALF_W - 8,
		sub_to_pixel(player.y) - sub_to_pixel(camera.y) + SCREEN_HALF_H - 8,
		TILE_ATTR(PAL0, 0, 0, player.direction));
	// Weapon
	if(playerWeapon[currentWeapon].type != 0) {
		weaponSprite = SPR_addSprite(weapon_info[playerWeapon[currentWeapon].type].sprite,
			sub_to_pixel(player.x) - sub_to_pixel(camera.x) + SCREEN_HALF_W - 12,
			sub_to_pixel(player.y) - sub_to_pixel(camera.y) + SCREEN_HALF_H - 8,
			TILE_ATTR(PAL1, 0, 0, player.direction));
	} else {
		weaponSprite = NULL;
	}
	// Clear bullets
	for(u8 i = 0; i < MAX_BULLETS; i++) {
		playerBullet[i].ttl = 0; // No bullets
		playerBullet[i].sprite = NULL;
	}
	// Clear air and map name
	airSprite = NULL;
	airTime = 0;
	playerAir = playerMaxAir;
	mapNameSprite = NULL;
	mapNameTTL = 0;
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
		playerDead = true;
		return;
	}
	// Handle air when underwater, unless a script is running
	if(tsc_running()) {
		SPR_SAFERELEASE(airSprite);
	} else {
		if(player.underwater) {
			if(--playerAir == 0) {
				// Spoilers
				//if(stageID == STAGE_ALMOND && system_get_flag(FLAG_CORE_DEFEATED)) {
				//	tsc_call_event();
				//} else {
					player.health = 0;
					SPR_SAFERELEASE(airSprite);
					SPR_SAFEANIM(player.sprite, 8);
					tsc_call_event(PLAYER_DROWN_EVENT);
					return;
				//}
			}
		} else if(playerAir < playerMaxAir) {
			playerAir = playerMaxAir;
			airTime = 60;
			if(playerAir > playerMaxAir) playerAir = playerMaxAir;
		} else {
			if(airTime > 0) {
				airTime--;
			} else {
				SPR_SAFERELEASE(airSprite);
			}
		}
		player_update_air_display();
	}
	player_update_shooting();
	player_update_bullets();
	player_update_interaction();
	player_draw();
	if(mapNameTTL > 0 && --mapNameTTL == 0) {
		SPR_SAFERELEASE(mapNameSprite);
	}
}

Bullet *bullet_colliding(Entity *e) {
	for(u8 i = 0; i < MAX_BULLETS; i++) {
		if(playerBullet[i].ttl == 0) continue;
		/*
		bounding_box *bb = &weapon_info[playerWeapon.type].hit_box; //&playerBullet[i].hit_box;
		bounding_box adjb = 
			playerBullet[i].dir == DIR_LEFT ? { bb->left, bb->top, bb->right, bb->bottom }
			: playerBullet[i].dir == DIR_UP ? { bb->top, bb->right, bb->bottom, bb->left }
			: playerBullet[i].dir == DIR_RIGHT ? { bb->right, bb->bottom, bb->left, bb->top }
			: { bb->bottom, bb->left, bb->top, bb->right };
		if(sub_to_pixel(playerBullet[i].x) - adjb.left >= 
			sub_to_pixel(e->x) + (e->direction ? e->hit_box.left : e->hit_box.right)) continue;
		if(sub_to_pixel(playerBullet[i].x) + adjb.right <= 
			sub_to_pixel(e->x) - (e->direction ? e->hit_box.right : e->hit_box.left)) continue;
		if(sub_to_pixel(playerBullet[i].y) - adjb.top >= 
			sub_to_pixel(e->y) + e->hit_box.bottom) continue;
		if(sub_to_pixel(playerBullet[i].y) + adjb.bottom <= 
			sub_to_pixel(e->y) - e->hit_box.top) continue;
		return b;
		* */
		/*
		bounding_box *bb = &playerBullet[i].hit_box, eb = &e->hit_box;
		s16 bx = sub_to_pixel(b->x), by = sub_to_pixel(b->y),
			ex = sub_to_pixel(e->x), ey = sub_to_pixel(e->y);
		s16 bx1 = sub_to_pixel(b->x) - (b->dirH ? b->hit_box.right : b->hit_box.left),
			bx2 = sub_to_pixel(b->x) + (b->dirH ? b->hit_box.left : b->hit_box.right),
			by1 = sub_to_pixel(b->y) - (b->dirV ? b->hit_box.bottom : b->hit_box.top),
			by2 = sub_to_pixel(b->y) + (b->dirV ? b->hit_box.top : b->hit_box.bottom),
			ex1 = sub_to_pixel(e->x) - (e->direction ? e->hit_box.right : e->hit_box.left),
			ex2 = sub_to_pixel(e->x) + (e->direction ? e->hit_box.left : e->hit_box.right),
			ey1 = sub_to_pixel(e->y) - e->hit_box.top,
			ey2 = sub_to_pixel(e->y) + e->hit_box.bottom;
		if(bx - (playerBullet[i].dirH ? bb->right : bb->left) 
				< ex + (e->direction ? eb->left : eb->right) &&
			bx + (playerBullet[i].dirH ? bb->left : bb->right) 
				< ex - (e->direction ? eb->right : eb->left) &&
			by - (playerBullet[i].dirV ? bb->right : bb->left) 
				< ex + (e->direction ? eb->left : eb->right) &&
			bx - (playerBullet[i].dirH ? bb->right : bb->left) 
				< ex + (e->direction ? eb->left : eb->right)) {
			return b;
		}
		* */
		Bullet *b = &playerBullet[i];
		s16 bx = sub_to_pixel(b->x), by = sub_to_pixel(b->y);
		if(bx - 4 < sub_to_pixel(e->x) + (e->direction ? e->hit_box.right : e->hit_box.left) &&
			bx + 4 > sub_to_pixel(e->x) - (e->direction ? e->hit_box.left : e->hit_box.right) &&
			by - 4 < sub_to_pixel(e->y) + e->hit_box.bottom &&
			by + 4 > sub_to_pixel(e->y) - e->hit_box.top) return b;
	}
	return NULL;
}

void player_update_shooting() {
	// Weapon switching
	if((player.controller[0]&BUTTON_Y) && !(player.controller[1]&BUTTON_Y)) {
		for(u8 i = (currentWeapon-1) % 8; i != currentWeapon; i = (i-1) % 8) {
			if(playerWeapon[i].type > 0) {
				SPR_SAFERELEASE(weaponSprite);
				currentWeapon = i;
				if(weapon_info[playerWeapon[i].type].sprite != NULL) {
					weaponSprite = SPR_addSprite(weapon_info[playerWeapon[i].type].sprite, 
						0, 0, TILE_ATTR(PAL1, 1, 0, player.direction));
				}
				break;
			}
		}
	} else if((player.controller[0]&BUTTON_Z) && !(player.controller[1]&BUTTON_Z)) {
		for(u8 i = (currentWeapon+1) % 8; i != currentWeapon; i = (i+1) % 8) {
			if(playerWeapon[i].type > 0) {
				SPR_SAFERELEASE(weaponSprite);
				currentWeapon = i;
				if(weapon_info[playerWeapon[i].type].sprite != NULL) {
					weaponSprite = SPR_addSprite(weapon_info[playerWeapon[i].type].sprite, 
						0, 0, TILE_ATTR(PAL1, 1, 0, player.direction));
				}
				break;
			}
		}
	// Shooting
	} else if((player.controller[0]&BUTTON_B) && !(player.controller[1]&BUTTON_B)) {
		Weapon *w = &playerWeapon[currentWeapon];
		Bullet *b = NULL;
		switch(w->type) {
			case WEAPON_POLARSTAR:
			// Polar Star allows 3 bullets max
			for(u8 i = 0; i < 3; i++) {
				if(playerBullet[i].ttl > 0) continue;
				b = &playerBullet[i];
				break;
			}
			if(b == NULL) break;
			if(w->level == 3) sound_play(0x31, 5);
			else sound_play(0x20, 5);
			b->sprite = SPR_addSprite(
				weapon_info[2].bulletSprite[w->level-1], 0, 0, TILE_ATTR(PAL0, 0, 0, 0));
			b->damage = weapon_info[w->type].damage[w->level - 1];
			b->ttl = 20 + w->level * 5;
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
				b->x = player.x - pixel_to_sub(12) + pixel_to_sub(24) * player.direction;
				b->y = player.y + pixel_to_sub(4);
				b->x_speed = pixel_to_sub(-4) + pixel_to_sub(8) * player.direction;
				b->y_speed = 0;
			}
			break;
			case WEAPON_FIREBALL:
			// Fireball also 3 max... TODO doublecheck that's correct
			for(u8 i = 0; i < 3; i++) {
				if(playerBullet[i].ttl > 0) continue;
				b = &playerBullet[i];
				break;
			}
			if(b == NULL) break;
			sound_play(0x22, 5);
			b->sprite = SPR_addSprite(
				weapon_info[2].bulletSprite[w->level-1], 0, 0, TILE_ATTR(PAL0, 0, 0, 0));
			b->damage = weapon_info[w->type].damage[w->level - 1];
			b->ttl = 20 + w->level * 5;
			break;
			case WEAPON_MISSILES:
			for(u8 i = 0; i < 3; i++) {
				if(playerBullet[i].ttl > 0) continue;
				b = &playerBullet[i];
				break;
			}
			if(b == NULL) break;
			
			break;
			case WEAPON_BUBBLER:
			break;
			case WEAPON_MACHINEGUN:
			break;
		}
	}
}

void player_update_bullets() {
	for(u8 i = 0; i < MAX_BULLETS; i++) {
		Bullet *b = &playerBullet[i];
		if(b->ttl == 0) continue;
		b->x += b->x_speed;
		b->y += b->y_speed;
		u16 bx = sub_to_block(b->x), by = sub_to_block(b->y);
		u8 block = stage_get_block_type(bx, by);
		// Check if bullet is colliding with a breakable block
		if(block == 0x43) {
			b->ttl = 0;
			SPR_SAFERELEASE(b->sprite);
			sound_play(SOUND_BREAK, 8);
			stage_replace_block(bx, by, 0);
		} else if(block == 0x41) { // Bullet hit a wall
			b->ttl = 0;
			SPR_SAFERELEASE(b->sprite);
			sound_play(0x1F, 5);
			// TODO: Add the sprite and effect for hitting a wall
		} else if(--b->ttl == 0) { // Bullet time to live expired
			SPR_SAFERELEASE(b->sprite);
		} else {
			SPR_SAFEMOVE(b->sprite, 
				sub_to_pixel(b->x - camera.x) + SCREEN_HALF_W - 8,
				sub_to_pixel(b->y - camera.y) + SCREEN_HALF_H - 8);
		}
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

void player_show() {
	playerShow = true;
	SPR_SAFEVISIBILITY(player.sprite, VISIBLE);
}

void player_hide() {
	playerShow = false;
	SPR_SAFEVISIBILITY(player.sprite, HIDDEN);
}

void player_show_map_name(u8 ttl) {
	// Define sprite for map name
	mapNameSprite = SPR_addSpriteEx(&SPR_Dummy16x1, SCREEN_HALF_W - 64, SCREEN_HALF_H - 32,
		TILE_ATTR_FULL(PAL0, 1, 0, 0, TILE_FACEINDEX + 36), 0, SPR_FLAG_AUTO_SPRITE_ALLOC);
	SPR_SAFEVISIBILITY(mapNameSprite, VISIBLE);
	// Create a string of tiles in RAM
	u32 nameTiles[16][8];
	for(u8 i = 0; i < 16; i++) {
		u8 chr = stage_info[stageID].name[i] - 0x20;
		if(chr >= 0x60) chr = 0;
		memcpy(nameTiles[i], &TS_SprFont.tiles[chr * 8], 32);
	}
	// Transfer tile array to VRAM
	SYS_disableInts();
	VDP_loadTileData(nameTiles[0], TILE_FACEINDEX + 36, 16, true);
	SYS_enableInts();
	mapNameTTL = ttl;
}

void draw_air_percent(u8 percent) {
	u32 numberTiles[3][8];
	memcpy(numberTiles[0], percent == 100 ? &TS_Numbers.tiles[8] : TILE_BLANK, 32);
	memcpy(numberTiles[1], &TS_Numbers.tiles[((percent / 10) % 10) * 8], 32);
	memcpy(numberTiles[2], &TS_Numbers.tiles[(percent % 10) * 8], 32);
	SYS_disableInts();
	VDP_loadTileData(numberTiles[0], TILE_AIRINDEX + 4, 3, true);
	SYS_enableInts();
}

void player_update_air_display() {
	if(playerAir == playerMaxAir) {
		if(airTime == 60) {
			draw_air_percent(100);
		} else if(airTime > 0) {
			SPR_SAFEVISIBILITY(airSprite, (airTime & 8) ? VISIBLE : HIDDEN);
		}
	} else {
		if(airSprite == NULL) { // Initialize and draw full "AIR" text
			airTime = 0;
			airSprite = SPR_addSpriteEx(&SPR_Air, SCREEN_HALF_W - 28, SCREEN_HALF_H - 24,
				TILE_ATTR_FULL(PAL0, 1, 0, 0, TILE_AIRINDEX), 0, SPR_FLAG_AUTO_SPRITE_ALLOC);
			SPR_SAFEVISIBILITY(airSprite, VISIBLE);
			SYS_disableInts();
			VDP_loadTileData(SPR_TILESET(SPR_Air, 0, 0)->tiles, TILE_AIRINDEX, 4, true);
			SYS_enableInts();
		} else { // Just blink the small down arrow thing
			SYS_disableInts();
			airTime++;
			if((airTime & 31) == 0) {
				VDP_loadTileData(TILE_BLANK, TILE_AIRINDEX, 1, true);
			} else if((airTime & 31) == 15) {
				VDP_loadTileData(SPR_TILESET(SPR_Air, 0, 0)->tiles, TILE_AIRINDEX, 1, true);
			}
			SYS_enableInts();
		}
		// Calculate air percent and display the value
		if((airTime % 12) == 0) draw_air_percent(100 * playerAir / playerMaxAir);
	}
}

void player_draw() {
	// Sprite Animation
	u8 anim;
	if(player.grounded) {
		if(player.controller[0]&BUTTON_UP) {
			if((player.controller[0]&BUTTON_RIGHT) || (player.controller[0]&BUTTON_LEFT)) {
				anim = ANIM_LOOKUPWALK;
			} else {
				anim = ANIM_LOOKUP;
			}
		} else if((player.controller[0]&BUTTON_RIGHT) || (player.controller[0]&BUTTON_LEFT)) {
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
	// Set animation if it changed
	if(player.spriteAnim != anim) {
		player.spriteAnim = anim;
		// Frame change is a workaround for tapping looking like quote is floating
		SPR_SAFEANIMFRAME(player.sprite, anim, 
			anim==ANIM_WALKING || anim==ANIM_LOOKUPWALK ? 1 : 0);
	}
	// Change direction if pressing left or right
	if(player.controller[0]&BUTTON_RIGHT && !player.direction) {
		player.direction = 1;
		SPR_SAFEHFLIP(player.sprite, 1);
	} else if(player.controller[0]&BUTTON_LEFT && player.direction) {
		player.direction = 0;
		SPR_SAFEHFLIP(player.sprite, 0);
	}
	SPR_SAFEMOVE(player.sprite,
		sub_to_pixel(player.x) - sub_to_pixel(camera.x) + SCREEN_HALF_W - 8,
		sub_to_pixel(player.y) - sub_to_pixel(camera.y) + SCREEN_HALF_H - 8);
	// Blink during invincibility frames
	SPR_SAFEVISIBILITY(player.sprite, playerShow && 
		!((playerIFrames >> 1) & 1) ? VISIBLE : HIDDEN);
	// Weapon sprite
	if(playerWeapon[currentWeapon].type > 0 && weaponSprite != NULL) {
		u8 wanim = 0;
		if(anim==ANIM_LOOKUP || anim==ANIM_LOOKUPWALK || anim==ANIM_LOOKUPJUMP) wanim = 1;
		else if(anim==ANIM_LOOKDOWNJUMP) wanim = 2;
		SPR_SAFEANIM(weaponSprite, wanim);
		SPR_SAFEHFLIP(weaponSprite, player.direction);
		SPR_SAFEVISIBILITY(weaponSprite, playerShow && 
			!((playerIFrames >> 1) & 1) ? VISIBLE : HIDDEN);
		SPR_SAFEMOVE(weaponSprite,
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
			effect_create_damage(player.damage_value,
					sub_to_pixel(player.x) - 8, sub_to_pixel(player.y) - 4, 60);
			player.damage_value = 0;
		}
	}
	Entity *e = entityList;
	while(e != NULL) {
		// Handle special cases
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
		if((e->eflags|e->nflags) & (NPC_SOLID | NPC_SPECIALSOLID)) {
			collision = entity_react_to_collision(&player, e);
			if(collision.bottom && ((e->eflags|e->nflags) & NPC_BOUNCYTOP)) {
				player.y_speed = pixel_to_sub(-1);
				player.grounded = false;
			}
		}
		// Enemies
		if(e->attack > 0 && !tsc_running()) {
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
				// Show damage numbers
				effect_create_damage(-e->attack,
						sub_to_pixel(player.x), sub_to_pixel(player.y), 60);
				// Take health
				if(player.health <= e->attack) {
					// If health reached 0 we are dead
					player.health = 0;
					SPR_SAFERELEASE(player.sprite);
					effect_create_smoke(2, player.x, player.y);
					sound_play(SOUND_DIE, 15);
					tsc_call_event(PLAYER_DEFEATED_EVENT);
					break;
				}
				player.health -= e->attack;
				sound_play(SOUND_HURT, 5);
				playerIFrames = INVINCIBILITY_FRAMES;
				// Knock back
				player.y_speed = -0x300; // 1.5 pixels per frame
				player.grounded = false;
			}
		}
		e = e->next;
	}
}

void player_pause() {
	player_hide();
	SPR_SAFEVISIBILITY(weaponSprite, HIDDEN);
	for(u16 i = 0; i < MAX_BULLETS; i++) {
		SPR_SAFEVISIBILITY(playerBullet[i].sprite, HIDDEN);
	}
}

void player_unpause() {
	if(pauseCancelsIFrames) playerIFrames = 0;
	player_show();
	SPR_SAFEVISIBILITY(weaponSprite, VISIBLE);
	for(u16 i = 0; i < MAX_BULLETS; i++) {
		SPR_SAFEVISIBILITY(playerBullet[i].sprite, VISIBLE);
	}
}

bool player_invincible() {
	return playerIFrames > 0 || tsc_running();
}

bool player_inflict_damage(s16 damage) {
	// Take health
	if(player.health <= damage) {
		// If health reached 0 we are dead
		player.health = 0;
		effect_create_smoke(2, player.x, player.y);
		sound_play(SOUND_DIE, 15);
		tsc_call_event(PLAYER_DEFEATED_EVENT);
		return true;
	}
	player.health -= damage;
	sound_play(SOUND_HURT, 5);
	playerIFrames = INVINCIBILITY_FRAMES;
	return false;
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
		//sprite_delete(playerWeapon[currentWeapon].sprite);
		for(u8 i = 0; i < 8; i++) {
			if(playerWeapon[i].type > 0) continue;
			w = &playerWeapon[i];
			//w->sprite = sprite_create(weapon_info[id].sprite, PAL1, true);
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
		//w->sprite = sprite_create(weapon_info[id_give].sprite, PAL1, true);
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
