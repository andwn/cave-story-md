#ifndef INC_AI_COMMON_H_
#define INC_AI_COMMON_H_

#include "common.h"
#include "entity.h"

#define STATE_DEFEATED 900

// Entity behavior and AI
void ai_activate_base(Entity *e);
void ai_update_base(Entity *e);
bool ai_setstate_base(Entity *e, u16 state);

#endif /* INC_AI_COMMON_H_ */
