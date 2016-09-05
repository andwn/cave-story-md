#ifndef INC_SPRITE_H_
#define INC_SPRITE_H_

#include <genesis.h>

#define MAX_SPRITE	80

u8 spr_num;
VDPSprite sprites[MAX_SPRITE];

// Append sprite to the end of the list, order front -> back
#define sprite_add(s) {                                                                        \
	if(spr_num < MAX_SPRITE) {                                                                 \
		s.link = spr_num+1;                                                                    \
		sprites[spr_num++] = s;                                                                \
	}                                                                                          \
}

#define sprite_pos(s, px, py) {                                                                \
	(s)->x = (px) + 128;                                                                       \
	(s)->y = (py) + 128;                                                                       \
}

#define sprites_clear() spr_num = 0

// Send sprite list to VDP, first element always sent
#define sprites_send() {                                                                       \
	if(spr_num < MAX_SPRITE) sprite[spr_num-1].link = 0;                                       \
	DMA_doDma(DMA_VRAM, (u32)sprite, VDP_SPRITE_TABLE, max(spr_num*4, 4), 2);                  \
}

#endif
