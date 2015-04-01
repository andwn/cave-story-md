#ifndef INC_AI_BALROG_H_
#define INC_AI_BALROG_H_

#include "common.h"
#include "entity.h"
#include "ai_common.h"

// 12 - Balrog (Cutscene)
void ai_update_balrog_scene(Entity *e);
bool ai_setstate_balrog_scene(Entity *e, u16 state);
// 19 - Balrog (Busts In)
// 9 - Balrog (Drops In)

// 68 - Boss: Balrog (Mimiga Village)
void ai_update_balrog_boss1(Entity *e);
bool ai_setstate_balrog_boss1(Entity *e, u16 state);
// 10 - Boss: Balrog (Grasstown)
// Balfrog
// Balrog (Missiles)

#endif /* INC_AI_BALROG_H_ */
