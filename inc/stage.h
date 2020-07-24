/*
 * "Stage" refers to a level map. SGDK has a "Map" structure that is not used here,
 * so stage was chosen to avoid conflict
 * "Block" refers to 16x16 in-game tiles. It is named this way to differentiate
 * it from the 8x8 tiles used to store graphical data in VDP
 */

#define BLOCK_SIZE 16

// Tile attributes (PXA)
// High nybble: flags that can be used in combination
#define BLOCK_SLOPE			0x10
#define BLOCK_WATER			0x20
#define BLOCK_FOREGROUND	0x40
#define BLOCK_WIND			0x80
// Low nybble: distinct values
// These are the values for normal tiles, they don't apply to SLOPE or WIND
// For SLOPE values, check the heightmap in entity.h
// WIND values are directions, see the enum in common.h
#define BLOCK_SOLID			0x1
#define BLOCK_DAMAGE		0x2
#define BLOCK_SPECIAL		0x3
#define BLOCK_NPCSOLID		0x4
#define BLOCK_BULLETPASS	0x5
#define BLOCK_PLAYERSOLID	0x6

// Helper macros
// This will get the block (in the tileset) that is at a specific location in the stage grid
//#define stage_get_block(x, y) 
//	(stageBlocks[((uint16_t) stageTable[(uint16_t)(y)]) + ((uint16_t)(x))])
// Like above, but will return the attributes (solid, water, damage, etc)
//#define stage_get_block_type(x, y) 
//	(tileset_info[stageTileset].PXA[stage_get_block(((uint16_t)(x)), ((uint16_t)(y)))])
// Shortcut version of above
//#define blk(xf, xoff, yf, yoff) 
//	stage_get_block_type(((((int32_t)(xf))>>CSF)+((int32_t)(xoff)))>>4,((((int32_t)(yf))>>CSF)+((int32_t)(yoff)))>>4)

// I should have made this a long time ago
enum StageIndex {
	STAGE_0,
	STAGE_ARTHURS_HOUSE,
	STAGE_EGG_CORRIDOR,
	STAGE_EGG_NO_00,
	STAGE_EGG_NO_06,
	STAGE_EGG_OBSERVATORY,
	STAGE_GRASSTOWN,
	STAGE_SANTAS_HOUSE,
	STAGE_CHAKOS_HOUSE,
	STAGE_LABYRINTH_I,			// The room Misery warps you to with a high up switch
	STAGE_SAND_ZONE,
	STAGE_MIMIGA_VILLAGE,
	STAGE_FIRST_CAVE,
	STAGE_START_POINT,
	STAGE_MIMIGA_SHACK,			// Room with first Balrog boss (the one you can skip)
	STAGE_MIMIGA_RESERVOIR,
	STAGE_MIMIGA_GRAVEYARD,
	STAGE_MIMIGA_GARDEN,
	STAGE_GRASSTOWN_SHELTER,	// Where Kazuma gets stuck
	STAGE_MIMIGA_TOWN_HALL,
	STAGE_MIMIGA_SAVE_POINT,	// Small room with save & refill
	STAGE_EGG_SAVE_POINT,		// Save room at the very end
	STAGE_EGG_PASSAGEWAY,		// Cthulhu's Abode
	STAGE_EGG_NO_01,
	STAGE_ARTHURS_HOUSE_2,		// Scene after Egg Corridor
	STAGE_POWER_ROOM,			// Grasstown, second Balrog fight
	STAGE_GRASSTOWN_SAVE,		// Small save & refill room
	STAGE_EXECUTION_CHAMBER,	// Press puzzle in Grasstown
	STAGE_GRASSTOWN_GUM,		// Where you fight Balfrog
	STAGE_SAND_ZONE_BAR,		// Where you fight Curly
	STAGE_GRASSTOWN_HUT,		// House where Ravil attacks you
	STAGE_WATERWAY_BOSS,
	STAGE_SAND_ZONE_BAR_2,		// Save room with puppy
	STAGE_JENKA_1,				// Jenka's House
	STAGE_SAND_ZONE_HOUSE,		// Dark house with a puppy
	STAGE_SAND_ZONE_STOREHOUSE,	// Toroko boss
	STAGE_JENKA_2,				// For the scene with Balrog, I think
	STAGE_SAND_ZONE_2,			// For the scene where Misery warps you
	STAGE_LABYRINTH_H,			// Room after I, before W
	STAGE_LABYRINTH_W,			// Main room with Monster X
	STAGE_LABYRINTH_O,			// Clinic
	STAGE_LABYRINTH_D,			// Pooh Black
	STAGE_LABYRINTH_A,			// Shop
	STAGE_LABYRINTH_B,			// Booster
	STAGE_LABYRINTH_S,			// Last Balrog fight
	STAGE_LABYRINTH_M,			// Large section with AI Curly
	STAGE_DARK_PLACE,			// Room before core, entrance to Waterway
	STAGE_CORE,
	STAGE_WATERWAY,
	STAGE_EGG_CORRIDOR_2,		// Busted up Egg Corridor
	STAGE_EGG_PASSAGEWAY_2,
	STAGE_EGG_OBSERVATORY_2,
	STAGE_EGG_NO_00_2,
	STAGE_OUTER_WALL,
	STAGE_EGG_SAVE_POINT_2,
	STAGE_OUTER_WALL_STORAGE,	// Room with Itoh, connects Outer Wall and Plantation
	STAGE_PLANTATION,
	STAGE_PLANTATION_JAIL_1,	// Jail room where you get Sue's letter
	STAGE_MOMORIN_HIDEOUT,
	STAGE_PLANTATION_LOUNGE,
	STAGE_PLANTATION_TELEROOM,	// Lower west room with teleporter
	STAGE_PLANTATION_JAIL_2,	// West room with jail cells
	STAGE_BALCONY,
	STAGE_LAST_CAVE_1,			// Normal ending route
	STAGE_THRONE_ROOM,			// Misery boss
	STAGE_KINGS_TABLE,			// Muscle Doctor boss
	STAGE_PREFAB_HOUSE,			// House in Balcony with save & bed
	STAGE_LAST_CAVE_2,			// Best Ending route
	STAGE_BLACK_SPACE,			// Undead Core boss
	STAGE_LITTLE_HOUSE,
	STAGE_BALCONY_2,			// When escaping before ending sequence
	STAGE_FALLING,				// Quote falls and gets caught by Kazuma
	STAGE_INTRO,				// Intro scene when the game is launched
	STAGE_WATERWAY_CABIN,		// Where you drain Curly
	STAGE_ENDING_LABYRINTH,		// These are small maps made for the credits
	STAGE_ENDING_JENKA,
	STAGE_ENDING_POWER_ROOM,
	STAGE_ENDING_GRAVEYARD,
	STAGE_ENDING_SKY,			// Scene with everyone in the helicopter
	STAGE_PREFAB_HOUSE_2,		// Hell entrance
	STAGE_HELL_B1,
	STAGE_HELL_B2,
	STAGE_HELL_B3,
	STAGE_MA_PIGNON,
	STAGE_HELL_PASSAGEWAY,		// When you enter from Plantation
	STAGE_HELL_PASSAGEWAY_2,	// When you enter after defeating Heavy Press
	STAGE_HELL_STATUE,
	STAGE_SEAL_CHAMBER,
	STAGE_HELL_OUTER_PASSAGE,	// Room before Seal Chamber with the hologram dog
	STAGE_ENDING_LAB,			// Laboratory scene
	STAGE_GUNSMITH,
	STAGE_ISLAND,				// Displays an image of the island falling
	STAGE_SEAL_CHAMBER_2,		// Scene after defeating Ballos
	STAGE_ENDING_BALCONY,
	STAGE_CLOCK,				// Room in Outer Wall with the counter
};

