// Equips are bit flags in the original Cave Story, even in the TSC commands
#define EQUIP_BOOSTER08		0x001
#define EQUIP_MAPSYSTEM		0x002
#define EQUIP_ARMSBARRIER	0x004
#define EQUIP_TURBOCHARGE	0x008
#define EQUIP_AIRTANK		0x010
#define EQUIP_BOOSTER20		0x020
#define EQUIP_MIMIMASK		0x040
#define EQUIP_WHIMSICAL		0x080
#define EQUIP_CLOCK			0x100

#define MAX_ITEMS 24

#define PLAYER_HARD_HIT_BOX ((bounding_box) { 6, 6, 5, 8 })
#define PLAYER_SOFT_HIT_BOX ((bounding_box) { 4, 4, 4, 4 })

enum BoosterState {
	BOOST_OFF = 0,
	BOOST_UP,
	BOOST_DOWN,
	BOOST_HOZ,
	BOOST_08
};

extern uint8_t currentWeapon; // Index 0-7 of which slot in the array the currently used weapon is
extern const uint8_t spur_time[2][4];

// The player is an entity, as to better interact with entities & physics
// Not all variables in Entity are used but most are
extern Entity player;
extern VDPSprite playerSprite;

//uint8_t playerShow;
extern uint8_t playerIFrames;
extern uint8_t playerMoveMode;
extern uint8_t lookingDown;
// Max health - current health is player.health (Entity)
extern uint16_t playerMaxHealth;
// When this is TRUE, quote can not be controlled by player input
extern uint8_t controlsLocked;
// What is currently equipped (see the flags at the top of this file)
extern uint16_t playerEquipment;
// What items the player has and will show up in the inventory screen
extern uint8_t playerInventory[MAX_ITEMS];

extern uint8_t shoot_cooldown, mgun_chargetime, playerNoBump;

extern Entity *playerPlatform;
extern uint8_t playerPlatformTime;

extern uint8_t playerBoosterFuel, playerBoostState, lastBoostState;

extern uint16_t mapNameTTL;

extern uint8_t iSuckAtThisGameSHIT;

extern uint8_t missileEmptyFlag;

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

// Returns TRUE if the player is blinking or otherwise should not be damaged (cutscene)
uint8_t player_invincible();
// Inflict damage on the player, will start the animation, knockback, red numbers,
// sound, iframes, and check for death
uint8_t player_inflict_damage(uint16_t damage);

// Makes the player sprite visible/invisible
void player_show();
void player_hide();
void player_pause();
void player_unpause();

// Shows the name of the map for a specified amount of time (frames)
// This would make more sense in stage, but stage_update() is in vblank
void player_show_map_name(uint8_t ttl);

// TSC Stuff
void player_give_weapon(uint8_t id, uint8_t ammo); // <AM+
void player_take_weapon(uint8_t id); // <AM-
uint8_t player_has_weapon(uint8_t id); // <AMJ
void player_trade_weapon(uint8_t id_take, uint8_t id_give, uint8_t ammo); // <TAM
void player_refill_ammo(); // <AE+
void player_delevel_weapons(); // <ZAM

Weapon *player_find_weapon(uint8_t id);

void player_heal(uint8_t health); // <LI+
void player_maxhealth_increase(uint8_t health); // <ML+

void player_give_item(uint8_t id); // <IT+
void player_take_item(uint8_t id); // <IT-
uint8_t player_has_item(uint8_t id); // <ITJ

void player_equip(uint16_t id); // <EQ+
void player_unequip(uint16_t id); // <EQ-
