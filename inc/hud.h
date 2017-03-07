// Loads the initial HUD sprite, only stage_load() should call this
void hud_create();
// Makes the HUD visible
void hud_show();
// Hides the HUD (doesn't unload)
void hud_hide();
// Updates numbers + other info if they changed
void hud_update();
