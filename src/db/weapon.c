#include "tables.h"
#include "resources.h"

const weapon_info_def weapon_info[WEAPON_COUNT] = {
	{ NULL,       PAL0, { 0, 0, 0}, "N/A" },
	{ &SPR_Polar, PAL1, {10,20,30}, "Snake" },
	{ &SPR_Polar, PAL1, {10,20,10}, "Polar Star" },
	{ &SPR_Fireb, PAL1, {10,20,20}, "Fireball" },
	{ &SPR_MGun,  PAL0, {30,40,10}, "Machine Gun" },
	{ &SPR_Missl, PAL1, {10,20,10}, "Missiles" },
	{ NULL,       PAL0, { 0, 0, 0}, "N/A" },
	{ &SPR_Polar, PAL1, {10,20,30}, "Bubbler" },
	{ NULL,       PAL0, { 0, 0, 0}, "N/A" },
	{ &SPR_Polar, PAL1, {10,20,30}, "King's Blade" },
	{ &SPR_Missl, PAL0, {10,20,10}, "Super Missiles" },
	{ NULL,       PAL0, { 0, 0, 0}, "N/A" },
	{ &SPR_Polar, PAL0, {10,20,30}, "Nemesis" },
	{ &SPR_Polar, PAL1, {10,20,30}, "Spur" },
};
