#ifndef INC_NPC_H_
#define INC_NPC_H_

#include "common.h"

#define NPC_COUNT 361

u16 npc_flags(u16 type);
u16 npc_health(u16 type);
u8 npc_palette(u16 type);
u8 npc_hurtSound(u16 type);
u8 npc_deathSound(u16 type);
u8 npc_deathSmoke(u16 type);
u16 npc_experience(u16 type);
u16 npc_attack(u16 type);
bounding_box npc_hitBox(u16 type);
bounding_box npc_displayBox(u16 type);

#endif /* INC_NPC_H_ */
