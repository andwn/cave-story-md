#ifndef INC_WEAPON_H_
#define INC_WEAPON_H_

#include <genesis.h>
#include "common.h"
#include "entity.h"

// Number of different weapon types
#define WEAPON_COUNT 14
// Maximum number of weapons the player can carry in their inventory
#define MAX_WEAPONS 5
// Length of player's bullet array, a global maximum
// Different weapons have different limits
#define MAX_BULLETS 10

// Weapon 06, 08, and 11 do not exist
// As weapon IDs are hard coded into game scripts, this order cannot be changed
enum {
	WEAPON_NONE, 
	WEAPON_SNAKE, 
	WEAPON_POLARSTAR, 
	WEAPON_FIREBALL, 
	WEAPON_MACHINEGUN, 
	WEAPON_MISSILE, 
	WEAPON_06, 
	WEAPON_BUBBLER, 
	WEAPON_BLADE_SLASH, 
	WEAPON_BLADE, 
	WEAPON_SUPERMISSILE, 
	WEAPON_11, 
	WEAPON_NEMESIS, 
	WEAPON_SPUR
};

// Represents a weapon currently owned by the player
typedef struct {
	u16 energy;
	u16 next;
	u16 maxammo;
	u16 ammo;
	u8 type;
	u8 level;
	u8 sheet;
} Weapon;

// An active bullet created by the player (or curly)
typedef struct {
	VDPSprite sprite;
	bounding_box hit_box;
	s32 x, y;
	s16 x_speed, y_speed;
	u8 type;
	u8 level;
	u8 damage;
	u8 ttl;
	u8 sheet;
	u8 dir;
	u8 hits;
} Bullet;

#define weapon_fire(w) weapon_fire_array[(w).type](&(w))

void weapon_fire_none(Weapon *w);
void weapon_fire_snake(Weapon *w);
void weapon_fire_polarstar(Weapon *w);
void weapon_fire_fireball(Weapon *w);
void weapon_fire_machinegun(Weapon *w);
void weapon_fire_missile(Weapon *w);
void weapon_fire_bubbler(Weapon *w);
void weapon_fire_blade(Weapon *w);
void weapon_fire_supermissile(Weapon *w);
void weapon_fire_nemesis(Weapon *w);
void weapon_fire_spur(Weapon *w);

typedef void (*WeaponFunc)(Weapon *w);
extern const WeaponFunc weapon_fire_array[WEAPON_COUNT];

#define bullet_update(b); ({ if((b).ttl > 0) bullet_update_array[(b).type](&(b)); })

void bullet_update_none(Bullet *b);
void bullet_update_snake(Bullet *b);
void bullet_update_polarstar(Bullet *b);
void bullet_update_fireball(Bullet *b);
void bullet_update_machinegun(Bullet *b);
void bullet_update_missile(Bullet *b);
void bullet_update_bubbler(Bullet *b);
void bullet_update_blade(Bullet *b);
void bullet_update_blade_slash(Bullet *b);
void bullet_update_supermissile(Bullet *b);
void bullet_update_nemesis(Bullet *b);
void bullet_update_spur(Bullet *b);

typedef void (*BulletFunc)(Bullet *b);
extern const BulletFunc bullet_update_array[WEAPON_COUNT];

Bullet* bullet_colliding(Entity *e);
void bullet_missile_explode(Bullet *b);
// Used by Curly boss to know when to enable her shield
u8 bullet_missile_is_exploding();

#endif
