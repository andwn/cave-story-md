#include "tables.h"
#include "resources.h"

const tileset_info_def tileset_info[TILESET_COUNT] = {
	{ NULL, NULL, NULL },
	{ &TS_Almond, &PAL_Almond, PXA_Almond },
	{ &TS_Barr, &PAL_Barr, PXA_Barr },
	{ &TS_Cave, &PAL_Cave, PXA_Cave },
	{ &TS_Cent, &PAL_Cent, PXA_Cent },
	{ &TS_EggIn, &PAL_EggIn, PXA_EggIn },
	{ &TS_Eggs, &PAL_Eggs, PXA_Eggs },
	{ &TS_EggX, &PAL_EggX, PXA_EggX },
	{ &TS_Fall, &PAL_Fall, PXA_Fall },
	{ &TS_Gard, &PAL_Gard, PXA_Gard },
	{ &TS_Hell, &PAL_Hell, PXA_Hell },
	{ &TS_Jail, &PAL_Jail, PXA_Jail },
	{ &TS_Labo, &PAL_Labo, PXA_Labo },
	{ &TS_Maze, &PAL_Maze, PXA_Maze },
	{ &TS_Mimi, &PAL_Mimi, PXA_Mimi },
	{ &TS_Oside, &PAL_Oside, PXA_Oside },
	/* 0x10 (16) */
	{ &TS_Pens, &PAL_Pens, PXA_Pens },
	{ &TS_River, &PAL_River, PXA_River },
	{ &TS_Sand, &PAL_Sand, PXA_Sand },
	{ &TS_Store, &PAL_Store, PXA_Store },
	{ &TS_Weed, &PAL_Weed, PXA_Weed },
	{ &TS_Blcny, &PAL_Blcny, PXA_Blcny },
	// Tilesets which have been split off their main ones to reduce size
	{ &TS_EggX2, &PAL_EggX, PXA_EggX },
	{ &TS_Eggs2, &PAL_Eggs, PXA_Eggs2 },
	{ &TS_MazeM, &PAL_Maze, PXA_MazeM },
	{ &TS_Kings, &PAL_Blcny, PXA_Kings },
	{ &TS_Statue,&PAL_Hell, PXA_Statue },
	{ &TS_Ring2, &PAL_Blcny, PXA_Ring2 },
	{ &TS_Ring3, &PAL_Ring3, PXA_Ring3 },
};
