#include "ai.h"

#include "npc.h"

void generic_npc_states(Entity *e) {
	switch(e->state) {
		case 0:		// stand
		{
			e->frame = 0;
			e->x_speed = 0;
			e->y_speed = 0;
			if(e->type != OBJ_KAZUMA) {
				RANDBLINK(e, 3, 200);
			}
		}
		break;
		case 3:		// walking
		case 4:
		{
			ANIMATE(e, 8, 1,0,2,0);
			MOVE_X(SPEED(0x200));
		}
		break;
		case 5:		// face away
		{
			e->frame = e->type == OBJ_KAZUMA ? 3 : 4;
			e->x_speed = 0;
		}
		break;
		case 8:		// walk (alternate state used by OBJ_NPC_JACK)
		{
			if (e->type == OBJ_JACK) {
				e->state = 4;
				e->frame = 1;
			}
		}
		break;
	}
}

static const u32 tan_table[33] = {
	0,
	402,
	806,
	1215,
	1629,
	2051,
	2485,
	2931,
	3393,
	3874,
	4378,
	4910,
	5473,
	6075,
	6723,
	7424,
	8192,
	9038,
	9981,
	11045,
	12260,
	13667,
	15326,
	17320,
	19777,
	22895,
	27005,
	32704,
	41183,
	55225,
	83174,
	166752,
	// It is impossible for this to be less than anything, guaranteed to break the while loop
	0xFFFFFFFF, 
};

// My brain hurts... don't read this
u8 get_angle(s32 curx, s32 cury, s32 tgtx, s32 tgty) {
	u8 angle = 32;
	s32 xdist = (tgtx - curx);
	s32 ydist = (tgty - cury);
	if (xdist==0) return (tgty > cury) ? 0x40 : 0xC0; // Undefined
	u32 ratio = (abs(ydist) << 13) / abs(xdist);
	while(tan_table[33 - (angle--)] < ratio);
	angle += 1;
	angle <<= 1;
	if (curx > tgtx) angle = 0x100 - angle;
	if (cury > tgty) angle = 0x80 - angle;
	return angle;
}
