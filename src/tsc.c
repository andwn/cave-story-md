#include "common.h"

#include "audio.h"
#include "bank_data.h"
#include "camera.h"
#include "dma.h"
#include "entity.h"
#include "effect.h"
#include "error.h"
#include "gamemode.h"
#include "hud.h"
#include "joy.h"
#include "memory.h"
#include "npc.h"
#include "player.h"
#include "resources.h"
#include "sheet.h"
#include "stage.h"
#include "string.h"
#include "system.h"
#include "tables.h"
#include "tools.h"
#include "vdp.h"
#include "window.h"
#include "xgm.h"

#include "ai.h"
#include "tsc.h"

#define HEAD_EVENT_COUNT 14 // There are exactly 14
#define MAX_EVENTS 106 // Largest is ArmsItem with 106

// TSC Commands
#define FIRST_CMD 0x80
#define CMD_MSG 0x80
#define CMD_MS2 0x81
#define CMD_MS3 0x82
#define CMD_CLO 0x83
#define CMD_CLR 0x84
#define CMD_NUM 0x85
#define CMD_GIT 0x86
#define CMD_FAC 0x87
#define CMD_CAT 0x88
#define CMD_SAT 0x89
#define CMD_TUR 0x8a
#define CMD_YNJ 0x8b
#define CMD_END 0x8c
#define CMD_EVE 0x8d
#define CMD_TRA 0x8e
#define CMD_INI 0x8f
#define CMD_LDP 0x90
#define CMD_ESC 0x91
#define CMD_CMU 0x92
#define CMD_FMU 0x93
#define CMD_RMU 0x94
#define CMD_SOU 0x95
#define CMD_SPS 0x96
#define CMD_CPS 0x97
#define CMD_SSS 0x98
#define CMD_CSS 0x99
#define CMD_NOD 0x9a
#define CMD_WAI 0x9b
#define CMD_WAS 0x9c
#define CMD_MM0 0x9d
#define CMD_MOV 0x9e
#define CMD_MYB 0x9f
#define CMD_MYD 0xa0
#define CMD_UNI 0xa1
#define CMD_UNJ 0xa2
#define CMD_KEY 0xa3
#define CMD_PRI 0xa4
#define CMD_FRE 0xa5
#define CMD_HMC 0xa6
#define CMD_SMC 0xa7
#define CMD_LI_ADD 0xa8
#define CMD_ML_ADD 0xa9
#define CMD_ANP 0xaa
#define CMD_CNP 0xab
#define CMD_MNP 0xac
#define CMD_DNA 0xad
#define CMD_DNP 0xae
#define CMD_INP 0xaf
#define CMD_SNP 0xb0
#define CMD_BOA 0xb1
#define CMD_BSL 0xb2
#define CMD_NCJ 0xb3
#define CMD_ECJ 0xb4
#define CMD_AE_ADD 0xb5
#define CMD_ZAM 0xb6
#define CMD_AM_ADD 0xb7
#define CMD_AM_SUB 0xb8
#define CMD_TAM 0xb9
#define CMD_AMJ 0xba
#define CMD_EQ_ADD 0xbb
#define CMD_EQ_SUB 0xbc
#define CMD_IT_ADD 0xbd
#define CMD_IT_SUB 0xbe
#define CMD_ITJ 0xbf
#define CMD_FL_ADD 0xc0
#define CMD_FL_SUB 0xc1
#define CMD_FLJ 0xc2
#define CMD_SK_ADD 0xc3
#define CMD_SK_SUB 0xc4
#define CMD_SKJ 0xc5
#define CMD_FOB 0xc6
#define CMD_FOM 0xc7
#define CMD_FON 0xc8
#define CMD_QUA 0xc9
#define CMD_FAI 0xca
#define CMD_FAO 0xcb
#define CMD_FLA 0xcc
#define CMD_MLP 0xcd
#define CMD_MNA 0xce
#define CMD_CMP 0xcf
#define CMD_MP_ADD 0xd0
#define CMD_MPJ 0xd1
#define CMD_CRE 0xd2
#define CMD_SIL 0xd3
#define CMD_CIL 0xd4
#define CMD_SLP 0xd5
#define CMD_PS_ADD 0xd6
#define CMD_SVP 0xd7
#define CMD_STC 0xd8
#define CMD_XX1 0xd9
#define CMD_SMP 0xda
#define LAST_CMD 0xda

typedef struct {
	uint16_t number;
	const uint8_t *data;
} Event;

uint8_t tscEventCount;
uint8_t tscState;
uint16_t lastRunEvent;
uint8_t inFade;
uint16_t teleportEvent[8];
uint8_t showingBossHealth;

// Array of pointers to each event in the current TSC
Event headEvents[HEAD_EVENT_COUNT];
Event stageEvents[MAX_EVENTS];
uint16_t lastRunEvent;

const uint8_t *curCommand;
uint8_t cmd;

uint16_t waitTime;

uint16_t promptJump;

uint8_t teleMenuActive;
uint8_t teleMenuSlotCount;
uint16_t teleMenuEvent[8];
uint8_t teleMenuSelection;
uint8_t teleMenuSheet;
VDPSprite teleMenuSprite[8];


Entity *bossBarEntity;
uint16_t bossMaxHealth;
uint16_t bossHealth;

uint16_t lastAmmoNum;

uint8_t tsc_load(Event *eventList, const uint8_t *TSC, uint8_t max);

void tsc_show_teleport_menu();

uint8_t execute_command();
uint8_t tsc_read_byte();
uint16_t tsc_read_word();

enum { ARMSITEM, HEAD, STAGESEL, CREDITS, };

