#ifndef INC_AI_MIMIGA_H_
#define INC_AI_MIMIGA_H_

#include "common.h"
#include "entity.h"
#include "ai_common.h"

// 60 and 63 - Toroko / Toroko attack with stick
void ai_update_toroko(Entity *e);
//void ai_setstate_toroko(Entity *e, u16 state);

// 67 - Misery (float)
void ai_update_misery_float(Entity *e);
bool ai_setstate_misery_float(Entity *e, u16 state);

#endif /* INC_AI_MIMIGA_H_ */
