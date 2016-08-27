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

#ifdef PAL
#define INVINCIBILITY_FRAMES 100
#define AIR_TICKS 10
#define BOOSTER_FUEL 50
#else
#define INVINCIBILITY_FRAMES 120
#define AIR_TICKS 12
#define BOOSTER_FUEL 60
#endif

u16 dummyController[2] = { 0, 0 };
bool playerDead;
bool playerShow;

Sprite *weaponSprite = NULL;
u8 playerWeaponCount = 0;

Sprite *mapNameSprite = NULL;
u16 mapNameTTL = 0;

Sprite *airSprite = NULL;
u8 airPercent = 100;
u8 airTick = 0;
u8 airDisplayTime = 0;

bool blockl, blocku, blockr, blockd;
u8 ledge_time;
u8 walk_time;

u8 mgun_shoottime, mgun_chargetime;

Sprite *airTankSprite = NULL;

void player_update_bounds();
void player_update_booster();
void player_update_interaction();
void player_update_air_display();

void player_prev_weapon();
void player_next_weapon();

// Default values for player
void player_init() {
	controlsLocked = false;
	playerShow = true;
	player.direction = 0;
	player.eflags = NPC_IGNORE44|NPC_SHOWDAMAGE;
	player.controller = &joystate;
	playerMaxHealth = 3;
	player.health = 3;
	player.x = block_to_sub(10) + pixel_to_sub(8);
	player.y = block_to_sub(8) + pixel_to_sub(8);
	player.x_next = player.x;
	player.y_next = player.y;
	player.x_speed = 0;
	player.y_speed = 0;
	player.enableSlopes = true;
	player.damage_time = 0;
	player.damage_value = 0;
	player.direction = 1;
	player.spriteAnim = 0;
	player_reset_sprites();
	player.hit_box = (bounding_box){ 6, 6, 5, 8 };
	ledge_time = 0;
	mgun_shoottime = 0;
	mgun_chargetime = 0;
	playerEquipment = 0; // Nothing equipped
	for(u8 i = 0; i < MAX_ITEMS; i++) playerInventory[i] = 0; // Empty inventory
	for(u8 i = 0; i < MAX_WEAPONS; i++) playerWeapon[i].type = 0; // No Weapons
	playerDead = false;
	playerMoveMode = 0;
	currentWeapon = 0;
	airPercent = 100;
	airTick = 0;
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
	airTankSprite = NULL;
	// Clear air and map name
	mapNameSprite = NULL;
	mapNameTTL = 0;
	airSprite = NULL;
	airDisplayTime = 0;
	playerPlatform = NULL;
}

