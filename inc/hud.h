#ifndef INC_HUD_H_
#define INC_HUD_H_

#include "common.h"

// When this gets flipped on the HUD will be redrawn next vblank
u8 hudRedrawPending;

// Loads the initial HUD sprite, only stage_load() should call this
void hud_create();
// Makes the HUD visible
void hud_show();
// Hides the HUD (doesn't unload)
void hud_hide();
// Updates numbers + other info if they changed
void hud_update();
// Reloads HUD tiles to display current values if necessary
void hud_update_vblank();

#endif /* INC_HUD_H_ */
