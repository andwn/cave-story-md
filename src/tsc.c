#include "tsc.h"
#include "sprite.h"
#include "audio.h"
#include "player.h"
#include "entity.h"
#include "stage.h"
#include "input.h"
#include "camera.h"
#include "resources.h"
#include "system.h"
#include "vdp_ext.h"
#include "tables.h"
#include "common.h"
#include "hud.h"

// Execution State
#define TSC_IDLE 0 // Not executing any events
#define TSC_RUNNING 1 // Executing event commands
#define TSC_WAITTIME 2 // Waiting on a timer before continuing
#define TSC_WAITINPUT 3 // Waiting for player to press C
#define TSC_WAITNPC 4 // Waiting for an NPC to finish something
#define TSC_WAITGROUNDED 5 // Waiting for the player to touch the ground
#define TSC_RESTARTGAME 6 // Return to title screen
#define TSC_RELOADSAVE 7 // Reload save data (try again)

#define TILE_WINDOWINDEX (TILE_FONTINDEX + 0x60)
#define WINDOW_ATTR(x) TILE_ATTR_FULL(PAL0, true, false, false, TILE_WINDOWINDEX+x)
#define WINDOW_XPOS 2
#define WINDOW_YPOS 19
#define PROMPT_XPOS 22
#define PROMPT_YPOS 19
#define MSG_LEFT_COLUMN 3
#define MSG_TOP_ROW 20
#define MSG_LEFT_FACE 11

#define HEAD_EVENT_COUNT 13 // There are exactly 13
#define MAX_EVENTS 80 // Largest is Plantation with 76

// TSC Commands
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

// Array of pointers to each event in the current TSC
u16 event_nums[MAX_EVENTS];
const u8 *event_index[MAX_EVENTS];

u16 head_nums[HEAD_EVENT_COUNT];
const u8 *head_events[HEAD_EVENT_COUNT];

const u8 *curCommand = NULL;

u8 exeMode = TSC_IDLE;

u16 waitTime;

bool msgWindowOpen = false;
u8 msgTextX = 0;
u8 msgTextY = 0;
u8 showingFace = 0;
bool promptingYesNo = false;
u16 waitYesNo = 0;
bool answerYesNo = false;
u8 handSpr;

bool execute_command();
u8 tsc_read_byte();
u16 tsc_read_word();

void tsc_init() {
	exeMode = TSC_IDLE;
	VDP_loadTileSet(&TS_Window, TILE_WINDOWINDEX, true);
	const u8 *headTSC = TSC_Head;
	u8 loadedEvents = 0;
	for(u16 i = 1; loadedEvents < HEAD_EVENT_COUNT; i++) {
		if(headTSC[i] == 0xFF && headTSC[i+1] == 0xFF) {
			head_nums[loadedEvents] = headTSC[i+2]+(headTSC[i+3]<<8);
			head_events[loadedEvents] = &headTSC[i+4];
			loadedEvents++;
			i += 3;
		}
	}
}

void tsc_load(u8 id) {
	const u8 *TSC = stage_info[id].TSC;
	tscEventCount = TSC[0];
	if(tscEventCount > MAX_EVENTS) {
		char str[32] = "Too many events: ";
		intToStr(tscEventCount, &str[17], 1);
		SYS_die(str);
	}
	u8 loadedEvents = 0;
	for(u16 i = 1; loadedEvents < tscEventCount; i++) {
		if(TSC[i] == 0xFF && TSC[i+1] == 0xFF) {
			event_nums[loadedEvents] = TSC[i+2]+(TSC[i+3]<<8);
			event_index[loadedEvents] = &TSC[i+4];
			loadedEvents++;
			i += 3;
		}
	}
}

void tsc_call_event(u16 number) {
	if(number < 50) {
		for(u8 i = 0; i < HEAD_EVENT_COUNT; i++) {
			if(head_nums[i] == number) {
				exeMode = TSC_RUNNING;
				curCommand = head_events[i];
				return;
			}
		}
	} else {
		for(u8 i = 0; i < tscEventCount; i++) {
			if(event_nums[i] == number) {
				exeMode = TSC_RUNNING;
				curCommand = event_index[i];
				return;
			}
		}
	}
	curCommand = event_index[0];
}

