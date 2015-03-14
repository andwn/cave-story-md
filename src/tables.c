#include "tables.h"
#include "resources.h"

// Stage Info
const stage_info_def stage_info[STAGE_COUNT] = {
	{ PXM_0,		PXE_0,		TSC_0,		&PAL_Regu,	0,	0,	"" },
	{ PXM_Pens1,	PXE_Pens1,	TSC_Pens1,	&PAL_Regu,	16,	0,	"Arthur's House" },
	{ PXM_Eggs,		PXE_Eggs,	TSC_Eggs,	&PAL_Regu,	6,	7,	"Egg Corridor" },
	{ PXM_EggX,		PXE_EggX,	TSC_EggX,	&PAL_Regu,	7,	0,	"Egg No. 00" },
	{ PXM_Egg6,		PXE_Egg6,	TSC_Egg6,	&PAL_Regu,	5,	0,	"Egg No. 06" },
	{ PXM_EggR,		PXE_EggR,	TSC_EggR,	&PAL_Regu,	19,	0,	"Egg Observation Room" },
	{ PXM_Weed,		PXE_Weed,	TSC_Weed,	&PAL_Regu,	20,	7,	"Grasstown" },
	{ PXM_Santa,	PXE_Santa,	TSC_Santa,	&PAL_Regu,	2,	0,	"Santa's House" },
	{ PXM_Chako,	PXE_Chako,	TSC_Chako,	&PAL_Regu,	2,	0,	"Chako's House" },
	{ PXM_MazeI,	PXE_MazeI,	TSC_MazeI,	&PAL_Regu,	13,	8,	"Labyrinth I" },
	{ PXM_Sand,		PXE_Sand,	TSC_Sand,	&PAL_Regu,	18,	7,	"Sand Zone" },
	{ PXM_Mimi,		PXE_Mimi,	TSC_Mimi,	&PAL_Regu,	14,	2,	"Mimiga Village" },
	{ PXM_Cave,		PXE_Cave,	TSC_Cave,	&PAL_Regu,	3,	0,	"First Cave" },
	{ PXM_Start,	PXE_Start,	TSC_Start,	&PAL_Regu,	3,	0,	"Start Point" },
	{ PXM_Barr,		PXE_Barr,	TSC_Barr,	&PAL_Regu,	14,	0,	"Shack" },
	{ PXM_Pool,		PXE_Pool,	TSC_Pool,	&PAL_Regu,	14,	2,	"Reservoir" },
	{ PXM_Cemet,	PXE_Cemet,	TSC_Cemet,	&PAL_Regu,	14,	2,	"Graveyard" },
	{ PXM_Plant,	PXE_Plant,	TSC_Plant,	&PAL_Regu,	14,	2,	"Yamashita Farm" },
	{ PXM_Shelt,	PXE_Shelt,	TSC_Shelt,	&PAL_Regu,	19,	0,	"Shelter" },
	{ PXM_Comu,		PXE_Comu,	TSC_Comu,	&PAL_Regu,	16,	0,	"Assembly Hall" },
	{ PXM_MiBox,	PXE_MiBox,	TSC_MiBox,	&PAL_Regu,	14,	0,	"Shack" },
	{ PXM_EgEnd1,	PXE_EgEnd1,	TSC_EgEnd1,	&PAL_Regu,	19,	0,	"Side Room" },
	{ PXM_Cthu,		PXE_Cthu,	TSC_Cthu,	&PAL_Regu,	19,	0,	"Cthulhu's Abode" },
	{ PXM_Egg1,		PXE_Egg1,	TSC_Egg1,	&PAL_Regu,	5,	0,	"Egg No. 01" },
	{ PXM_Pens2,	PXE_Pens2,	TSC_Pens2,	&PAL_Regu,	16,	0,	"Arthur's House" },
	{ PXM_Malco,	PXE_Malco,	TSC_Malco,	&PAL_Regu,	2,	0,	"Power Room" },
	{ PXM_WeedS,	PXE_WeedS,	TSC_WeedS,	&PAL_Regu,	2,	0,	"Save Point" },
	{ PXM_WeedD,	PXE_WeedD,	TSC_WeedD,	&PAL_Regu,	19,	0,	"Execution Chamber" },
	{ PXM_Frog,		PXE_Frog,	TSC_Frog,	&PAL_Regu,	20,	0,	"Gum" },
	{ PXM_Curly,	PXE_Curly,	TSC_Curly,	&PAL_Regu,	18,	0,	"Sand Zone Residence" },
	{ PXM_WeedB,	PXE_WeedB,	TSC_WeedB,	&PAL_Regu,	16,	0,	"Grasstown Hut" },
	{ PXM_Stream,	PXE_Stream,	TSC_Stream,	&PAL_Regu,	17,	2,	"Main Artery" },
	{ PXM_CurlyS,	PXE_CurlyS,	TSC_CurlyS,	&PAL_Regu,	16,	0,	"Small Room" },
	{ PXM_Jenka1,	PXE_Jenka1,	TSC_Jenka1,	&PAL_Regu,	2,	0,	"Jenka's House" },
	{ PXM_Dark,		PXE_Dark,	TSC_Dark,	&PAL_Regu,	18,	0,	"Deserted House" },
	{ PXM_Gard,		PXE_Gard,	TSC_Gard,	&PAL_Regu,	9,	5,	"Sand Zone Storehouse" },
	{ PXM_Jenka2,	PXE_Jenka2,	TSC_Jenka2,	&PAL_Regu,	2,	0,	"Jenka's House" },
	{ PXM_SandE,	PXE_SandE,	TSC_SandE,	&PAL_Regu,	18,	7,	"Sand Zone" },
	{ PXM_MazeH,	PXE_MazeH,	TSC_MazeH,	&PAL_Regu,	13,	8,	"Labyrinth H" },
	{ PXM_MazeW,	PXE_MazeW,	TSC_MazeW,	&PAL_Regu,	13,	8,	"Labyrinth W" },
	{ PXM_MazeO,	PXE_MazeO,	TSC_MazeO,	&PAL_Regu,	13,	0,	"Camp" },
	{ PXM_MazeD,	PXE_MazeD,	TSC_MazeD,	&PAL_Regu,	13,	0,	"Clinic Ruins" },
	{ PXM_MazeA,	PXE_MazeA,	TSC_MazeA,	&PAL_Regu,	19,	0,	"Labyrinth Shop" },
	{ PXM_MazeB,	PXE_MazeB,	TSC_MazeB,	&PAL_Regu,	13,	8,	"Labyrinth B" },
	{ PXM_MazeS,	PXE_MazeS,	TSC_MazeS,	&PAL_Regu,	13,	0,	"Boulder Chamber" },
	{ PXM_MazeM,	PXE_MazeM,	TSC_MazeM,	&PAL_Regu,	13,	10,	"Labyrinth M" },
	{ PXM_Drain,	PXE_Drain,	TSC_Drain,	&PAL_Regu,	3,	0,	"Dark Place" },
	{ PXM_Almond,	PXE_Almond,	TSC_Almond,	&PAL_Regu,	1,	0,	"Core" },
	{ PXM_River,	PXE_River,	TSC_River,	&PAL_Regu,	17,	6,	"Waterway" },
	{ PXM_Eggs2,	PXE_Eggs2,	TSC_Eggs2,	&PAL_Regu,	6,	7,	"Egg Corridor?" },
	{ PXM_Cthu2,	PXE_Cthu2,	TSC_Cthu2,	&PAL_Regu,	19,	0,	"Cthulhu's Abode?" },
	{ PXM_EggR2,	PXE_EggR2,	TSC_EggR2,	&PAL_Regu,	19,	0,	"Egg Observation Room?" },
	{ PXM_EggX,		PXE_EggX2,	TSC_EggX2,	&PAL_Regu,	7,	0,	"Egg No. 00" },
	{ PXM_Oside,	PXE_Oside,	TSC_Oside,	&PAL_Regu,	15,	9,	"Outer Wall" },
	{ PXM_EgEnd2,	PXE_EgEnd2,	TSC_EgEnd2,	&PAL_Regu,	19,	0,	"Side Room" },
	{ PXM_Itoh,		PXE_Itoh,	TSC_Itoh,	&PAL_Regu,	19,	0,	"Storehouse" },
	{ PXM_Cent,		PXE_Cent,	TSC_Cent,	&PAL_Regu,	4,	6,	"Plantation" },
	{ PXM_Jail1,	PXE_Jail1,	TSC_Jail1,	&PAL_Regu,	11,	0,	"Jail No. 1" },
	{ PXM_Momo,		PXE_Momo,	TSC_Momo,	&PAL_Regu,	11,	0,	"Hideout" },
	{ PXM_Lounge,	PXE_Lounge,	TSC_Lounge,	&PAL_Regu,	11,	0,	"Rest Area" },
	{ PXM_CentW,	PXE_CentW,	TSC_CentW,	&PAL_Regu,	19,	0,	"Teleporter" },
	{ PXM_Jail2,	PXE_Jail2,	TSC_Jail2,	&PAL_Regu,	19,	0,	"Jail No. 2" },
	{ PXM_Blcny1,	PXE_Blcny1,	TSC_Blcny1,	&PAL_Regu,	21,	4,	"Balcony" },
	{ PXM_Priso1,	PXE_Priso1,	TSC_Priso1,	&PAL_Regu,	11,	0,	"Last Cave" },
	{ PXM_Ring1,	PXE_Ring1,	TSC_Ring1,	&PAL_Regu,	21,	4,	"Throne Room" },
	{ PXM_Ring2,	PXE_Ring2,	TSC_Ring2,	&PAL_Regu,	21,	4,	"The King's Table" },
	{ PXM_Prefa1,	PXE_Prefa1,	TSC_Prefa1,	&PAL_Regu,	16,	0,	"Prefab House" },
	{ PXM_Priso2,	PXE_Priso2,	TSC_Priso2,	&PAL_Regu,	11,	0,	"Last Cave" },
	{ PXM_Ring3,	PXE_Ring3,	TSC_Ring3,	&PAL_Regu,	21,	0,	"Black Space" },
	{ PXM_Little,	PXE_Little,	TSC_Little,	&PAL_Regu,	16,	0,	"Little House" },
	{ PXM_Blcny2,	PXE_Blcny2,	TSC_Blcny2,	&PAL_Regu,	21,	4,	"Balcony" },
	{ PXM_Fall,		PXE_Fall,	TSC_Fall,	&PAL_Regu,	8,	0,	"Ending" },
	{ PXM_Kings,	PXE_Kings,	TSC_Kings,	&PAL_Regu,	21,	0,	"Intro" },
	{ PXM_Pixel,	PXE_Pixel,	TSC_Pixel,	&PAL_Regu,	16,	0,	"Waterway Cabin" },
	{ PXM_e_Maze,	PXE_e_Maze,	TSC_e_Maze,	&PAL_Regu,	13,	0,	"Credits" },
	{ PXM_e_Jenk,	PXE_e_Jenk,	TSC_e_Jenk,	&PAL_Regu,	2,	0,	"Credits" },
	{ PXM_e_Malc,	PXE_e_Malc,	TSC_e_Malc,	&PAL_Regu,	2,	0,	"Credits" },
	{ PXM_e_Ceme,	PXE_e_Ceme,	TSC_e_Ceme,	&PAL_Regu,	14,	0,	"Credits" },
	{ PXM_e_Sky,	PXE_e_Sky,	TSC_e_Sky,	&PAL_Regu,	8,	0,	"Credits" },
	{ PXM_Prefa2,	PXE_Prefa2,	TSC_Prefa2,	&PAL_Regu,	16,	0,	"Prefab House" },
	{ PXM_Hell1,	PXE_Hell1,	TSC_Hell1,	&PAL_Regu,	10,	0,	"Sacred Ground B1" },
	{ PXM_Hell2,	PXE_Hell2,	TSC_Hell2,	&PAL_Regu,	10,	0,	"Sacred Ground B2" },
	{ PXM_Hell3,	PXE_Hell3,	TSC_Hell3,	&PAL_Regu,	10,	0,	"Sacred Ground B3" },
	{ PXM_Mapi,		PXE_Mapi,	TSC_Mapi,	&PAL_Regu,	3,	0,	"Storage" },
	{ PXM_Hell4,	PXE_Hell4,	TSC_Hell4,	&PAL_Regu,	10,	0,	"Passage?" },
	{ PXM_Hell42,	PXE_Hell42,	TSC_Hell42,	&PAL_Regu,	10,	0,	"Passage?" },
	{ PXM_Statue,	PXE_Statue,	TSC_Statue,	&PAL_Regu,	10,	0,	"Statue Chamber" },
	{ PXM_Ballo1,	PXE_Ballo1,	TSC_Ballo1,	&PAL_Regu,	10,	0,	"Seal Chamber" },
	{ PXM_Ostep,	PXE_Ostep,	TSC_Ostep,	&PAL_Regu,	21,	0,	"Corridor" },
	{ PXM_e_Labo,	PXE_e_Labo,	TSC_e_Labo,	&PAL_Regu,	12,	0,	"Credits" },
	{ PXM_Pole,		PXE_Pole,	TSC_Pole,	&PAL_Gunsmith,3,0,	"Hermit Gunsmith" },
	{ PXM_Island,	PXE_Island,	TSC_Island,	&PAL_Regu,	21,	0,	"" },
	{ PXM_Ballo2,	PXE_Ballo2,	TSC_Ballo2,	&PAL_Regu,	10,	0,	"Seal Chamber" },
	{ PXM_e_Blcn,	PXE_e_Blcn,	TSC_e_Blcn,	&PAL_Regu,	21,	0,	"Credits" },
	{ PXM_Clock,	PXE_Clock,	TSC_Clock,	&PAL_Regu,	15,	0,	"Clock" },
};

