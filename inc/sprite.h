#ifndef INC_SPRITE_H_
#define INC_SPRITE_H_

#include <genesis.h>

u8 spr_num;
VDPSprite sprites[MAX_VDP_SPRITE];

// Append sprite to the end of the list, order front -> back
#define sprite_add(s) {                                                                        \
	if(spr_num < MAX_VDP_SPRITE) {                                                             \
		sprites[spr_num] = s;                                                                  \
		sprites[spr_num].link = spr_num+1;                                                     \
		spr_num++;                                                                             \
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

#define sprite_pri(s, pri) { (s).attribut &= ~(1<<13); (s).attribut |= (pri) << 15; }
#define sprite_pal(s, pal) { (s).attribut &= ~(3<<13); (s).attribut |= (pal) << 13; }
#define sprite_vflip(s, flip) { (s).attribut &= ~(1<<12); (s).attribut |= (flip) << 12; }
#define sprite_hflip(s, flip) { (s).attribut &= ~(1<<11); (s).attribut |= (flip) << 11; }
#define sprite_index(s, index) { (s).attribut &= ~0x3FF; (s).attribut |= (index); }

// Send sprite list to VDP
#define sprites_send() {                                                                       \
	if(spr_num) {                                                                              \
		sprites[spr_num-1].link = 0;                                                           \
		DMA_doDma(DMA_VRAM, (u32)sprites, VDP_SPRITE_TABLE, spr_num*4, 2);                     \
		spr_num = 0;                                                                           \
	}                                                                                          \
}

#endif