// Load window tiles & the global "head" events
void tsc_init() {
	inFade = FALSE;
	showingBossHealth = FALSE;
	bossBarEntity = FALSE;
	bossHealth = 0;
	tscState = TSC_IDLE;
	cmd = '\n';
	teleMenuSlotCount = 0;
	teleMenuSelection = 0;
	memset(teleMenuEvent, 0, 16);
	vdp_tiles_load_from_rom(TS_Window.tiles, TILE_WINDOWINDEX, TS_Window.numTile);
	tsc_load(headEvents, (const uint8_t*)TSC_GLOB[HEAD], HEAD_EVENT_COUNT);
}

void tsc_load_stage(uint8_t id) {
	if(id == ID_ARMSITEM) { // Stage index 255 is a special case for the item menu
		tscEventCount = tsc_load(stageEvents, (const uint8_t*)TSC_GLOB[ARMSITEM], MAX_EVENTS);
	} else if(id == ID_TELEPORT) {
		tscEventCount = tsc_load(stageEvents, (const uint8_t*)TSC_GLOB[STAGESEL], MAX_EVENTS);
	} else if(id == ID_CREDITS) {
		tscEventCount = tsc_load(stageEvents, (const uint8_t*)TSC_GLOB[CREDITS], MAX_EVENTS);
	} else {
		tscEventCount = tsc_load(stageEvents, (const uint8_t*)TSC_STAGE[id], MAX_EVENTS);
	}
}

uint8_t tsc_load(Event *eventList, const uint8_t *TSC, uint8_t max) {
	// First byte of TSC is the number of events
	uint8_t eventCount = TSC[0];
	// Make sure it isn't more than can be handled
	if(eventCount > max) {
		char str[40];
		sprintf(str, "Too many events: %hu\nIn TSC at: %06lX", eventCount, (uint32_t) TSC);
		error_other(str);
	}
	// Step through ROM data until finding all the events
	uint8_t loadedEvents = 0;
	for(uint16_t i = 1; loadedEvents < eventCount; i++) {
		// The event marker is a word 0xFFFF
		if(TSC[i] == 0xFF && TSC[i+1] == 0xFF) {
			eventList[loadedEvents].number = TSC[i+2]+(TSC[i+3]<<8);
			eventList[loadedEvents].data = &TSC[i+4];
			loadedEvents++;
			i += 3;
		}
	}
	return loadedEvents;
}

void tsc_call_event(uint16_t number) {
    window_set_textmode(TM_NORMAL);
	// Events under 50 will be in Head.tsc
	if(number < 50) {
		for(uint8_t i = 0; i < HEAD_EVENT_COUNT; i++) {
			if(headEvents[i].number == number) {
			    lastRunEvent = number;
			    //linesSinceLastNOD = 0;
				tscState = TSC_RUNNING;
				curCommand = headEvents[i].data;
				return;
			}
		}
	} else {
		for(uint8_t i = 0; i < tscEventCount; i++) {
			if(stageEvents[i].number == number) {
                lastRunEvent = number;
                //linesSinceLastNOD = 0;
				tscState = TSC_RUNNING;
				curCommand = stageEvents[i].data;
				return;
			}
		}
	}
}

uint8_t tsc_update() {
	switch(tscState) {
		case TSC_IDLE: break; // Nothing to update
		case TSC_RUNNING:
		{
			for(;;) {
				uint8_t result = execute_command();
				if(result > 0) return result - 1;
			}
		}
		break;
		case TSC_WAITTIME:
		{
			// Game will lock up if <WAI0000 is used. Don't do that
			if(waitTime == 0) {
				tscState = TSC_RUNNING;
			} else {
				waitTime--;
				// ArmsItem uses <WAI9999 after everything. The original game probably uses this
				// in some weird way, but it'll just freeze here, so don't <WAI in the pause menu
				if(paused) {
					waitTime = 0;
				// Check the wait time again to prevent underflowing
				} else if(waitTime > 0 && (cfg_ffwd && (joystate & btn[cfg_btn_ffwd])) && gamemode == GM_GAME) {
					if(cfg_ffwd) {
						// Fast-forward while holding A, update active entities a second time
						// to double their movement speed (unless <PRI is set)
						waitTime--;
						if(!gameFrozen) entities_update(FALSE);
					}
				}
			}
		}
		break;
		case TSC_WAITINPUT:
		{
			if(joy_pressed(btn[cfg_btn_jump]) || (cfg_ffwd && (joystate & btn[cfg_btn_ffwd]))) {
				tscState = TSC_RUNNING;
				if(cfg_language >= LANG_JA && cfg_language <= LANG_KO) {
					window_draw_jchar(FALSE, ' '); // Clear blinking cursor
				} else {
					window_draw_char(' '); // Clear blinking cursor
				}
			}
		}
		break;
		case TSC_PROMPT:
		{
			if(window_prompt_update()) {
				// Answering No will jump to another event
				tscState = TSC_RUNNING;
				if(!window_prompt_answer()) tsc_call_event(promptJump);
			}
		}
		break;
		case TSC_TELEMENU:
		{
			if(joy_pressed(btn[cfg_btn_jump])) {
				// Reload current stage's TSC, and run event for warp
				teleMenuActive = FALSE;
				tsc_load_stage(stageID);
				tsc_call_event(teleMenuEvent[teleMenuSelection]);
				tscState = TSC_RUNNING;
			} else if(joy_pressed(btn[cfg_btn_shoot])) { // Cancel
				teleMenuActive = FALSE;
				tsc_load_stage(stageID);
				// Manually force event to end
				tscState = TSC_IDLE;
				gameFrozen = FALSE;
				window_set_face(0, FALSE);
				window_close();
				controlsLocked = FALSE;
				hud_show();
			} else if(joy_pressed(BUTTON_LEFT)) {
				sprite_index(teleMenuSprite[teleMenuSelection], 
						sheets[teleMenuSheet].index + teleMenuSelection*16);
				if(teleMenuSelection == 0) {
					teleMenuSelection = teleMenuSlotCount - 1;
				} else {
					teleMenuSelection--;
				}
				sound_play(SND_MENU_MOVE, 5);
				// Writes location name in the window
				tsc_call_event(1000 + teleMenuSelection + 1);
				while(tscState != TSC_IDLE) execute_command();
				tscState = TSC_TELEMENU; // Don't break away from the menu
			} else if(joy_pressed(BUTTON_RIGHT)) {
				sprite_index(teleMenuSprite[teleMenuSelection], 
						sheets[teleMenuSheet].index + teleMenuSelection*16);
				if(teleMenuSelection == teleMenuSlotCount - 1) {
					teleMenuSelection = 0;
				} else {
					teleMenuSelection++;
				}
				sound_play(SND_MENU_MOVE, 5);
				
				tsc_call_event(1000 + teleMenuSelection + 1);
				while(tscState != TSC_IDLE) execute_command();
				tscState = TSC_TELEMENU;
			} else { // Doing nothing, blink cursor
				bossHealth ^= 8;
				sprite_index(teleMenuSprite[teleMenuSelection], 
						sheets[teleMenuSheet].index + teleMenuSelection*16 + bossHealth);
			}
		    vdp_sprites_add(teleMenuSprite, teleMenuSlotCount);
			// --WARP-- text
		    vdp_sprite_add(&teleMenuSprite[6]);
		    vdp_sprite_add(&teleMenuSprite[7]);
		}
		break;
		case TSC_WAITGROUNDED:
		{
			if(player.grounded) tscState = TSC_RUNNING;
		}
		break;
		default:
		{
			tscState = TSC_IDLE;
		}
		break;
	}
	return 0;
}

