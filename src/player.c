#include "common.h"

#include "ai.h"
#include "audio.h"
#include "camera.h"
#include "dma.h"
#include "effect.h"
#include "entity.h"
#include "input.h"
#include "joy.h"
#include "kanji.h"
#include "memory.h"
#include "npc.h"
#include "resources.h"
#include "sheet.h"
#include "sprite.h"
#include "stage.h"
#include "system.h"
#include "tables.h"
#include "tools.h"
#include "tsc.h"
#include "vdp.h"
#include "vdp_tile.h"
#include "vdp_ext.h"
#include "weapon.h"

#include "player.h"

#define PLAYER_SPRITE_TILES_QUEUE() ({ \
	uint8_t f = player.frame + ((playerEquipment & EQUIP_MIMIMASK) ? 10 : 0); \
	TILES_QUEUE(SPR_TILES(&SPR_Quote,0,f),TILE_PLAYERINDEX,4); \
})

VDPSprite weaponSprite;
uint8_t playerWeaponCount;

uint8_t mapNameSpriteNum;
VDPSprite mapNameSprite[4];

VDPSprite airSprite[2];
uint8_t airPercent;
uint8_t airTick;
uint8_t airDisplayTime;

uint8_t blockl, blocku, blockr, blockd;
uint8_t ledge_time;
uint8_t walk_time;

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
	lookingDown = FALSE;
	mgun_shoottime = 0;
	mgun_chargetime = 0;
	playerEquipment = 0; // Nothing equipped
	for(uint8_t i = 0; i < MAX_ITEMS; i++) playerInventory[i] = 0; // Empty inventory
	playerWeaponCount = 0;
	for(uint8_t i = 0; i < MAX_WEAPONS; i++) playerWeapon[i].type = 0; // No Weapons
	playerMoveMode = 0;
	currentWeapon = 0;
	airPercent = 100;
	airTick = 0;
	airDisplayTime = 0;
	playerIFrames = 0;
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
	uint8_t tile = stage_get_block_type(sub_to_block(player.x), sub_to_block(player.y));
	if(!playerMoveMode) { // Normal movement
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
				if(player.x_speed >  SPEED(0x480)) player.x_speed =  SPEED(0x480);
				if(player.x_speed < -SPEED(0x480)) player.x_speed = -SPEED(0x480);
				if(player.y_speed >  SPEED(0x3FF)) player.y_speed =  SPEED(0x3FF);
				if(player.y_speed < -SPEED(0x4FF)) player.y_speed = -SPEED(0x4FF);
			} else {
				if(player.x_speed >  SPEED(0x5FF)) player.x_speed =  SPEED(0x5FF);
				if(player.x_speed < -SPEED(0x5FF)) player.x_speed = -SPEED(0x5FF);
				if(player.y_speed >  SPEED(0x5FF)) player.y_speed =  SPEED(0x5FF);
				if(player.y_speed < -SPEED(0x5FF)) player.y_speed = -SPEED(0x5FF);
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
		if(playerBoostState == BOOST_OFF && sub_to_block(player.y) < stageHeight - 1) {
			uint8_t blockl_next, blocku_next, blockr_next, blockd_next;
			// Ok so, making the collision with ceiling <= 0 pushes the player out of
			// the ceiling during the opening scene with Kazuma on the computer.
			// Hi Kazuma!
			blocku_next = player.y_speed < 0 ? collide_stage_ceiling(&player) : FALSE;
			blockl_next = (player.x_speed <= 0 || playerPlatform) ? 
							collide_stage_leftwall(&player) : FALSE;
			blockr_next = (player.x_speed >= 0 || playerPlatform) ? 
							collide_stage_rightwall(&player) : FALSE;
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
			} else if(!joy_down(btn[cfg_btn_jump]) && !blockd_next && blockd) {
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
				// playerPlatformTime prevents the player from being too "loose" with platforms
				// They would slip off if the platform moves diagonally
				if(box.bottom == 0) {
					if(++playerPlatformTime > 1) playerPlatform = NULL;
				} else {
					player.grounded = TRUE;
					player.y_next += pixel_to_sub(1);
					playerPlatformTime = 0;
				}
			}
		}
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
	}
	player.x = player.x_next;
	player.y = player.y_next;
	// Damage Tiles / Death check / IFrames
	if(!playerIFrames && player.health > 0) {
		// Match foreground (0x40) and fore+water (0x60) but not wind (0x80) or slope (0x10)
		if((tile & 0xDF) == 0x42) {
			// To reduce the number of objects on screen, the spikes that
			// Ballos creates in the last phase merge with the stage once
			// they rise fully. They should only do 2 damage though, so I
			// added this check.
			if(!tscState) player_inflict_damage((stageID == STAGE_SEAL_CHAMBER) ? 2 : 10);
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
				airTick = TIME(10);
				airPercent--;
				if(airPercent == 0) {
					// Spoilers
					if(system_get_flag(ALMOND_DROWN_FLAG)) {
						tsc_call_event(ALMOND_DROWN_EVENT);
					} else {
						player.health = 0;
						player.frame = 8;
						PLAYER_SPRITE_TILES_QUEUE();
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
	if(JOY_getJoypadType(JOY_1) == JOY_TYPE_PAD3) {
		// 3 button controller cycles with A
		if(joy_pressed(btn[cfg_btn_ffwd])) {
			player_next_weapon();
		}
	} else {
		// 6 button cycles with Y/Z
		if(joy_pressed(btn[cfg_btn_lswap])) {
			player_prev_weapon();
		} else if(joy_pressed(btn[cfg_btn_rswap])) {
			player_next_weapon();
		}
	}
	// Shooting
	Weapon *w = &playerWeapon[currentWeapon];
	if(iSuckAtThisGameSHIT) w->ammo = w->maxammo;
	if(w->type == WEAPON_MACHINEGUN) {
		if(mgun_shoottime > 0) mgun_shoottime--;
		if(joy_down(btn[cfg_btn_shoot])) {
			if(mgun_shoottime == 0) {
				if(w->ammo > 0) {
					weapon_fire(*w);
					w->ammo--;
				} else {
					sound_play(SND_GUN_CLICK, 5);
				}
				mgun_shoottime = IS_PALSYSTEM ? 9 : 10;
			}
		} else {
			if(w->ammo < 100) {
				if(mgun_chargetime > 0) {
					mgun_chargetime--;
				} else {
					w->ammo++;
					mgun_chargetime = (playerEquipment & EQUIP_TURBOCHARGE) ? 2 : 4;
				}
			}
		}
	} else if(w->type == WEAPON_BUBBLER) { // mgun_shoottime and mgun_chargetime reused here
		w->maxammo = 100;
		uint8_t chargespeed;
		if(w->level == 1) {
			chargespeed = TIME(25); // Twice per second
			if(joy_pressed(btn[cfg_btn_shoot])) weapon_fire(*w);
		} else {
			chargespeed = 3; // Around 12-15 per second
			if(mgun_shoottime > 0) mgun_shoottime--;
			if(joy_down(btn[cfg_btn_shoot]) && mgun_shoottime == 0) {
				weapon_fire(*w);
				mgun_shoottime = IS_PALSYSTEM ? 9 : 10;
			}
		}
		if(!joy_down(btn[cfg_btn_shoot]) && w->ammo < 100) {
			if(mgun_chargetime > 0) {
				mgun_chargetime--;
			} else {
				w->ammo++;
				mgun_chargetime = chargespeed;
			}
		}
	} else {
		if(joy_pressed(btn[cfg_btn_shoot])) weapon_fire(*w);
	}
	player_update_bullets();
	if(player.grounded) {
		playerBoosterFuel = TIME(51);
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
	uint16_t acc;
	uint16_t fric;
	uint16_t max_speed = SPEED(810);
	if(player.grounded) {
		acc = SPEED(85);
		fric = SPEED(51);
	} else {
		acc = SPEED(32);
		fric = 0;
	}
	// 2 kinds of water, actual water blocks & background water in Core
	if((blk(player.x, 0, player.y, 0) & BLOCK_WATER) ||
			(stageBackgroundType == 4 && water_entity && player.y > water_entity->y)) {
		player.underwater = TRUE;
		// Half everything, maybe inaccurate?
		acc >>= 1;
		max_speed >>= 1;
		fric >>= 1;
	} else {
		player.underwater = FALSE;
	}
	// Stop player from moving faster if they exceed max speed
	if(joy_down(BUTTON_LEFT)) {
		if(player.x_speed >= -max_speed) player.x_speed -= acc;
	}
	if(joy_down(BUTTON_RIGHT)) {
		if(player.x_speed <= max_speed) player.x_speed += acc;
	}
	// But only slow them down on the ground
	if(player.grounded) {
		if(abs(player.x_speed) <= fric) player.x_speed = 0;
		else if(player.x_speed < 0) player.x_speed += fric;
		else if(player.x_speed > 0) player.x_speed -= fric;
	}
}

void player_update_jump() {
	uint16_t jumpSpeed = 	SPEED(0x500);
	uint16_t gravity = 		SPEED(0x50);
	uint16_t gravityJump = 	SPEED(0x20);
	uint16_t maxFallSpeed = SPEED(0x5FF);
	if(player.underwater) {
		jumpSpeed >>= 1;
		gravity >>= 1;
		gravityJump >>= 1;
		maxFallSpeed >>= 1;
	}
	if(player.jump_time > 0) {
		if(joy_down(btn[cfg_btn_jump])) {
			player.jump_time--;
		} else {
			player.jump_time = 0;
		}
	}
	if(player.jump_time > 0) return;
	if(player.grounded) {
		if(joy_pressed(btn[cfg_btn_jump])) {
			player.grounded = FALSE;
			player.y_speed = -jumpSpeed;
			// Maybe possibly fix jump height?
			player.jump_time = IS_PALSYSTEM ? 0 : 3;
			
			sound_play(SND_PLAYER_JUMP, 3);
		}
	} else if((playerEquipment & (EQUIP_BOOSTER08 | EQUIP_BOOSTER20)) &&
			joy_pressed(btn[cfg_btn_jump])) {
		player_start_booster();
	} else if(playerBoostState == BOOST_OFF) {
		if(joy_down(btn[cfg_btn_jump]) && (player.y_speed <= 0 || player.underwater)) {
			player.y_speed += gravityJump;
		} else {
			player.y_speed += gravity;
		}
		if(player.y_speed > maxFallSpeed) player.y_speed = maxFallSpeed;
	}
}

void player_update_float() {
	uint16_t acc = 		SPEED(0x100);
	uint16_t fric = 	SPEED(0x80);
	uint16_t max_speed = SPEED(0x400);
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
	for(uint8_t i = 0; i < MAX_BULLETS; i++) {
		if(playerBullet[i].ttl) {
			bullet_update(playerBullet[i]);
			playerBullet[i].extent = (extent_box) {
				.x1 = (playerBullet[i].x >> CSF) - (playerBullet[i].hit_box.left),
				.y1 = (playerBullet[i].y >> CSF) - (playerBullet[i].hit_box.top),
				.x2 = (playerBullet[i].x >> CSF) + (playerBullet[i].hit_box.right),
				.y2 = (playerBullet[i].y >> CSF) + (playerBullet[i].hit_box.bottom),
			};
		}
	}
}

void player_update_interaction() {
	// Interaction with entities when pressing down
	if(cfg_updoor ? joy_pressed(BUTTON_UP) : joy_pressed(BUTTON_DOWN)) {
		Entity *e = entityList;
		while(e) {
			if((e->eflags & NPC_INTERACTIVE) && entity_overlapping(&player, e)) {
				// To avoid triggering it twice
				oldstate |= cfg_updoor ? BUTTON_UP : BUTTON_DOWN;
				if(e->event > 0) {
					// Quote should look down while the game logic is frozen
					// Manually send sprite frame since draw() is not called
					lookingDown = TRUE;
					player.frame = 6;
					PLAYER_SPRITE_TILES_QUEUE();
					tsc_call_event(e->event);
					return;
				}
			}
			e = e->next;
		}
		// Question mark above head
		effect_create_misc(EFF_QMARK, (player.x >> CSF), (player.y >> CSF) - 8);
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
	if(!joy_down(btn[cfg_btn_jump])) playerBoostState = BOOST_OFF;
	if(playerBoostState == BOOST_OFF) return;
	// player seems to want it active...check the fuel
	if(playerBoosterFuel == 0) {
		playerBoostState = BOOST_OFF;
		return;
	} else {
		playerBoosterFuel--;
	}
	// ok so then, booster is active right now
	uint8_t sputtering = FALSE;
	
	if (joy_down(BUTTON_LEFT)) player.dir = 0;
	else if (joy_down(BUTTON_RIGHT)) player.dir = 1;

	uint8_t blockl = collide_stage_leftwall(&player),
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
			// I believe the player should not constantly fly upward after
			// getting hit but need to verify what the original CS does
			//if(playerIFrames) {
			//	if(player.y_speed > 0) player.y_speed -= SPEED(0x20);
			//	if(player.y_speed < 0) player.y_speed += SPEED(0x20);
			//}
			//if (joy_down(BUTTON_DOWN)) player.y_speed += SPEED(0x20);
			//if (joy_down(BUTTON_UP)) player.y_speed -= SPEED(0x20);
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

static uint16_t GetKanjiIndex(uint8_t len) {
	if(len < 4) {
		return TILE_NAMEINDEX + len * 4;
	} else {
		return TILE_FONTINDEX + (len - 4) * 4;
	}
}

static uint16_t GetNextChar(uint8_t index) {
	uint16_t chr = JStageName[stageID * 16 + index];
	if(chr >= 0xE0 && chr < 0xFF) {
		return (chr - 0xE0) * 0x60 + (JStageName[stageID * 16 + index + 1] - 0x20) + 0x100;
	} else {
		return chr;
	}
}

void player_show_map_name(uint8_t ttl) {
	// Create a string of tiles in RAM
	uint32_t nameTiles[16][8];
	uint8_t len = 0;
	if(cfg_language) {
		uint8_t i = 0;
		while(i < 16) {
			uint16_t chr1 = GetNextChar(i++);
			if(chr1 == 0) break; // End of string
			if(chr1 > 0xFF) i++;
			uint16_t chr2 = GetNextChar(i++);
			if(chr2 > 0xFF) i++;
			kanji_loadtilesforsprite(GetKanjiIndex(len), chr1, chr2);
			len += chr2 ? 2 : 1;
		}
	} else {
		for(uint8_t i = 0; i < 16; i++) {
			uint8_t chr = stage_info[stageID].name[i] - 0x20;
			if(chr < 0x60) len++;
			else break;
			memcpy(nameTiles[i], &TS_SysFont.tiles[chr * 8], 32);
		}
		if(len) VDP_loadTileData(nameTiles[0], TILE_NAMEINDEX, 16, TRUE);
	}
	// Transfer tile array to VRAM
	if(len > 0) {
		uint8_t charwidth = cfg_language ? 2 : 1;
		mapNameSpriteNum = 0;
		uint16_t x = SCREEN_HALF_W - len * 4 * charwidth;
		for(uint8_t i = 0; i < len; i += (4 / charwidth)) {
			uint8_t sindex = i / (4 / charwidth);
			mapNameSprite[sindex] = (VDPSprite) {
				.x = x + 128,
				.y = SCREEN_HALF_H - 32 + 128,
				.size = SPRITE_SIZE(min(4,(len-i) * charwidth), charwidth)
			};
			uint16_t tindex = i * (cfg_language ? 4 : 1);
			if(cfg_language && i >= 4) {
				mapNameSprite[sindex].attribut = 
						TILE_ATTR_FULL(PAL0,1,0,0,TILE_FONTINDEX+tindex-16);
			} else {
				mapNameSprite[sindex].attribut = 
						TILE_ATTR_FULL(PAL0,1,0,0,TILE_NAMEINDEX+tindex);
			}
			x += 32;
			mapNameSpriteNum++;
		}
		mapNameTTL = ttl;
	}
}

void draw_air_percent() {
	uint32_t numberTiles[3][8];
	memcpy(numberTiles[0], airPercent == 100 ? &TS_Numbers.tiles[8] : TILE_BLANK, 32);
	memcpy(numberTiles[1], &TS_Numbers.tiles[((airPercent / 10) % 10) * 8], 32);
	memcpy(numberTiles[2], &TS_Numbers.tiles[(airPercent % 10) * 8], 32);
	
	VDP_loadTileData(numberTiles[0], TILE_AIRINDEX + 4, 3, TRUE);
	
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
			
			VDP_loadTileData(TILE_BLANK, TILE_AIRINDEX, 1, TRUE);
			
		} else if((airDisplayTime % 32) == 15) {
			const SpriteDefinition *spr = cfg_language ? &SPR_J_Air : &SPR_Air;
			VDP_loadTileData(SPR_TILES(spr, 0, 0), TILE_AIRINDEX, 1, TRUE);
			
		}
		// Calculate air percent and display the value
		if(airTick == TIME(10)) draw_air_percent();
		sprite_addq(airSprite, 2);
	}
}

void player_draw() {
	// Special case when player drowns
	if(!airPercent) {
		sprite_pos(playerSprite,
				sub_to_pixel(player.x) - sub_to_pixel(camera.x) + SCREEN_HALF_W - 8,
				sub_to_pixel(player.y) - sub_to_pixel(camera.y) + SCREEN_HALF_H - 8);
		sprite_add(playerSprite);
		return;
	} else if(!player.health) {
		return; // Don't draw the player if we died in a way that is not drowning
	}
	enum { STAND, WALK1, WALK2, LOOKUP, UPWALK1, UPWALK2, LOOKDN, JUMPDN };
	// Sprite Animation
	player.oframe = player.frame;
	if(playerMoveMode) {
		player.frame = WALK2;
	} else {
		if(player.grounded) {
			if(joy_down(BUTTON_UP) && !controlsLocked) {
				lookingDown = FALSE;
				if(joystate&(BUTTON_LEFT|BUTTON_RIGHT)) {
					ANIMATE(&player, 7, UPWALK1, LOOKUP, UPWALK2, LOOKUP);
				} else {
					player.frame = LOOKUP;
					player.animtime = 0;
				}
			} else if(joystate&(BUTTON_LEFT|BUTTON_RIGHT) && !controlsLocked) {
				lookingDown = FALSE;
				ANIMATE(&player, 7, WALK1, STAND, WALK2, STAND);
			} else if(joy_pressed(BUTTON_DOWN) && !controlsLocked) {
				lookingDown = TRUE;
				player.frame = LOOKDN;
				player.animtime = 0;
			} else if(!lookingDown) {
				player.frame = STAND;
				player.animtime = 0;
			}
		} else {
			lookingDown = FALSE;
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
	}
	// Set frame if it changed
	if(player.frame != player.oframe) PLAYER_SPRITE_TILES_QUEUE();
	// Blink during invincibility frames
	if(!player.hidden && !(playerIFrames & 2)) {
		// Change direction if pressing left or right
		if(playerMoveMode) {
			player.dir = RIGHT;
		} else if(!controlsLocked) {
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
			uint8_t vert = 0, vdir = 0;
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
	if(cfg_iframebug) playerIFrames = 0;
}

uint8_t player_invincible() {
	return playerIFrames > 0 || tscState;
}

uint8_t player_inflict_damage(int16_t damage) {
	// Show damage numbers
	effect_create_damage(-damage, sub_to_pixel(player.x), sub_to_pixel(player.y));
	// Take health
	if((!iSuckAtThisGameSHIT || damage > 99) && player.health <= damage) {
		// If health reached 0 we are dead
		player.health = 0;
		// Clear smoke & fill up with smoke around player
		effects_clear_smoke();
		for(uint8_t i = MAX_SMOKE; i--; ) {
			effect_create_smoke(sub_to_pixel(player.x) + (random() % 90 ) - 45, 
								sub_to_pixel(player.y) + (random() % 90 ) - 45);
		}
		sound_play(SND_PLAYER_DIE, 15);
		tsc_call_event(PLAYER_DEFEATED_EVENT);
		return TRUE;
	}
	if(!iSuckAtThisGameSHIT) player.health -= damage;
	sound_play(SND_PLAYER_HURT, 5);
	playerIFrames = TIME(100);
	// Halve damage applied to weapon energy if we have the arms barrier
	if(playerEquipment & EQUIP_ARMSBARRIER) damage = (damage + 1) >> 1;
	// Decrease weapon exp
	if(!iSuckAtThisGameSHIT && damage > 0 && playerWeapon[currentWeapon].type != 0) {
		Weapon *w = &playerWeapon[currentWeapon];
		if(w->energy < damage) {
			if(w->level > 1) {
				w->level -= 1;
				w->energy += weapon_info[w->type].experience[w->level - 1];
				w->energy -= damage;
				sheets_refresh_weapon(w);
				entity_create(player.x, player.y, 
						cfg_language ? OBJ_LEVELDOWN_JA : OBJ_LEVELDOWN, 0);
			} else {
				w->energy = 0;
			}
		} else {
			w->energy -= damage;
		}
	}
	// Don't knock back in <UNI:0001 mode
	if(!playerMoveMode) player.y_speed = -SPEED(0x400);
	player.grounded = FALSE;
	return FALSE;
}

void player_update_bounds() {
	if(sub_to_block(player.y) > stageHeight) {
		player.health = 0;
		tsc_call_event(PLAYER_OOB_EVENT);
	}
}

void player_prev_weapon() {
	for(int16_t i = currentWeapon - 1; i % MAX_WEAPONS != currentWeapon; i--) {
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
	for(int16_t i = currentWeapon + 1; i % MAX_WEAPONS != currentWeapon; i++) {
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

Weapon *player_find_weapon(uint8_t id) {
	for(uint8_t i = 0; i < MAX_WEAPONS; i++) {
		if(playerWeapon[i].type == id) return &playerWeapon[i];
	}
	return NULL;
}

void player_give_weapon(uint8_t id, uint8_t ammo) {
	Weapon *w = player_find_weapon(id);
	if(!w) {
		for(uint8_t i = 0; i < MAX_WEAPONS; i++) {
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

void player_take_weapon(uint8_t id) {
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

uint8_t player_has_weapon(uint8_t id) {
	for(uint8_t i = 0; i < MAX_WEAPONS; i++) {
		if(playerWeapon[i].type == id) return TRUE;
	}
	return FALSE;
}

void player_trade_weapon(uint8_t id_take, uint8_t id_give, uint8_t ammo) {
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
		if(ammo != 0) { // Only change ammo if parameter is nonzero
			w->maxammo = ammo;
			w->ammo = ammo;
		}
		sheets_load_weapon(w);
	}
}

void player_refill_ammo() {
	for(uint8_t i = 0; i < MAX_WEAPONS; i++) {
		playerWeapon[i].ammo = playerWeapon[i].maxammo;
	}
}

void player_delevel_weapons() {
	for(uint8_t i = 0; i < MAX_WEAPONS; i++) {
		playerWeapon[i].level = 1;
		playerWeapon[i].energy = 0;
		sheets_refresh_weapon(&playerWeapon[i]);
	}
}

void player_heal(uint8_t health) {
	player.health += health;
	if(player.health > playerMaxHealth) player.health = playerMaxHealth;
}

void player_maxhealth_increase(uint8_t health) {
	player.health += health;
	playerMaxHealth += health;
}

void player_give_item(uint8_t id) {
	for(uint8_t i = 0; i < MAX_ITEMS; i++) {
		if(playerInventory[i] == 0) {
			playerInventory[i] = id;
			break;
		}
	}
}

void player_take_item(uint8_t id) {
	uint8_t i = 0;
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

uint8_t player_has_item(uint8_t id) {
	//printf("Polling for item %hu", id);
	for(uint8_t i = 0; i < MAX_ITEMS; i++) {
		//printf("See: %hu", playerInventory[i]);
		if(playerInventory[i] == id) return TRUE;
	}
	return FALSE;
}

void player_equip(uint16_t id) {
	playerEquipment |= id;
	// Force the sprite to change so the mimiga mask will show immediately
	if(id & EQUIP_MIMIMASK) PLAYER_SPRITE_TILES_QUEUE();
}

void player_unequip(uint16_t id) {
	playerEquipment &= ~id;
	if(id & EQUIP_MIMIMASK) PLAYER_SPRITE_TILES_QUEUE();
}
