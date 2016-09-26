#ifndef INC_EFFECT_H_
#define INC_EFFECT_H_

#include "common.h"

/*
 * Number & Smoke effects, will be more than that eventually
 */

#define MAX_DAMAGE 4
#define MAX_SMOKE 8
#define MAX_MISC 4

// Only send tiles for 1 damage string per frame
u8 dqueued;

// Initialize default (0, NULL) values to avoid weird glitches
void effects_init();
// Clear all effects
void effects_clear();
// Just clear the smoke
void effects_clear_smoke();
// Per frame update of effects
void effects_update();

// Creates a damage string (maximum of 4)
// Positive values will be white, negative will be red, zero is ignored
// Values must be limited to 3 digits
void effect_create_damage(s16 num, s16 x, s16 y);
// Creates a single puff of smoke
void effect_create_smoke(s16 x, s16 y);

void effect_create_misc(u8 type, s16 x, s16 y);

#endif /* INC_EFFECT_H_ */
