#ifndef INC_HUD_H_
#define INC_HUD_H_

#include "common.h"

bool hudRedrawPending;

// Loads sprites for HUD and displays it on screen
void hud_show();
// Unloads HUD sprites
void hud_hide();
// Updates numbers + other info if they changed
void hud_update();
// This injects tiles into the health and energy bars
// Called during vblank so DMA can be used
void hud_update_vblank();

#endif /* INC_HUD_H_ */