// Tileset Info
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
	{ &TS_Pens, &PAL_Pens, PXA_Pens },
	{ &TS_River, &PAL_River, PXA_River },
	{ &TS_Sand, &PAL_Sand, PXA_Sand },
	{ &TS_Store, &PAL_Store, PXA_Store },
	{ &TS_Weed, &PAL_Weed, PXA_Weed },
	{ &TS_White, &PAL_White, PXA_White },
};

// Background Info
const background_info_def background_info[BACKGROUND_COUNT] = {
	{ NULL, NULL, 0, 0, 0 },
	{ &BG_Blue, &PAL_BkBlue, 0, 8, 8 },
	{ &BG_Blue, &PAL_BkBlue, 0, 8, 8 },
	{ &BG_Blue, &PAL_BkBlue, 0, 8, 8 },
	{ &BG_Fog, &PAL_BkFog, 1, 16, 4 },
	{ &BG_Gard, &PAL_BkGard, 0, 6, 8 },
	{ &BG_Gray, &PAL_BkGray, 0, 8, 8 },
	{ &BG_Green, &PAL_BkGreen, 0, 8, 8 },
	{ &BG_Maze, &PAL_BkMaze, 0, 8, 8 },
	{ &BG_Maze, &PAL_BkMaze, 0, 8, 8 },
	{ &BG_Red, &PAL_BkRed, 0, 4, 4 },
	{ &BG_Red, &PAL_BkRed, 0, 4, 4 },
};

