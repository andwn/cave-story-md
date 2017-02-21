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
s8 selectedItem = 0;

// Prevents incomplete sprite list from being sent to VDP (flickering)
volatile u8 ready;

void draw_itemmenu(u8 resetCursor);
u8 update_pause();
void itemcursor_move(s8 oldindex, s8 index);
void gen_maptile(u16 bx, u16 by, u16 index);

// Initializes or re-initializes the game after "try again"
void game_reset(u8 load);

void vblank() {
	dqueued = FALSE;
	//if(water_screenlevel != WATER_DISABLE) vblank_water(); // Water effect
	if(ready) {
		if(inFade) { spr_num = 0; }
		else { sprites_send(); }
		ready = FALSE;
	} else {
		puts("Vint before main loop finished!");
	}
	stage_update(); // Scrolling
}

u8 game_main(u8 load) {
	SYS_disableInts();
	VDP_setEnable(FALSE);
	// This is the SGDK font with a blue background for the message window
	VDP_loadTileSet(&TS_MsgFont, TILE_FONTINDEX, TRUE);
	SYS_setVIntCallback(vblank);
	//SYS_setHIntCallback(hblank_water);
	effects_init();
	game_reset(load);
	
	VDP_setEnable(TRUE);
	VDP_setWindowPos(0, 0);
	
	// Load game doesn't run a script that fades in and shows the HUD, so do it manually
	if(load) {
		hud_show();
		VDP_fadeTo(0, 63, VDP_getCachedPalette(), 20, TRUE);
	}
	SYS_enableInts();
	
	paused = FALSE;
	u8 ending = 0;
	
	while(TRUE) {
		input_update();
		if(paused) {
			paused = update_pause();
		} else {
			// Pressing start opens the item menu (unless a script is running)
			if(joy_pressed(BUTTON_START) && !tscState) {
				// This unloads the stage's script and loads the "ArmsItem" script in its place
				tsc_load_stage(255);
				draw_itemmenu(TRUE);
				paused = TRUE;
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
					//if(showingBossHealth) tsc_update_boss_health();
				}
				// Restore controller locking if it was locked
				joystate = lockstate;
				oldstate = oldlockstate;
				// Run the next set of commands in a script if it is running
				u8 rtn = tsc_update();
				// Nonzero return values exit the game, or switch to the ending sequence
				if(rtn > 0) {
					if(rtn == 1) { // Return to title screen
						SYS_hardReset();
					} else if(rtn == 2) {
						game_reset(TRUE); // Reload save
						hud_show();
						playerIFrames = 0;
						VDP_fadeTo(0, 63, VDP_getCachedPalette(), 20, TRUE);
						continue;
					} else if(rtn == 3) {
						game_reset(FALSE); // Start from beginning
						continue;
					} else { // End credits
						break;
					}
				}
				window_update();
				// Get the sprites ready
				effects_update();
				player_draw();
				entities_draw();
			}
		}
		system_update();
		ready = TRUE;
		VDP_waitVSync();
	}
	return ending;
}

void game_reset(u8 load) {
	camera_init();
	tsc_init();
	hud_create();
	// Default sprite sheets
	sheets_load_stage(255, TRUE, TRUE);
	gameFrozen = FALSE;
	if(load) {
		if(load == 1) system_load();
		if(load >= 4) system_load_levelselect(load - 4);
		const SpriteDefinition *wepSpr = weapon_info[playerWeapon[currentWeapon].type].sprite;
		if(wepSpr) TILES_QUEUE(SPR_TILES(wepSpr,0,0), TILE_WEAPONINDEX,6);
		playerHasMimiMask = player_has_item(18);
		//sheets_refresh_weapons();
	} else {
		system_new();
		tsc_call_event(GAME_START_EVENT);
	}
	SHEET_LOAD(&SPR_Bonk, 1, 1, 1, 1, 0,0);
	// Load up the main palettes
	VDP_setCachedPalette(PAL0, PAL_Main.data);
	VDP_setCachedPalette(PAL1, PAL_Sym.data);
	VDP_setPaletteColors(0, PAL_FadeOut, 64);
}

