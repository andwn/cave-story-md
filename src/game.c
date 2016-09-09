#include "gamemode.h"

#include <genesis.h>
#include "resources.h"
#include "input.h"
#include "system.h"
#include "player.h"
#include "stage.h"
#include "camera.h"
#include "tables.h"
#include "tsc.h"
#include "vdp_ext.h"
#include "effect.h"
#include "hud.h"
#include "weapon.h"
#include "window.h"
#include "audio.h"
#include "ai.h"
#include "sheet.h"
#include "sprite.h"

// Item menu stuff
VDPSprite itemSprite[MAX_ITEMS];
u8 selectedItem = 0;

void draw_itemmenu();
bool update_pause();
void itemcursor_move(u8 oldindex, u8 index);

// Initializes or re-initializes the game after "try again"
void game_reset(bool load);

void vblank() {
	if(water_screenlevel != WATER_DISABLE) vblank_water(); // Water effect
	sprites_send();
	stage_update(); // Scrolling
	if(hudRedrawPending) hud_update_vblank();
}

u8 game_main(bool load) {
	SYS_disableInts();
	VDP_setEnable(false);
	// This is the SGDK font with a blue background for the message window
	VDP_loadTileSet(&TS_MsgFont, TILE_FONTINDEX, true);
	SYS_setVIntCallback(vblank);
	SYS_setHIntCallback(hblank_water);
	effects_init();
	game_reset(load);
	
	VDP_setEnable(true);
	VDP_setWindowPos(0, 0);
	SYS_enableInts();
	// Load game doesn't run a script that fades in and shows the HUD, so do it manually
	if(load) {
		hud_show();
		VDP_fadeTo(0, 63, VDP_getCachedPalette(), 20, true);
	}
	
	paused = false;
	u8 ending = 0;
	
	while(true) {
		input_update();
		if(paused) {
			paused = update_pause();
		} else {
			// Pressing start opens the item menu (unless a script is running)
			if(!tscState && joy_pressed(BUTTON_START)) {
				draw_itemmenu();
				// This unloads the stage's script and loads the "ArmsItem" script in its place
				tsc_load_stage(255);
				paused = true;
			} else {
				// HUD on top
				hud_update();
				// Handle controller locking
				u16 lockstate = joystate, oldlockstate = oldstate;
				if(controlsLocked) joystate = oldstate = 0;
				// Don't update this stuff if a script is using <PRI
				if(!gameFrozen) {
					camera_update();
					player_update();
					entities_update();
					if(showingBossHealth) tsc_update_boss_health();
				}
				// Restore controller locking if it was locked
				joystate = lockstate;
				oldstate = oldlockstate;
				// Run the next set of commands in a script if it is running
				u8 rtn = tsc_update();
				// Nonzero return values exit the game, or switch to the ending sequence
				if(rtn > 0) {
					if(rtn == 1) {
						ending = 0; // No ending, return to title
						break;
					} else if(rtn == 2) {
						game_reset(true); // Reload save
						hud_show();
						VDP_fadeTo(0, 63, VDP_getCachedPalette(), 20, true);
						continue;
					} else if(rtn == 3) {
						game_reset(false); // Start from beginning
						continue;
					} else if(rtn == 4) {
						ending = 1; // Normal ending
						break;
					} else if(rtn == 5) {
						ending = 2; // Good ending
						break;
					}
				}
				// Get the sprites ready
				effects_update();
				entities_draw_fore();
				player_draw();
				entities_draw_back();
				system_update();
			}
		}
		VDP_waitVSync();
	}
	
	if(ending) { // You are a winner
		
	} else { // Going back to title screen
		// Screw this just doing a hard reset
		SYS_hardReset();
	}
	return ending;
}

void game_reset(bool load) {
	camera_init();
	tsc_init();
	gameFrozen = false;
	if(load) {
		system_load();
		const SpriteDefinition *wepSpr = weapon_info[playerWeapon[currentWeapon].type].sprite;
		if(wepSpr) TILES_QUEUE(SPR_TILES(wepSpr,0,0), TILE_WEAPONINDEX,6);
	} else {
		system_new();
		tsc_call_event(GAME_START_EVENT);
	}
	// Load up the main palettes
	VDP_setCachedPalette(PAL0, PAL_Main.data);
	VDP_setCachedPalette(PAL1, PAL_Sym.data);
	VDP_setPaletteColors(0, PAL_FadeOut, 64);
	// Default sprite sheets
	sheets_init();
}

