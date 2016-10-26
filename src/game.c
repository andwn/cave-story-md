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

// Prevents incomplete sprite list from being sent to VDP (flickering)
volatile u8 ready;

void draw_itemmenu(u8 resetCursor);
u8 update_pause();
void itemcursor_move(u8 oldindex, u8 index);
void gen_maptile(u16 bx, u16 by, u16 index);

// Initializes or re-initializes the game after "try again"
void game_reset(u8 load);

void vblank() {
	dqueued = FALSE;
	//if(water_screenlevel != WATER_DISABLE) vblank_water(); // Water effect
	if(ready) {
		sprites_send();
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
					if(rtn == 1) {
						ending = 0; // No ending, return to title
						break;
					} else if(rtn == 2) {
						game_reset(TRUE); // Reload save
						hud_show();
						VDP_fadeTo(0, 63, VDP_getCachedPalette(), 20, TRUE);
						continue;
					} else if(rtn == 3) {
						game_reset(FALSE); // Start from beginning
						continue;
					} else if(rtn == 4) {
						ending = 1; // Normal ending
						break;
					} else if(rtn == 5) {
						ending = 2; // Good ending
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
	
	if(ending) { // You are a winner
		
	} else { // Going back to title screen
		// Screw this just doing a hard reset
		SYS_hardReset();
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
		system_load();
		const SpriteDefinition *wepSpr = weapon_info[playerWeapon[currentWeapon].type].sprite;
		if(wepSpr) TILES_QUEUE(SPR_TILES(wepSpr,0,0), TILE_WEAPONINDEX,6);
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
	SYS_disableInts();
	// Fill the top part
	VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,TILE_WINDOWINDEX), 1, 0);
	VDP_fillTileMap(VDP_PLAN_WINDOW, TILE_WINDOWINDEX+1, 2, 36); // Upper mid
	VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,TILE_WINDOWINDEX+2), 38, 0);
	for(u16 y = 1; y < 19; y++) {
		VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,TILE_WINDOWINDEX+3), 1, y);
		VDP_fillTileMap(VDP_PLAN_WINDOW, TILE_FONTINDEX, y*64 + 2, 36); // Body
		VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,TILE_WINDOWINDEX+5), 38, y);
	}
	VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,TILE_WINDOWINDEX+6), 1, 19);
	VDP_fillTileMap(VDP_PLAN_WINDOW, TILE_WINDOWINDEX+7, 19*64 + 2, 36); // Lower mid
	VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,TILE_WINDOWINDEX+8), 38, 19);
	// Load the 4 tiles for the selection box. Since the menu can never be brought up
	// during scripts we overwrite the face image
	VDP_loadTileSet(&TS_ItemSel, TILE_FACEINDEX, TRUE);
	// Redraw message box at the bottom of the screen
	window_open(FALSE);
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
			SHEET_LOAD(sprDef, 1, 6, TILE_SHEETINDEX+held*6, TRUE, item,0);
			itemSprite[i] = (VDPSprite){
				.x = 36 + (i % 8) * 32 + 128, 
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
		return FALSE;
	} else {
		// Every cursor move and item selection runs a script
		// Weapons are 1000 + ID
		// Items are 5000 + ID
		// Item descriptions are 6000 + ID
		if(tscState) {
			u8 overid = playerInventory[selectedItem];
			tsc_update();
			// Item was comsumed, have to adjust the icons
			if(playerInventory[selectedItem] != overid) {
				draw_itemmenu(FALSE);
				//for(u8 i = selectedItem; i < MAX_ITEMS - 1; i++) {
				//	if(playerInventory[i + 1]) {
				//		itemSprite[i].attribut = itemSprite[i + 1].attribut;
				//	} else {
				//		itemSprite[i].y = 0;
				//		break;
				//	}
				//}
			}
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
	return TRUE;
}

void itemcursor_move(u8 oldindex, u8 index) {
	// Erase old position
	u16 x = 4 + (oldindex % 8) * 4;
	u16 y = 11 + (oldindex / 8) * 2;
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FONTINDEX, x,   y);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FONTINDEX, x+3, y);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FONTINDEX, x,   y+1);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FONTINDEX, x+3, y+1);
	// Draw new position
	x = 4 + (index % 8) * 4;
	y = 11 + (index / 8) * 2;
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FACEINDEX,   x,   y);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FACEINDEX+1, x+3, y);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FACEINDEX+2, x,   y+1);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FACEINDEX+3, x+3, y+1);
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
