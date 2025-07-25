// Loads the initial HUD sprite, only stage_load() should call this
void hud_create(void);

void hud_force_redraw(void);
void hud_force_energy(void); // Force redraw but only for weapon energy
// Makes the HUD visible
void hud_show(void);
// Hides the HUD (doesn't unload)
void hud_hide(void);
// Updates numbers + other info if they changed
void hud_update(void);