void draw_itemmenu() {
	SYS_disableInts();
	// Fill the top part with blue
	// TODO: Make this prettier
	VDP_fillTileMap(VDP_PLAN_WINDOW, TILE_FONTINDEX, 0, 64 * 20);
	// Load the 4 tiles for the selection box. Since the menu can never be brought up
	// during scripts we overwrite the face image
	VDP_loadTileSet(&TS_ItemSel, TILE_FACEINDEX, true);
	// Redraw message box at the bottom of the screen
	window_clear();
	// Weapons
	VDP_drawTextWindow("--ARMS--", 16, 3);
	// TODO: Draw the status for all weapons the player owns under --ARMS--
	// Items
	VDP_drawTextWindow("--ITEM--", 16, 10);
	u8 held = 0;
	for(u16 i = 0; i < MAX_ITEMS; i++) {
		u16 item = playerInventory[i];
		if(item > 0) {
			// Wonky workaround to use either PAL_Sym or PAL_Main
			const SpriteDefinition *sprDef = &SPR_ItemImage;
			u16 pal = PAL1;
			if(ITEM_PAL[item]) {
				sprDef = &SPR_ItemImageG;
				pal = PAL0;
			}
			// Clobber the entity/bullet shared sheets
			SHEET_LOAD(sprDef, 1, 6, TILE_SHEETINDEX+held*6, true, item,0);
			itemSprite[i] = (VDPSprite){
				.x = 24 + (i % 8) * 32 + 128, 
				.y = 88 + (i / 8) * 16 + 128, 
				.size = SPRITE_SIZE(3, 2),
				.attribut = TILE_ATTR_FULL(pal,1,0,0,TILE_SHEETINDEX+held*6)
			};
			held++;
		} else {
			itemSprite[i] = (VDPSprite){};
		}
	}
	// Draw item cursor at first index (default selection)
	itemcursor_move(0, 0);
	// These routines handle hiding or deleting sprites
	//player_pause();
	//entities_pause();
	//hud_hide();
	// Make the window plane fully overlap the game
	VDP_setWindowPos(0, 28);
	SYS_enableInts();
}

bool update_pause() {
	// Start will close the menu and resume the game
	if(joy_pressed(BUTTON_START)) {
		// Reload shared sheets we clobbered
		SYS_disableInts();
		sheets_init();
		sheets_load_stage(stageID);
		SYS_enableInts();
		
		selectedItem = 0;
		// Reload TSC Events for the current stage
		tsc_load_stage(stageID);
		// Put the sprites for player/entities/HUD back
		player_unpause();
		//entities_unpause();
		//hud_show();
		VDP_setWindowPos(0, 0);
		return false;
	} else {
		// Every cursor move and item selection runs a script
		// Weapons are 1000 + ID
		// Items are 5000 + ID
		// Item descriptions are 6000 + ID
		if(tscState) {
			tsc_update();
		} else if(joy_pressed(BUTTON_C) && playerInventory[selectedItem] > 0) {
			tsc_call_event(6000 + playerInventory[selectedItem]);
		} else if(joy_pressed(BUTTON_LEFT)) {
			itemcursor_move(selectedItem, selectedItem > 0 ? selectedItem - 1 : MAX_ITEMS - 1);
			sound_play(SND_MENU_MOVE, 5);
			selectedItem--;
			selectedItem %= 32;
			tsc_call_event(5000 + playerInventory[selectedItem]);
		} else if(joy_pressed(BUTTON_UP)) {
			itemcursor_move(selectedItem, selectedItem < 8 ? selectedItem + 24 : selectedItem - 8);
			sound_play(SND_MENU_MOVE, 5);
			selectedItem = selectedItem < 8 ? selectedItem + 24 : selectedItem - 8;
			selectedItem %= 32;
			tsc_call_event(5000 + playerInventory[selectedItem]);
		} else if(joy_pressed(BUTTON_RIGHT)) {
			itemcursor_move(selectedItem, (selectedItem + 1) % 32);
			sound_play(SND_MENU_MOVE, 5);
			selectedItem++;
			selectedItem %= 32;
			tsc_call_event(5000 + playerInventory[selectedItem]);
		} else if(joy_pressed(BUTTON_DOWN)) {
			itemcursor_move(selectedItem, (selectedItem + 8) % 32);
			sound_play(SND_MENU_MOVE, 5);
			selectedItem += 8;
			selectedItem %= 32;
			tsc_call_event(5000 + playerInventory[selectedItem]);
		}
		for(u8 i = MAX_ITEMS; i--; ) if(itemSprite[i].y) sprite_add(itemSprite[i]);
	}
	return true;
}

void itemcursor_move(u8 oldindex, u8 index) {
	// Erase old position
	u16 x = 3 + (oldindex % 8) * 4;
	u16 y = 11 + (oldindex / 8) * 2;
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FONTINDEX, x,   y);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FONTINDEX, x+3, y);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FONTINDEX, x,   y+1);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FONTINDEX, x+3, y+1);
	// Draw new position
	x = 3 + (index % 8) * 4;
	y = 11 + (index / 8) * 2;
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FACEINDEX,   x,   y);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FACEINDEX+1, x+3, y);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FACEINDEX+2, x,   y+1);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FACEINDEX+3, x+3, y+1);
}
