#include "common.h"

#include "audio.h"
#include "dma.h"
#include "entity.h"
#include "joy.h"
#include "memory.h"
#include "player.h"
#include "psg.h"
#include "resources.h"
#include "sram.h"
#include "stage.h"
#include "string.h"
#include "tools.h"
#include "tsc.h"
#include "vdp.h"
#include "vdp_bg.h"
#include "vdp_ext.h"
#include "weapon.h"
#include "z80_ctrl.h"

#include "system.h"

// ASCII string "CSMD" used as a sort of checksum to verify save data exists
#define STR_CSMD 0x43534D44
// When save data is not found "TEST" is written to verify SRAM is usable
#define STR_TEST 0x54455354

#define CFG_MAGIC 0x12345678

// Supports 0-4095, official game uses 0-4000
#define FLAGS_LEN 128

typedef struct { uint8_t hour, minute, second, frame; } Time;

static const uint8_t *FileList[] = {
	LS_00, LS_01, LS_02, LS_03, LS_04, LS_05, LS_06, LS_07,
	LS_08, LS_09, LS_10, LS_11, LS_12, LS_13, LS_14, LS_15,
	LS_16, LS_17, LS_18, LS_19, LS_20, LS_21,
};

uint8_t cfg_btn_jump = 5;
uint8_t cfg_btn_shoot = 4;
uint8_t cfg_btn_ffwd = 6;
uint8_t cfg_btn_rswap = 8;
uint8_t cfg_btn_lswap = 9;
uint8_t cfg_btn_map = 10;
uint8_t cfg_btn_pause = 7;

uint8_t cfg_language = 0;
uint8_t cfg_ffwd = TRUE;
uint8_t cfg_updoor = FALSE;
uint8_t cfg_hellquake = TRUE;
uint8_t cfg_iframebug = TRUE;

uint8_t sram_state = SRAM_UNCHECKED;

uint8_t counterEnabled = FALSE;
Time time, counter;

uint32_t flags[FLAGS_LEN];
uint32_t skip_flags = 0;

static uint8_t LS_readByte(uint8_t file, uint32_t addr);
static uint16_t LS_readWord(uint8_t file, uint32_t addr);
static uint32_t LS_readLong(uint8_t file, uint32_t addr);

void system_set_flag(uint16_t flag, uint8_t value) {
	printf("Setting flag %hu %s", flag, value ? "ON" : "OFF");
	if(value) flags[flag>>5] |= 1<<(flag&31);
	else flags[flag>>5] &= ~(1<<(flag&31));
}

uint8_t system_get_flag(uint16_t flag) {
	return (flags[flag>>5] & (1<<(flag&31))) > 0;
}

void system_set_skip_flag(uint16_t flag, uint8_t value) {
	printf("Setting skip flag %hu %s", flag, value ? "ON" : "OFF");
	if(value) skip_flags |= (1<<flag);
	else skip_flags &= ~(1<<flag);
}

uint8_t system_get_skip_flag(uint16_t flag) {
	return (skip_flags & (1<<flag)) > 0;
}

void system_update() {
	if(++time.frame >= FPS) {
		time.frame = 0;
		if(++time.second >= 60) {
			time.second = 0;
			if(++time.minute >= 60) {
				time.hour++;
				time.minute = 0;
				printf("You have been playing for %hu hour(s)", time.hour);
			}
		}
	}
	if(counterEnabled) {
		if(++counter.frame >= FPS) {
			counter.frame = 0;
			if(++counter.second >= 60) {
				counter.second = 0;
				if(++counter.minute >= 60) {
					counter.hour++;
					counter.minute = 0;
				}
			}
		}
	}
}

void system_new() {
	puts("Starting a new game");
	counterEnabled = FALSE;
	time.hour = time.minute = time.second = time.frame = 0;
	for(uint16_t i = 0; i < FLAGS_LEN; i++) flags[i] = 0;
	if(sram_state == SRAM_INVALID) system_set_flag(FLAG_DISABLESAVE, TRUE);
	player_init();
	stage_load(13);
}