void player_update() {
	if(playerDead) return;
	u8 tile = stage_get_block_type(sub_to_block(player.x), sub_to_block(player.y));
	if(debuggingEnabled && (joystate&BUTTON_A)) { // Float - no collision
		entity_update_float(&player);
		player.x_next = player.x + player.x_speed;
		player.y_next = player.y + player.y_speed;
	} else if(playerMoveMode == 0) { // Normal movement
		// Wind/Water current
		if(tile & 0x80) {
			switch(tile & 0x03) {
			case 0: player.x_speed -= SPEED(0x88); break;
			case 1: player.y_speed -= SPEED(0x80); break;
			case 2: player.x_speed += SPEED(0x88); break;
			case 3: player.y_speed += SPEED(0x50); break;
			}
		}
		entity_update_movement(&player);
		if(playerBoostState != BOOST_OFF) {
			player_update_booster();
		} else {
			bool blockl_next, blocku_next, blockr_next, blockd_next;
			blocku_next = player.y_speed < 0 ? collide_stage_ceiling(&player) : false;
			blockl_next = player.x_speed <= 0 ? collide_stage_leftwall(&player) : false;
			blockr_next = player.x_speed >= 0 ? collide_stage_rightwall(&player) : false;
			if(ledge_time == 0) {
				if(player.grounded) {
					player.grounded = collide_stage_floor_grounded(&player);
				} else if(player.y_speed >= 0) {
					player.grounded = collide_stage_floor(&player);
				}
			}
			blockd_next = player.grounded;
			// Here I do something weird to emulate the way the game pushes quote
			// into small gaps
			// Actually, I think this is how the original game does it. The speedrun
			// tutorial mentioned trying to hug the floors while you jump up to them because
			// it gives a minor speed boost
			if(!blockl_next && blockl && joy_down(BUTTON_LEFT)) {
				player.x_speed -= 0xE0;
				player.x_next = player.x + player.x_speed;
			}
			if(!blockr_next && blockr && joy_down(BUTTON_RIGHT)) {
				player.x_speed += 0xE0;
				player.x_next = player.x + player.x_speed;
			}
			if(ledge_time > 0) {
				ledge_time--;
				player.y_next += 0x600;
				blockl_next = player.x_speed < 0 ? collide_stage_leftwall(&player) : false;
				blockr_next = player.x_speed > 0 ? collide_stage_rightwall(&player) : false;
				player.y_next -= 0x600;
			} else if(player.jump_time == 0 && !blockd_next && blockd) {
				player.y_speed += 0x80;
				ledge_time = 4;
			}
			blockl = blockl_next;
			blocku = blocku_next;
			blockr = blockr_next;
			blockd = blockd_next;
			//entity_update_collision(&player);
			if(playerPlatform != NULL) {
				player.x_next += playerPlatform->x_speed;
				player.y_next += playerPlatform->y_speed;
				player.hit_box.bottom++;
				bounding_box box = entity_react_to_collision(&player, playerPlatform, false);
				player.hit_box.bottom--;
				if(box.bottom == 0) {
					playerPlatform = NULL;
				} else {
					player.grounded = true;
					player.y_next += pixel_to_sub(1);
				}
			}
		}
		player_update_bounds();
	} else { // Move mode 1 - for ironhead
		entity_update_float(&player);
		player.x_next = player.x + player.x_speed;
		player.y_next = player.y + player.y_speed;
		if(player.x_speed < 0) {
			collide_stage_leftwall(&player);
		} else if (player.x_speed > 0) {
			collide_stage_rightwall(&player);
		}
		// No "grounded" when floating, but don't go through the floor
		if(player.y_speed > 0) {
			collide_stage_floor(&player);
		} else if(player.y_speed < 0) {
			collide_stage_ceiling(&player);
		}
		player_update_bounds();
	}
	player.x = player.x_next;
	player.y = player.y_next;
	// Damage Tiles / Death check / IFrames
	if(!playerIFrames) {
		if(tile == 0x42) {
			player_inflict_damage(10);
		}
		if(player.health == 0) {
			playerIFrames = 0;
			playerDead = true;
			return;
		}
	} else {
		playerIFrames--;
	}
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
	// Handle air when underwater, unless a script is running
	if(tsc_running()) {
		SPR_SAFERELEASE(airSprite);
	} else {
		if(player.underwater && !(playerEquipment & EQUIP_AIRTANK)) {
			if(airTick == 0) {
				airTick = AIR_TICKS;
				airPercent--;
				if(airPercent == 0) {
					// Spoilers
					if(system_get_flag(ALMOND_DROWN_FLAG)) {
						tsc_call_event(ALMOND_DROWN_EVENT);
					} else {
						player.health = 0;
						SPR_SAFERELEASE(airSprite);
						SPR_SAFEANIM(player.sprite, 8);
						tsc_call_event(PLAYER_DROWN_EVENT);
						return;
					}
				}
			} else {
				airTick--;
			}
		} else if(airPercent < 100) {
			airPercent = 100;
			airTick = 0;
			airDisplayTime = 60;
		} else {
			if(airDisplayTime > 0) {
				airDisplayTime--;
			} else {
				SPR_SAFERELEASE(airSprite);
			}
		}
		player_update_air_display();
	}
	// Weapon switching
	if((player.controller[0]&BUTTON_Y) && !(player.controller[1]&BUTTON_Y)) {
		player_prev_weapon();
	} else if((player.controller[0]&BUTTON_Z) && !(player.controller[1]&BUTTON_Z)) {
		player_next_weapon();
	}
	// Shooting
	if(playerWeapon[currentWeapon].type == WEAPON_MACHINEGUN) {
		if(mgun_shoottime > 0) mgun_shoottime--;
		if(player.controller[0]&BUTTON_B) {
			if(mgun_shoottime == 0) {
				if(playerWeapon[currentWeapon].ammo > 0) {
					weapon_fire(playerWeapon[currentWeapon]);
					playerWeapon[currentWeapon].ammo--;
				} else {
					sound_play(SND_GUN_CLICK, 5);
				}
				mgun_shoottime = 9;
			}
		} else {
			player.y_mark = 0;
			if(playerWeapon[currentWeapon].ammo < 100) {
				if(mgun_chargetime > 0) {
					mgun_chargetime--;
				} else {
					playerWeapon[currentWeapon].ammo++;
					mgun_chargetime = (playerEquipment & EQUIP_TURBOCHARGE) ? 2 : 4;
				}
			}
		}
	} else {
		if((player.controller[0]&BUTTON_B) && !(player.controller[1]&BUTTON_B)) {
			weapon_fire(playerWeapon[currentWeapon]);
		}
	}
	player_update_bullets();
	if(player.grounded) {
		playerBoosterFuel = BOOSTER_FUEL;
		player_update_interaction();
	}
	player_draw();
	if(mapNameTTL > 0 && --mapNameTTL == 0) {
		SPR_SAFERELEASE(mapNameSprite);
	}
}