void tsc_show_boss_health() {
	showingBossHealth = TRUE;
	// Fill map name space with boss bar
	static const char boss[4] = "Boss";
	for(uint8_t i = 0; i < 4; i++) {
		vdp_tiles_load_from_rom(&TS_SysFont.tiles[8*(boss[i]-0x20)], TILE_NAMEINDEX+i, 1);
	}
	for(uint8_t i = 0; i < 8; i++) {
		vdp_tiles_load_from_rom(&TS_HudBar.tiles[8*7], TILE_NAMEINDEX+4+i, 1);
	}
	// Create sprites to display the string
	memset(teleMenuSprite, 0, sizeof(VDPSprite) * 8);
	uint8_t yoff = pal_mode ? 24 : 16;
	teleMenuSprite[5] = (VDPSprite) { 
		.x = 160 + 32 + 128, .y = SCREEN_HEIGHT - yoff + 128,
		.size = SPRITE_SIZE(4,1), .attr = TILE_ATTR(PAL0,1,0,0,TILE_NAMEINDEX)
	};
	teleMenuSprite[6] = (VDPSprite) { 
		.x = 160 + 64 + 128, .y = SCREEN_HEIGHT - yoff + 128,
		.size = SPRITE_SIZE(4,1), .attr = TILE_ATTR(PAL0,1,0,0,TILE_NAMEINDEX+4)
	};
	teleMenuSprite[7] = (VDPSprite) { 
		.x = 160 + 96 + 128, .y = SCREEN_HEIGHT - yoff + 128,
		.size = SPRITE_SIZE(4,1), .attr = TILE_ATTR(PAL0,1,0,0,TILE_NAMEINDEX+8)
	};
}

void tsc_hide_boss_health() {
	bossBarEntity = NULL;
	showingBossHealth = FALSE;
}

void tsc_update_boss_health() {
	if(!bossBarEntity || bossBarEntity->state == STATE_DELETE) {
		bossBarEntity = NULL;
		showingBossHealth = FALSE;
		return;
	}
	if(bossHealth != bossBarEntity->health) {
		bossHealth = bossBarEntity->health;
		if(bossHealth == 0) {
			// Boss is dead hide the bar
			bossBarEntity = NULL;
			showingBossHealth = FALSE;
			return;
		}
		uint16_t hp = bossHealth, inc = bossMaxHealth >> 3, i;
		// Filled tiles
		for(i = 0; i < 8 && hp >= inc; i++) {
			hp -= inc;
		}
		// If boss health is full no need to go any further
		if(bossHealth == bossMaxHealth) return;
		// Draw a partial filled tile
		if(inc) {
			uint16_t index = min((((uint16_t)(hp << 3)) / inc) << 3, 72);
			vdp_tiles_load_from_rom(&TS_HudBar.tiles[index], TILE_NAMEINDEX+4+i, 1);
		} else {
			// Don't divide by zero
			vdp_tiles_load_from_rom(&TS_HudBar.tiles[8*3], TILE_NAMEINDEX+4+i, 1);
		}
		// Draw empty tile after it
		if(++i < 8) {
			vdp_tiles_load_from_rom(TS_HudBar.tiles, TILE_NAMEINDEX+4+i, 1);
		}
	}
	
    vdp_sprite_add(&teleMenuSprite[5]);
    vdp_sprite_add(&teleMenuSprite[6]);
    vdp_sprite_add(&teleMenuSprite[7]);
}

