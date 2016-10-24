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
#include "sheet.h"
#include "sprite.h"

#ifdef PAL
#define INVINCIBILITY_FRAMES 100
#define AIR_TICKS 10
#define BOOSTER_FUEL 50
#else
#define INVINCIBILITY_FRAMES 120
#define AIR_TICKS 12
#define BOOSTER_FUEL 60
#endif

VDPSprite weaponSprite;
u8 playerWeaponCount = 0;

u8 mapNameSpriteNum;
VDPSprite mapNameSprite[4];
u16 mapNameTTL = 0;

VDPSprite airSprite[2];
u8 airPercent = 100;
u8 airTick = 0;
u8 airDisplayTime = 0;

u8 blockl, blocku, blockr, blockd;
u8 ledge_time;
u8 walk_time;

VDPSprite airTankSprite;

void player_update_bounds();
void player_update_booster();
void player_update_interaction();
void player_update_air_display();

void player_update_movement();
void player_update_walk();
void player_update_jump();
void player_update_float();

void player_prev_weapon();
void player_next_weapon();

// Default values for player
void player_init() {
	controlsLocked = FALSE;
	player.hidden = FALSE;
	player.dir = 0;
	player.eflags = NPC_IGNORE44|NPC_SHOWDAMAGE;
	playerMaxHealth = 3;
	player.health = 3;
	player.x = block_to_sub(10) + pixel_to_sub(8);
	player.y = block_to_sub(8) + pixel_to_sub(8);
	player.x_next = player.x;
	player.y_next = player.y;
	player.x_speed = 0;
	player.y_speed = 0;
	player.enableSlopes = TRUE;
	player.damage_time = 0;
	player.damage_value = 0;
	player.dir = 1;
	player.hit_box = (bounding_box){ 6, 6, 5, 8 };
	player.frame = 255;
	ledge_time = 0;
	mgun_shoottime = 0;
	mgun_chargetime = 0;
	playerEquipment = 0; // Nothing equipped
	for(u8 i = 0; i < MAX_ITEMS; i++) playerInventory[i] = 0; // Empty inventory
	for(u8 i = 0; i < MAX_WEAPONS; i++) playerWeapon[i].type = 0; // No Weapons
	playerMoveMode = 0;
	currentWeapon = 0;
	airPercent = 100;
	airTick = 0;
	// Booster trail sprite tiles
	VDP_loadTileData(SPR_TILES(&SPR_Boost, 0, 0), 12, 4, TRUE);
	// AIR Sprite
	VDP_loadTileData(SPR_TILES(&SPR_Air, 0, 0), TILE_AIRINDEX, 4, TRUE);
	airSprite[0] = (VDPSprite) {
		.x = SCREEN_HALF_W - 28 + 128, .y = SCREEN_HALF_H - 24 + 128, 
		.size = SPRITE_SIZE(4, 1), .attribut = TILE_ATTR_FULL(PAL0,1,0,0,TILE_AIRINDEX)
	};
	airSprite[1] = (VDPSprite){
		.x = SCREEN_HALF_W + 8 + 128, .y = SCREEN_HALF_H - 24 + 128, 
		.size = SPRITE_SIZE(3, 1), .attribut = TILE_ATTR_FULL(PAL0,1,0,0,TILE_AIRINDEX+4)
	};
	// Air Tank sprite
	VDP_loadTileData(SPR_TILES(&SPR_Bubble, 0, 0), TILE_AIRTANKINDEX, 9, TRUE);
	airTankSprite = (VDPSprite) {
		.size = SPRITE_SIZE(3,3),
		.attribut = TILE_ATTR_FULL(PAL0,0,0,0,TILE_AIRTANKINDEX)
	};
	// Player sprite
	playerSprite = (VDPSprite) {
		.size = SPRITE_SIZE(2,2),
		.attribut = TILE_ATTR_FULL(PAL0,0,0,1,TILE_PLAYERINDEX)
	};
}

