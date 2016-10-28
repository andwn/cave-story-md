#include "tables.h"
#include "resources.h"

const background_info_def background_info[BACKGROUND_COUNT] = {
	{ NULL, 	PAL2, 2, 0, 0 },
	{ &BG_Arthur,PAL0,0, 8, 8 }, // Arthur's House
	{ &BG_Blue, PAL2, 0, 8, 8 }, // Mimiga Village, Grasstown, Labyrinth B
	{ &BG_Arthur,PAL0,3, 8, 8 }, // Main Artery
	{ NULL, 	PAL2, 5, 0, 0 }, // Balcony
	{ &BG_Gard, PAL2, 0, 6, 8 }, // Sand Zone Storehouse
	{ &BG_Gray, PAL2, 0, 8, 8 }, // Boulder Chamber
	{ NULL,		PAL2, 1, 0, 0 }, // Outer Wall
	{ &BG_Maze, PAL3, 0, 8, 8 }, // Labyrinth W - PAL_X
	{ &BG_Maze2,PAL3, 0, 8, 8 }, // Labyrinth I, O - PAL_XB
	{ &BG_Red, 	PAL3, 0, 4, 4 }, // Labyrinth M
	{ NULL,		PAL2, 4, 0, 0 }, // Almond, Dark Place
	{ &BG_Green,PAL3, 0, 8, 8 }, // Sand Zone, Waterway, Plantation
	{ &BG_Eggs, PAL2, 0, 8, 8 }, // Egg Corridor
};
