#include "common.h"

/*
 * Module to organize sprite sheets for manual tile allocation
 * 
 * SGDK will often duplicate tiles uploads for sprites especially after they are deleted
 * and there are a lot of the same sprite existing at the same time. This is the cause for
 * every laggy area in the game so far. I work around this by manually picking out specific
 * sprites and adding them here. The objects which use this are responsible for knowing
 * their sprite sheet's frame width, height, number and animation
 */
 
enum { 
	SHEET_NONE, SHEET_PSTAR, SHEET_MGUN, SHEET_FBALL, SHEET_HEART, SHEET_MISSILE, SHEET_ENERGY,
	SHEET_BAT, SHEET_CRITTER, SHEET_PIGNON, SHEET_JELLY, SHEET_FROG, SHEET_PUCHI, SHEET_CROW, 
	SHEET_GAUDI, SHEET_FUZZ, 
};

// Frame count/height/time not included, the object will assume their own values
typedef struct {
	u8 id; // One of the values in the enum above
	u8 size; // Number of tiles
	u16 index; // VDP tile index
} Sheet;

Sheet sheets[10];

void sheets_init();

void sheets_refresh_polarstar(u8 level);
void sheets_refresh_machinegun(u8 level);
void sheets_refresh_fireball(u8 level);

void sheets_load_stage(u16 sid);
