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
	WEAPON_SPUR_TAIL, 
	WEAPON_NEMESIS, 
	WEAPON_SPUR
};

// Represents a weapon currently owned by the player
struct Weapon {
	uint16_t energy;
	uint16_t next;
	uint16_t maxammo;
	uint16_t ammo;
	uint8_t type;
	uint8_t level;
	uint8_t sheet;
    // 15 + 1 = 16
    uint8_t _padding;
};

// An active bullet created by the player (or curly)
struct Bullet {
	Sprite sprite;
	bounding_box hit_box;
	extent_box extent;
	//Entity *last_hit[1];

    uint8_t sheet;
    uint8_t dir;
    uint8_t hits;
    uint8_t state;

	int32_t x, y;
	int16_t x_speed, y_speed;
	uint8_t type;
	uint8_t level;
	uint8_t damage;
	uint8_t ttl;

    BulletFunc onFrame;
    Entity *last_hit[4];
    // 60 + 4 = 64
    uint8_t _padding[4];
};

extern Weapon playerWeapon[MAX_WEAPONS];
extern Bullet playerBullet[MAX_BULLETS];

#define MAX_WHIMSICAL_STAR	3
typedef struct WStarBullet {
	Sprite sprite;
	int32_t x;
	int32_t y;
	int16_t x_px;
	int16_t y_px;
	int16_t x_speed;
	int16_t y_speed;
} WStarBullet;

extern uint8_t wstarCollideIndex;
extern WStarBullet wstarBullet[MAX_WHIMSICAL_STAR];

void wstar_reset(void);
void wstar_update(void);

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

extern const WeaponFunc weapon_fire_array[WEAPON_COUNT];

static inline void bullet_deactivate(Bullet *b) {
    b->extent.x1 = 0xFFFF;
    b->ttl = 0;
}

#define bullet_update(b); bullet_update_array[(b).type](&(b))

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
void bullet_update_spur_tail(Bullet *b);
void bullet_update_wstar(Bullet *b);

extern const BulletFunc bullet_update_array[WEAPON_COUNT];

void bullet_missile_explode(Bullet *b);
// Used by Curly boss to know when to enable her shield
uint8_t bullet_missile_is_exploding(void);