static void tsc_render_warp_text() {
	const uint32_t *ts = cfg_language == LANG_JA ? TS_MenuTextJ.tiles : TS_MenuTextE.tiles;
	// Copy our string to VRAM
    dma_queue(DmaVRAM, (uint32_t) (ts + (16<<3)), TILE_NAMEINDEX << 5, (8 * TILE_SIZE) >> 1, 2);
	// Create sprites to display the string
	teleMenuSprite[6] = (VDPSprite) { 
		.x = 160 - 32 + 128, .y = 32 + 128,
		.size = SPRITE_SIZE(4,1), .attr = TILE_ATTR(PAL0,1,0,0,TILE_NAMEINDEX)
	};
	teleMenuSprite[7] = (VDPSprite) { 
		.x = 160 + 128, .y = 32 + 128,
		.size = SPRITE_SIZE(4,1), .attr = TILE_ATTR(PAL0,1,0,0,TILE_NAMEINDEX+4)
	};
}

void tsc_show_teleport_menu() {
	//mapNameTTL = 0; // We will be clobbering the tiles that display the map name
	memset(teleMenuSprite, 0, sizeof(VDPSprite) * 8);
	tsc_render_warp_text();

	SHEET_FIND(teleMenuSheet, SHEET_TELE);
    teleMenuSlotCount = 0;
	for(uint8_t i = 0; i < 8; i++) {
        if(teleportEvent[i]) teleMenuSlotCount++;
	}
	uint16_t spr_x = 160 - (teleMenuSlotCount * 20);
	uint8_t iterCount = 0;
    for(uint8_t i = 0; i < 8; i++) {
        if(teleportEvent[i] == 0) continue;
        teleMenuEvent[iterCount] = teleportEvent[i];
        teleMenuSprite[iterCount] = (VDPSprite) {
                .x = spr_x + 128, .y = 48 + 128,
                .size = SPRITE_SIZE(4, 2),
                .attr = TILE_ATTR(PAL0,1,0,0,sheets[teleMenuSheet].index + (i-1)*16)
        };
        iterCount++;
        spr_x += 40;
    }
	tsc_load_stage(ID_TELEPORT);
	if(teleMenuSlotCount > 0) {
		teleMenuActive = TRUE;
		tsc_call_event(1000 + teleMenuSelection + 1);
		while(tscState != TSC_IDLE) execute_command();
		tscState = TSC_TELEMENU;
		vdp_set_window(0, pal_mode ? 245 : 244);
		controlsLocked = TRUE;
		// I use bossHealth to tick back and forth between 0 and 8 so the cursor will flash
		// since bosses don't happen in Arthur's house
		bossHealth = 0;
	} else {
        teleMenuActive = TRUE;
		tsc_call_event(1000);
		while(tscState != TSC_IDLE) execute_command();
        teleMenuActive = FALSE;
        tsc_load_stage(stageID);
        tsc_call_event(0); // So script will end after input
        tscState = TSC_WAITINPUT;

	}
}

