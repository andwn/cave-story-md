#include "sprite.h"

#define MAX_SPRITES 80

Sprite sprites[MAX_SPRITES];
bool spriteEnabled[MAX_SPRITES];

void sprites_init() {
	SPR_init(0);
	sprites_clear();
}

void sprites_clear() {
	SPR_clear();
	for(u8 i = 0; i < MAX_SPRITES; i++) {
		SPR_setNeverVisible(&sprites[i], true);
		spriteEnabled[i] = false;
	}
}

void sprites_update() {
	SPR_update(sprites, MAX_SPRITES);
}

u8 sprite_create(const SpriteDefinition *def, u16 palette, bool priority) {
	for(u8 i = 0; i < MAX_SPRITES; i++) {
		if(spriteEnabled[i]) continue;
		SPR_initSprite(&sprites[i], def, 0, 0,
				TILE_ATTR(palette, priority, false, false));
		spriteEnabled[i] = true;
		return i;
	}
	return SPRITE_NONE;
}

void sprite_set_tileindex(u8 sprite, u16 index) {
	SPR_setVRAMTileIndex(&sprites[sprite], index);
}

void sprite_set_position(u8 sprite, s16 x, s16 y) {
	if(sprite >= MAX_SPRITES || !spriteEnabled[sprite]) return;
	SPR_setPosition(&sprites[sprite], x, y);
}

void sprite_set_attr(u8 sprite, u16 attr) {
	if(sprite >= MAX_SPRITES || !spriteEnabled[sprite]) return;
	SPR_setAttribut(&sprites[sprite], attr);
}

void sprite_set_visible(u8 sprite, bool visible) {
	if(sprite >= MAX_SPRITES || !spriteEnabled[sprite]) return;
	SPR_setNeverVisible(&sprites[sprite], !visible);
}

void sprite_set_animation(u8 sprite, u8 anim) {
	if(sprite >= MAX_SPRITES || !spriteEnabled[sprite]) return;
	SPR_setAnim(&sprites[sprite], anim);
}

void sprite_set_animframe(u8 sprite, u8 anim, u8 frame) {
	if(sprite >= MAX_SPRITES || !spriteEnabled[sprite]) return;
	SPR_setAnimAndFrame(&sprites[sprite], anim, frame);
}

void sprite_set_frame(u8 sprite, u8 frame) {
	if(sprite >= MAX_SPRITES || !spriteEnabled[sprite]) return;
	SPR_setFrame(&sprites[sprite], frame);
}

void sprite_delete(u8 sprite) {
	if(sprite >= MAX_SPRITES || !spriteEnabled[sprite]) return;
	SPR_setNeverVisible(&sprites[sprite], true);
	spriteEnabled[sprite] = false;
}

void sprite_set_direction(u8 sprite, u8 direction) {
	u16 attr = sprites[sprite].attribut;
	if(direction > 0) attr |= TILE_ATTR_HFLIP_MASK;
	else attr &= ~TILE_ATTR_HFLIP_MASK;
	SPR_setAttribut(&sprites[sprite], attr);
}

Sprite* sprite_get_direct(u8 sprite) {
	return &sprites[sprite];
}
