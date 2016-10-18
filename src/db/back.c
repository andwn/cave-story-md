#include "tables.h"
#include "resources.h"

const background_info_def background_info[BACKGROUND_COUNT] = {
	{ NULL, 	PAL2, 2, 0, 0 },
	{ &BG_Arthur,PAL0,0, 8, 8 }, // Arthur's House
	{ &BG_Blue, PAL2, 0, 8, 8 }, // Mimiga Village, Grasstown
	{ &BG_Grass,PAL0, 3, 8, 8 }, // Main Artery
	{ &BG_Fog, 	PAL2, 1, 16,4 }, // Balcony
	{ &BG_Gard, PAL2, 0, 6, 8 }, // Sand Zone Building
	{ &BG_Gray, PAL2, 0, 8, 8 }, // Waterway
	{ &BG_Green,PAL2, 1, 16,4 }, // Outer Wall
	{ &BG_Maze, PAL3, 0, 8, 8 }, // Labyrinth W - PAL_X
	{ &BG_Maze2,PAL3, 0, 8, 8 }, // Labyrinth I, O, B - PAL_XB
	{ &BG_Red, 	PAL3, 0, 4, 4 }, // Labyrinth M
	{ &BG_Gray, PAL3, 0, 8, 8 }, // Plantation
	{ &BG_Green,PAL3, 0, 8, 8 }, // Sand Zone
	{ &BG_Eggs, PAL2, 0, 8, 8 }, // Egg Corridor
};