u8 tsc_update() {
	switch(exeMode) {
	case TSC_IDLE:
		break; // Nothing to update
	case TSC_RUNNING:
		for(;;) {
			u8 result = execute_command();
			if(result > 0) return result - 1;
		}
		break;
	case TSC_WAITTIME:
		waitTime--;
		if(waitTime == 0) exeMode = TSC_RUNNING;
		break;
	case TSC_WAITINPUT:
		if(joy_pressed(BUTTON_C)) {
			exeMode = TSC_RUNNING;
			if(promptingYesNo) {
				if(!answerYesNo) tsc_call_event(waitYesNo);
				sound_play(SOUND_CONFIRM, 5);
				promptingYesNo = false;
				sprite_delete(handSpr);
			}
		} else if(promptingYesNo) {
			if(joy_pressed(BUTTON_LEFT) | joy_pressed(BUTTON_RIGHT)) {
				answerYesNo = !answerYesNo;
				sound_play(SOUND_CURSOR, 5);
				sprite_set_position(handSpr,
					tile_to_pixel(29-(answerYesNo*6)), tile_to_pixel(20)-4);
			}
		}
		break;
	case TSC_WAITGROUNDED:
		if(player.grounded) exeMode = TSC_RUNNING;
		break;
	default:
		SYS_die("Invalid TSC State");
		break;
	}
	return 0;
}

void window_open() {
	VDP_setTileMapXY(WINDOW, WINDOW_ATTR(0), 2, 19);
	for(u8 x = 3; x < 37; x++)
		VDP_setTileMapXY(WINDOW, WINDOW_ATTR(1), x, 19);
	VDP_setTileMapXY(WINDOW, WINDOW_ATTR(2), 37, 19);
	for(u8 y = 20; y < 26; y++) {
		VDP_setTileMapXY(WINDOW, WINDOW_ATTR(6), 2, y);
		for(u8 x = 3; x < 37; x++) {
			VDP_setTileMapXY(WINDOW, WINDOW_ATTR(7), x, y);
		}
		VDP_setTileMapXY(WINDOW, WINDOW_ATTR(8), 37, y);
	}
	VDP_setTileMapXY(WINDOW, WINDOW_ATTR(12), 2, 26);
	for(u8 x = 3; x < 37; x++)
		VDP_setTileMapXY(WINDOW, WINDOW_ATTR(13), x, 26);
	VDP_setTileMapXY(WINDOW, WINDOW_ATTR(14), 37, 26);
	msgTextX = MSG_LEFT_COLUMN;
	msgTextY = MSG_TOP_ROW;
	VDP_setWindowPos(0, 243);
	msgWindowOpen = true;
}

void window_open_prompt() {
	char str[16];
	VDP_setTileMapXY(WINDOW, WINDOW_ATTR(3), 22, 19);
	for(u8 x = 23; x < 36; x++)
		VDP_setTileMapXY(WINDOW, WINDOW_ATTR(4), x, 19);
	VDP_setTileMapXY(WINDOW, WINDOW_ATTR(5), 36, 19);
	VDP_setTileMapXY(WINDOW, WINDOW_ATTR(9), 22, 20);
	for(u8 x = 23; x < 36; x++) {
		VDP_setTileMapXY(WINDOW, WINDOW_ATTR(10), x, 20);
	}
	VDP_setTileMapXY(WINDOW, WINDOW_ATTR(11), 36, 20);
	VDP_setTileMapXY(WINDOW, WINDOW_ATTR(15), 22, 21);
	for(u8 x = 23; x < 36; x++)
		VDP_setTileMapXY(WINDOW, WINDOW_ATTR(16), x, 21);
	VDP_setTileMapXY(WINDOW, WINDOW_ATTR(17), 36, 21);
	strcpy(str, "Yes / No");
	for(u8 i = 0; str[i] != '\0'; i++) {
		VDP_setTileMapXY(WINDOW,
			TILE_FONTINDEX + str[i] - 0x20, 25 + i, 20);
		msgTextX++;
	}
	sound_play(SOUND_PROMPT, 5);
	// Load hand sprite and move next to yes
	handSpr = sprite_create(&SPR_Pointer, PAL0, true);
	sprite_set_position(handSpr, tile_to_pixel(23), tile_to_pixel(20)-4);
	answerYesNo = true; // Yes is default
	promptingYesNo = true;
}

void tsc_unpause_debug() {
	if(msgWindowOpen) VDP_setReg(0x12, 243);
}

