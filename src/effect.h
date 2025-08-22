#ifndef EFFECT_H
#define EFFECT_H

/*
 * Number & Smoke effects, will be more than that eventually
 */

#define MAX_DAMAGE 4
#define MAX_MISC 16

enum { 
	EFF_SMOKE, EFF_BONKL, EFF_BONKR, EFF_ZZZ, EFF_BOOST8, EFF_BOOST2, EFF_QMARK, 
	EFF_FANL, EFF_FANU, EFF_FANR, EFF_FAND, EFF_SPLASH,
	EFF_PSTAR_HIT, EFF_MGUN_HIT, EFF_BUBB_POP, EFF_FIRETRAIL, EFF_SNAKETRAIL,
	EFF_DISSIPATE, EFF_GIB,
};

typedef struct {
	uint8_t ttl, type;
	Sprite sprite;
	uint8_t timer, timer2;
	int16_t x, y;
	int8_t x_speed, y_speed;
} Effect;

// Only send tiles for 1 damage string per frame
extern uint8_t dqueued;

// Initialize default (0, NULL) values to avoid weird glitches
void effects_init(void);
// Clear all effects
void effects_clear(void);
// Reload static effect tiles
void effects_reload_tiles(void);
// Per frame update of effects
void effects_update(void);

// Creates a damage string (maximum of 4)
// Positive values will be white, negative will be red, zero is ignored
// Values must be limited to 3 digits
void effect_create_damage(int16_t num, Entity *follow, int16_t xoff, int16_t yoff);

Effect* effect_create(uint8_t type, int16_t x, int16_t y, uint8_t only_one);

// Fading

extern int8_t wipeFadeTimer;

// Fade the screen out using diamond pattern (blocking)
void do_fadeout_wipe(uint8_t dir);

// Begin fading the screen in using diamond pattern (non-blocking)
void start_fadein_wipe(uint8_t dir);

// Frame step for fade in effect
void update_fadein_wipe(void);

#endif
