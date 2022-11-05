#include "common.h"

#include "audio.h"
#include "res/system.h"
#include "res/local.h"
#include "camera.h"
#include "md/dma.h"
#include "effect.h"
#include "entity.h"
#include "hud.h"
#include "md/joy.h"
#include "cjk.h"
#include "math.h"
#include "md/stdlib.h"
#include "npc.h"
#include "resources.h"
#include "sheet.h"
#include "stage.h"
#include "system.h"
#include "tables.h"
#include "md/comp.h"
#include "tsc.h"
#include "md/sys.h"
#include "md/vdp.h"
#include "weapon.h"

#include "player.h"
#include "md/xgm.h"

#define PLAYER_SPRITE_TILES_QUEUE() ({ \
	uint8_t f = player.frame + ((playerEquipment & EQUIP_MIMIMASK) ? 10 : 0); \
	TILES_QUEUE(SPR_TILES(&SPR_Quote,0,f),TILE_PLAYERINDEX,4); \
})

uint8_t currentWeapon;
Entity player;
VDPSprite playerSprite;
uint8_t playerIFrames;
uint8_t playerMoveMode;
uint8_t lookingDown;
uint16_t playerMaxHealth;
uint8_t controlsLocked;
uint16_t playerEquipment;
uint8_t playerInventory[MAX_ITEMS];
uint8_t shoot_cooldown, mgun_chargetime, playerNoBump;
Entity *playerPlatform;
uint8_t playerPlatformTime;
uint8_t playerBoosterFuel, playerBoostState, lastBoostState;
uint16_t mapNameTTL;
uint8_t iSuckAtThisGameSHIT;
uint8_t missileEmptyFlag;

const uint8_t spur_time[2][4] = {
	{ 0, 40, 60, 200 }, // NTSC
	{ 0, 33, 50, 166 }, // PAL
};

VDPSprite weaponSprite;

uint8_t mapNameSpriteNum;
VDPSprite mapNameSprite[4];

VDPSprite airSprite[2];
uint8_t airPercent;
uint8_t airTick;
uint8_t airDisplayTime;

uint8_t blockl, blocku, blockr, blockd;
uint8_t ledge_time;

VDPSprite airTankSprite;

static void player_update_booster();
static void player_update_interaction();
static void player_update_air_display();

static void player_update_movement();
static void player_update_walk();
static void player_update_jump();
static void player_update_float();

static void player_prev_weapon();
static void player_next_weapon();

// Default values for player
void player_init() {
	controlsLocked = FALSE;
	player.hidden = FALSE;
	player.dir = 0;
	player.flags = NPC_IGNORE44|NPC_SHOWDAMAGE;
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
	shoot_cooldown = 0;
	mgun_chargetime = 0;
	playerEquipment = 0; // Nothing equipped
	for(uint8_t i = 0; i < MAX_ITEMS; i++) playerInventory[i] = 0; // Empty inventory
	for(uint8_t i = 0; i < MAX_WEAPONS; i++) playerWeapon[i].type = 0; // No Weapons
    for(uint8_t i = 0; i < MAX_BULLETS; i++) playerBullet[i].extent.x1 = 0xFFFF;
	playerMoveMode = 0;
	currentWeapon = 0;
	airPercent = 100;
	airTick = 0;
	airDisplayTime = 0;
	playerIFrames = 0;
	mapNameTTL = 0;
	missileEmptyFlag = FALSE;
	// Booster trail sprite tiles
	vdp_tiles_load(SPR_TILES(&SPR_Boost, 0, 0), 12, 4);
	// AIR Sprite
	const SpriteDefinition *spr = cfg_language == LANG_JA ? &SPR_J_Air : &SPR_Air;
	vdp_tiles_load(SPR_TILES(spr, 0, 0), TILE_AIRINDEX, 4);
	airSprite[0] = (VDPSprite) {
		.x = SCREEN_HALF_W - 28 + 128, .y = SCREEN_HALF_H - 24 + 128, 
		.size = SPRITE_SIZE(4, 1), .attr = TILE_ATTR(PAL0,1,0,0,TILE_AIRINDEX)
	};
	airSprite[1] = (VDPSprite) {
		.x = SCREEN_HALF_W + 8 + 128, .y = SCREEN_HALF_H - 24 + 128, 
		.size = SPRITE_SIZE(3, 1), .attr = TILE_ATTR(PAL0,1,0,0,TILE_AIRINDEX+4)
	};
	// Air Tank sprite
	vdp_tiles_load(SPR_TILES(&SPR_Bubble, 0, 0), TILE_AIRTANKINDEX, 9);
	airTankSprite = (VDPSprite) {
		.size = SPRITE_SIZE(3,3),
		.attr = TILE_ATTR(PAL0,0,0,0,TILE_AIRTANKINDEX)
	};
	// Player sprite
	playerSprite = (VDPSprite) {
		.size = SPRITE_SIZE(2,2),
		.attr = TILE_ATTR(PAL0,0,0,1,TILE_PLAYERINDEX)
	};
}