void draw_itemmenu(u8 resetCursor) {
	// Hide sprites
	spr_num = 0;
	sprite_add(((VDPSprite) { .x = 128, .y = 128, .size = SPRITE_SIZE(1, 1) }));
	ready = TRUE;
	VDP_waitVSync();
	
	SYS_disableInts();
	// Fill the top part
	VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,TILE_WINDOWINDEX), 1, 0);
	VDP_fillTileMap(VDP_PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,TILE_WINDOWINDEX+1), 2, 36);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,TILE_WINDOWINDEX+2), 38, 0);
	for(u16 y = 1; y < 19; y++) { // Body
		VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,TILE_WINDOWINDEX+3), 1, y);
		VDP_fillTileMap(VDP_PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,TILE_FONTINDEX), y*64 + 2, 36);
		VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,TILE_WINDOWINDEX+5), 38, y);
	}
	// Bottom
	VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,TILE_WINDOWINDEX+6), 1, 19);
	VDP_fillTileMap(VDP_PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,TILE_WINDOWINDEX+7), 19*64 + 2, 36);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,TILE_WINDOWINDEX+8), 38, 19);
	// Load the 4 tiles for the selection box. Since the menu can never be brought up
	// during scripts we overwrite the face image
	VDP_loadTileSet(&TS_ItemSel, TILE_FACEINDEX, TRUE);
	// Redraw message box at the bottom of the screen
	window_open(FALSE);
	// Weapons
	VDP_drawTextWindow("--ARMS--", 4, 3);
	//VDP_loadTileData(TS_Numbers.tiles, TILE_FACEINDEX+4, 10, TRUE);
	//VDP_loadTileData(SPR_TILES(&SPR_Numbers, 0, 0), TILE_SHEETINDEX+10, 10, TRUE);
	for(u16 i = 0; i < MAX_WEAPONS; i++) {
		Weapon *w = &playerWeapon[i];
		if(!w->type) continue;
		// X tile pos and VRAM index to put the ArmsImage tiles
		u16 x = 4 + i*6, y = 4;
		u16 index = TILE_FACEINDEX + 16 + i*4;
		VDP_loadTileData(SPR_TILES(&SPR_ArmsImage, 0, w->type), index, 4, TRUE);
		// 4 mappings for ArmsImage icon
		VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,index),   x,   y);
		VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,index+2), x+1, y);
		VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,index+1), x,   y+1);
		VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,index+3), x+1, y+1);
		// Level
		char str[6];
		sprintf(str, "Lv %hu", w->level);
		VDP_drawTextWindow(str, x, y+2);
		// Ammo & Max Ammo
		if(w->maxammo) {
			sprintf(str, " %3hu", w->ammo);
			VDP_drawTextWindow(str, x, y+3);
			sprintf(str, "/%3hu", w->maxammo);
			VDP_drawTextWindow(str, x, y+4);
		} else {
			VDP_drawTextWindow("  --", x, y+3);
			VDP_drawTextWindow("/ --", x, y+4);
		}
	}
	// Items
	VDP_drawTextWindow("--ITEM--", 4, 10);
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
			SHEET_LOAD(sprDef, 1, 6, TILE_SHEETINDEX+held*6, TRUE, item,0);
			itemSprite[i] = (VDPSprite){
				.x = 36 + (i % 6) * 32 + 128, 
				.y = 88 + (i / 6) * 16 + 128, 
				.size = SPRITE_SIZE(3, 2),
				.attribut = TILE_ATTR_FULL(pal,1,0,0,TILE_SHEETINDEX+held*6)
			};
			held++;
		} else {
			itemSprite[i] = (VDPSprite){};
		}
	}
	// Draw item cursor at first index (default selection)
	if(resetCursor) selectedItem = 0;
	itemcursor_move(0, selectedItem);
	tsc_call_event(5000 + playerInventory[selectedItem]);
	// Make the window plane fully overlap the game
	VDP_setWindowPos(0, 28);
	// Handle 0 items - if we don't draw any sprites at all, the non-menu sprites
	// will keep drawing. Draw a blank sprite in the upper left corner to work around this
	if(!held) {
		spr_num = 0;
		sprite_add(((VDPSprite) { .x = 128, .y = 128, .size = SPRITE_SIZE(1, 1) }));
	}
	SYS_enableInts();
}