void player_update() {
	u8 tile = stage_get_block_type(sub_to_block(player.x), sub_to_block(player.y));
	if(debuggingEnabled && (joystate&BUTTON_A)) { // Float - no collision
		player_update_float();
		player.x_next = player.x + player.x_speed;
		player.y_next = player.y + player.y_speed;
	} else if(playerMoveMode == 0) { // Normal movement
		// Wind/Water current
		if(tile & 0x80) {
			// This stops us fron getting stuck in ledges
			if((blk(player.x, 0, player.y, 6) & 0x83) == 0x81) {
				player.y_speed -= SPEED(0x80);
			} else {
				switch(tile & 0x03) {
					case 0: player.x_speed -= SPEED(0x88); break;
					case 1: player.y_speed -= SPEED(0x80); break;
					case 2: player.x_speed += SPEED(0x88); break;
					case 3: player.y_speed += SPEED(0x50); break;
				}
			}
			if(player.underwater) {
				// This makes it possible to reach the cabin... but now it's
				// impossible to jump over 2 of the spike areas...
				if(player.x_speed > SPEED(0x480)) player.x_speed = SPEED(0x480);
				if(player.x_speed < SPEED(-0x480)) player.x_speed = SPEED(-0x480);
				if(player.y_speed > SPEED(0x3FF)) player.y_speed = SPEED(0x3FF);
				if(player.y_speed < SPEED(-0x4FF)) player.y_speed = SPEED(-0x4FF);
			} else {
				if(player.x_speed > SPEED(0x5FF)) player.x_speed = SPEED(0x5FF);
				if(player.x_speed < SPEED(-0x5FF)) player.x_speed = SPEED(-0x5FF);
				if(player.y_speed > SPEED(0x5FF)) player.y_speed = SPEED(0x5FF);
				if(player.y_speed < SPEED(-0x5FF)) player.y_speed = SPEED(-0x5FF);
			}
		}
		player_update_movement();
		// Updates the booster while it is enabled
		if(playerBoostState != BOOST_OFF) {
			player_update_booster();
		}
		// Slow down the player when we stop the Booster 2.0
		// Has to run before collision since it recalculates our next position
		if (playerBoostState != lastBoostState) {
			if (playerBoostState == BOOST_OFF && (playerEquipment & EQUIP_BOOSTER20)) {
				switch(lastBoostState) {
					case BOOST_HOZ:
						player.x_speed >>= 1;
						player.x_next = player.x + player.x_speed;
					break;
					case BOOST_UP:
						player.y_speed >>= 1;
						player.y_next = player.y + player.y_speed;
					break;
				}
			}
		}
		lastBoostState = playerBoostState;
		// The above code to slow down the booster needs to always run. 
		// As such we check again whether the booster is enabled
		if(playerBoostState == BOOST_OFF) {
			u8 blockl_next, blocku_next, blockr_next, blockd_next;
			blocku_next = player.y_speed < 0 ? collide_stage_ceiling(&player) : FALSE;
			blockl_next = player.x_speed <= 0 ? collide_stage_leftwall(&player) : FALSE;
			blockr_next = player.x_speed >= 0 ? collide_stage_rightwall(&player) : FALSE;
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
				blockl_next = player.x_speed < 0 ? collide_stage_leftwall(&player) : FALSE;
				blockr_next = player.x_speed > 0 ? collide_stage_rightwall(&player) : FALSE;
				player.y_next -= 0x600;
			} else if(player.jump_time == 0 && !blockd_next && blockd) {
				player.y_speed += 0x80;
				ledge_time = 4;
			}
			blockl = blockl_next;
			blocku = blocku_next;
			blockr = blockr_next;
			blockd = blockd_next;
			if(playerPlatform) {
				player.x_next += playerPlatform->x_speed;
				player.y_next += playerPlatform->y_speed;
				player.hit_box.bottom++;
				bounding_box box = entity_react_to_collision(&player, playerPlatform);
				player.hit_box.bottom--;
				if(box.bottom == 0) {
					playerPlatform = NULL;
				} else {
					player.grounded = TRUE;
					player.y_next += pixel_to_sub(1);
				}
			}
		}
		// Die when player goes OOB
		player_update_bounds();
	} else { // Move mode 1 - for ironhead
		player_update_float();
		player.x_next = player.x + player.x_speed;
		player.y_next = player.y + player.y_speed;
		if(player.x_speed < 0) {
			collide_stage_leftwall(&player);
			// Check special noplayer blocks in main artery
			if(blk(player.x, -6, player.y, 0) == 0x46) {
				player.x_speed = 0;
				player.x_next += 2 << CSF;
			}
		} else if (player.x_speed > 0) {
			collide_stage_rightwall(&player);
			// Check special noplayer blocks in main artery
			if(blk(player.x, 6, player.y, 0) == 0x46) {
				player.x_speed = 0;
				player.x_next -= 2 << CSF;
			}
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
	if(!playerIFrames && player.health > 0) {
		// Match foreground (0x40) and fore+water (0x60) but not wind (0x80) or slope (0x10)
		if((tile & 0xDF) == 0x42) {
			player_inflict_damage(10);
		}
		if(player.health == 0) return;
	} else {
		playerIFrames--;
	}
	// This "damage_time" block is for energy numbers
	// When the player gets hurt damage numbers are created instantly
	if(player.damage_time > 0) {
		player.damage_time--;
		if(player.damage_time == 0) {
			effect_create_damage(player.damage_value,
					sub_to_pixel(player.x) - 8, sub_to_pixel(player.y) - 4);
			player.damage_value = 0;
		}
	}
	// Handle air when underwater, unless a script is running
	if(!tscState) {
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
						player.frame = 8;
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
		} else if(airDisplayTime > 0) {
			airDisplayTime--;
		}
		player_update_air_display();
	}
	// Weapon switching
	if(joy_pressed(BUTTON_Y)) {
		player_prev_weapon();
	} else if(joy_pressed(BUTTON_Z)) {
		player_next_weapon();
	}
	// Shooting
	if(playerWeapon[currentWeapon].type == WEAPON_MACHINEGUN) {
		if(mgun_shoottime > 0) mgun_shoottime--;
		if(joy_down(BUTTON_B)) {
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
		if(joy_pressed(BUTTON_B)) {
			weapon_fire(playerWeapon[currentWeapon]);
		}
	}
	player_update_bullets();
	if(player.grounded) {
		playerBoosterFuel = BOOSTER_FUEL;
		player_update_interaction();
	}
	//player_draw();
	if(mapNameTTL > 0) {
		mapNameTTL--;
		sprite_addq(mapNameSprite, mapNameSpriteNum);
	}
}

void player_update_movement() {
	player_update_walk();
	player_update_jump();
	player.x_next = player.x + player.x_speed;
	player.y_next = player.y + player.y_speed;
}

void player_update_walk() {
	s16 acc;
	s16 dec;
	s16 fric;
	s16 max_speed = MAX_WALK_SPEED;
	if(player.grounded) {
		acc = WALK_ACCEL;
		dec = WALK_ACCEL;
		fric = FRICTION;
	} else {
		acc = AIR_CONTROL;
		dec = AIR_CONTROL;
		fric = AIR_CONTROL;
	}
	if((stage_get_block_type(sub_to_block(player.x), sub_to_block(player.y)) & BLOCK_WATER) ||
			(water_entity && player.y > water_entity->y)) {
		player.underwater = TRUE;
		acc /= 2;
		max_speed /= 2;
		fric /= 2;
	} else {
		player.underwater = FALSE;
	}
	if(joy_down(BUTTON_LEFT)) {
		player.x_speed -= acc;
		if(player.x_speed < -max_speed) player.x_speed = min(player.x_speed + dec, -max_speed);
	} else if(joy_down(BUTTON_RIGHT)) {
		player.x_speed += acc;
		if(player.x_speed > max_speed) player.x_speed = max(player.x_speed - dec, max_speed);
	} else if(player.grounded) {
		if(player.x_speed < fric && player.x_speed > -fric) player.x_speed = 0;
		else if(player.x_speed < 0) player.x_speed += fric;
		else if(player.x_speed > 0) player.x_speed -= fric;
	}
}

void player_update_jump() {
	s16 jumpSpeed = 	JUMP_SPEED;
	s16 gravity = 		GRAVITY;
	s16 gravityJump = 	GRAVITY_JUMP;
	s16 maxFallSpeed = 	MAX_FALL_SPEED;
	if(player.underwater) {
		jumpSpeed /= 2;
		gravity /= 2;
		gravityJump /= 2;
		maxFallSpeed /= 2;
	}
	if(player.jump_time > 0) {
		if(joy_down(BUTTON_C)) {
			player.jump_time--;
		} else {
			player.jump_time = 0;
		}
	}
	if(player.jump_time > 0) return;
	if(player.grounded) {
		if(joy_pressed(BUTTON_C)) {
			player.grounded = FALSE;
			player.y_speed = -jumpSpeed;
			player.jump_time = MAX_JUMP_TIME;
			sound_play(SND_PLAYER_JUMP, 3);
		}
	} else if((playerEquipment & (EQUIP_BOOSTER08 | EQUIP_BOOSTER20)) &&
			joy_pressed(BUTTON_C)) {
		player_start_booster();
	} else if(playerBoostState == BOOST_OFF) {
		if(joy_down(BUTTON_C) && player.y_speed >= 0) {
			player.y_speed += gravityJump;
		} else if(player.underwater) {
			player.y_speed += gravityJump + 0x7;
		} else {
			player.y_speed += gravity;
		}
		if(player.y_speed > maxFallSpeed) player.y_speed = maxFallSpeed;
	}
}

void player_update_float() {
	s16 acc = 		WALK_ACCEL;
	s16 fric = 		FRICTION;
	s16 max_speed = MAX_WALK_SPEED;
	if (joy_down(BUTTON_LEFT)) {
		player.x_speed -= acc;
		if (player.x_speed < -max_speed) player.x_speed = -max_speed;
	} else if (joy_down(BUTTON_RIGHT)) {
		player.x_speed += acc;
		if (player.x_speed > max_speed) player.x_speed = max_speed;
	} else {
		if (player.x_speed < fric && player.x_speed > -fric) player.x_speed = 0;
		else if (player.x_speed < 0) player.x_speed += fric;
		else if (player.x_speed > 0) player.x_speed -= fric;
	}
	if (joy_down(BUTTON_UP)) {
		player.y_speed -= acc;
		if (player.y_speed < -max_speed) player.y_speed = -max_speed;
	} else if (joy_down(BUTTON_DOWN)) {
		player.y_speed += acc;
		if (player.y_speed > max_speed) player.y_speed = max_speed;
	} else {
		if(player.y_speed < fric && player.y_speed > -fric) player.y_speed = 0;
		else if (player.y_speed < 0) player.y_speed += fric;
		else if (player.y_speed > 0) player.y_speed -= fric;
	}
}

void player_update_bullets() {
	for(u8 i = 0; i < MAX_BULLETS; i++) {
		bullet_update(playerBullet[i]);
	}
}

void player_update_interaction() {
	// Interaction with entities when pressing down
	if(joy_pressed(BUTTON_DOWN)) {
		Entity *e = entityList;
		while(e) {
			if((e->eflags & NPC_INTERACTIVE) && entity_overlapping(&player, e)) {
				oldstate |= BUTTON_DOWN; // To avoid triggering it twice
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
			player.dir = 0;
			playerBoostState = BOOST_HOZ;
		} else if (joy_down(BUTTON_RIGHT)) {
			player.dir = 1;
			playerBoostState = BOOST_HOZ;
		} else if (joy_down(BUTTON_DOWN)) {
			playerBoostState = BOOST_DOWN;
		} else if (joy_down(BUTTON_UP)) {
			playerBoostState = BOOST_UP;
		}
		
		if (playerBoostState == BOOST_UP || playerBoostState == BOOST_DOWN)
			player.x_speed = 0;
		
		switch(playerBoostState) {
			case BOOST_UP:
				player.y_speed = -SPEED(0x5ff);
			break;
			case BOOST_DOWN:
				player.y_speed = SPEED(0x5ff);
			break;
			case BOOST_HOZ:
				player.y_speed = 0;
				if (joy_down(BUTTON_LEFT))
					player.x_speed = -SPEED(0x5ff);
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
	u8 sputtering = FALSE;
	
	if (joy_down(BUTTON_LEFT)) player.dir = 0;
	else if (joy_down(BUTTON_RIGHT)) player.dir = 1;

	u8 blockl = collide_stage_leftwall(&player),
			blockr = collide_stage_rightwall(&player);
	// Don't bump in the opposite direction when hitting the ceiling
	if(player.y_speed <= 0 && collide_stage_ceiling(&player)) player.y_speed = 0;
	collide_stage_floor(&player);

	switch(playerBoostState) {
		case BOOST_HOZ:
		{
			if ((!player.dir && blockl) || (player.dir && blockr)) {
				player.y_speed = -SPEED(0x100);
			}
			if (joy_down(BUTTON_DOWN)) player.y_speed += SPEED(0x20);
			if (joy_down(BUTTON_UP)) player.y_speed -= SPEED(0x20);
		}
		break;
		case BOOST_UP:
		{
			player.y_speed -= SPEED(0x20);
			if(player.y_speed < -SPEED(0x5FF)) player.y_speed = -SPEED(0x5FF);
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
			if (player.y_speed < -SPEED(0x400)) {
				player.y_speed += SPEED(0x20);
				sputtering = TRUE;	// no sound/smoke this frame
			} else {
				player.y_speed -= SPEED(0x20);
			}
		}
		break;
	}
	// smoke and sound effects
	if ((playerBoosterFuel % 5) == 1 && !sputtering) {
		sound_play(SND_BOOSTER, 3);
		effect_create_misc(playerBoostState == BOOST_08 ? EFF_BOOST8 : EFF_BOOST2, 
				player.x >> CSF, (player.y >> CSF) + 6);
	}
}

void player_show_map_name(u8 ttl) {
	// Create a string of tiles in RAM
	u32 nameTiles[16][8];
	u8 len = 0;
	for(u8 i = 0; i < 16; i++) {
		u8 chr = stage_info[stageID].name[i] - 0x20;
		if(chr < 0x60) len++;
		else break;
		memcpy(nameTiles[i], &font_lib.tiles[chr * 8], 32);
	}
	// Transfer tile array to VRAM
	if(len > 0) {
		SYS_disableInts();
		VDP_loadTileData(nameTiles[0], TILE_NAMEINDEX, 16, TRUE);
		SYS_enableInts();
		mapNameSpriteNum = 0;
		u16 x = SCREEN_HALF_W - len * 4;
		for(u8 i = 0; i < len; i += 4) {
			mapNameSprite[i/4] = (VDPSprite) {
				.x = x + 128,
				.y = SCREEN_HALF_H - 32 + 128,
				.size = SPRITE_SIZE(min(4,len-i), 1),
				.attribut = TILE_ATTR_FULL(PAL0,1,0,0,TILE_NAMEINDEX+i)
			};
			x += 32;
			mapNameSpriteNum++;
		}
		mapNameTTL = ttl;
	}
}

void draw_air_percent() {
	u32 numberTiles[3][8];
	memcpy(numberTiles[0], airPercent == 100 ? &TS_Numbers.tiles[8] : TILE_BLANK, 32);
	memcpy(numberTiles[1], &TS_Numbers.tiles[((airPercent / 10) % 10) * 8], 32);
	memcpy(numberTiles[2], &TS_Numbers.tiles[(airPercent % 10) * 8], 32);
	SYS_disableInts();
	VDP_loadTileData(numberTiles[0], TILE_AIRINDEX + 4, 3, TRUE);
	SYS_enableInts();
}

void player_update_air_display() {
	// Blink for a second after getting out of the water
	if(airPercent == 100) {
		if(airDisplayTime == TIME(50)) {
			draw_air_percent();
		} else if(airDisplayTime & 8) {
			sprite_addq(airSprite, 2);
		}
	} else {
		airDisplayTime++;
		if((airDisplayTime % 32) == 0) {
			SYS_disableInts();
			VDP_loadTileData(TILE_BLANK, TILE_AIRINDEX, 1, TRUE);
			SYS_enableInts();
		} else if((airDisplayTime % 32) == 15) {
			SYS_disableInts();
			VDP_loadTileData(SPR_TILES(&SPR_Air, 0, 0), TILE_AIRINDEX, 1, TRUE);
			SYS_enableInts();
		}
		// Calculate air percent and display the value
		if(airTick == AIR_TICKS) draw_air_percent();
		sprite_addq(airSprite, 2);
	}
}

void player_draw() {
	// Don't draw the player if we died
	if(!player.health) return;
	enum { STAND, WALK1, WALK2, LOOKUP, UPWALK1, UPWALK2, LOOKDN, JUMPDN };
	// Sprite Animation
	player.oframe = player.frame;
	if(player.grounded) {
		if(joy_down(BUTTON_UP) && !controlsLocked) {
			if(joystate&(BUTTON_LEFT|BUTTON_RIGHT)) {
				ANIMATE(&player, 7, UPWALK1, LOOKUP, UPWALK2, LOOKUP);
			} else {
				player.frame = LOOKUP;
				player.animtime = 0;
			}
		} else if(joystate&(BUTTON_LEFT|BUTTON_RIGHT) && !controlsLocked) {
			ANIMATE(&player, 7, WALK1, STAND, WALK2, STAND);
		} else if(joy_down(BUTTON_DOWN) && !controlsLocked) {
			player.frame = LOOKDN;
			player.animtime = 0;
		} else {
			player.frame = STAND;
			player.animtime = 0;
		}
	} else {
		player.animtime = 0;
		if(joy_down(BUTTON_UP) && !controlsLocked) {
			player.frame = UPWALK1;
		} else if(joy_down(BUTTON_DOWN) && !controlsLocked) {
			player.frame = JUMPDN;
		} else {
			player.frame = WALK1;
		}
	}
	if(player.animtime % 14 == 7) sound_play(SND_PLAYER_WALK, 2);
	// Set frame if it changed
	if(player.frame != player.oframe) {
		TILES_QUEUE(SPR_TILES(&SPR_Quote,0,player.frame),TILE_PLAYERINDEX,4);
	}
	// Blink during invincibility frames
	if(!player.hidden && !(playerIFrames & 2)) {
		// Change direction if pressing left or right
		if(!controlsLocked) {
			if(joy_down(BUTTON_RIGHT)) {
				player.dir = 1;
			} else if(joy_down(BUTTON_LEFT)) {
				player.dir = 0;
			}
		}
		sprite_hflip(playerSprite, player.dir);
		sprite_pos(playerSprite,
				sub_to_pixel(player.x) - sub_to_pixel(camera.x) + SCREEN_HALF_W - 8,
				sub_to_pixel(player.y) - sub_to_pixel(camera.y) + SCREEN_HALF_H - 8);
		sprite_add(playerSprite);
		if(playerWeapon[currentWeapon].type > 0) {
			u8 vert = 0, vdir = 0;
			if(player.frame==LOOKUP || player.frame==UPWALK1 || player.frame==UPWALK2) {
				vert = 1;
				vdir = 0;
			} else if(player.frame==JUMPDN) {
				vert = 1;
				vdir = 1;
			}
			weaponSprite = (VDPSprite){
				.x = (player.x>>CSF) - (camera.x>>CSF) + SCREEN_HALF_W - (vert?4:12) + 128,
				.y = (player.y>>CSF) - (camera.y>>CSF) + SCREEN_HALF_H - (vert?8:0) + 128,
				.size = SPRITE_SIZE(vert ? 1 : 3, vert ? 3 : 1),
				.attribut = TILE_ATTR_FULL(PAL1,0,vdir,player.dir,TILE_WEAPONINDEX+vert*3)
			};
			sprite_add(weaponSprite);
		}
		if(player.underwater && (playerEquipment & EQUIP_AIRTANK)) {
			sprite_pos(airTankSprite, 
					(player.x>>CSF) - (camera.x>>CSF) + SCREEN_HALF_W - 12,
					(player.y>>CSF) - (camera.y>>CSF) + SCREEN_HALF_H - 12);
			sprite_add(airTankSprite);
		}
	}
}

void player_unpause() {
	// Sometimes player is left stuck after pausing
	controlsLocked = FALSE;
	// Simulates a bug which allows skipping Chako's fireplace in Grasstown
	playerIFrames = 0;
}

u8 player_invincible() {
	return playerIFrames > 0 || tscState;
}

u8 player_inflict_damage(s16 damage) {
	// Halve damage if we have the arms barrier
	if(playerEquipment & EQUIP_ARMSBARRIER) damage = (damage + 1) >> 1;
	// Show damage numbers
	effect_create_damage(-damage, sub_to_pixel(player.x), sub_to_pixel(player.y));
	// Take health
	if(player.health <= damage) {
		// If health reached 0 we are dead
		player.health = 0;
		// Clear smoke & fill up with smoke around player
		effects_clear_smoke();
		for(u8 i = MAX_SMOKE; i--; ) {
			effect_create_smoke(sub_to_pixel(player.x) + (random() % 90 ) - 45, 
								sub_to_pixel(player.y) + (random() % 90 ) - 45);
		}
		sound_play(SND_PLAYER_DIE, 15);
		tsc_call_event(PLAYER_DEFEATED_EVENT);
		return TRUE;
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
				sheets_refresh_weapon(w);
			} else {
				w->energy = 0;
			}
		} else {
			w->energy -= damage;
		}
	}
	// Knock back
	player.y_speed = SPEED(-0x500); // 2.5 pixels per frame
	player.grounded = FALSE;
	return FALSE;
}

void player_update_bounds() {
	if(player.y_next > block_to_sub(stageHeight + 1)) {
		player.health = 0;
		tsc_call_event(PLAYER_OOB_EVENT);
	}
}

void player_prev_weapon() {
	for(s16 i = currentWeapon - 1; i % MAX_WEAPONS != currentWeapon; i--) {
		if(i < 0) i = MAX_WEAPONS - 1;
		if(playerWeapon[i].type > 0) {
			currentWeapon = i;
			if(weapon_info[playerWeapon[i].type].sprite) {
			TILES_QUEUE(SPR_TILES(weapon_info[playerWeapon[i].type].sprite,0,0),
					TILE_WEAPONINDEX,6);
			}
			sound_play(SND_SWITCH_WEAPON, 5);
			break;
		}
	}
}

void player_next_weapon() {
	for(s16 i = currentWeapon + 1; i % MAX_WEAPONS != currentWeapon; i++) {
		if(i >= MAX_WEAPONS) i = 0;
		if(playerWeapon[i].type > 0) {
			currentWeapon = i;
			if(weapon_info[playerWeapon[i].type].sprite) {
			TILES_QUEUE(SPR_TILES(weapon_info[playerWeapon[i].type].sprite,0,0),
					TILE_WEAPONINDEX,6);
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
	if(!w) {
		for(u8 i = 0; i < MAX_WEAPONS; i++) {
			if(playerWeapon[i].type > 0) continue;
			w = &playerWeapon[i];
			w->type = id;
			w->level = 1;
			w->energy = 0;
			w->maxammo = ammo;
			w->ammo = ammo;
			// Normally weapons are not automatically switched but when picking up the
			// polar star, our first weapon, that is what happens. So make sure the
			// sprite tiles get loaded when we pick it up.
			if(w->type == WEAPON_POLARSTAR) {
				TILES_QUEUE(SPR_TILES(weapon_info[WEAPON_POLARSTAR].sprite,0,0),
					TILE_WEAPONINDEX,6);
			}
			sheets_load_weapon(w);
			break;
		}
	} else {
		w->maxammo += ammo;
		w->ammo += ammo;
	}
}

void player_take_weapon(u8 id) {
	Weapon *w = player_find_weapon(id);
	if(w) {
		if(playerWeapon[currentWeapon].type == id) {
			player_next_weapon();
		}
		w->type = 0;
		w->level = 0;
		w->energy = 0;
		w->maxammo = 0;
		w->ammo = 0;
	}
}

u8 player_has_weapon(u8 id) {
	for(u8 i = 0; i < MAX_WEAPONS; i++) {
		if(playerWeapon[i].type == id) return TRUE;
	}
	return FALSE;
}

void player_trade_weapon(u8 id_take, u8 id_give, u8 ammo) {
	Weapon *w = player_find_weapon(id_take);
	if(w) {
		if(id_take == playerWeapon[currentWeapon].type) {
			if(weapon_info[w->type].sprite) {
			TILES_QUEUE(SPR_TILES(weapon_info[w->type].sprite,0,0),
					TILE_WEAPONINDEX,6);
			}
		}
		w->type = id_give;
		w->level = 1;
		w->energy = 0;
		w->maxammo = ammo;
		w->ammo = ammo;
		sheets_load_weapon(w);
	}
}

void player_refill_ammo() {
	for(u8 i = 0; i < MAX_WEAPONS; i++) {
		playerWeapon[i].ammo = playerWeapon[i].maxammo;
	}
}

void player_delevel_weapons() {
	for(u8 i = 0; i < MAX_WEAPONS; i++) {
		playerWeapon[i].level = 1;
		playerWeapon[i].energy = 0;
		sheets_refresh_weapon(&playerWeapon[i]);
	}
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
	u8 i = 0;
	for(; i < MAX_ITEMS; i++) {
		if(playerInventory[i] == id) {
			playerInventory[i] = 0;
			break;
		}
	}
	// Fill in the gap
	for(; i < MAX_ITEMS - 1; i++) {
		playerInventory[i] = playerInventory[i + 1];
	}
	playerInventory[MAX_ITEMS - 1] = 0; // Don't duplicate, blank the last one
}

u8 player_has_item(u8 id) {
	for(u8 i = 0; i < MAX_ITEMS; i++) {
		if(playerInventory[i] == id) return TRUE;
	}
	return FALSE;
}

void player_equip(u16 id) {
	playerEquipment |= id;
}

void player_unequip(u16 id) {
	playerEquipment &= ~id;
}
