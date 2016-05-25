#ifndef INC_EFFECT_H_
#define INC_EFFECT_H_

#include "common.h"

void effects_clear();
void effects_update();

void effect_create_damage(s16 num, s16 x, s16 y, u8 ttl);
void effect_create_smoke(u8 type, s16 x, s16 y);

#endif /* INC_EFFECT_H_ */
