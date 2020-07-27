#include "common.h"

#include "resources.h"
#include "vdp.h"
#include "weapon.h"

#include "tables.h"

const weapon_info_def weapon_info[WEAPON_COUNT] = {
	{ NULL,       PAL0, { 0, 0, 0} },
	{ &SPR_Snake, PAL1, {10,20,10} }, // Snake
	{ &SPR_Polar, PAL1, {10,20,10} }, // Polar Star
	{ &SPR_Fireb, PAL1, {10,20,20} }, // Fireball
	{ &SPR_MGun,  PAL0, {30,40,10} }, // Machine Gun
	{ &SPR_Missl, PAL1, {10,20,10} }, // Missiles
	{ NULL,       PAL0, { 0, 0, 0} },
	{ &SPR_Bubbl, PAL1, {10,20,10} }, // Bubbler
	{ NULL,       PAL0, { 0, 0, 0} }, // N/A - but used for Blade AOE
	{ &SPR_Polar, PAL1, {10,20,10} }, // Blade
	{ &SPR_Missl2,PAL0, {10,20,10} }, // Super Missiles
	{ NULL,       PAL0, { 0, 0, 0} },
	{ &SPR_Nemes, PAL0, {10,20,10} }, // Nemesis
	{ &SPR_Polar, PAL1, {10,20,30} }, // Spur
};
