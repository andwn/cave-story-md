// This is the first event after hitting "New Game"
#define GAME_START_EVENT 200
// Events that get called when killed, drowned, out of bounds, etc
#define PLAYER_DEFEATED_EVENT 40
#define PLAYER_DROWN_EVENT 41
#define PLAYER_OOB_EVENT 42

#define ALMOND_DROWN_FLAG	4000
#define ALMOND_DROWN_EVENT	1100

// Special "stage" IDs to load TSCs for menu/credits
#define ID_ARMSITEM	0xFF
#define ID_TELEPORT	0xFE
#define ID_CREDITS	0x00

enum TSC_STATE {
	TSC_IDLE,			// Not executing any script
	TSC_RUNNING,		// Executing event commands
	TSC_WAITTIME,		// Waiting on a timer before continuing
	TSC_WAITINPUT,		// Waiting for player to press C
	TSC_PROMPT,			// Prompting yes/no
	TSC_TELEMENU, 		// Displaying the teleporter menu
	TSC_WAITGROUNDED, 	// Waiting for the player to touch the ground
};

// Number of events loaded by tsc_load(), for debugging
extern uint8_t tscEventCount;

extern uint8_t tscState;
extern uint16_t lastRunEvent;

extern uint8_t inFade;

// As teleporter locations are made available to the player, this list is populated
// with which event is called when selecting a particular area.
// This is in the header so that system_save() can write the values to SRAM
extern uint16_t teleportEvent[8];

// True while boss health is being displayed in the corner
extern uint8_t showingBossHealth;

// Initialize default values to avoid strange glitches
// Also loads persistent head.tsc and arms.tsc events
void tsc_init();
// Called by stage_load(), fills up the event list with pointers to each event
void tsc_load_stage(uint8_t id);
// Per frame update, will advance the script or wait for something based on whats happening
uint8_t tsc_update();
// This will begin executing a scripted event if it exists
void tsc_call_event(uint16_t number);

//void tsc_unpause_debug();

void tsc_show_boss_health();
void tsc_hide_boss_health();
void tsc_update_boss_health();
