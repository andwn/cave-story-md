#include "common.h"
#include "bank_data.h"
#include "resources.h"
#include "tables.h"

const tileset_info_def tileset_info[TILESET_COUNT] = {
	{ 0, NULL, NULL, NULL },
	{ 0, UFTC_Almond, &PAL_Almond, PXA_Almond },
	{ 0, UFTC_Barr, &PAL_Barr, PXA_Barr },
	{ 0, UFTC_Cave, &PAL_Cave, PXA_Cave },
	{ 0, UFTC_Cent, &PAL_Cent, PXA_Cent },
	{ 0, UFTC_EggIn, &PAL_EggIn, PXA_EggIn },
	{ 0, UFTC_Eggs, &PAL_Eggs, PXA_Eggs },
	{ 0, UFTC_EggX, &PAL_EggX, PXA_EggX },
	{ 0, UFTC_Fall, &PAL_Fall, PXA_Fall },
	{ 0, UFTC_Gard, &PAL_Gard, PXA_Gard },
	{ 0, UFTC_Hell, &PAL_Hell, PXA_Hell },
	{ 0, UFTC_Jail, &PAL_Jail, PXA_Jail },
	{ 0, UFTC_Labo, &PAL_Labo, PXA_Labo },
	{ 0, UFTC_Maze, &PAL_Maze, PXA_Maze },
	{ 0, UFTC_Mimi, &PAL_Mimi, PXA_Mimi },
	{ 0, UFTC_Oside, &PAL_Oside, PXA_Oside },
	/* 0x10 (16) */
	{ 0, UFTC_Pens, &PAL_Pens, PXA_Pens },
	{ 0, UFTC_River, &PAL_River, PXA_River },
	{ 0, UFTC_Sand, &PAL_Sand, PXA_Sand },
	{ 0, UFTC_Store, &PAL_Store, PXA_Store },
	{ 0, UFTC_Weed, &PAL_Weed, PXA_Weed },
	{ 0, UFTC_Blcny, &PAL_Blcny, PXA_Blcny },
	// Tilesets which have been split off their main ones to reduce size
	{ 0, UFTC_EggX2, &PAL_EggX, PXA_EggX },
	{ 0, UFTC_Eggs2, &PAL_Eggs, PXA_Eggs2 },
	{ 0, UFTC_MazeM, &PAL_Maze, PXA_MazeM },
	{ 0, UFTC_Kings, &PAL_Blcny, PXA_Kings },
	{ 0, UFTC_Statue,&PAL_Hell, PXA_Statue },
	{ 0, UFTC_Ring2, &PAL_Blcny, PXA_Ring2 },
	{ 0, UFTC_Ring3, &PAL_Ring3, PXA_Ring3 },
};
