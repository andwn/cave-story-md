#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

// Cycle through animations
void ai_kazuComp_onUpdate(Entity *e) {
	if((++e->state_time % 30 == 1) && (random() % 4 == 0)) {
		s16 newAnim = e->sprite->animInd + 1;
		if(newAnim >= 3) newAnim = 0;
		SPR_SAFEANIM(e->sprite, newAnim);
		e->state_time = 0;
	}
}
