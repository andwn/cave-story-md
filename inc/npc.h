#ifndef INC_NPC_H_
#define INC_NPC_H_

#include "common.h"

/*
 * Cave Story has a file named npc.tbl that contains a bunch of information about each
 * NPC Type (361 of them). This file is included into the ROM without modification,
 * and this module is used to grab the values needed from it
 */

#define NPC_COUNT 361

// NPC Flags
// Player can't pass through the NPC
#define NPC_SOLID			0x0001
// Ignore tile type 0x44. Normally NPCs cannot pass them
#define NPC_IGNORE44		0x0002
// Can't be damaged and makes a *clink* sound
#define NPC_INVINCIBLE		0x0004
// Goes through walls, used on flying enemies
#define NPC_IGNORESOLID		0x0008
// I think only the Behemoth enemy uses this
#define NPC_BOUNCYTOP		0x0010
// Can be damaged with player bullets
#define NPC_SHOOTABLE		0x0020
// This seems to be like a normal solid but allows penetrating just 1 pixel into the hitbox
#define NPC_SPECIALSOLID	0x0040
// Touching the bottom, top, and back does not hurt
#define NPC_FRONTATKONLY	0x0080
// Custom, depends on the NPC type
#define NPC_OPTION1			0x0100
// Trigger event [Entity.event] on death
#define NPC_EVENTONDEATH	0x0200
// Drops health/energy/missiles
#define NPC_DROPPOWERUP		0x0400
// Only appears if the flag [Entity.id] is set
#define NPC_ENABLEONFLAG	0x0800
// Custom, depends on the NPC type
#define NPC_OPTION2			0x1000
// Calls event [Entity.event] when player presses down
#define NPC_INTERACTIVE		0x2000
// Only appears if the flag [Entity.id] is unset
#define NPC_DISABLEONFLAG	0x4000
// Pops up red numbers when damaged
#define NPC_SHOWDAMAGE		0x8000

// NPC Flags
u16 npc_flags(u16 type);
// Starting health
u16 npc_health(u16 type);
// This is actually an index of which file contains the sprites.. don't use it
u8 npc_palette(u16 type);
// Sound to make when the NPC is damaged by player's weapon
u8 npc_hurtSound(u16 type);
// Sound to play when defeated
u8 npc_deathSound(u16 type);
// Type of smoke effect to create when defeated (small, mid, large)
u8 npc_deathSmoke(u16 type);
// Amount of weapon energy that is dropped
u16 npc_experience(u16 type);
// Damage to player when colliding
u16 npc_attack(u16 type);
// Area relative to the center where collision may take place
bounding_box npc_hitBox(u16 type);
// Area relative to the center where the sprite is displayed
bounding_box npc_displayBox(u16 type);

#endif /* INC_NPC_H_ */
