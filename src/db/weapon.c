#include "tables.h"
#include "resources.h"

const weapon_info_def weapon_info[WEAPON_COUNT] = {
	{ NULL,       { 0, 0, 0}, "N/A" },
	{ &SPR_Polar, {10,20,30}, "Snake" },
	{ &SPR_Polar, {10,20,10}, "Polar Star" },
	{ &SPR_Fireb, {10,20,20}, "Fireball" },
	{ &SPR_MGun,  {30,40,10}, "Machine Gun" },
	{ &SPR_Missl, {10,20,10}, "Missiles" },
	{ NULL,       { 0, 0, 0}, "N/A" },
	{ &SPR_Polar, {10,20,30}, "Bubbler" },
	{ NULL,       { 0, 0, 0}, "N/A" },
	{ &SPR_Polar, {10,20,30}, "King's Blade" },
	{ &SPR_Missl, {10,20,10}, "Super Missiles" },
	{ NULL,       { 0, 0, 0}, "N/A" },
	{ &SPR_Polar, {10,20,30}, "Nemesis" },
	{ &SPR_Polar, {10,20,30}, "Spur" },
};
