#ifndef INC_SPRITE_H_
#define INC_SPRITE_H_

#include <genesis.h>
#include "common.h"

/**
 * Get tileset from SpriteDefinition
 */
#define SPR_TILESET(spr, a, f) (spr.animations[a]->frames[f]->tileset)

/**
 * Clear the sprite list. Call at the beginning of a frame before adding sprites
 */
void sprites_clear();

/**
 * Queues DMA transfer of sprite list. Call before VDP_waitVSync() or in VInt
 */
void sprites_update();

/**
 * Add data to the sprite list. Call for each sprite to be displayed per frame
 */
void sprite_add(s16 x, s16 y, u16 attr, u8 size);

#endif /* INC_SPRITE_H_ */