uint8_t execute_command() {
	uint16_t args[4];
	cmd = tsc_read_byte();
	if(cmd >= 0x80 && cmd < 0xE0) {
		switch(cmd) {
		case CMD_MSG: // Display message box (bottom - visible)
		{
			window_open(0);
		}
		break;
		case CMD_MS2: // Display message box (top - invisible)
		{
			// Hide face or else doctor will talk with Misery's face graphic
			window_set_face(0, 0);
			window_open(1);
		}
		break;
		case CMD_MS3: // Display message box (top - visible)
		{
			window_open(1);
		}
		break;
		case CMD_CLO: // Close message box
		{
			window_close();
		}
		break;
		case CMD_CLR: // Clear message box
		{
			window_clear();
		}
		break;
		case CMD_NUM: // Show number (1) in message box
		{
			args[0] = tsc_read_word();
			char str[6];
			sprintf(str, "%u", lastAmmoNum);
			for(uint8_t i = 0; str[i] != 0 && i < 6; i++) {
				window_draw_char(str[i]);
			}
		}
		break;
		case CMD_GIT: // Display item (1) in message box
		{
			args[0] = tsc_read_word();
			if(args[0] >= 1000) {
				window_show_item(args[0] - 1000);
			} else {
				window_show_weapon(args[0]);
			}
		}
		break;
		case CMD_FAC: // Display face (1) in message box
		{
			args[0] = tsc_read_word();
			window_set_face(args[0], TRUE);
		}
		break;
		case CMD_CAT: // All 3 of these display text instantly
		{
			window_set_textmode(TM_ALL);
		}
		break;
		case CMD_SAT:
		{
			window_set_textmode(TM_ALL);
		}
		break;
		case CMD_TUR:
		{
			window_set_textmode(TM_MSG);
		}
		break;
		case CMD_YNJ: // Prompt Yes/No and jump to event (1) if No
		{
			args[0] = tsc_read_word();
			promptJump = args[0];
			window_prompt_open();
			tscState = TSC_PROMPT;
			return 1;
		}
		break;
		case CMD_END: // End the event
		{
			tscState = TSC_IDLE;
			if(!paused && !teleMenuActive) {
				gameFrozen = FALSE;
				window_set_face(0, FALSE);
				window_close();
                window_set_textmode(TM_NORMAL);
				controlsLocked = FALSE;
				hud_show();
			}
			return 1;
		}
		break;
		case CMD_EVE: // Jump to event (1)
		{
			args[0] = tsc_read_word();
			tsc_call_event(args[0]);
		}
		break;
		case CMD_TRA: // Teleport to stage (1), run event (2), coords (3),(4)
		{
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			args[2] = tsc_read_word();
			args[3] = tsc_read_word();
			if(gamemode == GM_CREDITS) {
				if(args[0] == 0) {
					vdp_map_clear(VDP_PLAN_A);
					entities_clear();
					sheets_load_stage(0, FALSE, TRUE);
					vdp_sprites_clear();
					Entity *p = entity_create(pixel_to_sub(240), pixel_to_sub(140), OBJ_NPC_PLAYER, 0);
					p->state = 100;
					p->event = 400;
					tsc_load_stage(ID_CREDITS);
				} else {
					stage_load_credits(args[0]);
				}
				tsc_call_event(args[1]);
				return 1;
			} else {
				if(args[0] == 0) return 5; // Room ID 0 is credits
				player.x = block_to_sub(args[2]) + pixel_to_sub(8);
				player.y = block_to_sub(args[3]) + pixel_to_sub(8);
				player.x_speed = 0;
				player.y_speed = 0;
				player.grounded = FALSE;
				gameFrozen = FALSE;
				window_set_face(0, FALSE);
				window_close();
				stage_load(args[0]);
				tsc_call_event(args[1]);
				return 1;
			}
		}
		break;
		case CMD_INI: // Start from beginning (try again without save data)
		{
			return 4;
		}
		break;
		case CMD_ESC: // Restart the game
		{
			return 2;
		}
		break;
		case CMD_LDP: // Reload save file (try again)
		{
			return 3;
		}
		break;
		case CMD_CMU: // Play song (1), stop if 0
		{
			args[0] = tsc_read_word();
			song_play(args[0]);
		}
		break;
		case CMD_FMU: // Fade out music (we just stop for now)
		{
			song_stop();
		}
		break;
		case CMD_RMU: // Resume previously playing music
		{
			song_resume();
		}
		break;
		case CMD_SOU: // Play sound (1)
		{
			args[0] = tsc_read_word();
			sound_play(args[0], 5);
		}
		break;
		
		// TODO: Persistent sounds, skip for now
		case CMD_SPS: break;
		case CMD_CPS: break;
		case CMD_SSS: args[0] = tsc_read_word(); break;
		case CMD_CSS: break;
			
		case CMD_NOD: // Wait for player input
		{
			tscState = TSC_WAITINPUT;
			return 1;
		}
		break;
		case CMD_WAI: // Wait (1) frames
		{
			args[0] = tsc_read_word();
			tscState = TSC_WAITTIME;
			waitTime = TIME(args[0]);
			return 1;
		}
		break;
		case CMD_WAS: // Wait for player to hit the ground
		{
			tscState = TSC_WAITGROUNDED;
			return 1;
		}
		break;
		case CMD_MM0: // Halt player movement
		{
			player.x_speed = 0;
			player.y_speed = 0;
		}
		break;
		case CMD_MOV: // Move player to coordinates (1),(2)
		{
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			player.x = block_to_sub(args[0]) + (8 << CSF);
			player.y = block_to_sub(args[1]) + (8 << CSF);
			player.grounded = FALSE;
		}
		break;
		case CMD_MYB: // Bounce player in direction (1)
		{
			args[0] = tsc_read_word();
			if(args[0] == 0) { // Right
				player.x_speed = SPEED(0x200);
			} else if(args[0] == 2) { // Left
				player.x_speed = -SPEED(0x200);
			}
			player.y_speed = -SPEED(0x400);
		}
		break;
		case CMD_MYD: // Change direction to (1)
		{
			args[0] = tsc_read_word();
			if(args[0] == 0) { // Left
				player.dir = 0;
			} else if(args[0] == 2) { // Right
				player.dir = 1;
			}
			player.x_speed = 0;
			lookingDown = FALSE;
		}
		break;
		case CMD_UNI: // Change movement type to (1)
		{
			args[0] = tsc_read_word();
			playerMoveMode = args[0];
		}
		break;
		case CMD_UNJ: // If movement type is (1) jump to event (2)
		{
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			if(playerMoveMode == args[0]) {
				tsc_call_event(args[1]);
			}
		}
		break;
		case CMD_KEY: // Lock controls and hide the HUD
		{
			controlsLocked = TRUE;
			hud_hide();
			gameFrozen = FALSE;
		}
		break;
		case CMD_PRI: // Lock controls
		{
			controlsLocked = TRUE;
			gameFrozen = TRUE;
		}
		break;
		case CMD_FRE: // Unlock controls
		{
			controlsLocked = FALSE;
			hud_show();
			gameFrozen = FALSE;
		}
		break;
		case CMD_HMC: // Hide player character
		{
			player.hidden = TRUE;
		}
		break;
		case CMD_SMC: // Show player character
		{
			player.hidden = FALSE;
		}
		break;
		case CMD_LI_ADD: // Restore health by (1)
		{
			args[0] = tsc_read_word();
			player_heal(args[0]);
		}
		break;
		case CMD_ML_ADD: // Increase max health by (1)
		{
			args[0] = tsc_read_word();
			player_maxhealth_increase(args[0]);
		}
		break;
		case CMD_ANP: // Give all entities of event (1) script state (2) with direction (3)
		{
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			args[2] = tsc_read_word();
			entities_set_state(args[0], args[1], mddir(args[2]));
		}
		break;
		case CMD_CNP: // Change all entities of event (1) to type (2) with direction (3)
		{
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			args[2] = tsc_read_word();
			entities_replace(args[0], args[1], mddir(args[2]), 0);
		}
		break;
		case CMD_MNP: // Move entity of event (1) to (2),(3) with direction (4)
		{
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			args[2] = tsc_read_word();
			args[3] = tsc_read_word();
			entities_move(args[0], args[1], args[2], mddir(args[3]));
		}
		break;
		case CMD_DNA: // Delete all entities of type (1)
		{
			args[0] = tsc_read_word();
			entities_clear_by_type(args[0]);
		}
		break;
		case CMD_DNP: // Delete all entities with event # (1)
		{
			args[0] = tsc_read_word();
			entities_clear_by_event(args[0]);
		}
		break;
		// Change entity of event (1) to type (2) with direction (3) and set option 2 flag
		case CMD_INP:
		{
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			args[2] = tsc_read_word();
			entities_replace(args[0], args[1], mddir(args[2]), 0x1000);
		}
		break;
		case CMD_SNP: // Create entity (1) at (2),(3) with direction (4)
		{
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			args[2] = tsc_read_word();
			args[3] = tsc_read_word();
			if(args[0] == OBJ_THE_CAST) {
				entity_create_ext(block_to_sub(args[1])+(8<<CSF), 
								  block_to_sub(args[2])+(8<<CSF), args[0], 0, args[3], 0);
			} else {
				Entity *e = entity_create(block_to_sub(args[1])+(8<<CSF), 
										  block_to_sub(args[2])+(8<<CSF), args[0], 0);
				e->dir = mddir(args[3]);
			}
		}
		break;
		case CMD_BOA: // Set boss state to (1)
		{
			args[0] = tsc_read_word();
			// The real cave story has the stage boss created at stage load in a dormant state.
			// NXEngine also does this, but I don't, instead waiting until the boss is used
			// In a <BOA command to create it. A bit hacky but it works.
			if(stageID == 0x0A && args[0] == 20) {
				// Omega in Sand Zone
				bossEntity = entity_create(0, 0, 360 + BOSS_OMEGA, 0);
				bossEntity->event = 210;
				bossEntity->state = 20;
			} else if(stageID == 0x27 && args[0] == 1) {
				// Monster X - #0301 <BOA0001
				bossEntity = entity_create(0, 0, 360 + BOSS_MONSTERX, 0);
				bossEntity->event = 1000;
				bossEntity->state = 1;
			} else if(stageID == 0x2F && args[0] == 200) {
				// Core
				bossEntity = entity_create(0, 0, 360 + BOSS_CORE, 0);
				bossEntity->event = 1000;
				bossEntity->state = 200;
			} else if(stageID == 0x33 && args[0] == 20) {
				// Dragon Sisters
				bossEntity = entity_create(0, 0, 360 + BOSS_SISTERS, 0);
				bossEntity->event = 1000;
				bossEntity->state = 20;
				bossBarEntity = bossEntity;
				bossMaxHealth = bossHealth = bossBarEntity->health;
				tsc_show_boss_health();
			} else if(stageID == 87 && args[0] == 100) {
				// Ballos
				bossEntity = entity_create(0, 0, 360 + BOSS_BALLOS, 0);
				bossEntity->event = 1000;
				bossEntity->state = 100;
			} else if(bossEntity) {
				bossEntity->state = args[0];
			}
		}
		break;
		case CMD_BSL: // Start boss fight with entity (1)
		{
			args[0] = tsc_read_word();
			// Value of 0 looks for major boss
			if(!args[0]) {
				if((bossBarEntity = bossEntity)) {
					bossMaxHealth = bossHealth = bossBarEntity->health;
					tsc_show_boss_health();
					break;
				}
			} // Other values search for entity with event # of the parameter
			else if((bossBarEntity = entity_find_by_event(args[0]))) {
				bossMaxHealth = bossHealth = bossBarEntity->health;
				tsc_show_boss_health();
				break;
			}
		}
		break;
		case CMD_NCJ: // If entity type (1) exists jump to event (2)
		{
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			if(entity_find_by_type(args[0])) tsc_call_event(args[1]);
		}
		break;
		case CMD_ECJ: // If entity id (1) exists jump to event (2)
		{
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			if(entity_find_by_id(args[0])) tsc_call_event(args[1]);
		}
		break;
		case CMD_AE_ADD: // Refill all weapon ammo
		{
			player_refill_ammo();
		}
		break;
		case CMD_ZAM: // Make all weapons level 1
		{
			player_delevel_weapons();
		}
		break;
		case CMD_AM_ADD: // Give weapon (1) and ammo (2)
		{
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			player_give_weapon(args[0], args[1]);
			lastAmmoNum = args[1];
		}
		break;
		case CMD_AM_SUB: // Remove weapon (1)
		{
			args[0] = tsc_read_word();
			player_take_weapon(args[0]);
		}
		break;
		case CMD_TAM: // Trade weapon (1) for (2) with (3) max ammo
		{
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			args[2] = tsc_read_word();
			player_trade_weapon(args[0], args[1], args[2]);
		}
		break;
		case CMD_AMJ: // If player has weapon (1) jump to event (2)
		{
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			if(player_has_weapon(args[0])) tsc_call_event(args[1]);
		}
		break;
		// These two equip commands actually give a flag between 1<<0 and 1<<8
		case CMD_EQ_ADD: // Equip item (1)
		{
			args[0] = tsc_read_word();
			player_equip(args[0]);
		}
		break;
		case CMD_EQ_SUB: // Remove equip (1)
		{
			args[0] = tsc_read_word();
			player_unequip(args[0]);
		}
		break;
		case CMD_IT_ADD: // Give item (1)
			args[0] = tsc_read_word();
			player_give_item(args[0]);
			break;
		case CMD_IT_SUB: // Remove item (1)
		{
			args[0] = tsc_read_word();
			player_take_item(args[0]);
		}
		break;
		case CMD_ITJ: // If player has item (1) jump to event (2)
		{
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			if(player_has_item(args[0])) tsc_call_event(args[1]);
		}
		break;
		case CMD_FL_ADD: // Set flag (1)
		{
			args[0] = tsc_read_word();
			system_set_flag(args[0], TRUE);
		}
		break;
		case CMD_FL_SUB: // Unset flag (1)
		{
			args[0] = tsc_read_word();
			system_set_flag(args[0], FALSE);
		}
		break;
		case CMD_FLJ: // If flag (1) is TRUE jump to event (2)
		{
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			if(system_get_flag(args[0])) tsc_call_event(args[1]);
		}
		break;
		case CMD_SK_ADD: // Set skip flag (1)
		{
			args[0] = tsc_read_word();
			system_set_skip_flag(args[0], TRUE);
		}
		break;
		case CMD_SK_SUB: // Unset skip flag (1)
		{
			args[0] = tsc_read_word();
			system_set_skip_flag(args[0], FALSE);
		}
		break;
		case CMD_SKJ: // If skip flag (1) is TRUE jump to event (2)
		{
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			if(system_get_skip_flag(args[0])) tsc_call_event(args[1]);
		}
		break;
		case CMD_FOB: // Focus on boss (1) with (2) ticks
		{
			if(bossEntity) camera.target = bossEntity;
		}
		break;
		case CMD_FON: // Focus on NPC (1) with (2) ticks
		{
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			Entity *e = entity_find_by_event(args[0]);
			camera.target = e ? e : &player;
		}
		break;
		case CMD_FOM: // Focus on player at (1) speed
		{
			args[0] = tsc_read_word();
			camera.target = &player;
		}
		break;
		case CMD_QUA: // Shake camera for (1) frames
		{
			args[0] = tsc_read_word();
			camera_shake(args[0]);
		}
		break;
		case CMD_FAI: // Fading, in direction (1)
		{
			args[0] = tsc_read_word();
			stage_setup_palettes();
			if(gamemode != GM_CREDITS) {
				inFade = FALSE; // Unlock sprites from updating
				vdp_vsync(); // Wait a frame to let the sprites redraw
				aftervsync();
				vdp_fade(PAL_FadeOut, NULL, 4, TRUE);
			} else {
				vdp_color_next(0, 0x200);
				vdp_fade(PAL_FadeOutBlue, NULL, 4, TRUE);
			}
		}
		break;
		case CMD_FAO:
		{
			args[0] = tsc_read_word();
			if(gamemode != GM_CREDITS) {
                vdp_vsync();
                player_draw();
                entities_draw();
                ready = TRUE;
                aftervsync();
				vdp_fade(NULL, PAL_FadeOut, 4, FALSE);
				// Blank the sprite list in VRAM
				//vdp_sprites_clear();
				inFade = TRUE;
			} else {
				vdp_fade(NULL, PAL_FadeOutBlue, 4, TRUE);
				waitTime = 20;
				return 1;
			}
		}
		break;
		case CMD_FLA: // Flash screen white
		{
			vdp_fade(PAL_FullWhite, NULL, 4, TRUE);
		}
		break;
		case CMD_MLP: // Show the map
		{
			do_map();
		}
		break;
		case CMD_MNA: // Show stage name
		{
			player_show_map_name(180);
		}
		break;
		case CMD_CMP: // Change stage tile at (1),(2) to type (3)
		{
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			args[2] = tsc_read_word();
			if(args[0] >= stageWidth || args[1] >= stageHeight) {
				// Outside the map -- don't do this
				break;
			}
			// When I crushed some larger tilesets to better fit VRAM I inadvertently broke
			// CMP for maps using those tilesets. Thankfully TSC instructions are not critical
			// code, so I can put in this hacky section which fixes specific scripts
			if(stageID == 14) { // Mimiga Village Shack -- Balrog busts through door
				if(args[2] == 80 || args[2] == 81 || args[2] == 82) args[2] += 32;
				else args[2] += 19;
			} else if(stageTileset == 25) { // Throne Room / Ostep
				if(args[2] == 18) args[2] = 49;
				if(args[2] == 21) args[2] = 50;
            } else if(stageTileset == 26) { // Hell
                if(args[2] == 28) args[2] = 25;
			} else if(stageTileset == 27) { // King's Table
				if(args[2] == 18) args[2] = 40;
				if(args[2] == 21) args[2] = 41;
			} else if(stageTileset == 28) { // Black Space
                if (args[2] == 18) args[2] = 33;
                if (args[2] == 21) args[2] = 34;
            }
			// Puff of smoke
			stage_replace_block(args[0], args[1], args[2]);
			effect_create_smoke(block_to_pixel(args[0]) + 8, block_to_pixel(args[1]) + 8);
		}
		break;
		// Map flags are unused but exist, keeping them here just in case
		case CMD_MP_ADD: // Map flag (1)
			args[0] = tsc_read_word();
			break;
		case CMD_MPJ: // If map flag (1) set jump to event (2)
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			break;
		case CMD_CRE: // Show credits
		{
			return 5;
		}
		break;
		case CMD_SIL: // Show illustration (1) in the credits
		{
			args[0] = tsc_read_word();
			credits_show_image(args[0]);
		}
		break;
		case CMD_CIL: // Clear illustration in the credits
		{
			credits_clear_image();
		}
		break;
		case CMD_SLP: // Show the teleporter menu
		{
			tsc_show_teleport_menu();
			return 1;
		}
		break;
		case CMD_PS_ADD: // Set teleporter slot (1) to event (2)
		{
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			if(args[0] < 8) teleportEvent[args[0]] = args[1];
		}
		break;
		case CMD_SVP: // Save
		{
			system_save();
		}
		break;
		case CMD_STC: // Save counter
		{
			system_save_counter(system_counter_ticks());
		}
		break;
		case CMD_XX1: // Island effect
		{
			args[0] = tsc_read_word();

			ready = TRUE;
			vdp_vsync(); aftervsync();

            disable_ints;
            z80_request();
			vdp_set_display(FALSE);
			// Disable camera
			camera.target = NULL;
			camera.x = pixel_to_sub(SCREEN_HALF_W);
			camera.y = pixel_to_sub(SCREEN_HALF_H);
			camera.x_shifted = 0;
			camera.y_shifted = 0;
			// Reset plane positions
			vdp_hscroll(VDP_PLAN_A, 0);
			vdp_vscroll(VDP_PLAN_A, 0);
			vdp_hscroll(VDP_PLAN_B, 0);
			vdp_vscroll(VDP_PLAN_B, 0);
			// Clear planes
			vdp_map_clear(VDP_PLAN_B);
			vdp_map_clear(VDP_PLAN_A);
			// Background sky/mountains
			vdp_tiles_load_from_rom(TS_XXBack.tiles, TILE_TSINDEX, TS_XXBack.numTile);
			vdp_map_fill_rect(VDP_PLAN_B, TILE_ATTR(PAL3,0,0,0,TILE_TSINDEX), 10, 10, 20, 10, 1);
			// Foreground trees
			vdp_tiles_load_from_rom(TS_XXFore.tiles, TILE_BACKINDEX, TS_XXFore.numTile);
			vdp_map_fill_rect(VDP_PLAN_A, TILE_ATTR(PAL3,1,0,0,TILE_BACKINDEX), 10, 16, 20, 4, 1);
			// Draw high prio black tiles over the top to hide island
			static const uint32_t blackTile[8] = { 
				0x11111111,0x11111111,0x11111111,0x11111111,
				0x11111111,0x11111111,0x11111111,0x11111111
			};
			vdp_tiles_load_from_rom(blackTile, 1, 1);
			vdp_map_fill_rect(VDP_PLAN_A, TILE_ATTR(PAL0,1,0,0,1), 10, 6, 20, 4, 0);
			
			// Island sprite
			SHEET_LOAD(&SPR_XXIsland, 1, 15, TILE_SHEETINDEX, 1, 0,0);
			VDPSprite island[2] = {
				(VDPSprite) { 
					.x = 160 - 20 + 128, .y = 64 - 8 + 128, .size = SPRITE_SIZE(4, 3),
					.attr = TILE_ATTR(PAL3,0,0,0,TILE_SHEETINDEX)
				},
				(VDPSprite) {
					.x = 160 + 12 + 128, .y = 64 - 8 + 128, .size = SPRITE_SIZE(1, 3),
					.attr = TILE_ATTR(PAL3,0,0,0,TILE_SHEETINDEX+12)
				}
			};

			vdp_colors_next(48, PAL_XX.data, 16);
			vdp_colors(48, PAL_XX.data, 16);
			vdp_set_display(TRUE);
            z80_release();
            enable_ints;

			song_stop();
			
			uint16_t t = TIME_10(350);
			while(--t) {
				if(t > TIME_8(150) || !args[0]) {
					if((t % TIME_8(5)) == 0) {
						island[0].y++;
						island[1].y++;
					}
				}
				vdp_sprites_add(island, 2);
				system_update();
				ready = TRUE;
				vdp_vsync();
				vdp_sprites_update();
			}
		}
		break;
		case CMD_SMP: // Subtract 1 from tile index at position (1), (2)
		{
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			uint8_t b = stage_get_block(args[0], args[1]);
			if (b > 0) stage_replace_block(args[0], args[1], b - 1);
		}
		break;
		default: break;
		}
	} else {
		uint16_t kanji = 0;
		uint8_t doublebyte = FALSE;
		if(cmd >= 0xE0 && cmd < 0xFF) {
			doublebyte = TRUE;
			if(cfg_language >= LANG_JA && cfg_language <= LANG_KO) { // Get kanji index from cmd and next byte
				kanji = (cmd - 0xE0) * 0x60 + (tsc_read_byte() - 0x20);
			} else {
				tsc_read_byte();
				cmd = '?';
			}
		}
		if(cmd == 0x01) {
            doublebyte = TRUE;
            if(cfg_language > LANG_EN && cfg_language < LANG_JA) { // Get ext char index from next byte
                cmd = tsc_read_byte() + 0x80;
            } else {
                tsc_read_byte();
                cmd = '?';
            }
		}
		if(window_is_open()) {
			if(cmd == '\n' || window_tick() || (cfg_ffwd && (joystate & btn[cfg_btn_ffwd]))) {
				if(cfg_language >= LANG_JA && cfg_language <= LANG_KO) {
					window_draw_jchar(doublebyte, doublebyte ? kanji : cmd);
				} else {
					window_draw_char(cmd);
				}
				if(cfg_msg_blip && !(cfg_ffwd && (joystate & btn[cfg_btn_ffwd])) 
						&& window_get_textmode() == TM_NORMAL && cmd != '\n') {
					sound_play(SND_MSG, 2);
				}
			} else {
				curCommand -= doublebyte ? 2 : 1;
				return 1;
			}
		}
	}
	return 0;
}

uint8_t tsc_read_byte() {
	uint8_t byte = curCommand[0];
	curCommand++;
	return byte;
}

uint16_t tsc_read_word() {
	uint16_t word = curCommand[0]+(curCommand[1]<<8);
	curCommand += 2;
	return word;
}
