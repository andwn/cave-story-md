#include "hud.h"

#include <genesis.h>
#include "sprite.h"
#include "player.h"
#include "resources.h"
#include "tables.h"

u8 hudMaxHealth, hudHealth;

u8 hudSprite;
u8 hudHealthSprite[2];

bool showing = false;

void hud_redraw_health();

void hud_show() {
	if(showing) return;
	hudMaxHealth = playerMaxHealth;
	hudHealth = player.health;
	hudSprite = sprite_create(&SPR_Hud1, PAL0, true);
	sprite_set_position(hudSprite, tile_to_pixel(2), tile_to_pixel(3));
	hudHealthSprite[0] = sprite_create(&SPR_Numbers, PAL0, true);
	hudHealthSprite[1] = sprite_create(&SPR_Numbers, PAL0, true);
	sprite_set_position(hudHealthSprite[0], tile_to_pixel(3), tile_to_pixel(4));
	sprite_set_position(hudHealthSprite[1], tile_to_pixel(4), tile_to_pixel(4));
	sprite_set_animframe(hudHealthSprite[0], 0, hudHealth / 10);
	sprite_set_animframe(hudHealthSprite[1], 0, hudHealth % 10);
	showing = true;
}

void hud_hide() {
	if(!showing) return;
	sprite_delete(hudSprite);
	sprite_delete(hudHealthSprite[0]);
	sprite_delete(hudHealthSprite[1]);
	showing = false;
}

void hud_redraw_health() {
	// TODO: Health bar
	hudMaxHealth = playerMaxHealth;
	hudHealth = player.health;
	sprite_set_animframe(hudHealthSprite[0], 0, hudHealth / 10);
	sprite_set_animframe(hudHealthSprite[1], 0, hudHealth % 10);
}

void hud_change_health() {

}

void hud_change_weapon() {

}

void hud_change_ammo() {

}

void hud_update() {
	if(hudMaxHealth < playerMaxHealth) {
		// Redraw
		hud_redraw_health();
	} else if(hudHealth < player.health) {
		// Redraw
		hud_redraw_health();
	} else if(hudHealth > player.health) {
		// Slide health bar down
		hud_redraw_health();
	}
}