const song_info_def song_info[SONG_COUNT] = {
	{ NULL, "Silence" },
	{ BGM_Balcony, "Mischievous Robot" },
	{ BGM_Safety, "Safety" },
	{ BGM_GameOver, "Game Over" },
	{ BGM_Gravity, "Gravity" },
	{ BGM_Balcony, "On To Grasstown" },
	{ BGM_Balcony, "Meltdown 2" },
	{ BGM_EyesOfFlame, "Eyes Of Flame" },
	{ BGM_Balcony, "Gestation" },
	{ BGM_Balcony, "Mimiga Village" },
	{ BGM_Fanfare1, "Fanfare 1" },
	{ BGM_Balrog, "Balrog's Theme" },
	{ BGM_Balcony, "Cemetery" },
	{ BGM_Balcony, "Yamashita Farm" },
	{ BGM_Balcony, "Pulse" },
	{ BGM_Fanfare1, "Fanfare 2" },
	{ BGM_Fanfare1, "Fanfare 3" }, // 0x10
	{ BGM_Balcony, "Tyrant" },
	{ BGM_Run, "Run!" },
	{ BGM_Balcony, "Jenka 1" },
	{ BGM_Balcony, "Labyrinth Fight" },
	{ BGM_Access, "Access" },
	{ BGM_Balcony, "Oppression" },
	{ BGM_Balcony, "Geothermal" },
	{ BGM_Balcony, "Cave Story" },
	{ BGM_Balcony, "Moon Song" },
	{ BGM_Balcony, "Hero's End" },
	{ BGM_Balcony, "Scorching Back" },
	{ BGM_Quiet, "Quiet" },
	{ BGM_Balcony, "Last Cave" },
	{ BGM_Balcony, "Balcony" },
	{ BGM_Charge, "Charge" },
	{ BGM_EyesOfFlame, "Last Battle" }, // 0x20
	{ BGM_Balcony, "The Way Back Home" },
	{ BGM_Balcony, "Zombie" },
	{ BGM_Balcony, "Break Down" },
	{ BGM_Balcony, "Running Hell" },
	{ BGM_Balcony, "Jenka 2" },
	{ BGM_Balcony, "Living Waterway" },
	{ BGM_Balcony, "Seal Chamber" },
	{ BGM_Balcony, "Toroko's Theme" },
	{ BGM_Balcony, "White Stone Wall" },
};

