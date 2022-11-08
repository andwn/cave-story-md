#include "md/types.h"
#include "res/sfx.h"
#include "tables.h"

const sfx_info_def sfx_info[SOUND_COUNT] = {
    { NULL, 0 },
    { SFX_01, SFX_01_end }, // 1 - Cursor
    { SFX_02, SFX_02_end }, // 2 - Message
    { SFX_03, SFX_03_end }, // 3 - Head Bonk
    { SFX_04, SFX_04_end }, // 4 - Switch Weapon
    { SFX_05, SFX_05_end }, // 5 - Prompt
    { SFX_06, SFX_06_end }, // 6 - Hop
    { NULL, 0 },
    { SFX_08, SFX_08_end }, // 8 - ????
    { NULL, 0 },
    { NULL, 0 },
    { SFX_0B, SFX_0B_end }, // 11 - Door Open
    { SFX_0C, SFX_0C_end }, // 12 - Destroy Block
    { NULL, 0 },
    { SFX_0E, SFX_0E_end }, // 14 - Weapon Energy
    { SFX_0F, SFX_0F_end }, // 15 - Jump
    { SFX_10, SFX_10_end }, // 16 - Take Damage
    { SFX_11, SFX_11_end }, // 17 - Die
    { SFX_12, SFX_12_end }, // 18 - Confirm
    { NULL, 0 },
    { SFX_14, SFX_14_end }, // 20 - Refill
    { SFX_15, SFX_15_end }, // 21 - Bubble
    { SFX_16, SFX_16_end }, // 22 - Open Chest
    { SFX_17, SFX_17_end }, // 23 - Hit Ground
    { SFX_18, SFX_18_end }, // 24 - Walking
    { SFX_19, SFX_19_end }, // 25 - Funny Explode
    { SFX_1A, SFX_1A_end }, // 26 - Quake
    { SFX_1B, SFX_1B_end }, // 27 - Level Up
    { SFX_1C, SFX_1C_end }, // 28 - Shot Hit
    { SFX_1D, SFX_1D_end }, // 29 - Teleport
    { SFX_1E, SFX_1E_end }, // 30 - Critter Hop
    { SFX_1F, SFX_1F_end }, // 31 - Shot Bounce
    { SFX_20, SFX_20_end }, // 32 - Polar Star
    { SFX_21, SFX_21_end }, // 33 - ????
    { SFX_22, SFX_22_end }, // 34 - Fireball
    { SFX_23, SFX_23_end },
    { NULL, 0 },
    { SFX_25, SFX_25_end },
    { SFX_26, SFX_26_end },
    { SFX_27, SFX_27_end },
    { SFX_28, SFX_28_end },
    { NULL, 0 },
    { SFX_2A, SFX_2A_end }, // Get Missile
    { SFX_2B, SFX_2B_end },
    { SFX_2C, SFX_2C_end },
    { SFX_2D, SFX_2D_end }, // 45 - Energy Bounce
    { SFX_2E, SFX_2E_end },
    { SFX_2F, SFX_2F_end },
    { SFX_30, SFX_30_end },
    { SFX_31, SFX_31_end }, // 49 - Polar star Lv 3
    { SFX_32, SFX_32_end },
    { SFX_33, SFX_33_end },
    { SFX_34, SFX_34_end },
    { SFX_35, SFX_35_end },
    { SFX_36, SFX_36_end },
    { SFX_37, SFX_37_end },
    { SFX_38, SFX_38_end }, // Splash
    { SFX_39, SFX_39_end },
    { SFX_3A, SFX_3A_end },
    { SFX_3B, SFX_3B_end }, // Spur charge 1
    { SFX_3C, SFX_3C_end }, // 2
    { SFX_3D, SFX_3D_end }, // 3
    { SFX_3E, SFX_3E_end },
    { SFX_3F, SFX_3F_end },
    { SFX_40, SFX_40_end }, // 64 - Spur fire max
    { SFX_41, SFX_41_end }, // 65 - Spur full charge
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { SFX_46, SFX_46_end },
    { SFX_47, SFX_47_end },
    { SFX_48, SFX_48_end },
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
    { NULL, 0 },//{ SFX_96, SFX_96_end },
    { NULL, 0 },//{ SFX_97, SFX_97_end },
    { NULL, 0 },//{ SFX_98, SFX_98_end },
    { NULL, 0 },//{ SFX_99, SFX_99_end },
    { NULL, 0 },//{ SFX_9A, SFX_9A_end },
    { NULL, 0 },//{ SFX_9B, SFX_9B_end },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    // End of remapped section
    { NULL, 0 }, // 0x60 (96)
    { NULL, 0 },
    { NULL, 0 },
    { NULL, 0 },
    { SFX_64, SFX_64_end },
    { SFX_65, SFX_65_end },
    { SFX_66, SFX_66_end },
    { SFX_67, SFX_67_end },
    { SFX_68, SFX_68_end },
    { SFX_69, SFX_69_end },
    { SFX_6A, SFX_6A_end },
    { SFX_6B, SFX_6B_end },
    { SFX_6C, SFX_6C_end },
    { SFX_6D, SFX_6D_end },
    { SFX_6E, SFX_6E_end },
    { SFX_6F, SFX_6F_end },
    { SFX_70, SFX_70_end }, // 0x70 (112)
    { SFX_71, SFX_71_end },
    { SFX_72, SFX_72_end },
    { SFX_73, SFX_73_end },
    { SFX_74, SFX_74_end },
    { SFX_75, SFX_75_end }, // 0x75 (117)
};
