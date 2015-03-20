#include "hud.h"

#include <genesis.h>
#include "sprite.h"
#include "player.h"
#include "resources.h"
#include "tables.h"

#define HEALTH_DECREASE_TIME 5

u8 hudMaxHealth, hudHealth, hudHealthTime;
u8 hudWeapon, hudMaxAmmo, hudAmmo;
u8 hudLevel, hudMaxEnergy, hudEnergy;

u8 hudSprite;
u8 hudHealthSprite[2];
u8 hudWeaponSprite;
u8 hudLevelSprite;
u8 hudMaxAmmoSprite[2], hudAmmoSprite[2];

bool showing = false;

void hud_redraw_health();

void hud_show() {
	if(showing) return;
	// Health
	hudMaxHealth = playerMaxHealth;
	hudHealth = player.health;
	hudHealthTime = HEALTH_DECREASE_TIME;
	hudSprite = sprite_create(&SPR_Hud1, PAL0, true);
	sprite_set_position(hudSprite, tile_to_pixel(2), tile_to_pixel(3));
	hudHealthSprite[0] = sprite_create(&SPR_Numbers, PAL0, true);
	hudHealthSprite[1] = sprite_create(&SPR_Numbers, PAL0, true);
	sprite_set_position(hudHealthSprite[0], tile_to_pixel(3), tile_to_pixel(4));
	sprite_set_position(hudHealthSprite[1], tile_to_pixel(4), tile_to_pixel(4));
	sprite_set_animframe(hudHealthSprite[0], 0, hudHealth / 10);
	sprite_set_animframe(hudHealthSprite[1], 0, hudHealth % 10);
	// Weapon
	hudWeapon = playerWeapon[currentWeapon].type;
	hudLevel = playerWeapon[currentWeapon].level;
	hudWeaponSprite = sprite_create(&SPR_ArmsImage, PAL0, true);
	hudLevelSprite = sprite_create(&SPR_Numbers, PAL0, true);
	sprite_set_position(hudWeaponSprite, tile_to_pixel(2), tile_to_pixel(1));
	sprite_set_position(hudLevelSprite, tile_to_pixel(4), tile_to_pixel(3));
	sprite_set_animframe(hudWeaponSprite, 0, hudWeapon);
	sprite_set_animframe(hudLevelSprite, 0, hudLevel);
	showing = true;
}

void hud_hide() {
	if(!showing) return;
	sprite_delete(hudSprite);
	sprite_delete(hudHealthSprite[0]);
	sprite_delete(hudHealthSprite[1]);
	sprite_delete(hudWeaponSprite);
	sprite_delete(hudLevelSprite);
	showing = false;
}

void hud_redraw_health() {
	// TODO: Health bar
	hudMaxHealth = playerMaxHealth;
	hudHealth = player.health;
	sprite_set_animframe(hudHealthSprite[0], 0, hudHealth / 10);
	sprite_set_animframe(hudHealthSprite[1], 0, hudHealth % 10);
}

void hud_decrease_health() {
	hudHealthTime--;
	if(hudHealthTime == 0) {
		hudHealth--;
		hudHealthTime = HEALTH_DECREASE_TIME;
	}
	sprite_set_animframe(hudHealthSprite[0], 0, hudHealth / 10);
	sprite_set_animframe(hudHealthSprite[1], 0, hudHealth % 10);
}

void hud_redraw_weapon() {
	hudWeapon = playerWeapon[currentWeapon].type;
	hudLevel = playerWeapon[currentWeapon].level;
	sprite_set_frame(hudWeaponSprite, hudWeapon);
	sprite_set_frame(hudLevelSprite, hudLevel);
}

void hud_redraw_ammo() {

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
		hud_decrease_health();
	}
	if(hudWeapon != playerWeapon[currentWeapon].type ||
			hudLevel != playerWeapon[currentWeapon].level) {
		hud_redraw_weapon();
	}
}