void player_update_bullets() {
	for(u8 i = 0; i < MAX_BULLETS; i++) {
		bullet_update(playerBullet[i]);
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

void player_start_booster() {
	if(playerBoosterFuel == 0) return;
	player.jump_time = 0;
	// Pick a direction with Booster 2.0, default up
	if ((playerEquipment & EQUIP_BOOSTER20)) {
		playerBoostState = BOOST_UP;
		// in order of precedence
		if (joy_down(BUTTON_LEFT)) {
			player.direction = 0;
			playerBoostState = BOOST_HOZ;
		}
		if (joy_down(BUTTON_RIGHT)) {
			player.direction = 1;
			playerBoostState = BOOST_HOZ;
		}
		if (joy_down(BUTTON_DOWN)) {
			playerBoostState = BOOST_DOWN;
		}
		if (joy_down(BUTTON_UP)) {
			playerBoostState = BOOST_UP;
		}
		
		if (playerBoostState == BOOST_UP || playerBoostState == BOOST_DOWN)
			player.x_speed = 0;
		
		switch(playerBoostState) {
			case BOOST_UP:
				player.y_speed = SPEED(-0x5ff);
			break;
			case BOOST_DOWN:
				player.y_speed = SPEED(0x5ff);
			break;
			case BOOST_HOZ:
				player.y_speed = 0;
				if (joy_down(BUTTON_LEFT))
					player.x_speed = SPEED(-0x5ff);
				else
					player.x_speed = SPEED(0x5ff);
			break;
		}
	} else {
		playerBoostState = BOOST_08;
		// help it overcome gravity
		if (player.y_speed > SPEED(0x100))
			player.y_speed >>= 1;
	}
	sound_play(SND_BOOSTER, 3);
}

void player_update_booster() {
	if(!(playerEquipment & (EQUIP_BOOSTER08 | EQUIP_BOOSTER20))) playerBoostState = BOOST_OFF;
	if(!joy_down(BUTTON_C)) playerBoostState = BOOST_OFF;
	if(playerBoostState == BOOST_OFF) return;
	// player seems to want it active...check the fuel
	if(playerBoosterFuel == 0) {
		playerBoostState = BOOST_OFF;
		return;
	} else {
		playerBoosterFuel--;
	}
	// ok so then, booster is active right now
	bool sputtering = false;
	
	if (joy_down(BUTTON_LEFT)) player.direction = 0;
	else if (joy_down(BUTTON_RIGHT)) player.direction = 1;
	SPR_SAFEHFLIP(player.sprite, player.direction);

	bool blockl = collide_stage_leftwall(&player),
			blockr = collide_stage_rightwall(&player),
			blocku = collide_stage_ceiling(&player),
			blockd = collide_stage_floor(&player);

	switch(playerBoostState) {
		case BOOST_HOZ:
		{
			if ((player.direction == 0 && blockl) || \
				(player.direction == 1 && blockr)) {
				player.y_speed = SPEED(-0x100);
			}
			if (joy_down(BUTTON_DOWN)) player.y_speed -= SPEED(0x20);
			if (joy_down(BUTTON_UP)) player.y_speed += SPEED(0x20);
		}
		break;
		case BOOST_UP:
		{
			player.y_speed -= SPEED(0x20);
		}
		break;
		case BOOST_DOWN:
		{
			player.y_speed += SPEED(0x20);
		}
		break;
		case BOOST_08:
		{
			// top speed and sputtering
			if (player.y_speed < SPEED(-0x400)) {
				player.y_speed += SPEED(0x20);
				sputtering = true;	// no sound/smoke this frame
			} else {
				player.y_speed -= SPEED(0x20);
			}
		}
		break;
	}
	// smoke and sound effects
	if ((playerBoosterFuel % 5) == 1 && !sputtering) {
		sound_play(SND_BOOSTER, 3);
	}
}

void player_show() {
	playerShow = true;
	SPR_SAFEVISIBILITY(player.sprite, AUTO_FAST);
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
	u8 len = 0;
	for(u8 i = 0; i < 16; i++) {
		u8 chr = stage_info[stageID].name[i] - 0x20;
		if(chr >= 0x60) {
			chr = 0;
			if(len == 0) len = i;
		}
		memcpy(nameTiles[i], &TS_SprFont.tiles[chr * 8], 32);
	}
	if(len > 0) {
		SPR_SAFEMOVE(mapNameSprite, SCREEN_HALF_W - len * 4, SCREEN_HALF_H - 32);
	}
	// Transfer tile array to VRAM
	SYS_disableInts();
	VDP_loadTileData(nameTiles[0], TILE_FACEINDEX + 36, 16, true);
	SYS_enableInts();
	mapNameTTL = ttl;
}

void draw_air_percent() {
	u32 numberTiles[3][8];
	memcpy(numberTiles[0], airPercent == 100 ? &TS_Numbers.tiles[8] : TILE_BLANK, 32);
	memcpy(numberTiles[1], &TS_Numbers.tiles[((airPercent / 10) % 10) * 8], 32);
	memcpy(numberTiles[2], &TS_Numbers.tiles[(airPercent % 10) * 8], 32);
	SYS_disableInts();
	VDP_loadTileData(numberTiles[0], TILE_AIRINDEX + 4, 3, true);
	SYS_enableInts();
}

void player_update_air_display() {
	if(airPercent == 100) {
		if(airDisplayTime == 60) {
			draw_air_percent();
		} else if(airDisplayTime > 0) {
			SPR_SAFEVISIBILITY(airSprite, (airDisplayTime & 8) ? VISIBLE : HIDDEN);
		}
	} else {
		if(airSprite == NULL) { // Initialize and draw full "AIR" text
			airDisplayTime = 0;
			airSprite = SPR_addSpriteEx(&SPR_Air, SCREEN_HALF_W - 28, SCREEN_HALF_H - 24,
				TILE_ATTR_FULL(PAL0, 1, 0, 0, TILE_AIRINDEX), 0, SPR_FLAG_AUTO_SPRITE_ALLOC);
			SPR_SAFEVISIBILITY(airSprite, VISIBLE);
			SYS_disableInts();
			VDP_loadTileData(SPR_TILESET(SPR_Air, 0, 0)->tiles, TILE_AIRINDEX, 4, true);
			SYS_enableInts();
		} else { // Just blink the small down arrow thing
			SYS_disableInts();
			airDisplayTime++;
			if((airDisplayTime % 32) == 0) {
				VDP_loadTileData(TILE_BLANK, TILE_AIRINDEX, 1, true);
			} else if((airDisplayTime % 32) == 15) {
				VDP_loadTileData(SPR_TILESET(SPR_Air, 0, 0)->tiles, TILE_AIRINDEX, 1, true);
			}
			SYS_enableInts();
		}
		// Calculate air percent and display the value
		if(airTick == AIR_TICKS) {
			draw_air_percent();
			SPR_SAFEVISIBILITY(airSprite, VISIBLE);
		}
	}
}

void player_draw() {
	// Sprite Animation
	u8 anim;
	if(player.grounded) {
		if(player.controller[0]&BUTTON_UP) {
			if((player.controller[0]&BUTTON_RIGHT) || (player.controller[0]&BUTTON_LEFT)) {
				anim = ANIM_LOOKUPWALK;
				walk_time++;
			} else {
				anim = ANIM_LOOKUP;
				walk_time = 0;
			}
		} else if((player.controller[0]&BUTTON_RIGHT) || (player.controller[0]&BUTTON_LEFT)) {
			anim = ANIM_WALKING;
			walk_time++;
		} else if((player.controller[0]&BUTTON_DOWN)) {
			anim = ANIM_INTERACT;
			walk_time = 0;
		} else {
			anim = ANIM_STANDING;
			walk_time = 0;
		}
	} else {
		walk_time = 0;
		if((player.controller[0]&BUTTON_UP)) {
			anim = ANIM_LOOKUPJUMP;
		} else if((player.controller[0]&BUTTON_DOWN)) {
			anim = ANIM_LOOKDOWNJUMP;
		} else {
			anim = ANIM_JUMPING;
		}
	}
	if(walk_time == TIME(10)) {
		walk_time = 0;
		sound_play(SND_PLAYER_WALK, 2);
	}
	// Set animation if it changed
	if(player.spriteAnim != anim) {
		player.spriteAnim = anim;
		// Frame change is a workaround for tapping looking like quote is floating
		SPR_SAFEANIMFRAME(player.sprite, anim, 
			anim==ANIM_WALKING || anim==ANIM_LOOKUPWALK ? 1 : 0);
	}
	// Change direction if pressing left or right
	if((player.controller[0]&BUTTON_RIGHT) && !player.direction) {
		player.direction = 1;
		SPR_SAFEHFLIP(player.sprite, 1);
	} else if((player.controller[0]&BUTTON_LEFT) && player.direction) {
		player.direction = 0;
		SPR_SAFEHFLIP(player.sprite, 0);
	}
	SPR_SAFEMOVE(player.sprite,
		sub_to_pixel(player.x) - sub_to_pixel(camera.x) + SCREEN_HALF_W - 8,
		sub_to_pixel(player.y) - sub_to_pixel(camera.y) + SCREEN_HALF_H - 8);
	// Blink during invincibility frames
	SPR_SAFEVISIBILITY(player.sprite, playerShow && 
		!((playerIFrames >> 1) & 1) ? AUTO_FAST : HIDDEN);
	// Weapon sprite
	if(playerWeapon[currentWeapon].type > 0) {
		u8 wanim = 0;
		if(anim==ANIM_LOOKUP || anim==ANIM_LOOKUPWALK || anim==ANIM_LOOKUPJUMP) wanim = 1;
		else if(anim==ANIM_LOOKDOWNJUMP) wanim = 2;
		SPR_SAFEANIM(weaponSprite, wanim);
		SPR_SAFEHFLIP(weaponSprite, player.direction);
		SPR_SAFEVISIBILITY(weaponSprite, playerShow && 
			!((playerIFrames >> 1) & 1) ? AUTO_FAST : HIDDEN);
		SPR_SAFEMOVE(weaponSprite,
			sub_to_pixel(player.x) - sub_to_pixel(camera.x) + SCREEN_HALF_W - 12,
			sub_to_pixel(player.y) - sub_to_pixel(camera.y) + SCREEN_HALF_H - 8);
	}
	// Bubble shield
	if(player.underwater && (playerEquipment & EQUIP_AIRTANK)) {
		if(airTankSprite == NULL) {
			SPR_SAFEADD(airTankSprite, &SPR_Bubble, 0, 0, TILE_ATTR(PAL0, 0, 0, 0), 3);
		}
		SPR_SAFEMOVE(airTankSprite,
				(player.x >> CSF) - (camera.x >> CSF) + SCREEN_HALF_W - 12,
				(player.y >> CSF) - (camera.y >> CSF) + SCREEN_HALF_H - 12);
	} else {
		SPR_SAFERELEASE(airTankSprite);
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

void player_pause() {
	player_hide();
	SPR_SAFEVISIBILITY(weaponSprite, HIDDEN);
	for(u16 i = 0; i < MAX_BULLETS; i++) {
		SPR_SAFEVISIBILITY(playerBullet[i].sprite, HIDDEN);
	}
	if(mapNameTTL) SPR_SAFEVISIBILITY(mapNameSprite, HIDDEN);
}

void player_unpause() {
	if(pauseCancelsIFrames) playerIFrames = 0;
	player_show();
	SPR_SAFEVISIBILITY(weaponSprite, AUTO_FAST);
	for(u16 i = 0; i < MAX_BULLETS; i++) {
		SPR_SAFEVISIBILITY(playerBullet[i].sprite, AUTO_FAST);
	}
	if(mapNameTTL) SPR_SAFEVISIBILITY(mapNameSprite, AUTO_FAST);
}

bool player_invincible() {
	return playerIFrames > 0 || tsc_running();
}

bool player_inflict_damage(s16 damage) {
	// Halve damage if we have the arms barrier
	if(playerEquipment & EQUIP_ARMSBARRIER) damage = (damage + 1) >> 1;
	// Show damage numbers
	effect_create_damage(-damage, sub_to_pixel(player.x), sub_to_pixel(player.y), 60);
	// Take health
	if(player.health <= damage) {
		// If health reached 0 we are dead
		player.health = 0;
		SPR_SAFERELEASE(player.sprite);
		SPR_SAFERELEASE(weaponSprite);
		// Clear smoke & fill up with smoke around player
		effects_clear_smoke();
		for(u8 i = 0; i < MAX_SMOKE; i++) {
			effect_create_smoke(2, 
				sub_to_pixel(player.x) + (random() % 90 ) - 45, 
				sub_to_pixel(player.y) + (random() % 90 ) - 45);
		}
		sound_play(SND_PLAYER_DIE, 15);
		tsc_call_event(PLAYER_DEFEATED_EVENT);
		return true;
	}
	player.health -= damage;
	sound_play(SND_PLAYER_HURT, 5);
	playerIFrames = INVINCIBILITY_FRAMES;
	// Decrease weapon exp
	if(damage > 0 && playerWeapon[currentWeapon].type != 0) {
		Weapon *w = &playerWeapon[currentWeapon];
		if(w->energy < damage) {
			if(w->level > 1) {
				w->level -= 1;
				w->energy += weapon_info[w->type].experience[w->level - 1];
				w->energy -= damage;
			}
		} else {
			w->energy -= damage;
		}
	}
	// Knock back
	player.y_speed = SPEED(-0x500); // 2.5 pixels per frame
	player.grounded = false;
	return false;
}

void player_update_bounds() {
	if(player.y_next > block_to_sub(stageHeight + 1)) {
		player.health = 0;
		tsc_call_event(PLAYER_OOB_EVENT);
	}
}

void player_prev_weapon() {
	for(u8 i = currentWeapon - 1; i != currentWeapon; i--) {
		if(i == 0xFF) i = MAX_WEAPONS - 1;
		if(playerWeapon[i].type > 0) {
			SPR_SAFERELEASE(weaponSprite);
			currentWeapon = i;
			if(weapon_info[playerWeapon[i].type].sprite != NULL) {
				weaponSprite = SPR_addSprite(weapon_info[playerWeapon[i].type].sprite, 
					0, 0, TILE_ATTR(PAL1, 0, 0, player.direction));
			}
			sound_play(SND_SWITCH_WEAPON, 5);
			break;
		}
	}
}

void player_next_weapon() {
	for(u8 i = currentWeapon + 1; i != currentWeapon; i++) {
		if(i >= MAX_WEAPONS) i = 0;
		if(playerWeapon[i].type > 0) {
			SPR_SAFERELEASE(weaponSprite);
			currentWeapon = i;
			if(weapon_info[playerWeapon[i].type].sprite != NULL) {
				weaponSprite = SPR_addSprite(weapon_info[playerWeapon[i].type].sprite, 
					0, 0, TILE_ATTR(PAL1, 0, 0, player.direction));
			}
			sound_play(SND_SWITCH_WEAPON, 5);
			break;
		}
	}
}

Weapon *player_find_weapon(u8 id) {
	for(u8 i = 0; i < MAX_WEAPONS; i++) {
		if(playerWeapon[i].type == id) return &playerWeapon[i];
	}
	return NULL;
}

void player_give_weapon(u8 id, u8 ammo) {
	Weapon *w = player_find_weapon(id);
	if(w == NULL) {
		for(u8 i = 0; i < MAX_WEAPONS; i++) {
			if(playerWeapon[i].type > 0) continue;
			w = &playerWeapon[i];
			w->type = id;
			w->level = 1;
			w->energy = 0;
			w->maxammo = ammo;
			w->ammo = ammo;
			break;
		}
	} else {
		w->maxammo += ammo;
		w->ammo += ammo;
	}
}

void player_take_weapon(u8 id) {
	Weapon *w = player_find_weapon(id);
	if(w != NULL) {
		if(playerWeapon[currentWeapon].type == id) {
			SPR_SAFERELEASE(weaponSprite);
			player_next_weapon();
		}
		w->type = 0;
		w->level = 0;
		w->energy = 0;
		w->maxammo = 0;
		w->ammo = 0;
	}
}

bool player_has_weapon(u8 id) {
	for(u8 i = 0; i < MAX_WEAPONS; i++) {
		if(playerWeapon[i].type == id) return true;
	}
	return false;
}

void player_trade_weapon(u8 id_take, u8 id_give, u8 ammo) {
	Weapon *w = player_find_weapon(id_take);
	if(w != NULL) {
		if(id_take == playerWeapon[currentWeapon].type) {
			SPR_SAFERELEASE(weaponSprite);
			if(weapon_info[w->type].sprite != NULL) {
				weaponSprite = SPR_addSprite(weapon_info[w->type].sprite, 
					0, 0, TILE_ATTR(PAL1, 0, 0, player.direction));
			}
		}
		w->type = id_give;
		w->level = 1;
		w->energy = 0;
		w->maxammo = ammo;
		w->ammo = ammo;
	}
}

void player_refill_ammo() {
	for(u8 i = 0; i < MAX_WEAPONS; i++) {
		playerWeapon[i].ammo = playerWeapon[i].maxammo;
	}
}

void player_take_allweapons() {
	for(u8 i = 0; i < MAX_WEAPONS; i++) playerWeapon[i].type = 0;
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
