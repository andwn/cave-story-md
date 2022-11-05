#include "common.h"
#include "res/stage.h"
#include "resources.h"
#include "tables.h"

const tileset_info_def tileset_info[TILESET_COUNT] = {
	{ 0,   NULL, NULL, NULL },
	{ 94,  UFTC_Almond, &PAL_Almond, PXA_Almond },
	{ 80,  UFTC_Barr, &PAL_Barr, PXA_Barr },
	{ 80,  UFTC_Cave, &PAL_Cave, PXA_Cave },
	{ 128, UFTC_Cent, &PAL_Cent, PXA_Cent },
	{ 80,  UFTC_EggIn, &PAL_EggIn, PXA_EggIn },
	{ 74,  UFTC_Eggs, &PAL_Eggs, PXA_Eggs },
	{ 128, UFTC_EggX, &PAL_EggX, PXA_EggX },
	{ 16,  UFTC_Fall, &PAL_Fall, PXA_Fall },
	{ 88,  UFTC_Gard, &PAL_Gard, PXA_Gard },
	{ 106, UFTC_Hell, &PAL_Hell, PXA_Hell },
	{ 122, UFTC_Jail, &PAL_Jail, PXA_Jail },
	{ 40,  UFTC_Labo, &PAL_Labo, PXA_Labo },
	{ 92,  UFTC_Maze, &PAL_Maze, PXA_Maze },
	{ 120, UFTC_Mimi, &PAL_Mimi, PXA_Mimi },
	{ 64,  UFTC_Oside, &PAL_Oside, PXA_Oside },
	/* 0x10 (16) */
	{ 64,  UFTC_Pens, &PAL_Pens, PXA_Pens },
	{ 90,  UFTC_River, &PAL_River, PXA_River },
	{ 112, UFTC_Sand, &PAL_Sand, PXA_Sand },
	{ 104, UFTC_Store, &PAL_Store, PXA_Store },
	{ 122, UFTC_Weed, &PAL_Weed, PXA_Weed },
	{ 64,  UFTC_Blcny, &PAL_Blcny, PXA_Blcny },
	// Tilesets which have been split off their main ones to reduce size
	{ 128, UFTC_EggX2, &PAL_EggX, PXA_EggX },
	{ 103, UFTC_Eggs2, &PAL_Eggs, PXA_Eggs2 },
	{ 110, UFTC_MazeM, &PAL_Maze, PXA_MazeM },
	{ 52,  UFTC_Kings, &PAL_Blcny, PXA_Kings },
	{ 28,  UFTC_Statue,&PAL_Hell, PXA_Statue },
	{ 42,  UFTC_Ring2, &PAL_Blcny, PXA_Ring2 },
	{ 35,  UFTC_Ring3, &PAL_Ring3, PXA_Ring3 },
};