u8 update_pause() {
	// Start or B will close the menu and resume the game
	if((joy_pressed(BUTTON_START) || joy_pressed(BUTTON_B)) && !tscState) {
		// Hide the item sprites before loading the tiles back
		// This way we won't see heart/energy overlap the items for a split second
		spr_num = 0;
		sprite_add(((VDPSprite) { .x = 128, .y = 128, .size = SPRITE_SIZE(1, 1) }));
		ready = TRUE;
		VDP_waitVSync();
		// Reload shared sheets we clobbered
		SYS_disableInts();
		sheets_load_stage(stageID, TRUE, FALSE);
		SYS_enableInts();
		
		selectedItem = 0;
		// Reload TSC Events for the current stage
		tsc_load_stage(stageID);
		// Put the sprites for player/entities/HUD back
		player_unpause();
		controlsLocked = FALSE;
		gameFrozen = FALSE;
		hud_show();
		VDP_setWindowPos(0, 0);
		window_close();
		return FALSE;
	} else {
		// Every cursor move and item selection runs a script
		// Weapons are 1000 + ID
		// Items are 5000 + ID
		// Item descriptions are 6000 + ID
		if(tscState) {
			if(selectedItem >= 0) { // Item
				u8 overid = playerInventory[selectedItem];
				tsc_update();
				// Item was comsumed, have to adjust the icons
				if(playerInventory[selectedItem] != overid) {
					draw_itemmenu(FALSE);
				}
			} else { // Weapon
				tsc_update();
			}
		} else if(joy_pressed(BUTTON_C)) {
			if(selectedItem >= 0) { // Item
				if(playerInventory[selectedItem] > 0) {
					tsc_call_event(6000 + playerInventory[selectedItem]);
				}
			} else if(playerWeapon[selectedItem + 6].type > 0) { // Weapon
				currentWeapon = selectedItem + 6;
				sound_play(SND_SWITCH_WEAPON, 5);
				if(weapon_info[playerWeapon[currentWeapon].type].sprite) {
					TILES_QUEUE(
						SPR_TILES(weapon_info[playerWeapon[currentWeapon].type].sprite,0,0),
						TILE_WEAPONINDEX,6);
				}
			}
		} else if(joy_pressed(BUTTON_LEFT)) {
			s8 newsel = selectedItem % 6 != 0 ? selectedItem - 1 : selectedItem + 5;
			if(newsel == -1) newsel = -2;
			itemcursor_move(selectedItem, newsel);
			sound_play(SND_MENU_MOVE, 5);
			selectedItem = newsel;
			if(selectedItem >= 0) { // Item
				tsc_call_event(5000 + playerInventory[selectedItem]);
			} else { // Weapon
				tsc_call_event(1000 + playerWeapon[selectedItem + 6].type);
			}
		} else if(joy_pressed(BUTTON_UP)) {
			s8 newsel = selectedItem >= 0 ? selectedItem - 6 : selectedItem + 24;
			if(newsel == -1) newsel = -2;
			itemcursor_move(selectedItem, newsel);
			sound_play(SND_MENU_MOVE, 5);
			selectedItem = newsel;
			if(selectedItem >= 0) { // Item
				tsc_call_event(5000 + playerInventory[selectedItem]);
			} else { // Weapon
				tsc_call_event(1000 + playerWeapon[selectedItem + 6].type);
			}
		} else if(joy_pressed(BUTTON_RIGHT)) {
			s8 newsel = selectedItem % 6 != 5 ? selectedItem + 1 : selectedItem - 5;
			if(newsel == -1) newsel = -6;
			itemcursor_move(selectedItem, newsel);
			sound_play(SND_MENU_MOVE, 5);
			selectedItem = newsel;
			if(selectedItem >= 0) { // Item
				tsc_call_event(5000 + playerInventory[selectedItem]);
			} else { // Weapon
				tsc_call_event(1000 + playerWeapon[selectedItem + 6].type);
			}
		} else if(joy_pressed(BUTTON_DOWN)) {
			s8 newsel = selectedItem < MAX_ITEMS - 6 ? selectedItem + 6 : selectedItem - 24;
			if(newsel == -1) newsel = -2;
			itemcursor_move(selectedItem, newsel);
			sound_play(SND_MENU_MOVE, 5);
			selectedItem = newsel;
			if(selectedItem >= 0) { // Item
				tsc_call_event(5000 + playerInventory[selectedItem]);
			} else { // Weapon
				tsc_call_event(1000 + playerWeapon[selectedItem + 6].type);
			}
		}
		for(u8 i = MAX_ITEMS; i--; ) if(itemSprite[i].y) sprite_add(itemSprite[i]);
	}
	return TRUE;
}