void window_clear() {
	window_open();
	if(showingFace > 0) msgTextX = MSG_LEFT_FACE;
}

void window_close() {
	VDP_setWindowPos(0, 0);
	showingFace = 0;
	msgWindowOpen = false;
}

u8 execute_command() {
	u16 args[4];
	u8 cmd = tsc_read_byte();
	if(cmd >= 0x80) {
		switch(cmd) {
		case CMD_MSG: // Display message box (bottom - visible)
		case CMD_MS2: // Display message box (top - invisible)
		case CMD_MS3: // Display message box (top - visible)
			window_open();
			break;
		case CMD_CLO: // Close message box
			window_close();
			break;
		case CMD_CLR: // Clear message box
			window_clear();
			break;
		case CMD_NUM: // TODO: Show number (1) in message box
			args[0] = tsc_read_word();
			break;
		case CMD_GIT: // TODO: Display item (1) in message box
			args[0] = tsc_read_word();
			break;
		case CMD_FAC: // TODO: Display face (1) in message box
			args[0] = tsc_read_word();
			showingFace = args[0];
			if(showingFace > 0) msgTextX = MSG_LEFT_FACE;
			else msgTextX = MSG_LEFT_COLUMN;
			break;
		case CMD_CAT: // TODO: All 3 of these display text instantly
		case CMD_SAT:
		case CMD_TUR:
			break;
		case CMD_YNJ: // Prompt Yes/No and jump to event (1) if No
			args[0] = tsc_read_word();
			waitYesNo = args[0];
			window_open_prompt();
			exeMode = TSC_WAITINPUT;
			return 1;
		case CMD_END: // End the event
			exeMode = TSC_IDLE;
			window_close();
			player_unlock_controls();
			hud_show();
			//sprite_set_visible(hudSprite, true);
			return 1;
		case CMD_EVE: // Jump to event (1)
			args[0] = tsc_read_word();
			tsc_call_event(args[0]);
			break;
		case CMD_TRA: // Teleport to stage (1), run event (2), coords (3),(4)
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			args[2] = tsc_read_word();
			args[3] = tsc_read_word();
			player.x = block_to_sub(args[2]) + pixel_to_sub(8);
			player.y = block_to_sub(args[3]) + pixel_to_sub(8);
			window_close();
			//player_unlock_controls();
			//sprite_set_visible(hudSprite, true);
			stage_load(args[0]);
			tsc_call_event(args[1]);
			return 1;
		case CMD_INI: // Start from beginning (try again without save data)
			return 4;
		case CMD_ESC: // Restart the game
			return 2;
		case CMD_LDP: // Reload save file (try again)
			return 3;
		case CMD_CMU: // Play song (1), stop if 0
			args[0] = tsc_read_word();
			song_play(args[0]);
			break;
		case CMD_FMU: // Fade out music (we just stop for now)
			song_stop();
			break;
		case CMD_RMU: // Resume previously playing music
			song_resume();
			break;
		case CMD_SOU: // Play sound (1)
			args[0] = tsc_read_word();
			if(sound_info[args[0]].sound != NULL) {
				sound_play(args[0], 0);
			}
			break;
		case CMD_SPS: // TODO: Persistent sounds, skip for now
		case CMD_CPS:
		case CMD_CSS:
			break;
		case CMD_SSS:
			args[0] = tsc_read_word();
			break;
		case CMD_NOD: // Wait for player input
			exeMode = TSC_WAITINPUT;
			return 1;
		case CMD_WAI: // Wait (1) frames
			args[0] = tsc_read_word();
			exeMode = TSC_WAITTIME;
			waitTime = args[0];
			return 1;
		case CMD_WAS: // Wait for player to hit the ground
			exeMode = TSC_WAITGROUNDED;
			return 1;
		case CMD_MM0: // Halt player movement
			// TODO: Find out if this disables physics
			player.x_speed = 0;
			player.y_speed = 0;
			break;
		case CMD_MOV: // Move player to coordinates (1),(2)
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			player.x = block_to_sub(args[0]) + pixel_to_sub(8);
			player.y = block_to_sub(args[1]) + pixel_to_sub(8);
			break;
		case CMD_MYB: // Bounce player in direction (1)
			args[0] = tsc_read_word();
			if(args[0] == 0) { // Left
				player.x_speed = pixel_to_sub(-2);
			} else if(args[0] == 2) { // Right
				player.x_speed = pixel_to_sub(2);
			}
			player.y_speed = pixel_to_sub(-2);
			break;
		case CMD_MYD: // Change direction to (1)
			args[0] = tsc_read_word();
			if(args[0] == 0) { // Left
				playerFacing = 0;
			} else if(args[0] == 2) { // Right
				playerFacing = 1;
			}
			sprite_set_attr(player.sprite, TILE_ATTR(PAL0, false, false, playerFacing));
			break;
		case CMD_UNI: // TODO: Change movement type to (1)
			args[0] = tsc_read_word();
			break;
		case CMD_UNJ: // TODO: If movement type is (1) jump to event (2)
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			break;
		case CMD_KEY: // Lock controls and hide the HUD
			player_lock_controls();
			hud_hide();
			//sprite_set_visible(hudSprite, false);
			break;
		case CMD_PRI: // Lock controls
			player_lock_controls();
			break;
		case CMD_FRE: // Unlock controls
			player_unlock_controls();
			hud_show();
			//sprite_set_visible(hudSprite, true);
			break;
		case CMD_HMC: // Hide player character
			sprite_set_visible(player.sprite, false);
			break;
		case CMD_SMC: // Show player character
			sprite_set_visible(player.sprite, true);
			break;
		case CMD_LI_ADD: // Restore health by (1)
			args[0] = tsc_read_word();
			player_heal(args[0]);
			break;
		case CMD_ML_ADD: // Increase max health by (1)
			args[0] = tsc_read_word();
			player_maxhealth_increase(args[0]);
			break;
		case CMD_ANP: // TODO: Give all entities (1) script state (2) with direction (3)
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			args[2] = tsc_read_word();
			break;
		case CMD_CNP: // TODO: Change all entities (1) to type (2) with direction (3)
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			args[2] = tsc_read_word();
			break;
		case CMD_MNP: // TODO: Move entity (1) to (2),(3) with direction (4)
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			args[2] = tsc_read_word();
			args[3] = tsc_read_word();
			break;
		case CMD_DNA: // Delete all entities of type (1)
			args[0] = tsc_read_word();
			entities_clear(FILTER_TYPE, args[0]);
			break;
		case CMD_DNP: // Delete all entities with event # (1)
			args[0] = tsc_read_word();
			entities_clear(FILTER_EVENT, args[0]);
			break;
		// TODO: Change entity (1) to type (2) with direction (3) and set flag 0x1000?
		case CMD_INP:
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			args[2] = tsc_read_word();
			break;
		case CMD_SNP: // TODO: Create entity (1) at (2),(3) with direction (4)
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			args[2] = tsc_read_word();
			args[3] = tsc_read_word();
			break;
		case CMD_BOA: // TODO: Give map boss state (1)
			args[0] = tsc_read_word();
			break;
		case CMD_BSL: // TODO: Start boss fight with entity (1)
			args[0] = tsc_read_word();
			break;
		case CMD_NCJ: // TODO: If entity type (1) exists jump to event (2)
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			break;
		case CMD_ECJ: // TODO: If entity id (1) exists jump to event (2)
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			break;
		case CMD_AE_ADD: // Refill all weapon ammo
			player_refill_ammo();
			break;
		case CMD_ZAM: // Take away all weapons and ammo
			player_take_allweapons();
			break;
		case CMD_AM_ADD: // Give weapon (1) and ammo (2)
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			player_give_weapon(args[0], args[1]);
			break;
		case CMD_AM_SUB: // Remove weapon (1)
			args[0] = tsc_read_word();
			player_take_weapon(args[0]);
			break;
		case CMD_TAM: // Trade weapon (1) for (2) with (3) max ammo
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			args[2] = tsc_read_word();
			player_trade_weapon(args[0], args[1], args[2]);
			break;
		case CMD_AMJ: // If player has weapon (1) jump to event (2)
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			if(player_has_weapon(args[0])) tsc_call_event(args[1]);
			break;
		case CMD_EQ_ADD: // TODO: Equip item (1)
			args[0] = tsc_read_word();
			break;
		case CMD_EQ_SUB: // TODO: Remove equip (1)
			args[0] = tsc_read_word();
			break;
		case CMD_IT_ADD: // Give item (1)
			args[0] = tsc_read_word();
			player_give_item(args[0]);
			break;
		case CMD_IT_SUB: // Remove item (1)
			args[0] = tsc_read_word();
			player_take_item(args[0]);
			break;
		case CMD_ITJ: // If player has item (1) jump to event (2)
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			if(player_has_item(args[0])) tsc_call_event(args[1]);
			break;
		case CMD_FL_ADD: // Set flag (1)
			args[0] = tsc_read_word();
			system_set_flag(args[0], true);
			break;
		case CMD_FL_SUB: // Unset flag (1)
			args[0] = tsc_read_word();
			system_set_flag(args[0], false);
			break;
		case CMD_FLJ: // If flag (1) is true jump to event (2)
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			if(system_get_flag(args[0])) tsc_call_event(args[1]);
			break;
		case CMD_SK_ADD: // Set skip flag (1)
			args[0] = tsc_read_word();
			system_set_skip_flag(args[0], true);
			break;
		case CMD_SK_SUB: // Unset skip flag (1)
			args[0] = tsc_read_word();
			system_set_skip_flag(args[0], false);
			break;
		case CMD_SKJ: // If skip flag (1) is true jump to event (2)
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			if(system_get_skip_flag(args[0])) tsc_call_event(args[1]);
			break;
		case CMD_FOB: // Focus on boss/NPC (1) with (2) ticks
		case CMD_FON: // TODO: Figure out the difference between these
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			Entity *e = entity_find_by_id(args[0]);
			if(e != NULL) camera.target = e;
			else camera.target = &player;
			break;
		case CMD_FOM: // Focus on player at (1) speed
			args[0] = tsc_read_word();
			camera.target = &player;
			break;
		case CMD_QUA: // Shake camera for (1) frames
			args[0] = tsc_read_word();
			camera_shake(args[0]);
			break;
		case CMD_FAI: // TODO: Fading, in direction (1)
			args[0] = tsc_read_word();
			//VDP_setEnable(true);
			break;
		case CMD_FAO:
			args[0] = tsc_read_word();
			//VDP_setEnable(false);
			break;
		case CMD_FLA: // TODO: Flash screen white
			break;
		case CMD_MLP: // TODO: Show the map
			break;
		case CMD_MNA: // TODO: Show stage name
			break;
		case CMD_CMP: // TODO: Change stage tile at (1),(2) to type (3)
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			args[2] = tsc_read_word();
			break;
		case CMD_MP_ADD: // TODO: Map flag (1)
			args[0] = tsc_read_word();
			break;
		case CMD_MPJ: // TODO: If map flag (1) set jump to event (2)
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			break;
		case CMD_CRE: // TODO: Show credits
			break;
		case CMD_SIL: // TODO: Show illustration (1) in the credits
			args[0] = tsc_read_word();
			break;
		case CMD_CIL: // TODO: Clear illustration in the credits
			break;
		case CMD_SLP: // TODO: Show the teleporter menu
			break;
		case CMD_PS_ADD: // TODO: Set teleporter slot (1) to event (2)
			args[0] = tsc_read_word();
			args[1] = tsc_read_word();
			break;
		case CMD_SVP: // Save
			system_save();
			break;
		case CMD_STC: // TODO: Save counter
			break;
		case CMD_XX1: // TODO: Island effect
			args[0] = tsc_read_word();
			break;
		default:
			//intToHex(exeCursor[0], str, 2);
			//strcat(str, "h bad CMD");
			//SYS_die(str);
			break;
		}
	} else if(msgWindowOpen) {
		if(cmd == '\n') {
			msgTextY++;
			if(showingFace > 0) msgTextX = MSG_LEFT_FACE;
			else msgTextX = MSG_LEFT_COLUMN;
			if(msgTextY > 25) {
				//msgTextY = 20;
				window_clear();
			}
		} else {
			VDP_setTileMapXY(WINDOW, TILE_ATTR_FULL(PAL0, true, false, false,
					TILE_FONTINDEX + cmd - 0x20), msgTextX, msgTextY);
			msgTextX++;
		}
		return 1;
	} else {
		//intToHex(exeCursor[0], str, 2);
		//strcat(str, "h bad CMD");
		//SYS_die(str);
	}
	return 0;
}

u8 tsc_read_byte() {
	u8 byte = curCommand[0];
	curCommand++;
	return byte;
}

u16 tsc_read_word() {
	u16 word = curCommand[0]+(curCommand[1]<<8);
	curCommand += 2;
	return word;
}
