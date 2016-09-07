#ifndef INC_SPRITE_H_
#define INC_SPRITE_H_

#include <genesis.h>

u8 spr_num;
VDPSprite sprites[MAX_VDP_SPRITE];

// Append sprite to the end of the list, order front -> back
#define sprite_add(s) {                                                                        \
	if(spr_num < MAX_VDP_SPRITE) {                                                             \
		s.link = spr_num+1;                                                                    \
		sprites[spr_num++] = s;                                                                \
	}                                                                                          \
}

// Perform sprite_add against an array of VDPSprites
#define sprite_addq(arr, num) {                                                                \
	for(u8 i = num; i--; ) sprite_add(arr[i]);                                                 \
}

#define sprite_pos(s, px, py) {                                                                \
	(s).x = (px) + 128;                                                                        \
	(s).y = (py) + 128;                                                                        \
}

// Send sprite list to VDP
#define sprites_send() {                                                                       \
	if(spr_num) {                                                                              \
		sprites[spr_num-1].link = 0;                                                           \
		DMA_doDma(DMA_VRAM, (u32)sprites, VDP_SPRITE_TABLE, spr_num*4, 2);                     \
		spr_num = 0;                                                                           \
	}                                                                                          \
}

#endif
