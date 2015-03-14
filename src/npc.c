#include "npc.h"

#include <genesis.h>
#include "resources.h"

#define FLAGS_INDEX 0
#define HEALTH_INDEX (NPC_COUNT * 2)
#define PALETTE_INDEX (NPC_COUNT * 4)
#define HURTSOUND_INDEX (NPC_COUNT * 5)
#define DEATHSOUND_INDEX (NPC_COUNT * 6)
#define DEATHSMOKE_INDEX (NPC_COUNT * 7)
#define ENERGY_INDEX (NPC_COUNT * 8)
#define ATTACK_INDEX (NPC_COUNT * 12)
#define HITBOX_INDEX (NPC_COUNT * 16)
#define DISPLAYBOX_INDEX (NPC_COUNT * 20)

u16 npc_flags(u16 type) {
	return NPC_TABLE[type * 2 + FLAGS_INDEX] +
			(NPC_TABLE[type * 2 + FLAGS_INDEX + 1] << 8);
}

u16 npc_health(u16 type) {
	return NPC_TABLE[type * 2 + HEALTH_INDEX] +
			(NPC_TABLE[type * 2 + HEALTH_INDEX + 1] << 8);
}

u8 npc_palette(u16 type) {
	return NPC_TABLE[type * PALETTE_INDEX];
}

u8 npc_hurtSound(u16 type) {
	return NPC_TABLE[type * HURTSOUND_INDEX];
}

u8 npc_deathSound(u16 type) {
	return NPC_TABLE[type * DEATHSOUND_INDEX];
}

u8 npc_deathSmoke(u16 type) {
	return NPC_TABLE[type * DEATHSMOKE_INDEX];
}

u16 npc_experience(u16 type) {
	return NPC_TABLE[type * 4 + ENERGY_INDEX] +
			(NPC_TABLE[type * 4 + ENERGY_INDEX + 1] << 8);
}

u16 npc_attack(u16 type) {
	return NPC_TABLE[type * 4 + ATTACK_INDEX] +
			(NPC_TABLE[type * 4 + ATTACK_INDEX + 1] << 8);
}

bounding_box npc_hitBox(u16 type) {
	bounding_box b = {
			NPC_TABLE[type * 4 + HITBOX_INDEX],
			NPC_TABLE[type * 4 + HITBOX_INDEX + 1],
			NPC_TABLE[type * 4 + HITBOX_INDEX + 2],
			NPC_TABLE[type * 4 + HITBOX_INDEX + 3]
	};
	return b;
}

bounding_box npc_displayBox(u16 type) {
	bounding_box b = {
			NPC_TABLE[type * 4 + DISPLAYBOX_INDEX],
			NPC_TABLE[type * 4 + DISPLAYBOX_INDEX + 1],
			NPC_TABLE[type * 4 + DISPLAYBOX_INDEX + 2],
			NPC_TABLE[type * 4 + DISPLAYBOX_INDEX + 3]
	};
	return b;
}

