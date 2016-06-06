#ifndef INC_EFFECT_H_
#define INC_EFFECT_H_

#include "common.h"

/*
 * This manages the damage number and smoke effects.
 * Damage numbers are both the red "-5" and white "+5" numbers that pop up to show damage 
 * or weapon exp gain.
 * Small puffs of smoke appear for various reasons. There tend to be a lot of them, so the
 * sprite animation tiles are fully loaded into a dedicated section of VRAM to avoid
 * the sprite engine from thrashing.
 */

// Initialize default (0, NULL) values to avoid weird glitches
void effects_init();
// Clear all effects
void effects_clear();
// Per frame update of effects
void effects_update();

// Creates a damage string (maximum of 4)
// Positive values will be white, negative will be red, zero is ignored
// Values must be limited to 3 digits
// TTL means time to live, and is the number of frames until the effect expires
void effect_create_damage(s16 num, s16 x, s16 y, u8 ttl);
// Creates a single puff of smoke (maximum of 6)
void effect_create_smoke(u8 type, s16 x, s16 y);

#endif /* INC_EFFECT_H_ */
