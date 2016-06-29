#include "system.h"

#include <genesis.h>
#include <sram.h>
#include "stage.h"
#include "player.h"
#include "tsc.h"
#include "audio.h"
#include "vdp_ext.h"

// Official game supports 8000 but only uses 4000
// Decrease if there are memory issues
#define FLAGS_LEN 250

bool debuggingEnabled = false;
bool pauseCancelsIFrames = true;

struct {
	u8 hour, minute, second, frame;
} time;

u32 skip_flags = 0;

u32 flags[FLAGS_LEN];

void system_set_flag(u16 flag, bool value) {
	if(value) flags[flag/32] |= 1<<(flag%32);
	else flags[flag/32] &= ~(1<<(flag%32));
}

bool system_get_flag(u16 flag) {
	return (flags[flag/32] & (1<<(flag%32))) > 0;
}

void system_set_skip_flag(u16 flag, bool value) {
	if(value) skip_flags |= (1<<flag);
	else skip_flags &= ~(1<<flag);
}

bool system_get_skip_flag(u16 flag) {
	return (skip_flags & (1<<flag)) > 0;
}

void system_update() {
	time.frame++;
	if(time.frame >= 60) {
		time.second++;
		time.frame = 0;
		if(time.second >= 60) {
			time.minute++;
			time.second = 0;
			if(time.minute >= 60) {
				time.hour++;
			}
		}
	}
}

u8 system_get_frame() {
	return time.frame;
}

void system_drawtime(u16 x, u16 y) {
	char buf[4];
	uintToStr(time.hour, buf, 1);
	VDP_drawText(buf, x + (3 - strlen(buf)), y);
	VDP_drawText(":", x + 3, y);
	uintToStr(time.hour, buf, 2);
	VDP_drawText(buf, x + 4, y);
	VDP_drawText(":", x + 6, y);
	uintToStr(time.hour, buf, 2);
	VDP_drawText(buf, x + 7, y);
	VDP_drawText(".", x + 9, y);
	uintToStr(time.hour, buf, 2);
	VDP_drawText(buf, x + 10, y);
}

void system_new() {
	time.hour = time.minute = time.second = time.frame = 0;
	for(u16 i = 0; i < FLAGS_LEN; i++) flags[i] = 0;
	player_init();
	//stage_load(80);
	stage_load(13);
}

void system_save() {
	SRAM_enable();
	SRAM_writeWord(0x000, stageID);
	SRAM_writeWord(0x002, song_get_playing());
	SRAM_writeWord(0x004, sub_to_block(player.x));
	SRAM_writeWord(0x006, sub_to_block(player.y));
	SRAM_writeWord(0x008, playerMaxHealth);
	SRAM_writeWord(0x00A, player.health);
	SRAM_writeWord(0x00C, currentWeapon);
	SRAM_writeWord(0x00E, playerEquipment);
	SRAM_writeByte(0x010, time.hour); // Play time
	SRAM_writeByte(0x011, time.minute);
	SRAM_writeByte(0x012, time.second);
	SRAM_writeByte(0x013, time.frame);
	// Weapons (0x40)
	for(u8 i = 0; i < MAX_WEAPONS; i++) {
		SRAM_writeByte(0x20 + i*8, playerWeapon[i].type);
		SRAM_writeByte(0x21 + i*8, playerWeapon[i].level);
		SRAM_writeWord(0x22 + i*8, playerWeapon[i].energy);
		SRAM_writeWord(0x24 + i*8, playerWeapon[i].maxammo);
		SRAM_writeWord(0x26 + i*8, playerWeapon[i].ammo);
	}
	// Inventory (0x20)
	for(u8 i = 0; i < MAX_ITEMS; i++) {
		SRAM_writeByte(0x60 + i, playerInventory[i]);
	}
	// Teleporter locations
	for(u8 i = 0; i < 8; i++) {
		SRAM_writeWord(0x80 + i*2, teleportEvent[i]);
	}
	// Flags
	for (u16 i = 0; i < FLAGS_LEN; i++) {
		SRAM_writeLong(0x100 + i * 4, flags[i]);
	}
	SRAM_disable();
}

void system_load() {
	player_init();
	SRAM_enableRO();
	u16 rid = SRAM_readWord(0x00);
	u8 song = SRAM_readWord(0x02);
	player.x = block_to_sub(SRAM_readWord(0x04)) + pixel_to_sub(8);
	player.y = block_to_sub(SRAM_readWord(0x06)) + pixel_to_sub(8);
	playerMaxHealth = SRAM_readWord(0x08);
	player.health = SRAM_readWord(0x0A);
	currentWeapon = SRAM_readWord(0x0C);
	playerEquipment = SRAM_readWord(0x0E);
	time.hour = SRAM_readByte(0x10);
	time.minute = SRAM_readByte(0x11);
	time.second = SRAM_readByte(0x12);
	time.frame = SRAM_readByte(0x13);
	// Weapons
	for(u8 i = 0; i < MAX_WEAPONS; i++) {
		playerWeapon[i].type = SRAM_readByte(0x20 + i*8);
		playerWeapon[i].level = SRAM_readByte(0x21 + i*8);
		playerWeapon[i].energy = SRAM_readWord(0x22 + i*8);
		playerWeapon[i].maxammo = SRAM_readWord(0x24 + i*8);
		playerWeapon[i].ammo = SRAM_readWord(0x26 + i*8);
	}
	// Inventory (0x20)
	for(u8 i = 0; i < MAX_ITEMS; i++) {
		playerInventory[i] = SRAM_readByte(0x60 + i);
	}
	// Teleporter locations
	for(u8 i = 0; i < 8; i++) {
		teleportEvent[i] = SRAM_readWord(0x80 + i*2);
	}
	// Flags
	for (u16 i = 0; i < FLAGS_LEN; i++) {
		flags[i] = SRAM_readLong(0x100 + i * 4);
	}
	SRAM_disable();
	stage_load(rid);
	song_play(song);
}

u8 system_checkdata() {
	SRAM_enableRO();
	u32 t = SRAM_readLong(0x10);
	SRAM_disable();
	return t > 0;
}
