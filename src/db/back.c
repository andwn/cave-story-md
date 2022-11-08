#include "md/types.h"

#include "res/tiles.h"
#include "md/vdp.h"

#include "tables.h"

const background_info_def background_info[BACKGROUND_COUNT] = {
	{ NULL, 	PAL2, 2, 0, 0, NULL },
	{ NULL,     PAL0, 0, 8, 8, NULL },
	{ UFTC_bkBlue, PAL2, 0, 8, 8, NULL }, // Mimiga Village, Grasstown, Waterway, Plantation
	{ UFTC_bkStream,PAL2,3, 8, 8, NULL }, // Main Artery
	{ NULL, 	PAL2, 5, 0, 0, NULL }, // Balcony
	{ UFTC_bkGard, PAL2, 0, 6, 8, NULL }, // Sand Zone Storehouse
	{ UFTC_bkGray, PAL3, 0, 8, 8, NULL }, // Boulder Chamber
	{ NULL,		PAL2, 1, 0, 0, NULL }, // Outer Wall
	{ UFTC_bkMaze, PAL3, 0, 8, 8, NULL }, // Labyrinth W - PAL_X
	{ UFTC_bkMaze2,PAL3, 0, 8, 8, NULL }, // Labyrinth I, O - PAL_XB
	{ UFTC_bkRed, 	PAL3, 0, 4, 4, NULL }, // Labyrinth M
	{ NULL,		PAL2, 4, 0, 0, NULL }, // Almond, Dark Place
	{ UFTC_bkHell, PAL2, 0, 4, 4, NULL }, // Hell B1 / B2 / B3
	{ UFTC_bkEggs, PAL2, 0, 8, 8, NULL }, // Egg Corridor
	{ UFTC_bkLabB, PAL3, 0, 8, 8, NULL }, // Arthur's House, Labyrinth B, Itoh, Seal/Statue Chamber
	{ UFTC_bkFall, PAL2, 0, 8, 8, NULL }, // Fall
	{ UFTC_bkSand, PAL2, 0, 8, 8, NULL }, // Sand Zone
};
