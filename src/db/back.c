#include "common.h"

#include "resources.h"
#include "md/vdp.h"

#include "tables.h"

const background_info_def background_info[BACKGROUND_COUNT] = {
	{ NULL, 	PAL2, 2, 0, 0, NULL },
	{ NULL,     PAL0, 0, 8, 8, NULL },
	{ &BG_Blue, PAL2, 0, 8, 8, NULL }, // Mimiga Village, Grasstown, Waterway, Plantation
	{ &BG_Stream,PAL2,3, 8, 8, NULL }, // Main Artery
	{ NULL, 	PAL2, 5, 0, 0, NULL }, // Balcony
	{ &BG_Gard, PAL2, 0, 6, 8, NULL }, // Sand Zone Storehouse
	{ &BG_Gray, PAL3, 0, 8, 8, NULL }, // Boulder Chamber
	{ NULL,		PAL2, 1, 0, 0, NULL }, // Outer Wall
	{ &BG_Maze, PAL3, 0, 8, 8, NULL }, // Labyrinth W - PAL_X
	{ &BG_Maze2,PAL3, 0, 8, 8, NULL }, // Labyrinth I, O - PAL_XB
	{ &BG_Red, 	PAL3, 0, 4, 4, NULL }, // Labyrinth M
	{ NULL,		PAL2, 4, 0, 0, NULL }, // Almond, Dark Place
	{ &BG_Hell, PAL2, 0, 4, 4, NULL }, // Hell B1 / B2 / B3
	{ &BG_Eggs, PAL2, 0, 8, 8, NULL }, // Egg Corridor
	{ &BG_LabB, PAL3, 0, 8, 8, NULL }, // Arthur's House, Labyrinth B, Itoh, Seal/Statue Chamber
	{ &BG_Fall, PAL2, 0, 8, 8, NULL }, // Fall
	{ &BG_Sand, PAL2, 0, 8, 8, NULL }, // Sand Zone
};
