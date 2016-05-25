#include "effect.h"

#include <genesis.h>
#include "tables.h"
#include "sprite.h"
#include "resources.h"
#include "camera.h"
#include "vdp_ext.h"

#define MAX_EFFECTS 10
enum { EFF_DAMAGE, EFF_SMOKE };

typedef struct {
	u8 type, ttl;
	s16 x, y;
	u16 attr;
} Effect;

Effect effect[MAX_EFFECTS];

// Internal functions
void damage_update(Effect *eff);
void smoke_update(Effect *eff);

void effects_clear() {
	for(u8 i = 0; i < MAX_EFFECTS; i++) effect[i].ttl = 0;
}

void effects_update() {
	for(u8 i = 0; i < MAX_EFFECTS; i++) {
		if(effect[i].ttl == 0 || --effect[i].ttl == 0) continue;
		switch(effect[i].type) {
			case EFF_DAMAGE:
			damage_update(&effect[i]);
			break;
			case EFF_SMOKE:
			smoke_update(&effect[i]);
			break;
		}
	}
}

void effect_create_damage(s16 num, s16 x, s16 y, u8 ttl) {
	for(u8 i = 0; i < MAX_EFFECTS; i++) {
		if(effect[i].ttl > 0) continue;
		// Negative numbers are red and show '-' (Damage)
		// Positive are white and show '+' (Weapon energy)
		// Tiles need to be indexed with a positive, but remember if it was negative
		bool negative = num < 0;
		num = abs(num);
		u8 digitCount = 0; // Number of digit tiles: 1, 2, or 3 after loop
		u32 tiles[4][4] = {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};
		PUT_TILE(tiles[0], SPR_TILESET(SPR_Numbers, negative, 10, 0)->tiles); // - or +
		for(; num; digitCount++) {
			PUT_TILE(tiles[digitCount+1], SPR_TILESET(SPR_Numbers, negative, num % 10, 0)->tiles);
			num /= 10;
		}
		effect[i].type = EFF_DAMAGE;
		effect[i].ttl = 60; // 1 second
		effect[i].x = x;
		effect[i].y = y;
		effect[i].attr = TILE_ATTR_FULL(PAL0, true, false, false, TILE_NUMBERINDEX + (i * 4));
		SYS_disableInts();
		VDP_loadTileData(tiles, TILE_NUMBERINDEX + (i * 4), 4, true);
		SYS_enableInts();
		break;
	}
}

void damage_update(Effect *eff) {
	if(eff->ttl & 1) eff->y -= 1;
	sprite_add(eff->x - sub_to_pixel(camera.x), eff->y - sub_to_pixel(camera.y), 
		eff->attr, SPRITE_SIZE(4, 1));
}

void effect_create_smoke(u8 type, s16 x, s16 y) {
	for(u8 i = 0; i < MAX_EFFECTS; i++) {
		if(effect[i].ttl > 0) continue;
		effect[i].x = x;
		effect[i].y = y;
		effect[i].ttl = 48;
		//smoke[i].sprite = sprite_create(&SPR_Smoke, PAL1, false);
		//sprite_set_position(smoke[i].sprite,
		//		smoke[i].x - sub_to_pixel(camera.x) + SCREEN_HALF_W - 8,
		//		smoke[i].y - sub_to_pixel(camera.y) + SCREEN_HALF_H - 8);
		break;
	}
}

void smoke_update(Effect *eff) {
	//sprite_set_position(eff->sprite,
	//		eff->x - sub_to_pixel(camera.x) + SCREEN_HALF_W - 8,
	//		eff->y - sub_to_pixel(camera.y) + SCREEN_HALF_H - 8);
}
