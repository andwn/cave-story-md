#include "ai.h"

#include <genesis.h>
#include "audio.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "tsc.h"

// Kazuma appears 2 blocks above the computer
void ai_kazuComp_onCreate(Entity *e) {
	e->y += pixel_to_sub(16);
}

// Cycle through animations
void ai_kazuComp_onUpdate(Entity *e) {
	if((++e->state_time % 30 == 1) && (random() % 4 == 0)) {
		s16 newAnim = e->sprite->animInd + 1;
		if(newAnim >= 3) newAnim = 0;
		SPR_SAFEANIM(e->sprite, newAnim);
		e->state_time = 0;
	}
}