void system_save() {
	if(sram_state == SRAM_INVALID) return;
	puts("Writing game save to SRAM");
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
	for(uint8_t i = 0; i < MAX_WEAPONS; i++) {
		SRAM_writeByte(0x20 + i*8, playerWeapon[i].type);
		SRAM_writeByte(0x21 + i*8, playerWeapon[i].level);
		SRAM_writeWord(0x22 + i*8, playerWeapon[i].energy);
		SRAM_writeWord(0x24 + i*8, playerWeapon[i].maxammo);
		SRAM_writeWord(0x26 + i*8, playerWeapon[i].ammo);
	}
	SRAM_writeLong(0x5C, STR_CSMD);
	// Inventory (0x20)
	for(uint8_t i = 0; i < MAX_ITEMS; i++) {
		SRAM_writeByte(0x60 + i, playerInventory[i]);
	}
	// Teleporter locations
	for(uint8_t i = 0; i < 8; i++) {
		SRAM_writeWord(0x80 + i*2, teleportEvent[i]);
	}
	// Flags
	for (uint16_t i = 0; i < FLAGS_LEN; i++) {
		SRAM_writeLong(0x100 + i * 4, flags[i]);
	}
	SRAM_disable();
}

void system_load() {
	puts("Loading game save from SRAM");
	counterEnabled = FALSE;
	player_init();
	SRAM_enableRO();
	uint16_t rid = SRAM_readWord(0x00);
	uint8_t song = SRAM_readWord(0x02);
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
	for(uint8_t i = 0; i < MAX_WEAPONS; i++) {
		playerWeapon[i].type = SRAM_readByte(0x20 + i*8);
		playerWeapon[i].level = SRAM_readByte(0x21 + i*8);
		playerWeapon[i].energy = SRAM_readWord(0x22 + i*8);
		playerWeapon[i].maxammo = SRAM_readWord(0x24 + i*8);
		playerWeapon[i].ammo = SRAM_readWord(0x26 + i*8);
	}
	// Inventory (0x20)
	for(uint8_t i = 0; i < MAX_ITEMS; i++) {
		playerInventory[i] = SRAM_readByte(0x60 + i);
	}
	// Teleporter locations
	for(uint8_t i = 0; i < 8; i++) {
		teleportEvent[i] = SRAM_readWord(0x80 + i*2);
	}
	// Flags
	for (uint16_t i = 0; i < FLAGS_LEN; i++) {
		flags[i] = SRAM_readLong(0x100 + i * 4);
	}
	SRAM_disable();
	stage_load(rid);
	song_play(song);
}

void system_load_config() {
	static const uint16_t start = 0x300;
	
	SRAM_enableRO();
	uint32_t magic = SRAM_readLong(start);
	if(magic != CFG_MAGIC) {
		// No settings saved, keep defaults
		SRAM_disable();
		return;
	}
	
	uint8_t index = 4;
	cfg_btn_jump  = SRAM_readByte(start + index++);
	cfg_btn_shoot = SRAM_readByte(start + index++);
	cfg_btn_ffwd  = SRAM_readByte(start + index++);
	cfg_btn_rswap = SRAM_readByte(start + index++);
	cfg_btn_lswap = SRAM_readByte(start + index++);
	cfg_btn_map   = SRAM_readByte(start + index++);
	cfg_btn_pause = SRAM_readByte(start + index++);
	cfg_language  = SRAM_readByte(start + index++);
	cfg_ffwd      = SRAM_readByte(start + index++);
	cfg_updoor    = SRAM_readByte(start + index++);
	cfg_hellquake = SRAM_readByte(start + index++);
	cfg_iframebug = SRAM_readByte(start + index++);
	SRAM_disable();
}

void system_save_config() {
	static const uint16_t start = 0x300;
	
	SRAM_enable();
	SRAM_writeLong(start, CFG_MAGIC);
	
	uint8_t index = 4;
	SRAM_writeByte(start + index++, cfg_btn_jump);
	SRAM_writeByte(start + index++, cfg_btn_shoot);
	SRAM_writeByte(start + index++, cfg_btn_ffwd);
	SRAM_writeByte(start + index++, cfg_btn_rswap);
	SRAM_writeByte(start + index++, cfg_btn_lswap);
	SRAM_writeByte(start + index++, cfg_btn_map);
	SRAM_writeByte(start + index++, cfg_btn_pause);
	SRAM_writeByte(start + index++, cfg_language);
	SRAM_writeByte(start + index++, cfg_ffwd);
	SRAM_writeByte(start + index++, cfg_updoor);
	SRAM_writeByte(start + index++, cfg_hellquake);
	SRAM_writeByte(start + index++, cfg_iframebug);
	SRAM_disable();
}

