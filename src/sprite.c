#include "sprite.h"

// Hardware sprite
typedef struct {
	s16 y; // end 9 bits
	u8 size; // end 4 bits
	u8 link; // end 7 bits
	u16 attr;
	s16 x; // end 9 bits
} MDSprite;

u8 spriteCount;
MDSprite spriteList[MAX_SPRITE];

void sprites_clear() {
	spriteCount = 0;
}

void sprites_update() {
	if(spriteCount) {
		DMA_queueDma(DMA_VRAM, (u32) spriteList, VDP_getSpriteListAddress(), 
			(spriteCount * sizeof(MDSprite)) >> 1, 2);
	}
}

void sprite_add(s16 x, s16 y, u16 attr, u8 size) {
	if(spriteCount >= MAX_SPRITE) return;
	// Make sure sprite is on screen
	if(x < -32 || x >= 320 || y < -32 || y >= 224) return;
	// Screen top left is 128x128
	spriteList[spriteCount].x = x + 128;
	spriteList[spriteCount].y = y + 128;
	spriteList[spriteCount].size = size;
	spriteList[spriteCount].link = 0; // 0 is end of the chain
	spriteList[spriteCount].attr = attr;
	// Link previous sprite to this one
	if(spriteCount > 0) spriteList[spriteCount - 1].link = spriteCount;
	spriteCount++;
}
