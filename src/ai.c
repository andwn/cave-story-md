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

static u16 atan2(s32 y, s32 x) {
    if (x == y) { // x/y or y/x would return -1 since 1 isn't representable
        if (y > 0) { // 1/8
            return 0x80;
        } else if (y < 0) { // 5/8
            return 0x280;
        } else { // x = y = 0
            return 0;
        }
    }
    const s16 nabs_y = -abs(y), nabs_x = -abs(x);
    if (nabs_x < nabs_y) { // octants 1, 4, 5, 8
        const s16 y_over_x = (y << CSF) / x;
        const s16 correction = (879 * -abs(y_over_x)) >> CSF;
        const s16 unrotated = ((256 + 879 + correction) * y_over_x) >> CSF;
        if (x > 0) { // octants 1, 8
            return unrotated;
        } else { // octants 4, 5
            return 0x200 + unrotated;
        }
    } else { // octants 2, 3, 6, 7
        const s16 x_over_y = (x << CSF) / y;
        const s16 correction = (879 * -abs(x_over_y)) >> CSF;
        const s16 unrotated = ((256 + 879 + correction) * x_over_y) >> CSF;
        if (y > 0) { // octants 2, 3
            return 0x100 - unrotated;
        } else { // octants 6, 7
            return 0x300 - unrotated;
        }
    }
}

u16 get_angle(s32 curx, s32 cury, s32 tgtx, s32 tgty) {
	s32 xdist = (tgtx - curx);
	s32 ydist = (tgty - cury);
	if (xdist==0) return (tgty > cury) ? 0x100 : 0x300; // Undefined?
	return atan2(ydist, xdist);
}