void system_load_levelselect(uint8_t file) {
	puts("Loading game save from SRAM");
	counterEnabled = FALSE;
	player_init();
	uint16_t rid = LS_readWord(file, 0x00);
	uint8_t song = LS_readWord(file, 0x02);
	player.x = block_to_sub(LS_readWord(file, 0x04)) + pixel_to_sub(8);
	player.y = block_to_sub(LS_readWord(file, 0x06)) + pixel_to_sub(8);
	playerMaxHealth = LS_readWord(file, 0x08);
	player.health = LS_readWord(file, 0x0A);
	currentWeapon = LS_readWord(file, 0x0C);
	playerEquipment = LS_readWord(file, 0x0E);
	time.hour = LS_readByte(file, 0x10);
	time.minute = LS_readByte(file, 0x11);
	time.second = LS_readByte(file, 0x12);
	time.frame = LS_readByte(file, 0x13);
	// Weapons
	for(uint8_t i = 0; i < MAX_WEAPONS; i++) {
		playerWeapon[i].type = LS_readByte(file, 0x20 + i*8);
		playerWeapon[i].level = LS_readByte(file, 0x21 + i*8);
		playerWeapon[i].energy = LS_readWord(file, 0x22 + i*8);
		playerWeapon[i].maxammo = LS_readWord(file, 0x24 + i*8);
		playerWeapon[i].ammo = LS_readWord(file, 0x26 + i*8);
	}
	// Inventory (0x20)
	for(uint8_t i = 0; i < MAX_ITEMS; i++) {
		playerInventory[i] = LS_readByte(file, 0x60 + i);
	}
	// Teleporter locations
	for(uint8_t i = 0; i < 8; i++) {
		teleportEvent[i] = LS_readWord(file, 0x80 + i*2);
	}
	// Flags
	for (uint16_t i = 0; i < FLAGS_LEN; i++) {
		flags[i] = LS_readLong(file, 0x100 + i * 4);
	}
	stage_load(rid);
	song_play(song);
}

uint8_t system_checkdata() {
	// Read a specific spot in SRAM
	puts("Checking SRAM");
	SRAM_enableRO();
	uint32_t test = SRAM_readLong(0x5C);
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
			// Clear the hell timer just in case (thanks Fusion)
			SRAM_enable();
			for(uint16_t i = 0; i < 5; i++) SRAM_writeLong(0x48 + i*4, 0);
			SRAM_disable();
		} else {
			// Test failed, SRAM is unusable
			sram_state = SRAM_INVALID;
		}
	}
	switch(sram_state) {
		case SRAM_VALID_EMPTY:	puts("SRAM valid - no save data"); break;
		case SRAM_VALID_SAVE:	puts("SRAM valid - save exists"); break;
		case SRAM_INVALID:		puts("SRAM read/write test FAILED! Saving disabled"); break;
	}
	return sram_state;
}

void system_start_counter() {
	counter = (Time) { 0,0,0,0 };
	counterEnabled = TRUE;
}

uint32_t system_counter_ticks() {
	return counter.frame + counter.second*FPS + counter.minute*FPS*60 + counter.hour*FPS*60*60;
}

void system_counter_draw() {
	
}

uint32_t system_load_counter() {
	uint8_t buffer[20];
	uint32_t *result = (uint32_t*)buffer;
	// Read 20 bytes of 290.rec from SRAM
	SRAM_enableRO();
	for(uint16_t i = 0; i < 20; i++) {
		buffer[i] = SRAM_readByte(0x48 + i);
	}
	SRAM_disable();
	// Apply key
	for(uint16_t i = 0; i < 4; i++) {
		uint8_t key = buffer[16 + i];
		uint16_t j = i * 4;
		buffer[j] -= key;
		buffer[j+1] -= key;
		buffer[j+2] -= key;
		buffer[j+3] -= (key / 2);
	}
	// Ticks should be nonzero
	if(!result[0]) {
		return 0xFFFFFFFF;
	}
	// Make sure tick values match
	if((result[0] != result[1]) || (result[0] != result[2]) || (result[0] != result[3])) {
		return 0xFFFFFFFF;
	}
	// Convert LE -> BE
    return (buffer[0]<<24) + (buffer[1]<<16) + (buffer[2]<<8) + buffer[3];
}