void player_update() {
	uint8_t tile = stage_get_block_type(sub_to_block(player.x), sub_to_block(player.y));
	if(!playerMoveMode) { // Normal movement
		// Wind/Water current
		if(tile & 0x80) {
			// This stops us fron getting stuck in ledges
			if((blk(player.x, 0, player.y, 6) & 0x83) == 0x81) {
				player.y_speed -= SPEED_8(0x80);
			} else {
				switch(tile & 0x03) {
					case 0: player.x_speed -= SPEED_8(0x88); break;
					case 1: player.y_speed -= SPEED_8(0x80); break;
					case 2: player.x_speed += SPEED_8(0x88); break;
					case 3: player.y_speed += SPEED_8(0x50); break;
				}
			}
			if(player.underwater) {
				if(player.x_speed >  SPEED_12(0x4E0)) player.x_speed =  SPEED_12(0x4E0);
				if(player.x_speed < -SPEED_12(0x4E0)) player.x_speed = -SPEED_12(0x4E0);
				if(player.y_speed >  SPEED_12(0x420)) player.y_speed =  SPEED_12(0x420);
				if(player.y_speed < -SPEED_12(0x4FF)) player.y_speed = -SPEED_12(0x4FF);
			} else {
				if(player.x_speed >  SPEED_12(0x5FF)) player.x_speed =  SPEED_12(0x5FF);
				if(player.x_speed < -SPEED_12(0x5FF)) player.x_speed = -SPEED_12(0x5FF);
				if(player.y_speed >  SPEED_12(0x5FF)) player.y_speed =  SPEED_12(0x5FF);
				if(player.y_speed < -SPEED_12(0x5FF)) player.y_speed = -SPEED_12(0x5FF);
			}
		}
		player_update_movement();
		// Updates the booster while it is enabled
		if(playerBoostState != BOOST_OFF) {
			player_update_booster();
		}
		// Slow down the player when we stop the Booster 2.0.
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
		// As such we check again whether the booster is enabled.
		uint16_t px = player.x_next >> CSF, py = player.y_next >> CSF;
		uint16_t bx = px >> 4, by = py >> 4;
		if(playerBoostState == BOOST_OFF && bx > 0 && bx < stageWidth - 1 && by > 0 && by < stageHeight - 1) {
			uint8_t blockl_next, blocku_next, blockr_next, blockd_next;
			// Ok so, making the collision with ceiling <= 0 pushes the player out of
			// the ceiling during the opening scene with Kazuma on the computer.
			// Hi Kazuma!
			blocku_next = player.y_speed < 0 ? collide_stage_ceiling(&player) : FALSE;
			blockl_next = (player.x_speed <= 0 || playerPlatform) ? collide_stage_leftwall(&player) : FALSE;
			blockr_next = (player.x_speed >= 0 || playerPlatform) ? collide_stage_rightwall(&player) : FALSE;
			if(ledge_time == 0) {
				if(player.grounded) {
					player.grounded = collide_stage_floor_grounded(&player);
				} else if(player.y_speed >= 0) {
					player.grounded = collide_stage_floor(&player);
				}
			}
			blockd_next = player.grounded;
			// Here I do something weird to emulate the way the game pushes quote
			// into small gaps.
			if(!blockl_next && blockl && joy_down(JOY_LEFT)) {
				player.x_speed -= 0xE0;
				player.x_next = player.x + player.x_speed;
			}
			if(!blockr_next && blockr && joy_down(JOY_RIGHT)) {
				player.x_speed += 0xE0;
				player.x_next = player.x + player.x_speed;
			}
			if(ledge_time > 0) {
				ledge_time--;
				player.y_next += 0x600;
				blockl_next = player.x_speed < 0 ? collide_stage_leftwall(&player) : FALSE;
				blockr_next = player.x_speed > 0 ? collide_stage_rightwall(&player) : FALSE;
				player.y_next -= 0x600;
			} else if(!joy_down(btn[cfg_btn_jump]) && !blockd_next && blockd && !playerPlatform) {
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
				// playerPlatformTime prevents the player from being too "loose" with platforms.
				// They would slip off if the platform moves diagonally.
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
	// If somehow the player gets stuck in a wall, push them out
	if(blk(player.x, 0, player.y, 0) == 0x41) {
		if     (blk(player.x, -16, player.y,   0) != 0x41) player.x -= 0x2000;
		else if(blk(player.x,  16, player.y,   0) != 0x41) player.x += 0x2000;
		else if(blk(player.x,   0, player.y, -16) != 0x41) player.y -= 0x2000;
		else if(blk(player.x,   0, player.y,  16) != 0x41) player.y += 0x2000;
	}
	player.x = player.x_next;
	player.y = player.y_next;
	// Damage Tiles / Death check / IFrames
	if(!playerIFrames && player.health > 0) {
		// Match foreground (0x40) and fore+water (0x60) but not wind (0x80) or slope (0x10)
		if((tile & 0xDF) == 0x42) {
			// To reduce the amount objects on screen, the spikes that
			// Ballos creates in the last phase merge with the stage once
			// they rise fully. They should only do 2 damage though, so I
			// added this check.
			if(!tscState) player_inflict_damage((stageID == STAGE_SEAL_CHAMBER) ? 2 : 10);
		}
		if(player.health == 0) return;
	} else {
		playerIFrames--;
	}
	// This "damage_time" block is for weapon energy numbers.
	// When the player gets hurt, create damage numbers instantly.
	if(player.damage_time > 0) {
		player.damage_time--;
		if(player.damage_time == 0) {
			effect_create_damage(player.damage_value, &player, -8, -4);
			player.damage_value = 0;
		}
	}
	// Handle air when underwater, unless a script is running
	if(!tscState) {
		if(player.underwater && !(playerEquipment & EQUIP_AIRTANK)) {
			if(airTick == 0) {
				airTick = TIME_8(9);
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
	if(joytype == JOY_TYPE_PAD3) {
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
	// Bullets before shooting, fixes enemies in the wall not getting hit
	player_update_bullets();
	// Shooting
	Weapon *w = &playerWeapon[currentWeapon];
	if(iSuckAtThisGameSHIT) w->ammo = w->maxammo;
	if(w->type == WEAPON_MACHINEGUN) {
		if(shoot_cooldown > 0) shoot_cooldown--;
		if(joy_down(btn[cfg_btn_shoot])) {
			if(shoot_cooldown == 0) {
				if(w->ammo > 0) {
					missileEmptyFlag = FALSE;
					weapon_fire(*w);
					w->ammo--;
				} else {
					sound_play(SND_GUN_CLICK, 5);
					if(!missileEmptyFlag) {
						missileEmptyFlag = TRUE;
						entity_create(player.x, player.y, cfg_language == LANG_JA ? OBJ_EMPTY_JA : OBJ_EMPTY, 0);
					}
				}
				shoot_cooldown = (pal_mode || cfg_60fps) ? 7 : 8;
			}
		} else {
			shoot_cooldown = 0;
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
			chargespeed = TIME_8(25); // Twice per second
			if(joy_pressed(btn[cfg_btn_shoot])) weapon_fire(*w);
		} else {
			chargespeed = 3; // Around 12-15 per second
			if(shoot_cooldown > 0) shoot_cooldown--;
			if(joy_down(btn[cfg_btn_shoot]) && shoot_cooldown == 0) {
				weapon_fire(*w);
				shoot_cooldown = (pal_mode || cfg_60fps) ? 8 : 9;
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
	} else if(w->type == WEAPON_SPUR) {
		if(joy_pressed(btn[cfg_btn_shoot])) {
			weapon_fire(*w);
			shoot_cooldown = 4;
		} else if(joystate & btn[cfg_btn_shoot]) {
			uint8_t maxenergy = spur_time[pal_mode||cfg_60fps][w->level];
			if(!(w->level == 3 && w->energy == maxenergy)) {
				w->energy += (playerEquipment & EQUIP_TURBOCHARGE) ? 3 : 2;
				if(w->energy >= maxenergy) {
					if(w->level == 3) {
						w->energy = maxenergy;
						sound_play(SND_SPUR_MAXED, 5);
					} else {
						w->level++;
						w->energy = 0;
					}
				} else {
					mgun_chargetime++;
					if((mgun_chargetime & 3) == 1) {
						sound_play(SND_SPUR_CHARGE_1 + w->level - 1, 2);
					}
				}
			}
		} else {
			if(mgun_chargetime) {
				if(w->level > 1) {
					if(w->energy == spur_time[pal_mode||cfg_60fps][3]) w->level = 4;
					weapon_fire(*w);
				}
				mgun_chargetime = 0;
			}
			w->level = 1;
			w->energy = 0;
			hud_force_energy();
		}
	} else if(shoot_cooldown) {
		shoot_cooldown--;
	} else {
		if(joy_pressed(btn[cfg_btn_shoot])) {
			weapon_fire(*w);
			shoot_cooldown = 4;
		}
	}
	
	if(player.grounded) {
		playerBoosterFuel = (pal_mode || cfg_60fps) ? 51 : 61;
		player_update_interaction();
	}
	player_draw();
	if(mapNameTTL > 0) {
		mapNameTTL--;
		vdp_sprites_add(mapNameSprite, mapNameSpriteNum);
	}
}

static void player_update_movement() {
	player_update_walk();
	player_update_jump();
	player.x_next = player.x + player.x_speed;
	player.y_next = player.y + player.y_speed;
}

static void player_update_walk() {
	int16_t acc;
	int16_t fric;
	int16_t max_speed;
	if(pal_mode || cfg_60fps) {
		max_speed = 810;
		if(player.grounded) {
			acc = 85;
			fric = 51;
		} else {
			acc = 32;
			fric = 0;
		}
	} else {
		max_speed = 675;
		if(player.grounded) {
			acc = 71;
			fric = 42;
		} else {
			acc = 27;
			fric = 0;
		}
	}
	// 2 kinds of water, actual water blocks & background water in Core
	if((blk(player.x, 0, player.y, 0) & BLOCK_WATER) ||
			(stageBackgroundType == 4 && water_entity && player.y > water_entity->y)) {
		if(!player.underwater) {
			player.underwater = TRUE;
			sound_play(SND_SPLASH, 5);
			effect_create_misc(EFF_SPLASH, player.x >> CSF, player.y >> CSF, FALSE);
			effect_create_misc(EFF_SPLASH, player.x >> CSF, player.y >> CSF, FALSE);
			effect_create_misc(EFF_SPLASH, player.x >> CSF, player.y >> CSF, FALSE);
		}
		// Half everything, maybe inaccurate?
		acc >>= 1;
		max_speed >>= 1;
		fric >>= 1;
	} else {
		player.underwater = FALSE;
	}
	// Stop player from moving faster if they exceed max speed
	if(joy_down(JOY_LEFT)) {
		if(player.x_speed >= -max_speed) player.x_speed -= acc;
	}
	if(joy_down(JOY_RIGHT)) {
		if(player.x_speed <= max_speed) player.x_speed += acc;
	}
	// But only slow them down on the ground
	if(player.grounded) {
		if(abs(player.x_speed) <= fric) player.x_speed = 0;
		else if(player.x_speed < 0) player.x_speed += fric;
		else if(player.x_speed > 0) player.x_speed -= fric;
	}
}

static void player_update_jump() {
	int16_t jumpSpeed;
	int16_t gravity;
	int16_t gravityJump;
	int16_t maxFallSpeed;
	if(pal_mode || cfg_60fps) {
		// See issue #270 for the reason these values are off
		jumpSpeed = 	0x4E0; // 0x500
		gravity = 		0x50;
		gravityJump = 	0x21; // 0x20
		maxFallSpeed =	0x5FF;
	} else {
		jumpSpeed = 	0x42A;
		gravity = 		0x42;
		gravityJump = 	0x1A;
		maxFallSpeed =	0x4FF;
	}
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
			player.jump_time = (pal_mode || cfg_60fps) ? 0 : 3;
			player.jump_time += player.underwater ? 2 : 0;
			
			sound_play(SND_PLAYER_JUMP, 3);
		}
	} else if((playerEquipment & (EQUIP_BOOSTER08 | EQUIP_BOOSTER20)) &&
			joy_pressed(btn[cfg_btn_jump])) {
		player_start_booster();
	} else if(playerBoostState == BOOST_OFF) {
		if(joy_down(btn[cfg_btn_jump]) && (player.y_speed < 0 || player.underwater)) {
			player.y_speed += gravityJump;
		} else {
			player.y_speed += gravity;
		}
		if(player.y_speed > maxFallSpeed) player.y_speed = maxFallSpeed;
	}
}

static void player_update_float() {
	int16_t acc;
	int16_t fric;
	int16_t max_speed;
	if(pal_mode || cfg_60fps) {
		acc =		256;
		fric =		128;
		max_speed =	1024;
	} else {
		acc =		213;
		fric =		106;
		max_speed =	853;
	}
	if (joy_down(JOY_LEFT)) {
		player.x_speed -= acc;
		if (player.x_speed < -max_speed) player.x_speed = -max_speed;
	} else if (joy_down(JOY_RIGHT)) {
		player.x_speed += acc;
		if (player.x_speed > max_speed) player.x_speed = max_speed;
	} else {
		if (player.x_speed < fric && player.x_speed > -fric) player.x_speed = 0;
		else if (player.x_speed < 0) player.x_speed += fric;
		else if (player.x_speed > 0) player.x_speed -= fric;
	}
	if (joy_down(JOY_UP)) {
		player.y_speed -= acc;
		if (player.y_speed < -max_speed) player.y_speed = -max_speed;
	} else if (joy_down(JOY_DOWN)) {
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
            if(playerBullet[i].ttl) {
                playerBullet[i].extent = (extent_box) {
                        .x1 = (playerBullet[i].x >> CSF) - (playerBullet[i].hit_box.left),
                        .y1 = (playerBullet[i].y >> CSF) - (playerBullet[i].hit_box.top),
                        .x2 = (playerBullet[i].x >> CSF) + (playerBullet[i].hit_box.right),
                        .y2 = (playerBullet[i].y >> CSF) + (playerBullet[i].hit_box.bottom),
                };
            } else {
                playerBullet[i].extent = (extent_box) {};
            }
		}
	}
}

static void player_update_interaction() {
	// Interaction with entities when pressing down
	if(cfg_updoor ? joy_pressed(JOY_UP) : joy_pressed(JOY_DOWN)) {
		Entity *e = entityList;
		while(e) {
			if((e->flags & NPC_INTERACTIVE) && entity_overlapping(&player, e)) {
				// To avoid triggering it twice
				joystate_old |= cfg_updoor ? JOY_UP : JOY_DOWN;
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
		effect_create_misc(EFF_QMARK, (player.x >> CSF), (player.y >> CSF) - 12, TRUE);
	}
}

void player_start_booster() {
	if(playerBoosterFuel == 0) return;
	player.jump_time = 0;
	player.grounded = FALSE;
	playerPlatform = NULL;
	playerPlatformTime = 0;
	// Pick a direction with Booster 2.0, default up
	if ((playerEquipment & EQUIP_BOOSTER20)) {
		playerBoostState = BOOST_UP;
		// in order of precedence
		if (joy_down(JOY_LEFT)) {
			player.dir = 0;
			playerBoostState = BOOST_HOZ;
		} else if (joy_down(JOY_RIGHT)) {
			player.dir = 1;
			playerBoostState = BOOST_HOZ;
		} else if (joy_down(JOY_DOWN)) {
			playerBoostState = BOOST_DOWN;
		} else if (joy_down(JOY_UP)) {
			playerBoostState = BOOST_UP;
		}
		
		if (playerBoostState == BOOST_UP || playerBoostState == BOOST_DOWN)
			player.x_speed = 0;
		
		switch(playerBoostState) {
			case BOOST_UP:
				player.y_speed = -SPEED_12(0x600);
			break;
			case BOOST_DOWN:
				player.y_speed = SPEED_12(0x600);
			break;
			case BOOST_HOZ:
				player.y_speed = 0;
				if (joy_down(JOY_LEFT)) {
					player.x_speed = -SPEED_12(0x600);
                    // Little hack to prevent clipping against left wall
                    player.x += 0x100;
				} else {
					player.x_speed = SPEED_12(0x600);
                }
			break;
		}
	} else {
		playerBoostState = BOOST_08;
		// help it overcome gravity
		if (player.y_speed > SPEED_8(0xFF))
			player.y_speed >>= 1;
	}
	sound_play(SND_BOOSTER, 3);
}

static void player_update_booster() {
	if(!(playerEquipment & (EQUIP_BOOSTER08 | EQUIP_BOOSTER20))) playerBoostState = BOOST_OFF;
	if(!joy_down(btn[cfg_btn_jump])) {
	    // Decelerate after boosting down
	    //if(playerBoostState == BOOST_DOWN) player.y_speed >>= 2;
	    playerBoostState = BOOST_OFF;
	}
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
	
	if (joy_down(JOY_LEFT)) player.dir = 0;
	else if (joy_down(JOY_RIGHT)) player.dir = 1;

	// Don't bump in the opposite direction when hitting the ceiling
	if(player.y_speed <= 0 && collide_stage_ceiling(&player)) player.y_speed = 0;
	uint8_t nblockl = collide_stage_leftwall(&player),
			nblockr = collide_stage_rightwall(&player);
	collide_stage_floor(&player);
	player.grounded = FALSE;
	playerPlatform = NULL;
	playerPlatformTime = 0;

	switch(playerBoostState) {
		case BOOST_HOZ:
		{
            if(!player.dir) { // Left
                if(nblockl) {
                    player.y_speed = -SPEED_8(0xFF);
                    player.x += 0x100;
                }
            } else { // Right
                if(nblockr) {
                    player.y_speed = -SPEED_8(0xFF);
                    player.x -= 0x100;
                }
            }
		}
		break;
		case BOOST_UP:
		{
			player.y_speed -= SPEED_8(0x20);
			if(player.y_speed < -SPEED_12(0x600)) player.y_speed = -SPEED_12(0x600);
		}
		break;
		case BOOST_DOWN:
		{
			player.y_speed += SPEED_8(0x20);
		}
		break;
		case BOOST_08:
		{
			// top speed and sputtering
			if (player.y_speed < -SPEED_10(0x3FF)) {
				player.y_speed += SPEED_8(0x20);
				sputtering = TRUE;	// no sound/smoke this frame
			} else {
				player.y_speed -= SPEED_8(0x20);
			}
		}
		break;
	}
	// smoke and sound effects
	if((++player.timer2 >= TIME_8(5)) && !sputtering) {
		player.timer2 = 0;
		sound_play(SND_BOOSTER, 3);
		switch(playerBoostState) {
			case BOOST_08: effect_create_misc(EFF_BOOST8, player.x >> CSF, (player.y >> CSF) + 6, 0); break;
			case BOOST_HOZ: 
				if(player.dir) effect_create_misc(EFF_BOOST2, (player.x >> CSF) - 0, (player.y >> CSF) + 2, 0);
				else effect_create_misc(EFF_BOOST2, (player.x >> CSF) + 0, (player.y >> CSF) + 2, 0);
			break;
			case BOOST_UP: effect_create_misc(EFF_BOOST2, player.x >> CSF, (player.y >> CSF) + 6, 0); break;
			case BOOST_DOWN: effect_create_misc(EFF_BOOST2, player.x >> CSF, (player.y >> CSF) - 4, 0); break;
		}
	}
}

static uint16_t GetNextChar(uint8_t index) {
	const uint8_t *names = (const uint8_t*)STAGE_NAMES;
	uint16_t chr = names[stageID * 16 + index];
	if(chr >= 0xE0 && chr < 0xFF) {
		return (chr - 0xE0) * 0x60 + (names[stageID * 16 + index + 1] - 0x20) + 0x100;
	} else {
		return chr;
	}
}

static void show_map_jname(uint8_t ttl) {
    static const uint8_t mul6[20] = {
            0, 6, 12, 18, 24, 30, 36, 42, 48, 54, 60, 66, 72, 78, 84, 90, 96, 102, 108, 114
    };
    uint16_t len = 0;
    uint16_t i = 0;
    mapNameSpriteNum = 0;
    while(i < 16) {
        uint16_t chr1, chr2;
        chr1 = GetNextChar(i++);
        if(chr1 == 0) break; // End of string
        if(chr1 > 0xFF) i++;
        chr2 = GetNextChar(i++);
        if (chr2 > 0xFF) i++;
        cjk_drawsprite(mul6[mapNameSpriteNum], chr1, chr2);
        len += chr2 ? 2 : 1;
        mapNameSpriteNum++;
    }
    if(!len) return;

    uint16_t x = SCREEN_HALF_W - mul6[len] + 128;
    uint16_t y = SCREEN_HALF_H - 32 + 128;
    for(i = 0; i < len; i += 2) {
        uint16_t tind = mul6[i >> 1];
        mapNameSprite[i >> 1] = (VDPSprite) {
                .x = x, .y = y, .size = SPRITE_SIZE(min(3,(len-i)<<1), 2),
                .attr = TILE_ATTR(PAL0,1,0,0,TILE_FACEINDEX+tind)
        };
        x += 24;
    }
    mapNameTTL = ttl;
}

void player_show_map_name(uint8_t ttl) {
	// Boss bar overwrites the name
	if(stageID == STAGE_WATERWAY_BOSS) return;
	// Show kanji name
	if(cfg_language >= LANG_JA && cfg_language <= LANG_KO) {
        show_map_jname(ttl);
        return;
    }
	// English name
    const uint8_t *str = (uint8_t*) stage_info[stageID].name;
	if(cfg_language > 0) {
		str = ((const uint8_t*)STAGE_NAMES) + (stageID << 5);
	}
    //if((uint32_t) str >= 0x400000) {
    //    str = (const uint8_t*)(0x380000 | ((uint32_t)str & 0x7FFFF));
    //}
	//uint32_t nameTiles[16][8];
	uint16_t len = 0;
    uint16_t pos = 0;
    while(len < 16) {
        uint8_t chr = str[pos++];
        if(chr == 0x01) {
            chr = str[pos++];
            chr += 0x5F;
            len++;
        } else {
            chr -= 0x20;
            if (chr < 0x60) len++;
            else break;
        }
        //memcpy(nameTiles[len-1], &TS_SysFont.tiles[chr * 8], 32);
        vdp_tiles_load_uftc(UFTC_SysFont, TILE_NAMEINDEX + len-1, chr, 1);
    }
    if(len) {
        //vdp_tiles_load(nameTiles[0], TILE_NAMEINDEX, 16);
    } else {
        return;
    }
	// Transfer tile array to VRAM
    mapNameSpriteNum = 0;
    uint16_t x = SCREEN_HALF_W - (len<<2) + 128;
    uint16_t y = SCREEN_HALF_H - 32 + 128;
    for(uint16_t i = 0; i < len; i += 4) {
        uint8_t sind = i>>2;
        mapNameSprite[sind] = (VDPSprite) {
            .x = x, .y = y, .size = SPRITE_SIZE(min(4,len-i), 1)
        };
        mapNameSprite[sind].attr = TILE_ATTR(PAL0,1,0,0,TILE_NAMEINDEX+i);
        x += 32;
        mapNameSpriteNum++;
    }
    mapNameTTL = ttl;
}

static void draw_air_percent() {
	uint8_t airTemp = airPercent;
	uint32_t numberTiles[3][8];
	if(airTemp >= 100) {
		airTemp -= 100;
		memcpy(numberTiles[0], &TS_Numbers.tiles[8], 32);
	} else {
		memcpy(numberTiles[0], BlankData, 32);
	}
	memcpy(numberTiles[1], &TS_Numbers.tiles[div10[airTemp] * 8], 32);
	memcpy(numberTiles[2], &TS_Numbers.tiles[mod10[airTemp] * 8], 32);
	
	vdp_tiles_load(numberTiles[0], TILE_AIRINDEX + 4, 3);
	
}

static void player_update_air_display() {
	// Blink for a second after getting out of the water
	if(airPercent == 100) {
		if(airDisplayTime == TIME_8(50)) {
			draw_air_percent();
		} else if(airDisplayTime & 4) {
			vdp_sprites_add(airSprite, 2);
		}
	} else {
		airDisplayTime++;
		if((airDisplayTime & 31) == 0) {
			vdp_tiles_load(BlankData, TILE_AIRINDEX, 1);
		} else if((airDisplayTime & 31) == 15) {
			const SpriteDefinition *spr = cfg_language == LANG_JA ? &SPR_J_Air : &SPR_Air;
			vdp_tiles_load(SPR_TILES(spr, 0, 0), TILE_AIRINDEX, 1);
		}
		// Calculate air percent and display the value
		if(airTick == TIME_8(9)) draw_air_percent();
		vdp_sprites_add(airSprite, 2);
	}
}

void player_draw() {
	// Special case when player drowns
	if(!airPercent) {
		sprite_pos(playerSprite,
				sub_to_pixel(player.x) - sub_to_pixel(camera.x) + SCREEN_HALF_W - 8,
				sub_to_pixel(player.y) - sub_to_pixel(camera.y) + SCREEN_HALF_H - 8);
		vdp_sprite_add(&playerSprite);
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
			if(joy_down(JOY_UP) && !controlsLocked) {
				lookingDown = FALSE;
				if(joystate&(JOY_LEFT | JOY_RIGHT)) {
					static const uint8_t f[] = { UPWALK1, LOOKUP, UPWALK2, LOOKUP };
					if(++player.timer2 > TIME_8(5)) {
						player.timer2 = 0;
						if(++player.animtime > 3) player.animtime = 0;
					}
					player.frame = f[player.animtime];
				} else {
					player.frame = LOOKUP;
					player.animtime = 0;
				}
			} else if(joystate&(JOY_LEFT | JOY_RIGHT) && !controlsLocked) {
				lookingDown = FALSE;
				static const uint8_t f[] = { WALK1, STAND, WALK2, STAND };
				if(++player.timer2 > TIME_8(5)) {
					player.timer2 = 0;
					if(++player.animtime > 3) player.animtime = 0;
				}
				player.frame = f[player.animtime];
			} else if(joy_pressed(JOY_DOWN) && !controlsLocked) {
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
			if(joy_down(JOY_UP) && !controlsLocked) {
				player.frame = (player.y_speed > 0) ? UPWALK2 : UPWALK1;
			} else if(joy_down(JOY_DOWN) && !controlsLocked) {
				player.frame = JUMPDN;
			} else {
				player.frame = (player.y_speed > 0) ? WALK2 : WALK1;
			}
		}
		if((player.animtime & 1) && player.timer2 == 1) sound_play(SND_PLAYER_WALK, 2);
	}
	// Set frame if it changed
	if(player.frame != player.oframe) PLAYER_SPRITE_TILES_QUEUE();
	// Blink during invincibility frames
	if(!player.hidden && !(playerIFrames & 2)) {
		// Change direction if pressing left or right
		if(playerMoveMode) {
			player.dir = RIGHT;
		} else if(!controlsLocked) {
			if(joy_down(JOY_RIGHT)) {
				player.dir = 1;
			} else if(joy_down(JOY_LEFT)) {
				player.dir = 0;
			}
		}
		sprite_hflip(playerSprite, player.dir);
		sprite_pos(playerSprite,
				sub_to_pixel(player.x) - sub_to_pixel(camera.x) + SCREEN_HALF_W - 8,
				sub_to_pixel(player.y) - sub_to_pixel(camera.y) + SCREEN_HALF_H - 8);
		vdp_sprite_add(&playerSprite);
		if(playerWeapon[currentWeapon].type > 0 && playerWeapon[currentWeapon].type != WEAPON_BLADE) {
			uint16_t vert = 0, vdir = 0;
			if(player.frame==LOOKUP || player.frame==UPWALK1 || player.frame==UPWALK2) {
				vert = 1;
				vdir = 0;
			} else if(player.frame==JUMPDN) {
				vert = 1;
				vdir = 1;
			}
			if(vert) {
				weaponSprite = (VDPSprite) {
					.x = (player.x>>CSF) - (camera.x>>CSF) + SCREEN_HALF_W - 4 + 128,
					.y = (player.y>>CSF) - (camera.y>>CSF) + SCREEN_HALF_H - 8 + 128,
					.size = SPRITE_SIZE(1, 3),
					.attr = TILE_ATTR(PAL1,0,vdir,vdir ? !player.dir : player.dir,TILE_WEAPONINDEX+3),
				};
			} else {
				weaponSprite = (VDPSprite) {
					.x = (player.x>>CSF) - (camera.x>>CSF) + SCREEN_HALF_W - 12 + 128,
					.y = (player.y>>CSF) - (camera.y>>CSF) + SCREEN_HALF_H - 0 + 128,
					.size = SPRITE_SIZE(3, 1),
					.attr = TILE_ATTR(PAL1,0,0,player.dir,TILE_WEAPONINDEX),
				};
			}
			vdp_sprite_add(&weaponSprite);
		}
		if(player.underwater && (playerEquipment & EQUIP_AIRTANK)) {
			sprite_pos(airTankSprite, 
					(player.x>>CSF) - (camera.x>>CSF) + SCREEN_HALF_W - 12,
					(player.y>>CSF) - (camera.y>>CSF) + SCREEN_HALF_H - 12);
			vdp_sprite_add(&airTankSprite);
		}
	}
}

void player_unpause() {
	// Sometimes player is left stuck after pausing
	controlsLocked = FALSE;
	// Simulates a bug where you can get damaged, and pushed upwards, multiple times in succession (Chaco house skip)
	if(cfg_iframebug) playerIFrames = 0;
    // Redraw the boss bar in JA
    if(cfg_language >= LANG_JA && cfg_language <= LANG_KO && showingBossHealth) {
        tsc_show_boss_health();
        tsc_update_boss_health();
    }
}

uint8_t player_invincible() {
	return playerIFrames > 0 || tscState;
}

uint8_t player_inflict_damage(uint16_t damage) {
	// Show damage numbers
	effect_create_damage(-damage, &player, 0, 0);
	// Take health
	if((!iSuckAtThisGameSHIT || damage > 99) && player.health <= damage) {
		// If health reached 0 we are dead
		player.health = 0;
		// Clear smoke & fill up with smoke around player
		effects_clear_smoke();
		for(uint8_t i = MAX_SMOKE; i--; ) {
			effect_create_smoke(sub_to_pixel(player.x) + (rand() % 90 ) - 45,
                                sub_to_pixel(player.y) + (rand() % 90 ) - 45);
		}
		sound_play(SND_PLAYER_DIE, 15);
		tsc_call_event(PLAYER_DEFEATED_EVENT);
		return TRUE;
	}
	if(!iSuckAtThisGameSHIT) player.health -= damage;
	sound_play(SND_PLAYER_HURT, 5);
	playerIFrames = TIME_8(100);
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
                disable_ints();
                z80_pause_fast();
				sheets_refresh_weapon(w);
                z80_resume();
                enable_ints();
				entity_create(player.x, player.y, 
						cfg_language == LANG_JA ? OBJ_LEVELDOWN_JA : OBJ_LEVELDOWN, 0);
			} else {
				w->energy = 0;
			}
		} else {
			w->energy -= damage;
		}
	}
	// Don't knock back in <UNI:0001 mode
	if(!playerMoveMode) player.y_speed = -SPEED_10(0x3FF);
	player.grounded = FALSE;
	return FALSE;
}

static void player_prev_weapon() {
	for(int16_t i = currentWeapon - 1; i % MAX_WEAPONS != currentWeapon; i--) {
		if(i < 0) i = MAX_WEAPONS - 1;
		if(playerWeapon[i].type > 0) {
			currentWeapon = i;
			if(weapon_info[playerWeapon[i].type].sprite) {
			    TILES_QUEUE(SPR_TILES(weapon_info[playerWeapon[i].type].sprite,0,0),
					TILE_WEAPONINDEX,6);
			}
            if(playerWeapon[i].type == WEAPON_SPUR) {
                playerWeapon[i].level = 1;
                playerWeapon[i].energy = 0;
            }
			sound_play(SND_SWITCH_WEAPON, 5);
			break;
		}
	}
}

static void player_next_weapon() {
	for(int16_t i = currentWeapon + 1; i % MAX_WEAPONS != currentWeapon; i++) {
		if(i >= MAX_WEAPONS) i = 0;
		if(playerWeapon[i].type > 0) {
			currentWeapon = i;
			if(weapon_info[playerWeapon[i].type].sprite) {
			    TILES_QUEUE(SPR_TILES(weapon_info[playerWeapon[i].type].sprite,0,0),
					TILE_WEAPONINDEX,6);
			}
            if(playerWeapon[i].type == WEAPON_SPUR) {
                playerWeapon[i].level = 1;
                playerWeapon[i].energy = 0;
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
            disable_ints();
            z80_pause_fast();
			sheets_load_weapon(w);
            z80_resume();
            enable_ints();
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
		w->type = 0;
		w->level = 0;
		w->energy = 0;
		w->maxammo = 0;
		w->ammo = 0;
		if(playerWeapon[currentWeapon].type == id) {
			player_next_weapon();
		}
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
        disable_ints();
        z80_pause_fast();
		sheets_load_weapon(w);
        z80_resume();
        enable_ints();
	}
}

void player_refill_ammo() {
	for(uint8_t i = 0; i < MAX_WEAPONS; i++) {
		playerWeapon[i].ammo = playerWeapon[i].maxammo;
	}
}

void player_delevel_weapons() {
    disable_ints();
    z80_pause_fast();
	for(uint8_t i = 0; i < MAX_WEAPONS; i++) {
		playerWeapon[i].level = 1;
		playerWeapon[i].energy = 0;
		sheets_refresh_weapon(&playerWeapon[i]);
	}
    z80_resume();
    enable_ints();
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
