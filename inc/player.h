#ifndef INC_PLAYER_H_
#define INC_PLAYER_H_

#include <genesis.h>
#include "common.h"
#include "entity.h"
#include "weapon.h"

// Equips are bit flags in the original Cave Story, even in the TSC commands
#define EQUIP_BOOSTER08		0x001
#define EQUIP_MAPSYSTEM		0x002
#define EQUIP_ARMSBARRIER	0x004
#define EQUIP_TURBOCHARGE	0x008
#define EQUIP_AIRTANK		0x010
#define EQUIP_BOOSTER20		0x020
#define EQUIP_MAPIGNON		0x040
#define EQUIP_WHIMSICAL		0x080
#define EQUIP_CLOCK			0x100

#define MAX_ITEMS 32

Weapon playerWeapon[MAX_WEAPONS];
u8 currentWeapon; // Index 0-7 of which slot in the array the currently used weapon is

Bullet playerBullet[MAX_BULLETS];

// The player is an entity, as to better interact with entities & physics
// Not all variables in Entity are used but most are
Entity player;
u8 playerIFrames;
u8 playerMoveMode;
// Max health - current health is player.health (Entity)
u16 playerMaxHealth;
// When this is true, quote can not be controlled by player input
bool controlsLocked;
// What is currently equipped (see the flags at the top of this file)
u16 playerEquipment;
// What items the player has and will show up in the inventory screen
u8 playerInventory[MAX_ITEMS];

Entity *playerPlatform;

// Initialize everything for the player, sets default values on game start
void player_init();
// Reloads the player sprites after calling SPR_reset() in stage_load()
void player_reset_sprites();

// Per frame update for the player and related objects
void player_update();
void player_update_bullets();
void player_start_booster();
// Sprite animation
void player_draw();

// This is called by entities to check if it got hit by any bullets
// It will return the first bullet that is colliding with the given entity, if any
Bullet *bullet_colliding(Entity *e);

Weapon *player_find_weapon(u8 id);

// Returns true if the player is blinking or otherwise should not be damaged (cutscene)
bool player_invincible();
// Inflict damage on the player, will start the animation, knockback, red numbers,
// sound, iframes, and check for death
bool player_inflict_damage(s16 damage);

// Makes the player sprite visible/invisible
void player_show();
void player_hide();
void player_pause();
void player_unpause();

// Shows the name of the map for a specified amount of time (frames)
// This would make more sense in stage, but stage_update() is in vblank
void player_show_map_name(u8 ttl);

// TSC Stuff
void player_give_weapon(u8 id, u8 ammo); // <AM+
void player_take_weapon(u8 id); // <AM-
bool player_has_weapon(u8 id); // <AMJ
void player_trade_weapon(u8 id_take, u8 id_give, u8 ammo); // <TAM
void player_refill_ammo(); // <AE+
void player_take_allweapons(); // <ZAM

void player_heal(u8 health); // <LI+
void player_maxhealth_increase(u8 health); // <ML+

void player_give_item(u8 id); // <IT+
void player_take_item(u8 id); // <IT-
bool player_has_item(u8 id); // <ITJ

void player_equip(u16 id); // <EQ+
void player_unequip(u16 id); // <EQ-

void player_lock_controls(); // <PRI and <KEY
void player_unlock_controls(); // <KEY and <FRE

#endif /* INC_PLAYER_H_ */