void system_save_counter(uint32_t ticks) {
	uint8_t buffer[20];
	uint32_t *result = (uint32_t*)buffer;
	uint8_t *tickbuf = (uint8_t*)&ticks;
	// Generate random key
	result[4] = random();
	// Write to buffer BE -> LE 4 times
	for(uint16_t i = 0; i < 4; i++) {
		result[i] = (tickbuf[0]<<24) + (tickbuf[1]<<16) + (tickbuf[2]<<8) + tickbuf[3];
	}
	// Apply the key to each
	for(uint16_t i = 0; i < 4; i++) {
		uint8_t key = buffer[16 + i];
		uint16_t j = i * 4;
		buffer[j] += key;
		buffer[j+1] += key;
		buffer[j+2] += key;
		buffer[j+3] += (key / 2);
	}
	// Write to SRAM
	SRAM_enable();
	for(uint16_t i = 0; i < 20; i++) {
		SRAM_writeByte(0x48 + i, buffer[i]);
	}
	SRAM_disable();
}

static uint8_t LS_readByte(uint8_t file, uint32_t addr) {
	return FileList[file][addr];
}

static uint16_t LS_readWord(uint8_t file, uint32_t addr) {
	return (LS_readByte(file, addr) << 8) + LS_readByte(file, addr+1);
}

static uint32_t LS_readLong(uint8_t file, uint32_t addr) {
	return (LS_readWord(file, addr) << 16) + LS_readWord(file, addr+2);
}

// SGDK sys.c stuff

// main function
extern int main();

// exception state consumes 78 bytes of memory
__attribute__((externally_visible)) uint32_t registerState[8+8];
__attribute__((externally_visible)) uint32_t pcState;
__attribute__((externally_visible)) uint32_t addrState;
__attribute__((externally_visible)) uint16_t ext1State;
__attribute__((externally_visible)) uint16_t ext2State;
__attribute__((externally_visible)) uint16_t srState;

static void addValueU8(char *dst, char *str, uint8_t value)
{
    char v[16];

    strcat(dst, str);
    intToHex(value, v, 2);
    strcat(dst, v);
}

static void addValueU16(char *dst, char *str, uint16_t value)
{
    char v[16];

    strcat(dst, str);
    intToHex(value, v, 4);
    strcat(dst, v);
}

static void addValueU32(char *dst, char *str, uint32_t value)
{
    char v[16];

    strcat(dst, str);
    intToHex(value, v, 8);
    strcat(dst, v);
}

static uint16_t showValueU32U16(char *str1, uint32_t value1, char *str2, uint16_t value2, uint16_t pos)
{
    char s[64];

    strclr(s);
    addValueU32(s, str1, value1);
    addValueU16(s, str2, value2);

    VDP_drawText(s, 0, pos);

    return pos + 1;
}

static uint16_t showValueU16U32U16(char *str1, uint16_t value1, char *str2, uint32_t value2, char *str3, uint16_t value3, uint16_t pos)
{
    char s[64];

    strclr(s);
    addValueU16(s, str1, value1);
    addValueU32(s, str2, value2);
    addValueU16(s, str3, value3);

    VDP_drawText(s, 0, pos);

    return pos + 1;
}

static uint16_t showValueU32U16U16(char *str1, uint32_t value1, char *str2, uint16_t value2, char *str3, uint16_t value3, uint16_t pos)
{
    char s[64];

    strclr(s);
    addValueU32(s, str1, value1);
    addValueU16(s, str2, value2);
    addValueU16(s, str3, value3);

    VDP_drawText(s, 0, pos);

    return pos + 1;
}

static uint16_t showValueU32U32(char *str1, uint32_t value1, char *str2, uint32_t value2, uint16_t pos)
{
    char s[64];

    strclr(s);
    addValueU32(s, str1, value1);
    addValueU32(s, str2, value2);

    VDP_drawText(s, 0, pos);

    return pos + 1;
}

static uint16_t showValueU32U32U32(char *str1, uint32_t value1, char *str2, uint32_t value2, char *str3, uint32_t value3, uint16_t pos)
{
    char s[64];

    strclr(s);
    addValueU32(s, str1, value1);
    addValueU32(s, str2, value2);
    addValueU32(s, str3, value3);

    VDP_drawText(s, 0, pos);

    return pos + 1;
}

