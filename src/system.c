#include "system.h"

#include <genesis.h>
#include <sram.h>
#include "stage.h"
#include "player.h"
#include "tsc.h"
#include "audio.h"
#include "vdp_ext.h"

// ASCII string "CSMD" used as a sort of checksum to verify save data exists
#define STR_CSMD 0x43534D44
// When save data is not found "TEST" is written to verify SRAM is usable
#define STR_TEST 0x54455354

u8 sram_state = SRAM_UNCHECKED;

// Official game supports 8000 but only uses up to 4000 (the INDEX of 4000 is used, so 4001 flags)
// Max here is 4096
#define FLAGS_LEN 128

bool debuggingEnabled = false;
bool pauseCancelsIFrames = true;
bool checksumValid = true;

struct {
	u8 hour, minute, second, frame;
} time;

u32 skip_flags = 0;

u32 flags[FLAGS_LEN];

void system_set_flag(u16 flag, bool value) {
#ifdef KDB_SYS
	printf("Setting flag %hu %s", flag, value ? "ON" : "OFF");
#endif
	if(value) flags[flag/32] |= 1<<(flag%32);
	else flags[flag/32] &= ~(1<<(flag%32));
}

bool system_get_flag(u16 flag) {
	return (flags[flag/32] & (1<<(flag%32))) > 0;
}

void system_set_skip_flag(u16 flag, bool value) {
#ifdef KDB_SYS
	printf("Setting skip flag %hu %s", flag, value ? "ON" : "OFF");
#endif
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
				time.minute = 0;
#ifdef KDB_SYS
				printf("You have been playing for %hu hour(s)", time.hour);
#endif
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
#ifdef KDB_SYS
	puts("Starting a new game");
#endif
	time.hour = time.minute = time.second = time.frame = 0;
	for(u16 i = 0; i < FLAGS_LEN; i++) flags[i] = 0;
	if(sram_state == SRAM_INVALID) system_set_flag(FLAG_DISABLESAVE, true);
	player_init();
	stage_load(13);
}

void system_save() {
	if(sram_state == SRAM_INVALID) return;
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
	SRAM_writeLong(0x5C, STR_CSMD);
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
	if(sram_state == SRAM_INVALID) SYS_die("Invalid SRAM");
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
	// Read a specific spot in SRAM
#ifdef KDB_SYS
	puts("Checking SRAM");
#endif
	SRAM_enableRO();
	u32 test = SRAM_readLong(0x5C);
	SRAM_disable();
	// Anything there?
	if(test == STR_CSMD) {
		// Save data exists, this is the only state that should allow selecting "continue"
		sram_state = SRAM_VALID_SAVE;
	} else if(test == STR_TEST) {
		// No save data, but SRAM was validated before
		sram_state = SRAM_VALID_EMPTY;
	} else {
		// Nothing is there, try to write "TEST" and re-read
		SRAM_enable();
		SRAM_writeLong(0x5C, STR_TEST);
		SRAM_disable();
		SRAM_enableRO();
		test = SRAM_readLong(0x5C);
		SRAM_disable();
		if(test == STR_TEST) {
			// Test passed, game can be saved but not loaded
			sram_state = SRAM_VALID_EMPTY;
		} else {
			// Test failed, SRAM is unusable
			sram_state = SRAM_INVALID;
		}
	}
#ifdef KDB_SYS
	switch(sram_state) {
	case SRAM_VALID_EMPTY: puts("SRAM valid - no save data"); break;
	case SRAM_VALID_SAVE: puts("SRAM valid - save exists"); break;
	case SRAM_INVALID: puts("SRAM read/write test FAILED! Saving disabled"); break;
	}
#endif
	return sram_state;
}

void system_verifychecksum() {
	const u16 *CHECKSUM = (u16*) 0x18E;
	if(*CHECKSUM == 0) { // No checksum
		return;
	}
	u32 chk = 0;
	u32 *ptr = (u32*) 0x200;
	u16 loopCount = (0x100000 - 0x200) >> 6;
	while(loopCount--) {
		chk ^= *ptr++;
		chk ^= *ptr++;
		chk ^= *ptr++;
		chk ^= *ptr++;
		chk ^= *ptr++;
		chk ^= *ptr++;
		chk ^= *ptr++;
		chk ^= *ptr++;
		chk ^= *ptr++;
		chk ^= *ptr++;
		chk ^= *ptr++;
		chk ^= *ptr++;
		chk ^= *ptr++;
		chk ^= *ptr++;
		chk ^= *ptr++;
		chk ^= *ptr++;
	}
	u16 checksum = (chk >> 16) ^ (chk & 0xFFFF);
	if(checksum != *CHECKSUM) checksumValid = false;
}
