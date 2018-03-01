#include "common.h"

#include "resources.h"
#include "vdp.h"

#include "tables.h"

const background_info_def background_info[BACKGROUND_COUNT] = {
	{ NULL, 	PAL2, 2, 0, 0 },
	{ &BG_Arthur,PAL0,0, 8, 8 }, // Arthur's House
	{ &BG_Blue, PAL2, 0, 8, 8 }, // Mimiga Village, Grasstown, Waterway, Plantation
	{ &BG_Stream,PAL2,3, 8, 8 }, // Main Artery
	{ NULL, 	PAL2, 5, 0, 0 }, // Balcony
	{ &BG_Gard, PAL2, 0, 6, 8 }, // Sand Zone Storehouse
	{ &BG_Gray, PAL3, 0, 8, 8 }, // Boulder Chamber
	{ NULL,		PAL2, 1, 0, 0 }, // Outer Wall
	{ &BG_Maze, PAL3, 0, 8, 8 }, // Labyrinth W - PAL_X
	{ &BG_Maze2,PAL3, 0, 8, 8 }, // Labyrinth I, O - PAL_XB
	{ &BG_Red, 	PAL3, 0, 4, 4 }, // Labyrinth M
	{ NULL,		PAL2, 4, 0, 0 }, // Almond, Dark Place
	{ &BG_Hell, PAL2, 0, 4, 4 }, // Hell B1 / B2 / B3
	{ &BG_Eggs, PAL2, 0, 8, 8 }, // Egg Corridor
	{ &BG_LabB, PAL3, 0, 8, 8 }, // Labyrinth B, Itoh, Seal/Statue Chamber
	{ &BG_Fall, PAL2, 0, 8, 8 }, // Fall
	{ &BG_Sand, PAL2, 0, 8, 8 }, // Sand Zone
};