const sound_info_def sound_info[SOUND_COUNT] = {
	{ NULL, 0, "Silence" },
	{ SFX_Cursor, 1905, "Cursor" },
	{ SFX_Message, 2540, "Message" },
	{ SFX_Bonk, 1905, "Head Bonk" },
	{ SFX_SwitchWpn, 5080, "Switch Weapon" },
	{ SFX_Prompt, 3810, "Yes/No Prompt" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ SFX_Door, 3175, "Door Open" },
	{ SFX_BlockBreak, 6350, "Block Destroy" },
	{ NULL, 0, "Silence" },
	{ SFX_GetExp, 6350, "Get Weapon Exp" },
	{ SFX_Jump, 635, "Jump" },
	{ SFX_Hurt, 3175, "Take Damage" }, // 0x10
	{ SFX_Die, 25397, "Player Defeated" },
	{ SFX_Select, 6530, "Selection" },
	{ NULL, 0, "Silence" },
	{ SFX_Refill, 12700, "Health/Ammo Refill" },
	{ NULL, 0, "Silence" },
	{ SFX_Chest, 2540, "Open Chest" },
	{ SFX_Thud, 1905, "Landing Thud" },
	{ SFX_Walk, 635, "Walking" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ SFX_PolarStar, 3175, "Polar Star Lv1&2" }, // 0x20
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ SFX_GetItem, 3810, "Get Item" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" }, // 0x30
	{ SFX_PolarStar3, 6350, "Polar Star Lv3" },
	{ SFX_Squeak, 3810, "Squeak" },
	{ SFX_EnemyHurt1, 6350, "Enemy Hurt" },
	{ SFX_EnemyHurt2, 12700, "Big Enemy Hurt" },
	{ SFX_EnemyHurt3, 6350, "Small Enemy Hurt" },
	{ SFX_EnemyHurt4, 5080, "Cool Enemy Hurt" },
	{ SFX_Splash, 14000, "Splash" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" }, // Spur Lv 3
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" }, // 0x40
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" },
	{ NULL, 0, "Silence" }, // 72
};

#define SPR_Behem NULL
#define SPR_CrtHG NULL
#define SPR_BtlHG NULL
#define SPR_Basil NULL
#define SPR_BtlF1 NULL
#define SPR_Balrog NULL

const npc_info_def npc_info[NPC_COUNT] = {
	{ NULL, PAL0 }, // 0 - Nothing
	{ &SPR_EnergyS, PAL1 }, // 1 - Weapon Energy (Small)
	{ SPR_Behem, PAL1 }, // 2 - Enemy: Behemoth
	{ NULL, PAL0 }, // 3 - Nothing / Unknown
	{ &SPR_Smoke, PAL1 }, // 4 - Smoke
	{ SPR_CrtHG, PAL0 }, // 5 - Enemy: Critter (Hopping, Green)
	{ SPR_BtlHG, PAL0 }, // 6 - Enemy: Beetle (Horizontal, Green)
	{ SPR_Basil, PAL0 }, // 7 - Enemy: Basil
	{ SPR_BtlF1, PAL0 }, // 8 - Enemy: Beetle (Follow 1)
	{ SPR_Balrog, PAL3 }, // 9 - Balrog (Drops in)
	{ SPR_Balrog, PAL3 }, // 10 - Boss: Balrog (Shooting)
	{ NULL, PAL0 }, // 11 - Projectile: Balrog energy shot
	{ SPR_Balrog, PAL3 }, // 12 - Balrog (Cutscene)
	{ NULL, PAL1 }, // 13 - Forcefield
	{ NULL, PAL1 }, // 14 - Santa's Key
	{ &SPR_Chest, PAL1 }, // 15 - Treasure Chest (Closed)
	{ &SPR_Save, PAL1 }, // 16 - Save Point
	{ &SPR_Refill, PAL1 }, // 17 - Health / Ammo Refill
	{ &SPR_Door, PAL1 }, // 18 - Door
	{ SPR_Balrog, PAL3 }, // 19 - Balrog (Busts in)
	{ NULL, PAL0 }, // 20 - Computer
	{ NULL, PAL0 }, // 21 - Treasure Chest (Open)
	{ &SPR_Tele, PAL1 }, // 22 - Teleporter
	{ NULL, PAL0 }, // 23 - Teleporter Lights
	{ NULL, PAL0 }, // 24 - Enemy: Power Critter
	{ NULL, PAL0 }, // 25 - Lift Platform
	{ &SPR_Bat, PAL0 }, // 26 - Enemy: Bat (Black, circling)
	{ &SPR_Trap, PAL1 }, // 27 - Death trap
	{ NULL, PAL0 }, // 28 - Enemy: Critter (Flying)
	{ &SPR_Cthu, PAL3 }, // 29 - Cthulhu
	{ &SPR_Gunsmith, PAL3 }, // 30 - Hermit Gunsmith
	{ &SPR_Bat, PAL0 }, // 31 - Enemy: Bat (Black, Hanging)
	{ &SPR_LifeUp, PAL1 }, // 32 - Life Capsule
	{ NULL, PAL0 }, // 33 - Projectile: Balrog energy bounce
	{ &SPR_Bed, PAL1 }, // 34 - Bed
	{ NULL, PAL0 }, // 35 - Enemy: Mannan
	{ SPR_Balrog, PAL3 }, // 36 - Boss: Balrog (Hovering)
	{ &SPR_Sign, PAL1 }, // 37 - Sign Post
	{ &SPR_Fire, PAL1 }, // 38 - Fireplace Fire
	{ NULL, PAL1 }, // 39 - Save Sign
	{ NULL, PAL0 }, // 40 - Santa
	{ NULL, PAL0 }, // 41 - Busted Doorway
	{ NULL, PAL0 }, // 42 - Sue
	{ NULL, PAL0 }, // 43 - Blackboard
	{ NULL, PAL0 }, // 44 - Enemy: Polish
	{ NULL, PAL0 }, // 45 - Enemy: Baby
	{ NULL, PAL0 }, // 46 - Horiz/Vert Trigger
	{ NULL, PAL0 }, // 47 - Enemy: Sandcroc (Green)
	{ NULL, PAL0 }, // 48 - Projectile: Omega
	{ NULL, PAL0 }, // 49 - Enemy: Skullhead
	{ NULL, PAL0 }, // 50 - Projectile: Skeleton
	{ NULL, PAL0 }, // 51 - Enemy: Crow & Skullhead
	{ NULL, PAL3 }, // 52 - Blue Robot (Sitting)
	{ NULL, PAL0 }, // 53 - <CRASH>
	{ NULL, PAL0 }, // 54 - Enemy: Skullstep
	{ &SPR_Kazuma, PAL3 }, // 55 - Kazuma
	{ NULL, PAL0 }, // 56 - Enemy: Beetle (Horizontal, Brown)
	{ NULL, PAL0 }, // 57 - Enemy: Crow
	{ NULL, PAL0 }, // 58 - Enemy: Basu (1)
	{ &SPR_DoorE, PAL1 }, // 59 - Enemy: Door
	{ NULL, PAL0 }, // 60 - Toroko
	{ NULL, PAL0 }, // 61 - King
	{ NULL, PAL0 }, // 62 - Kazuma (Computer)
	{ NULL, PAL0 }, // 63 - Toroko (Attacking)
	{ NULL, PAL0 }, // 64 - Enemy: Critter (Hopping, Blue)
	{ &SPR_Bat, PAL0 }, // 65 - Enemy: Bat (Blue)
	{ NULL, PAL0 }, // 66 - Bubble
	{ NULL, PAL0 }, // 67 - Misery (Floating)
	{ SPR_Balrog, PAL3 }, // 68 - Boss: Balrog (Running)
	{ NULL, PAL0 }, // 69 - Enemy: Pignon
	{ &SPR_Sparkle, PAL1 }, // 70 - Sparkling Item
	{ NULL, PAL0 }, // 71 - Enemy: Chinfish
	{ NULL, PAL0 }, // 72 - Sprinkler
	{ NULL, PAL0 }, // 73 - Water Drop
	{ NULL, PAL0 }, // 74 - Jack
	{ NULL, PAL0 }, // 75 - Kanpachi (Fishing)
	{ NULL, PAL0 }, // 76 - Flowers
	{ NULL, PAL0 }, // 77 - Sandame's Pavilion
	{ NULL, PAL0 }, // 78 - Pot
	{ NULL, PAL0 }, // 79 - Mahin
	{ NULL, PAL0 }, // 80 - Enemy: Gravekeeper
	{ NULL, PAL0 }, // 81 - Enemy: Giant Pignon
	{ NULL, PAL0 }, // 82 - Misery (Standing)
	{ NULL, PAL0 }, // 83 - Igor (Cutscene)
	{ NULL, PAL0 }, // 84 - Projectile: Basu (1)
	{ NULL, PAL0 }, // 85 - Terminal
	{ NULL, PAL0 }, // 86 - Missile
	{ NULL, PAL0 }, // 87 - Heart
	{ NULL, PAL0 }, // 88 - Boss: Igor
	{ NULL, PAL0 }, // 89 - Igor (Defeated)
	{ NULL, PAL0 }, // 90 - Background
	{ NULL, PAL0 }, // 91 - Cage
	{ NULL, PAL0 }, // 92 - Sue (Computer)
	{ NULL, PAL0 }, // 93 - Chaco
	{ NULL, PAL0 }, // 94 - Enemy: Kulala
	{ NULL, PAL0 }, // 95 - Enemy: Jelly
	{ NULL, PAL0 }, // 96 - Fan (Left)
	{ NULL, PAL0 }, // 97 - Fan (Up)
	{ NULL, PAL0 }, // 98 - Fan (Right)
	{ NULL, PAL0 }, // 99 - Fan (Down)
	{ NULL, PAL0 }, // 100 - Grate
	{ NULL, PAL0 }, // 101 - Power Controls (Screen)
	{ NULL, PAL0 }, // 102 - Power Controls (Power Flow)
	{ NULL, PAL0 }, // 103 - Projectile: Mannan
	{ NULL, PAL0 }, // 104 - Enemy: Frog
	{ NULL, PAL0 }, // 105 - Balloon (Hey!, Low)
	{ NULL, PAL0 }, // 106 - Balloon (Hey!, High)
	{ NULL, PAL0 }, // 107 - Malco (Undamaged)
	{ NULL, PAL0 }, // 108 - Projectile: Balfrog
	{ NULL, PAL0 }, // 109 - Malco (Damaged)
	{ NULL, PAL0 }, // 110 - Enemy: Puchi
	{ NULL, PAL0 }, // 111 - Quote (Teleports Out)
	{ NULL, PAL0 }, // 112 - Quote (Teleports In)
	{ NULL, PAL0 }, // 113 - Prof. Booster
	{ NULL, PAL0 }, // 114 - Enemy: Press
	{ NULL, PAL0 }, // 115 - Enemy: Ravil
	{ NULL, PAL0 }, // 116 - Red Flowers (Petals)
	{ NULL, PAL0 }, // 117 - Curly
	{ NULL, PAL0 }, // 118 - Boss: Curly
	{ NULL, PAL0 }, // 119 - Tables & Chair
	{ NULL, PAL0 }, // 120 - Colon 1
	{ NULL, PAL0 }, // 121 - Colon 2
	{ NULL, PAL0 }, // 122 - Enemy: Colon
	{ NULL, PAL0 }, // 123 - Projectile: Curly
	{ NULL, PAL0 }, // 124 - Sunstone
	{ NULL, PAL0 }, // 125 - Hidden Heart/Missile
	{ NULL, PAL0 }, // 126 - Puppy (Runs Away)
	{ NULL, PAL0 }, // 127 - Glowy thing?
	{ NULL, PAL0 }, // 128 - Glowy thing?
	{ NULL, PAL0 }, // 129 - Glowy thing?
	{ NULL, PAL0 }, // 130 - Puppy (Tail Wag)
	{ NULL, PAL0 }, // 131 - Puppy (Sleeping)
	{ NULL, PAL0 }, // 132 - Puppy (Bark)
	{ NULL, PAL0 }, // 133 - Jenka
	{ NULL, PAL0 }, // 134 - Enemy: Armadillo
	{ NULL, PAL0 }, // 135 - Enemy: Skeleton
	{ NULL, PAL0 }, // 136 - Puppy (Carried)
	{ NULL, PAL0 }, // 137 - Large Doorway (Frame)
	{ NULL, PAL0 }, // 138 - Large Doorway (Doors)
	{ NULL, PAL0 }, // 139 - Doctor (Crowned)
	{ NULL, PAL0 }, // 140 - Boss: Frenzied Toroko
	{ NULL, PAL0 }, // 141 - <CRASH>
	{ NULL, PAL0 }, // 142 - Enemy: Flowercub
	{ NULL, PAL0 }, // 143 - Jenka (Collapsed)
	{ NULL, PAL0 }, // 144 - Toroko (Teleports In)
	{ NULL, PAL0 }, // 145 - <CRASH>
	{ NULL, PAL0 }, // 146 - Lightning
	{ NULL, PAL0 }, // 147 - Enemy: Critter (Hover)
	{ NULL, PAL0 }, // 148 - Projectile: Critter
	{ NULL, PAL0 }, // 149 - Moving Block (Horizontal)
	{ NULL, PAL0 }, // 150 - Quote
	{ NULL, PAL0 }, // 151 - Blue Robot
	{ NULL, PAL0 }, // 152 - Shutter (Stuck)
	{ NULL, PAL0 }, // 153 - Enemy: Gaudi
	{ NULL, PAL0 }, // 154 - Enemy: Gaudi (Defeated)
	{ NULL, PAL0 }, // 155 - Enemy: Gaudi (Flying)
	{ NULL, PAL0 }, // 156 - Projectile: Gaudi (Flying)
	{ NULL, PAL0 }, // 157 - Moving Block (Vertical)
	{ NULL, PAL0 }, // 158 - Projectile: Monster X
	{ NULL, PAL0 }, // 159 - Monster X (Defeated)
	{ NULL, PAL0 }, // 160 - Boss: Pooh Black
	{ NULL, PAL0 }, // 161 - Projectile: Pooh Black
	{ NULL, PAL0 }, // 162 - Pooh Black (Defeated)
	{ NULL, PAL0 }, // 163 - Dr. Gero
	{ NULL, PAL0 }, // 164 - Nurse Hasumi
	{ NULL, PAL0 }, // 165 - Curly (Collapsed)
	{ NULL, PAL0 }, // 166 - Chaba
	{ NULL, PAL0 }, // 167 - Prof. Booster (Fall)
	{ NULL, PAL0 }, // 168 - Boulder
	{ NULL, PAL0 }, // 169 - Boss: Balrog (Missiles)
	{ NULL, PAL0 }, // 170 - Projectile: Balrog (Missiles)
	{ NULL, PAL0 }, // 171 - Enemy: Fire Whirr
	{ NULL, PAL0 }, // 172 - Projectile: Fire Whirr
	{ NULL, PAL0 }, // 173 - Enemy: Gaudi Armor
	{ NULL, PAL0 }, // 174 - Projectile: Gaudi Armor
	{ NULL, PAL0 }, // 175 - Enemy: Gaudi Egg
	{ NULL, PAL0 }, // 176 - Enemy: Buyobuyo Base
	{ NULL, PAL0 }, // 177 - Enemy: Buyobuyo
	{ NULL, PAL0 }, // 178 - Projectile: Core (Spinner)
	{ NULL, PAL0 }, // 179 - Projectile: Core (Wisp)
	{ NULL, PAL0 }, // 180 - Curly (A.I.)
	{ NULL, PAL0 }, // 181 - Unknown
	{ NULL, PAL0 }, // 182 - Unknown
	{ NULL, PAL0 }, // 183 - Unknown
	{ NULL, PAL0 }, // 184 - Shutter (Large)
	{ NULL, PAL0 }, // 185 - Shutter (Small)
	{ NULL, PAL0 }, // 186 - Lift Block
	{ NULL, PAL0 }, // 187 - Enemy: Fuzz Core
	{ NULL, PAL0 }, // 188 - <CRASH>
	{ NULL, PAL0 }, // 189 - Projectile: Homing Flame
	{ NULL, PAL0 }, // 190 - Surface Robot
	{ NULL, PAL0 }, // 191 - Water Level
	{ NULL, PAL0 }, // 192 - Scooter
	{ NULL, PAL0 }, // 193 - Scooter (Pieces)
	{ NULL, PAL0 }, // 194 - Blue Robot (Pieces)
	{ NULL, PAL0 }, // 195 - Grate Mouth
	{ NULL, PAL0 }, // 196 - Motion Wall
	{ NULL, PAL0 }, // 197 - Enemy: Porcupine Fish
	{ NULL, PAL0 }, // 198 - Projectile: Ironhead
	{ NULL, PAL0 }, // 199 - Underwater Current
	{ NULL, PAL0 }, // 200 - Enemy: Dragon Zombie
	{ NULL, PAL0 }, // 201 - Dragon Zombie (Dead)
	{ NULL, PAL0 }, // 202 - Projectile: Dragon Zombie
	{ NULL, PAL0 }, // 203 - Enemy: Critter (Hopping, Aqua)
	{ NULL, PAL0 }, // 204 - Falling Spike (Small)
	{ NULL, PAL0 }, // 205 - Falling Spike (Large)
	{ NULL, PAL0 }, // 206 - Enemy: Counter Bomb
	{ NULL, PAL0 }, // 207 - Balloon (Countdown)
	{ NULL, PAL0 }, // 208 - Enemy: Basu (2)
	{ NULL, PAL0 }, // 209 - Projectile: Basu (2)
	{ NULL, PAL0 }, // 210 - Enemy: Beetle (Follow 2)
	{ NULL, PAL0 }, // 211 - Spikes
	{ NULL, PAL0 }, // 212 - Sky Dragon
	{ NULL, PAL0 }, // 213 - Enemy: Night Spirit
	{ NULL, PAL0 }, // 214 - Projectile: Night Spirit
	{ NULL, PAL0 }, // 215 - Enemy: Sandcroc (White)
	{ NULL, PAL0 }, // 216 - Debug Cat
	{ NULL, PAL0 }, // 217 - Itoh
	{ NULL, PAL0 }, // 218 - Unknown
	{ NULL, PAL0 }, // 219 - Generator: Smoke/Underwater Current
	{ NULL, PAL0 }, // 220 - Shovel Brigade
	{ NULL, PAL0 }, // 221 - Shovel Brigade (Walking)
	{ NULL, PAL0 }, // 222 - Prison Bars
	{ NULL, PAL0 }, // 223 - Momorin
	{ NULL, PAL0 }, // 224 - Chie
	{ NULL, PAL0 }, // 225 - Megane
	{ NULL, PAL0 }, // 226 - Kanpachi
	{ NULL, PAL0 }, // 227 - Bucket
	{ NULL, PAL0 }, // 228 - Droll (Guard)
	{ NULL, PAL0 }, // 229 - Red Flowers (Sprouts)
	{ NULL, PAL0 }, // 230 - Red Flowers (Blooming)
	{ NULL, PAL0 }, // 231 - Rocket
	{ NULL, PAL0 }, // 232 - Enemy: Orangebell
	{ NULL, PAL0 }, // 233 - <CRASH>
	{ NULL, PAL0 }, // 234 - Red Flowers (picked)
	{ NULL, PAL0 }, // 235 - Enemy: Midorin
	{ NULL, PAL0 }, // 236 - Enemy: Gunfish
	{ NULL, PAL0 }, // 237 - Projectile: Gunfish
	{ NULL, PAL0 }, // 238 - Enemy: Press (Killer)
	{ NULL, PAL0 }, // 239 - Cage Bars
	{ NULL, PAL0 }, // 240 - Mimiga (Jailed)
	{ NULL, PAL0 }, // 241 - Enemy: Critter (Hopping, Red)
	{ NULL, PAL0 }, // 242 - Enemy: Bat (Red)
	{ NULL, PAL0 }, // 243 - Generator: Bat (Red)
	{ NULL, PAL0 }, // 244 - Acid Drop
	{ NULL, PAL0 }, // 245 - Generator: Acid Drop
	{ NULL, PAL0 }, // 246 - Enemy: Press (Proximity)
	{ NULL, PAL0 }, // 247 - Boss: Misery
	{ NULL, PAL0 }, // 248 - Boss: Misery (Vanish)
	{ NULL, PAL0 }, // 249 - Projectile: Misery (Energy shot)
	{ NULL, PAL0 }, // 250 - Projectile: Misery (Lightning ball)
	{ NULL, PAL0 }, // 251 - Projectile: Misery (Lightning)
	{ NULL, PAL0 }, // 252 - <CRASH>
	{ NULL, PAL0 }, // 253 - Energy Capsule
	{ NULL, PAL0 }, // 254 - Helicopter
	{ NULL, PAL0 }, // 255 - <CRASH>
	{ NULL, PAL0 }, // 256 - Doctor (Crowned, Facing away)
	{ NULL, PAL0 }, // 257 - Red Crystal
	{ NULL, PAL0 }, // 258 - Mimiga (Sleeping)
	{ NULL, PAL0 }, // 259 - Curly (Carried, Unconscious)
	{ NULL, PAL0 }, // 260 - Shovel Brigade (Caged)
	{ NULL, PAL0 }, // 261 - Chie (Caged)
	{ NULL, PAL0 }, // 262 - Chaco (Caged)
	{ NULL, PAL0 }, // 263 - Boss: Doctor
	{ NULL, PAL0 }, // 264 - Projectile: Doctor (Red wave)
	{ NULL, PAL0 }, // 265 - Projectile: Doctor (Red ball, Quick)
	{ NULL, PAL0 }, // 266 - Projectile: Doctor (Red ball, Slow)
	{ NULL, PAL0 }, // 267 - Boss: Muscle Doctor
	{ NULL, PAL0 }, // 268 - Enemy: Igor
	{ NULL, PAL0 }, // 269 - Enemy: Bat (Red energy)
	{ NULL, PAL0 }, // 270 - Red Energy
	{ NULL, PAL0 }, // 271 - Underwater Block
	{ NULL, PAL0 }, // 272 - Generator: Underwater Block
	{ NULL, PAL0 }, // 273 - Projectile: Droll
	{ NULL, PAL0 }, // 274 - Enemy: Droll
	{ NULL, PAL0 }, // 275 - Puppy (With items)
	{ NULL, PAL0 }, // 276 - Boss: Red Demon
	{ NULL, PAL0 }, // 277 - Projectile: Red Demon
	{ NULL, PAL0 }, // 278 - Little Family
	{ NULL, PAL0 }, // 279 - Falling Block (Large)
	{ NULL, PAL0 }, // 280 - Sue (Teleported In)
	{ NULL, PAL0 }, // 281 - Doctor (Red energy form)
	{ NULL, PAL0 }, // 282 - Enemy: Mini Undead Core (Floats Forward)
	{ NULL, PAL0 }, // 283 - Enemy: Misery (Transformed)
	{ NULL, PAL0 }, // 284 - Enemy: Sue (Transformed)
	{ NULL, PAL0 }, // 285 - Projectile: Undead Core (Orange spiral shot)
	{ NULL, PAL0 }, // 286 - Orange Dot
	{ NULL, PAL0 }, // 287 - Orange Smoke
	{ NULL, PAL0 }, // 288 - Projectile: Undead Core (Glowing rock)
	{ NULL, PAL0 }, // 289 - Enemy: Critter (Hopping, Orange)
	{ NULL, PAL0 }, // 290 - Enemy: Bat (Orange)
	{ NULL, PAL0 }, // 291 - Mini Undead Core (Before fight)
	{ NULL, PAL0 }, // 292 - Quake
	{ NULL, PAL0 }, // 293 - Projectile: Undead Core (Huge energy shot)
	{ NULL, PAL0 }, // 294 - Quake & Generator: Falling Blocks
	{ NULL, PAL0 }, // 295 - Cloud
	{ NULL, PAL0 }, // 296 - Generator: Cloud
	{ NULL, PAL0 }, // 297 - <CRASH>
	{ NULL, PAL0 }, // 298 - Doctor (Uncrowned)
	{ NULL, PAL0 }, // 299 - Balrog/Misery (Bubble)
	{ NULL, PAL0 }, // 300 - Demon Crown
	{ NULL, PAL0 }, // 301 - Enemy: Fish Missile (Orange)
	{ NULL, PAL0 }, // 302 - Ending stuff?
	{ NULL, PAL0 }, // 303 - Unknown
	{ NULL, PAL0 }, // 304 - Gaudi (Sitting)
	{ NULL, PAL0 }, // 305 - Puppy (Small)
	{ NULL, PAL0 }, // 306 - Balrog (Nurse)
	{ NULL, PAL0 }, // 307 - Santa (Caged)
	{ NULL, PAL0 }, // 308 - Enemy: Stumpy
	{ NULL, PAL0 }, // 309 - Enemy: Bute
	{ NULL, PAL0 }, // 310 - Enemy: Bute (Sword)
	{ NULL, PAL0 }, // 311 - Enemy: Bute (Archer)
	{ NULL, PAL0 }, // 312 - Projectile: Bute (Archer)
	{ NULL, PAL0 }, // 313 - Boss: Ma Pignon
	{ NULL, PAL0 }, // 314 - Falling, Indestructible
	{ NULL, PAL0 }, // 315 - Enemy (Hopping, Disappears)
	{ NULL, PAL0 }, // 316 - Bute (Defeated)
	{ NULL, PAL0 }, // 317 - Enemy: Mesa
	{ NULL, PAL0 }, // 318 - Mesa (Defeated)
	{ NULL, PAL0 }, // 319 - <CRASH>
	{ NULL, PAL0 }, // 320 - Curly (Carried, Shooting)
	{ NULL, PAL0 }, // 321 - Unknown
	{ NULL, PAL0 }, // 322 - Enemy: Deleet
	{ NULL, PAL0 }, // 323 - Enemy: Bute (Generated)
	{ NULL, PAL0 }, // 324 - Generator: Bute
	{ NULL, PAL0 }, // 325 - Projectile: Heavy Press
	{ NULL, PAL0 }, // 326 - Itoh/Sue (Turning Human)
	{ NULL, PAL0 }, // 327 - <CRASH>
	{ NULL, PAL0 }, // 328 - Transmogrifier
	{ NULL, PAL0 }, // 329 - Building Fan
	{ NULL, PAL0 }, // 330 - Enemy: Rolling
	{ NULL, PAL0 }, // 331 - Projectile: Ballos (Bone)
	{ NULL, PAL0 }, // 332 - Projectile: Ballos (Shockwave)
	{ NULL, PAL0 }, // 333 - Projectile: Ballos (Lightning)
	{ NULL, PAL0 }, // 334 - Sweat
	{ NULL, PAL0 }, // 335 - Ika-chan
	{ NULL, PAL0 }, // 336 - Generator: Ika-chan
	{ NULL, PAL0 }, // 337 - Numahachi
	{ NULL, PAL0 }, // 338 - Enemy: Green Devil
	{ NULL, PAL0 }, // 339 - Generator: Green Devil
	{ NULL, PAL0 }, // 340 - Boss: Ballos
	{ NULL, PAL0 }, // 341 - <CRASH>
	{ NULL, PAL0 }, // 342 - <CRASH>
	{ NULL, PAL0 }, // 343 - <CRASH>
	{ NULL, PAL0 }, // 344 - <CRASH>
	{ NULL, PAL0 }, // 345 - Projectile: Ballos (Skull)
	{ NULL, PAL0 }, // 346 - <CRASH>
	{ NULL, PAL0 }, // 347 - Enemy: Hoppy
	{ NULL, PAL0 }, // 348 - Ballos Spikes (Rising)
	{ NULL, PAL0 }, // 349 - Statue
	{ NULL, PAL0 }, // 350 - Enemy: Bute (Archer, Red)
	{ NULL, PAL0 }, // 351 - Statue (Can shoot)
	{ NULL, PAL0 }, // 352 - King (Sword)
	{ NULL, PAL0 }, // 353 - Enemy: Bute (Sword, Red)
	{ NULL, PAL0 }, // 354 - Invisible Death Trap Wall
	{ NULL, PAL0 }, // 355 - <CRASH>
	{ NULL, PAL0 }, // 356 - Balrog (Rescue)
	{ NULL, PAL0 }, // 357 - Puppy (Ghost)
	{ NULL, PAL0 }, // 358 - Misery (Wind)
	{ NULL, PAL0 }, // 359 - Generator: Water Drop
	{ NULL, PAL0 }, // 360 - "Thank You!"
};

/*
const npc_info_def npc_info[NPC_COUNT] = {
	{ NPC_IGNORESOLID, // 0 - Nothing
		1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ 0,  // 1 - Weapon Energy
			1, &SPR_EnergyS, PAL1, 0, 0, 2, 1, 0, {4,3,4,3}, {8,8,8,8} },
	{ NPC_SOLID|NPC_BOUNCYTOP|NPC_SHOOTABLE|NPC_FRONTATKONLY|NPC_SHOWDAMAGE,
			8, NULL, PAL1, 72, 52, 3, 1, 3, {11,14,11,8}, {16,16,16,8} },
	{ NPC_IGNORESOLID, // 3 - Nothing
			1, NULL, PAL1, 0, 0, 2, 0, 0, {0,0,0,0}, {0,0,0,0} },
	{ 0, // 4 - Smoke
			1, &SPR_Smoke, PAL1, 0, 0, 2, 0, 0, {4,4,4,4}, {8,8,8,8} },
	{ NPC_SOLID|NPC_IGNORE44|NPC_SHOOTABLE|NPC_SHOWDAMAGE, // 5 - Critter (Green)
			4, NULL, PAL1, 71, 51, 2, 2, 2, {6,4,6,5}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_SOLID|NPC_IGNORESOLID|NPC_SHOOTABLE, // 9 - Balrog (Appear)
			96, NULL, PAL1, 72, 52, 3, 18, 0, {14,11,14,12}, {20,12,20,12} },
	{ NPC_SOLID|NPC_SHOOTABLE|NPC_SHOWDAMAGE, // 10 - Balrog (Boss)
			96, NULL, PAL1, 72, 52, 3, 18, 0, {14,11,14,12}, {20,12,20,12} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ 0, // 14 - TODO: Santa's Key
			0, NULL, PAL1, 0, 0, 0, 0, 0, {5,6,5,8}, {8,8,8,8} },
	{ 0, // 15 - Treasure Chest
			1000, &SPR_Chest, PAL1, 0, 0, 0, 0, 0, {5,6,5,8}, {8,8,8,8} },
	{ 0, // 16 - Save Point
			0, &SPR_Save, PAL1, 0, 0, 0, 0, 0, {5,6,5,8}, {8,8,8,8} },
	{ 0, // 17 - Ammo Refill
			0, &SPR_Refill, PAL1, 0, 0, 0, 0, 0, {5,6,5,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 18 - Door
			0, &SPR_Door, PAL1, 0, 0, 0, 0, 0, {5,8,5,8}, {8,16,8,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 20 - TODO: Computer
			1000, NULL, PAL1, 0, 0, 0, 0, 0, {4,8,4,8}, {16,16,16,8} },
	{ 0, // 21 - TODO: Chest Opened
			0, NULL, PAL1, 0, 0, 0, 0, 0, {5,0,5,8}, {8,0,8,8} },
	{ 0, // 22 - Teleporter
			0, &SPR_Tele, PAL1, 0, 0, 0, 0, 0, {10,1,10,8}, {12,24,12,8} },
	{ 0, // 23 - TODO: Teleporter Lights
			0, NULL, PAL1, 0, 0, 0, 0, 0, {12,4,12,0}, {12,4,12,0} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_SOLID|NPC_INVINCIBLE|NPC_IGNORESOLID, // 25 - TODO: Lift Platform
			0, NULL, PAL1, 0, 0, 0, 0, 0, {14,9,14,8}, {16,8,16,8} },
	{ NPC_IGNORESOLID|NPC_SHOOTABLE|NPC_SHOWDAMAGE, // 26 - Bat (Circling)
			4, &SPR_Bat, PAL0, 70, 50, 1, 2, 3, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 27 - Death Trap
			0, &SPR_Trap, PAL1, 0, 0, 0, 0, 127, {8,8,24,8}, {8,16,24,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ 0, // 29 - Cthulhu
			0, &SPR_Cthu, PAL3, 0, 0, 0, 0, 0, {4,4,4,4}, {8,16,8,8} },
	{ 0, // 30 - Hermit Gunsmith
			0, &SPR_Gunsmith, PAL3, 0, 0, 2, 0, 0, {6,6,6,6}, {8,8,8,8} },
	{ NPC_SHOOTABLE|NPC_SHOWDAMAGE, // 31 - Bat (Hanging)
			4, &SPR_Bat, PAL0, 70, 50, 1, 2, 3, {4,7,4,8}, {8,8,8,8} },
	{ 0, // 32 - Life Capsule
			0, &SPR_LifeUp, PAL1, 0, 0, 2, 0, 0, {6,8,6,6}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 34 - Bed
			1000, &SPR_Bed, PAL1, 0, 0, 0, 0, 0, {8,8,8,8}, {16,8,16,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ 0, // 37 - Signpost
			0, &SPR_Sign, PAL1, 0, 0, 0, 0, 0, {5,0,5,8}, {8,8,8,8} },
	{ 0, // 38 - Fire
			0, &SPR_Fire, PAL1, 0, 0, 0, 0, 0, {5,0,5,8}, {8,8,8,8} },
	{ 0, // 39 - TODO: Save Sign
			0, NULL, PAL1, 0, 0, 0, 0, 0, {5,0,5,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ 0, // 42 - TODO: Sue
			0, NULL, PAL3, 0, 0, 0, 0, 0, {5,8,5,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 43 - TODO: Blackboard
			0, NULL, PAL1, 0, 0, 0, 0, 0, {12,8,12,8}, {20,24,20,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ 0, // 46 - Trigger
			0, NULL, PAL1, 0, 0, 2, 0, 0, {6,6,6,6}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 52 - Blue Robot (sit)
			0, NULL, PAL3, 0, 0, 0, 0, 0, {5,8,5,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ 0, // 55 - Kazuma
			0, &SPR_Kazuma, PAL3, 0, 0, 0, 0, 0, {5,8,5,8}, {8,16,8,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID|NPC_SHOOTABLE|NPC_SHOWDAMAGE, // 59 - Door Enemy
			6, &SPR_DoorE, PAL1, 72, 52, 2, 2, 4, {5,8,5,8}, {8,16,8,8} },
	{ 0, // 60 - Toroko
			0, NULL, PAL3, 0, 0, 0, 0, 0, {4,8,4,8}, {8,8,8,8} },
	{ 0, // 61 - King
			0, NULL, PAL3, 0, 0, 0, 0, 0, {6,8,5,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 62 - Kazuma (Computer)
			0, NULL, PAL3, 0, 0, 0, 0, 0, {6,8,6,8}, {8,16,8,8} },
	{ NPC_IGNORESOLID, // 63 - Toroko (Attacking)
			100, NULL, PAL3, 70, 50, 2, 1, 0, {6,8,6,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 0 - Nothing
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID|NPC_SHOOTABLE|NPC_SHOWDAMAGE, // 65 - Bat (Blue)
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 68 - Balrog (Running)
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, // 70 - Sparkling Item
			1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
	{ NPC_IGNORESOLID, 1000, NULL, PAL1, 0, 0, 2, 0, 0, {4,3,4,8}, {8,8,8,8} },
};
*/
const weapon_info_def weapon_info[WEAPON_COUNT] = {
	{ NULL, {NULL,NULL,NULL}, {0,0,0}, {0,0,0}, {0,0,0} },
	{ NULL, {NULL,NULL,NULL}, {0,0,0}, {0,0,0}, {0,0,0} },
	{ &SPR_Polar, {&SPR_PolarB1,&SPR_PolarB2,&SPR_PolarB3}, {0x20,0x20,0x31}, {10,20,30}, {1,2,3} },
	{ NULL, {NULL,NULL,NULL}, {0,0,0}, {0,0,0}, {0,0,0} },
	{ NULL, {NULL,NULL,NULL}, {0,0,0}, {0,0,0}, {0,0,0} },
	{ NULL, {NULL,NULL,NULL}, {0,0,0}, {0,0,0}, {0,0,0} },
	{ NULL, {NULL,NULL,NULL}, {0,0,0}, {0,0,0}, {0,0,0} },
	{ NULL, {NULL,NULL,NULL}, {0,0,0}, {0,0,0}, {0,0,0} },
	{ NULL, {NULL,NULL,NULL}, {0,0,0}, {0,0,0}, {0,0,0} },
};
