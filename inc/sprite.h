#ifndef INC_SPRITE_H_
#define INC_SPRITE_H_

#include <genesis.h>
#include "common.h"

#define SPRITE_NONE 0xFF

void sprites_init();
void sprites_clear();
void sprites_update();

u8 sprite_create(const SpriteDefinition *def, u16 palette, bool priority);
void sprite_set_tileindex(u8 sprite, u16 index);
void sprite_set_position(u8 sprite, s16 x, s16 y);
void sprite_set_attr(u8 sprite, u16 attr);
void sprite_set_visible(u8 sprite, bool visible);
void sprite_set_animation(u8 sprite, u8 anim);
void sprite_set_animframe(u8 sprite, u8 anim, u8 frame);
void sprite_set_frame(u8 sprite, u8 frame);
void sprite_delete(u8 sprite);
void sprite_set_direction(u8 sprite, u8 direction);
Sprite* sprite_get_direct(u8 sprite);

#endif /* INC_SPRITE_H_ */