static uint16_t showRegisterState(uint16_t pos)
{
    uint16_t y = pos;

    y = showValueU32U32U32("D0=", registerState[0], " D1=", registerState[1], " D2=", registerState[2], y);
    y = showValueU32U32U32("D3=", registerState[3], " D4=", registerState[4], " D5=", registerState[5], y);
    y = showValueU32U32("D6=", registerState[6], " D7=", registerState[7], y);
    y = showValueU32U32U32("A0=", registerState[8], " A1=", registerState[9], " A2=", registerState[10], y);
    y = showValueU32U32U32("A3=", registerState[11], " A4=", registerState[12], " A5=", registerState[13], y);
    y = showValueU32U32("A6=", registerState[14], " A7=", registerState[15], y);

    return y;
}

static uint16_t showStackState(uint16_t pos)
{
    char s[64];
    uint16_t y = pos;
    uint32_t *sp = (uint32_t*) registerState[15];

    uint16_t i = 0;
    while(i < 24)
    {
        strclr(s);
        addValueU8(s, "SP+", i * 4);
        strcat(s, " ");
        y = showValueU32U32(s, *(sp + (i + 0)), " ", *(sp + (i + 1)), y);
        i += 2;
    }

    return y;
}

static uint16_t showExceptionDump(uint16_t pos)
{
    uint16_t y = pos;

    y = showValueU32U16("PC=", pcState, " SR=", srState, y) + 1;
    y = showRegisterState(y) + 1;
    y = showStackState(y);

    return y;
}

static uint16_t showException4WDump(uint16_t pos)
{
    uint16_t y = pos;

    y = showValueU32U16U16("PC=", pcState, " SR=", srState, " VO=", ext1State, y) + 1;
    y = showRegisterState(y) + 1;
    y = showStackState(y);

    return y;
}

static uint16_t showBusAddressErrorDump(uint16_t pos)
{
    uint16_t y = pos;

    y = showValueU16U32U16("FUNC=", ext1State, " ADDR=", addrState, " INST=", ext2State, y);
    y = showValueU32U16("PC=", pcState, " SR=", srState, y) + 1;
    y = showRegisterState(y) + 1;
    y = showStackState(y);

    return y;
}


// bus error default callback
void _bus_error_cb()
{
    VDP_init();
    VDP_drawText("BUS ERROR !", 10, 3);

    showBusAddressErrorDump(5);

    while(1);
}

// address error default callback
void _address_error_cb()
{
    VDP_init();
    VDP_drawText("ADDRESS ERROR !", 10, 3);

    showBusAddressErrorDump(5);

    while(1);
}

// illegal instruction exception default callback
void _illegal_instruction_cb()
{
    VDP_init();
    VDP_drawText("ILLEGAL INSTRUCTION !", 7, 3);

    showException4WDump(5);

    while(1);
}

// division by zero exception default callback
void _zero_divide_cb()
{
    VDP_init();
    VDP_drawText("DIVIDE BY ZERO !", 10, 3);

    showExceptionDump(5);

    while(1);
}

// CHK instruction default callback
void _chk_instruction_cb()
{
    VDP_init();
    VDP_drawText("CHK INSTRUCTION EXCEPTION !", 5, 10);

    showException4WDump(12);

    while(1);
}

// TRAPV instruction default callback
void _trapv_instruction_cb()
{
    VDP_init();
    VDP_drawText("TRAPV INSTRUCTION EXCEPTION !", 5, 3);

    showException4WDump(5);

    while(1);
}

// privilege violation exception default callback
void _privilege_violation_cb()
{
    VDP_init();
    VDP_drawText("PRIVILEGE VIOLATION !", 5, 3);

    showExceptionDump(5);

    while(1);
}

// trace default callback
void _trace_cb()
{

}

// error exception default callback
void _exception_cb()
{
    VDP_init();
    VDP_drawText("EXCEPTION ERROR !", 5, 3);

    showExceptionDump(5);

    while(1);
}

void _start_entry() {
    // initiate random number generator
    setRandomSeed(0xC427);
    // enable interrupts
    __asm__("move #0x2500,%sr");
    // init part
    MEM_init();
    VDP_init();
    DMA_init(0, 0);
    //PSG_init();
    JOY_init();
    // reseting z80 also reset the ym2612
    Z80_init();
    // let's the fun go on !
    main();
}

void SYS_die(char *err)
{
    VDP_init();
    VDP_drawText("A fatal error occured !", 2, 2);
    VDP_drawText("cannot continue...", 4, 3);
    if (err) VDP_drawText(err, 0, 5);

    while(1);
}
