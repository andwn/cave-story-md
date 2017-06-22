#define SRAM_UNCHECKED		0x00
#define SRAM_VALID_EMPTY	0x01
#define SRAM_VALID_SAVE		0x02
#define SRAM_INVALID		0xFF

#define FLAG_EVENTONDROWN		4000
#define FLAG_DISABLESAVE		4095

extern uint8_t cfg_language;
extern uint8_t debuggingEnabled;

// Global flags are persisted to save data
void system_set_flag(uint16_t flag, uint8_t value);
uint8_t system_get_flag(uint16_t flag);
// Skip flags remain in memory until power off, or new game
// Used to skip boss cutscenes when retrying
void system_set_skip_flag(uint16_t flag, uint8_t value);
uint8_t system_get_skip_flag(uint16_t flag);

// Increments play time
void system_update();

// Initializes variables with defaults when starting a new game
void system_new();
// Initializes variables from SRAM when loading a game save
void system_load();
// Stores variables and game state into SRAM
void system_save();
// Validates whether any save data exists in SRAM
uint8_t system_checkdata();
// Load to a stock save file from the level select list
void system_load_levelselect(uint8_t file);

// Counter stuff
void system_start_counter();
uint32_t system_counter_ticks();
void system_counter_draw();
uint32_t system_load_counter();
void system_save_counter(uint32_t ticks);

// SGDK sys.h stuff
void SYS_assertReset();
void SYS_hardReset();
void SYS_die(char *err);