void itemcursor_move(s8 oldindex, s8 index) {
	// Erase old position
	u16 x, y, w, h;
	if(oldindex >= 0) {
		x = 4 + (oldindex % 6) * 4;
		y = 11 + (oldindex / 6) * 2;
		w = 3; 
		h = 1;
	} else {
		x = 3 + (oldindex + 6) * 6;
		y = 4;
		w = 5;
		h = 4;
	}
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FONTINDEX, x,   y);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FONTINDEX, x+w, y);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FONTINDEX, x,   y+h);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FONTINDEX, x+w, y+h);
	// Draw new position
	if(index >= 0) {
		x = 4 + (index % 6) * 4;
		y = 11 + (index / 6) * 2;
		w = 3; 
		h = 1;
	} else {
		x = 3 + (index + 6) * 6;
		y = 4;
		w = 5;
		h = 4;
	}
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FACEINDEX,   x,   y);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FACEINDEX+1, x+w, y);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FACEINDEX+2, x,   y+h);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FACEINDEX+3, x+w, y+h);
}

void do_map() {
	// Disable sprites
	spr_num = 0;
	sprite_add(((VDPSprite) { .x = 128, .y = 128, .size = SPRITE_SIZE(1, 1) }));
	
	u16 mapx = (SCREEN_HALF_W - stageWidth / 2) / 8;
	u16 mapy = (SCREEN_HALF_H - stageHeight / 2) / 8;
	
	u16 index = TILE_SHEETINDEX;
	
	for(u16 y = 0; y < (stageHeight / 8) + (stageHeight % 8 > 0); y++) {
		SYS_disableInts();
		for(u16 x = 0; x < (stageWidth / 8) + (stageWidth % 8 > 0); x++) {
			gen_maptile(x*8, y*8, index);
			VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,index), mapx+x, mapy+y);
			index++;
		}
		SYS_enableInts();
		ready = TRUE;
		VDP_waitVSync();
	}
	
	VDPSprite whereami = (VDPSprite) {
		.x = mapx * 8 + sub_to_block(player.x) - 4 + 128,
		.y = mapy * 8 + sub_to_block(player.y) - 4 + 128,
		.size = SPRITE_SIZE(1,1),
		.attribut = TILE_ATTR_FULL(PAL0,1,0,0,1)
	};
	u16 blinkTimer = 0;
	
	while(!joy_pressed(BUTTON_B) && !joy_pressed(BUTTON_C)) {
		input_update();
		system_update();
		// Alternate between the small plus and transparency
		// We can't simply "not draw" the sprite because the VDP will draw it anyway
		whereami.attribut &= ~1;
		if(++blinkTimer & 16) whereami.attribut |= 1;
		sprite_add(whereami);
		ready = TRUE;
		VDP_waitVSync();
	}
	draw_itemmenu(FALSE);
}

void gen_maptile(u16 bx, u16 by, u16 index) {
	u32 tile[8];
	for(u16 y = 0; y < 8; y++) {
		if(by+y >= stageHeight) {
			tile[y] = 0x11111111;
		} else {
			tile[y] = 0;
			for(u16 x = 0; x < 8; x++) {
				tile[y] |= bx+x >= stageWidth ? (1 << ((7-x)*4))
						:  stage_get_block_type(bx+x, by+y) == 0x41 ? (11 << ((7-x)*4)) 
						:  stage_get_block_type(bx+x, by+y) == 0x43 ? (10 << ((7-x)*4))
						:  stage_get_block_type(bx+x, by+y) == 0x01 ? (9 << ((7-x)*4)) 
						:  (1 << ((7-x)*4));
			}
		}
	}
	DMA_doDma(DMA_VRAM, (u32)tile, index*32, 16, 2);
}
