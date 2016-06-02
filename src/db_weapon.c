#include "tables.h"
#include "resources.h"

const weapon_info_def weapon_info[WEAPON_COUNT] = {
	{ NULL, {NULL,NULL,NULL}, {0,0,0}, {0,0,0}, {0,0,0} },
	{ &SPR_Polar, {&SPR_PolarB1,&SPR_PolarB2,&SPR_PolarB3}, // 1 - Snake
		{0x20,0x20,0x31}, {10,20,30}, {1,2,4} },
	{ &SPR_Polar, {&SPR_PolarB1,&SPR_PolarB2,&SPR_PolarB3}, // 2 - Polar Star
		{0x20,0x20,0x31}, {10,20,30}, {1,2,4} },
	{ &SPR_Polar, {&SPR_PolarB1,&SPR_PolarB2,&SPR_PolarB3}, // 3 - Fireball
		{0x20,0x20,0x31}, {10,20,30}, {1,2,4} },
	{ &SPR_Polar, {&SPR_PolarB1,&SPR_PolarB2,&SPR_PolarB3}, // 4 - Machine Gun
		{0x20,0x20,0x31}, {10,20,30}, {1,2,4} },
	{ &SPR_Polar, {&SPR_PolarB1,&SPR_PolarB2,&SPR_PolarB3}, // 5 - Missile
		{0x20,0x20,0x31}, {10,20,30}, {3,6,9} },
	{ NULL, {NULL,NULL,NULL}, {0,0,0}, {0,0,0}, {0,0,0} },
	{ &SPR_Polar, {&SPR_PolarB1,&SPR_PolarB2,&SPR_PolarB3}, // 7 - Bubbler
		{0x20,0x20,0x31}, {10,20,30}, {1,2,4} },
	{ NULL, {NULL,NULL,NULL}, {0,0,0}, {0,0,0}, {0,0,0} },
	{ &SPR_Polar, {&SPR_PolarB1,&SPR_PolarB2,&SPR_PolarB3}, // 9 - King's Sword
		{0x20,0x20,0x31}, {10,20,30}, {1,2,4} },
	{ &SPR_Polar, {&SPR_PolarB1,&SPR_PolarB2,&SPR_PolarB3}, // 10 - Super Missile
		{0x20,0x20,0x31}, {10,20,30}, {1,2,4} },
	{ NULL, {NULL,NULL,NULL}, {0,0,0}, {0,0,0}, {0,0,0} },
	{ &SPR_Polar, {&SPR_PolarB1,&SPR_PolarB2,&SPR_PolarB3}, // 12 - Nemesis
		{0x20,0x20,0x31}, {10,20,30}, {1,2,4} },
	{ &SPR_Polar, {&SPR_PolarB1,&SPR_PolarB2,&SPR_PolarB3}, // 13 - Spur
		{0x20,0x20,0x31}, {10,20,30}, {4,4,4} },
};
