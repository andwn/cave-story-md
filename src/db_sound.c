#include "tables.h"
#include "resources.h"

const sound_info_def sound_info[SOUND_COUNT] = {
	{ NULL, 0 },
	{ SFX_01, 1905 }, // 1 - Cursor
	{ SFX_02, 2540 }, // 2 - Message
	{ SFX_03, 1905 }, // 3 - Head Bonk
	{ SFX_04, 5080 }, // 4 - Switch Weapon
	{ SFX_05, 3810 }, // 5 - Prompt
	{ SFX_06, 3175 }, // 6 - Hop
	{ NULL, 0 },
	{ SFX_08, 14000 }, // 8 - ????
	{ NULL, 0 },
	{ NULL, 0 },
	{ SFX_0B, 3175 }, // 11 - Door Open
	{ SFX_0C, 6350 }, // 12 - Destroy Block
	{ NULL, 0 },
	{ SFX_0E, 6350 }, // 14 - Weapon Energy
	{ SFX_0F, 635 }, // 15 - Jump
	{ SFX_10, 3175 }, // 16 - Take Damage
	{ SFX_11, 25397 }, // 17 - Die
	{ SFX_12, 6530 }, // 18 - Confirm
	{ NULL, 0 },
	{ SFX_14, 12700 }, // 20 - Refill
	{ SFX_15, 6350 }, // 21 - Bubble
	{ SFX_16, 2540 }, // 22 - Open Chest
	{ SFX_17, 1905 }, // 23 - Hit Ground
	{ SFX_18, 635 }, // 24 - Walking
	{ SFX_19, 12700 }, // 25 - Funny Explode
	{ SFX_1A, 14000 }, // 26 - Quake
	{ SFX_1B, 6350 }, // 27 - Level Up
	{ SFX_1C, 1905 }, // 28 - Shot Hit
	{ SFX_1D, 12700 }, // 29 - Teleport
	{ SFX_1E, 6350 }, // 30 - Critter Hop
	{ SFX_1F, 5080 }, // 31 - Shot Bounce
	{ SFX_20, 3175 }, // 32 - Polar Star
	{ SFX_21, 6350 }, // 33 - ????
	{ SFX_22, 2540 }, // 34 - Fireball
	{ SFX_23, 25397 },
	{ NULL, 0 },
	{ SFX_25, 2540 },
	{ SFX_26, 3810 },
	{ SFX_27, 3175 },
	{ SFX_28, 28000 },
	{ NULL, 0 },
	{ SFX_2A, 3175 }, // Get Missile
	{ SFX_2B, 1905 },
	{ SFX_2C, 12699 },
	{ SFX_2D, 3175 }, // 45 - Energy Bounce
	{ SFX_2E, 1270 },
	{ SFX_2F, 1270 },
	{ SFX_30, 5080 },
	{ SFX_31, 6350 }, // 49 - Polar star Lv 3
	{ SFX_32, 3810 },
	{ SFX_33, 6350 },
	{ SFX_34, 12700 },
	{ SFX_35, 6350 },
	{ SFX_36, 5080 },
	{ SFX_37, 3810 },
	{ SFX_38, 14000 }, // Splash
	{ SFX_39, 3175 },
	{ SFX_3A, 1270 },
	{ SFX_3B, 254 }, // Spur charge 1
	{ SFX_3C, 254 }, // 2
	{ SFX_3D, 254 }, // 3
	{ SFX_3E, 5080 },
	{ SFX_3F, 5080 },
	{ SFX_40, 5080 }, // 64 - Spur fire max
	{ SFX_41, 5080 }, // 65 - Spur full charge
	{ NULL, 0 },
	{ NULL, 0 },
	{ NULL, 0 },
	{ NULL, 0 },
	{ SFX_46, 6350 },
	{ SFX_47, 9524 },
	{ SFX_48, 13969 },
	{ NULL, 0 },
	{ NULL, 0 },
	{ NULL, 0 },
	{ NULL, 0 },
	{ NULL, 0 },
	{ NULL, 0 },
	{ NULL, 0 },
	// The following range 0x50 - 0x5F are remapped 0x90 - 0x9F
	{ NULL, 0 }, // 0x50 (80)
	{ NULL, 0 },
	{ NULL, 0 },
	{ NULL, 0 },
	{ NULL, 0 },
	{ NULL, 0 },
	{ SFX_96, 3175 },
	{ SFX_97, 6350 },
	{ SFX_98, 635 },
	{ SFX_99, 6350 },
	{ SFX_9A, 6350 },
	{ SFX_9B, 2540 },
	{ NULL, 0 },
	{ NULL, 0 },
	{ NULL, 0 },
	{ NULL, 0 },
	// End of remapped section
	{ NULL, 0 }, // 0x60 (96)
	{ NULL, 0 },
	{ NULL, 0 },
	{ NULL, 0 },
	{ SFX_64, 2540 },
	{ SFX_65, 39397 },
	{ SFX_66, 5747 },
	{ SFX_67, 14000 },
	{ SFX_68, 3175 },
	{ SFX_69, 3810 },
	{ SFX_6A, 6350 },
	{ SFX_6B, 6350 },
	{ SFX_6C, 6350 },
	{ SFX_6D, 2540 },
	{ SFX_6E, 2540 },
	{ SFX_6F, 1905 },
	{ SFX_70, 1905 }, // 0x70 (112)
	{ SFX_71, 1905 },
	{ SFX_72, 7620 },
	{ SFX_73, 25429 },
	{ SFX_74, 19048 },
	{ SFX_75, 6350 }, // 0x75 (117)
};