// Index of current stage in db/stage.c
uint16_t stageID;
// Size of the stage - how many blocks wide/high
uint16_t stageWidth, stageHeight;
//extern uint8_t *PXM;
//#define stageWidth	(((uint16_t*)PXM)[2])
//#define stageHeight	(((uint16_t*)PXM)[3])
// A multiplication lookup table for each row of stageBlocks
// Removes all mulu.w and __mulsi3 instructions in entity stage collision
extern uint16_t *stageTable;
// Copy of level layout data loaded into RAM
// This takes up extra space, but there are times where scripts make modifications to the
// level layout (allowing player to reach some areas) so it is necessary to do this
extern uint8_t stagePXM[];
extern uint8_t stageBlocks[];
extern const uint8_t *stagePXA;
// Which tileset (db/tileset.c) is used by the current stage
uint8_t stageTileset;
// Prepares to draw off-screen tiles when stage_update() is later called
// Camera calls this each time it scrolls past 1 block length (16 pixels)
int8_t morphingRow, morphingColumn;

extern uint8_t stageBackground;
uint16_t backScrollTimer;
uint8_t stageBackgroundType;

static inline uint8_t stage_get_block(uint16_t x, uint16_t y) {
	return stageBlocks[stageTable[y] + x];
}

static inline uint8_t stage_get_block_type(uint16_t x, uint16_t y) {
	return stagePXA[stage_get_block(x, y)];
}

static inline uint8_t blk(int32_t xf, int16_t xoff, int32_t yf, int16_t yoff) {
	uint16_t x = (xf >> CSF) + xoff;
	uint16_t y = (yf >> CSF) + yoff;
	return stage_get_block_type(x >> 4, y >> 4);
}

// Clears previous stage and switches to one with the given ID
void stage_load(uint16_t id);
void stage_load_credits(uint8_t id);
// Called by TSC, replaces one block with another and creates smoke
void stage_replace_block(int16_t bx, int16_t by, uint8_t index);
// Updates scrolling for the stage and draws blocks as they get near the screen
// It is ideal to call this during vblank
void stage_update();
// Put the palettes expected by the current stage into "next" cache
void stage_setup_palettes();
