#ifndef INC_PLAYER_H_
#define INC_PLAYER_H_

#include "common.h"
#include "entity.h"

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
#define MAX_WEAPONS 8

typedef struct {
	u8 sprite;
	u8 type;
	u8 level;
	u16 energy;
	u16 maxammo;
	u16 ammo;
} Weapon;

Weapon playerWeapon[MAX_WEAPONS];
u8 currentWeapon;

typedef struct {
	u8 sprite;
	s32 x, y;
	s16 x_speed, y_speed;
	u8 damage;
	u8 ttl;
} Bullet;

Bullet playerBullet[3];

u16 playerMaxAir, playerAir;
u16 playerMaxHealth;
u8 playerFacing;
bool controlsLocked;
u16 playerEquipment;
u8 playerInventory[MAX_ITEMS];

//u8 hudSprite;

Entity player;

// Initialize everything for the player, sets default values
void player_init();
// Reloads sprites for player, weapon, and HUD, called by stage_load
void player_reset_sprites();
// Per frame update for the player and related objects
void player_update();
// Sprite animation
void player_draw();

// TSC Stuff
void player_give_weapon(u8 id, u8 ammo); // AM+ Command
void player_take_weapon(u8 id); // AM- Command
bool player_has_weapon(u8 id); // AMJ Command
void player_trade_weapon(u8 id_take, u8 id_give, u8 ammo); // TAM Command
void player_refill_ammo(); // AE+ Command
void player_take_allweapons(); // ZAM Command

void player_heal(u8 health); // LI+ Command
void player_maxhealth_increase(u8 health); // ML+ Command

void player_give_item(u8 id); // IT+ Command
void player_take_item(u8 id); // IT- Command
bool player_has_item(u8 id); // ITJ Command

void player_lock_controls(); // PRI and KEY Commands
void player_unlock_controls(); // KEY and FRE Commands

#endif /* INC_PLAYER_H_ */
