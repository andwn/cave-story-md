#include "common.h"

#include "resources.h"

#include "tables.h"

const sound_info_def sound_info[SOUND_COUNT] = {
	{ NULL, 0 },
	{ SFX_01, 1825 }, // 1 - Cursor
	{ SFX_02, 2391 }, // 2 - Message
	{ SFX_03, 1655 }, // 3 - Head Bonk
	{ SFX_04, 4530 }, // 4 - Switch Weapon
	{ SFX_05, 3375 }, // 5 - Prompt
	{ SFX_06, 2998 }, // 6 - Hop
	{ NULL, 0 },
	{ SFX_08, 12284 }, // 8 - ????
	{ NULL, 0 },
	{ NULL, 0 },
	{ SFX_0B, 2995 }, // 11 - Door Open
	{ SFX_0C, 5705 }, // 12 - Destroy Block
	{ NULL, 0 },
	{ SFX_0E, 5778 }, // 14 - Weapon Energy
	{ SFX_0F, 615 }, // 15 - Jump
	{ SFX_10, 3095 }, // 16 - Take Damage
	{ SFX_11, 24983 }, // 17 - Die
	{ SFX_12, 6045 }, // 18 - Confirm
	{ NULL, 0 },
	{ SFX_14, 11543 }, // 20 - Refill
	{ SFX_15, 5358 }, // 21 - Bubble
	{ SFX_16, 2479 }, // 22 - Open Chest
	{ SFX_17, 1905 }, // 23 - Hit Ground
	{ SFX_18, 635 }, // 24 - Walking
	{ SFX_19, 12127 }, // 25 - Funny Explode
	{ SFX_1A, 12324 }, // 26 - Quake
	{ SFX_1B, 6234 }, // 27 - Level Up
	{ SFX_1C, 1877 }, // 28 - Shot Hit
	{ SFX_1D, 12224 }, // 29 - Teleport
	{ SFX_1E, 5899 }, // 30 - Critter Hop
	{ SFX_1F, 3756 }, // 31 - Shot Bounce
	{ SFX_20, 3041 }, // 32 - Polar Star
	{ SFX_21, 5710 }, // 33 - ????
	{ SFX_22, 2408 }, // 34 - Fireball
	{ SFX_23, 24935 },
	{ NULL, 0 },
	{ SFX_25, 2491 },
	{ SFX_26, 3572 },
	{ SFX_27, 2995 },
	{ SFX_28, 28000 },
	{ NULL, 0 },
	{ SFX_2A, 3175 }, // Get Missile
	{ SFX_2B, 1905 },
	{ SFX_2C, 12698 },
	{ SFX_2D, 2609 }, // 45 - Energy Bounce
	{ SFX_2E, 1035 },
	{ SFX_2F, 1156 },
	{ SFX_30, 4495 },
	{ SFX_31, 5826 }, // 49 - Polar star Lv 3
	{ SFX_32, 3810 },
	{ SFX_33, 6222 },
	{ SFX_34, 12151 },
	{ SFX_35, 5966 },
	{ SFX_36, 4846 },
	{ SFX_37, 3747 },
	{ SFX_38, 9642 }, // Splash
	{ SFX_39, 3102 },
	{ SFX_3A, 1270 },
	{ SFX_3B, 256 }, // Spur charge 1
	{ SFX_3C, 256 }, // 2
	{ SFX_3D, 256 }, // 3
	{ SFX_3E, 4972 },
	{ SFX_3F, 4968 },
	{ SFX_40, 4997 }, // 64 - Spur fire max
	{ SFX_41, 4700 }, // 65 - Spur full charge
	{ NULL, 0 },
	{ NULL, 0 },
	{ NULL, 0 },
	{ NULL, 0 },
	{ SFX_46, 5881 },
	{ SFX_47, 9031 },
	{ SFX_48, 13299 },
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
	{ SFX_96, 3108 },
	{ SFX_97, 5662 },
	{ SFX_98, 611 },
	{ SFX_99, 6349 },
	{ SFX_9A, 5735 },
	{ SFX_9B, 2306 },
	{ NULL, 0 },
	{ NULL, 0 },
	{ NULL, 0 },
	{ NULL, 0 },
	// End of remapped section
	{ NULL, 0 }, // 0x60 (96)
	{ NULL, 0 },
	{ NULL, 0 },
	{ NULL, 0 },
	{ SFX_64, 2291 },
	{ SFX_65, 37698 },
	{ SFX_66, 5686 },
	{ SFX_67, 13557 },
	{ SFX_68, 2962 },
	{ SFX_69, 3390 },
	{ SFX_6A, 5796 },
	{ SFX_6B, 6094 },
	{ SFX_6C, 6100 },
	{ SFX_6D, 2289 },
	{ SFX_6E, 2250 },
	{ SFX_6F, 1905 },
	{ SFX_70, 1843 }, // 0x70 (112)
	{ SFX_71, 1828 },
	{ SFX_72, 7415 },
	{ SFX_73, 23919 },
	{ SFX_74, 18956 },
	{ SFX_75, 6063 }, // 0x75 (117)
};
