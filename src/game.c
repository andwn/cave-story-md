#include "common.h"

#include "ai.h"
#include "audio.h"
#include "camera.h"
#include "dma.h"
#include "effect.h"
#include "entity.h"
#include "hud.h"
#include "input.h"
#include "joy.h"
#include "player.h"
#include "resources.h"
#include "sheet.h"
#include "sprite.h"
#include "stage.h"
#include "string.h"
#include "system.h"
#include "tables.h"
#include "tsc.h"
#include "vdp.h"
#include "vdp_bg.h"
#include "vdp_pal.h"
#include "vdp_tile.h"
#include "vdp_ext.h"
#include "weapon.h"
#include "window.h"

#include "gamemode.h"

// Item menu stuff
VDPSprite itemSprite[MAX_ITEMS];
int8_t selectedItem = 0;

void draw_itemmenu(uint8_t resetCursor);
uint8_t update_pause();
void itemcursor_move(int8_t oldindex, int8_t index);
uint8_t gen_maptile(uint16_t bx, uint16_t by, uint16_t index);

// Initializes or re-initializes the game after "try again"
void game_reset(uint8_t load);

void game_main(uint8_t load) {
	gamemode = GM_GAME;
	
	VDP_setPaletteColors(0, PAL_FadeOut, 64);
	VDP_setPaletteColor(15, 0x000);
	// This is the SGDK font with a blue background for the message window
	if(!cfg_language) {
		VDP_loadTileSet(&TS_MsgFont, TILE_FONTINDEX, TRUE);
	}
	effects_init();
	game_reset(load);
	VDP_setWindowPos(0, 0);
	// Load game doesn't run a script that fades in and shows the HUD, so do it manually
	if(load) {
		hud_show();
		VDP_fadeTo(0, 63, VDP_getCachedPalette(), 20, TRUE);
	}
	paused = FALSE;

	PF_INIT();
	
	while(TRUE) {
		PF_START_FRAME();

		input_update();
		if(paused) {
			paused = update_pause();
		} else {
			// Pressing start opens the item menu (unless a script is running)
			if(joy_pressed(btn[cfg_btn_pause]) && !tscState) {
				// This unloads the stage's script and loads the "ArmsItem" script in its place
				tsc_load_stage(255);
				draw_itemmenu(TRUE);
				paused = TRUE;
			} else if(joy_pressed(btn[cfg_btn_map]) && !tscState 
					&& (playerEquipment & EQUIP_MAPSYSTEM)) {
				// Shorthand to open map system
				VDP_setEnable(FALSE);
				if(stageBackgroundType == 4) {
					// Hide water
					static const uint32_t black[8] = {
						0x11111111,0x11111111,0x11111111,0x11111111,
						0x11111111,0x11111111,0x11111111,0x11111111
					};
					VDP_loadTileData(black, TILE_FACEINDEX, 1, TRUE);
					VDP_fillTileMap(VDP_PLAN_WINDOW, 
							TILE_ATTR_FULL(PAL0,1,0,0,TILE_FACEINDEX), 0, 64*30);
				} else {
					VDP_clearPlan(PLAN_WINDOW, TRUE);
				}
				VDP_setWindowPos(0, IS_PALSYSTEM ? 30 : 28);
				VDP_setEnable(TRUE);
				do_map();
				VDP_setEnable(FALSE);
				hud_force_redraw();
				VDP_loadTileData(TILE_BLANK,TILE_HUDINDEX+8,1,1);
				VDP_loadTileData(TILE_BLANK,TILE_HUDINDEX+9,1,1);
				VDP_loadTileData(TILE_BLANK,TILE_HUDINDEX+12,1,1);
				VDP_loadTileData(TILE_BLANK,TILE_HUDINDEX+13,1,1);
				sheets_load_stage(stageID, TRUE, FALSE);
				player_draw();
				entities_draw();
				hud_show();
				sprites_send();
				VDP_setWindowPos(0, 0);
				VDP_setEnable(TRUE);
			} else {
				// HUD on top
				hud_update();
				// Handle controller locking
				uint16_t lockstate = joystate, oldlockstate = oldstate;
				if(controlsLocked) joystate = oldstate = 0;
				// Don't update this stuff if a script is using <PRI
				if(!gameFrozen) {
					if(showingBossHealth) tsc_update_boss_health();
					camera_update();
					player_update();
					entities_update();
				}
				// Restore controller locking if it was locked
				joystate = lockstate;
				oldstate = oldlockstate;
				// Run the next set of commands in a script if it is running
				uint8_t rtn = tsc_update();
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
						VDP_setPaletteColors(0, PAL_FadeOut, 64);
						VDP_setPaletteColor(15, 0x000);
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

		PF_END_FRAME();

		vsync();
		aftervsync();
		
		PF_DRAW();
	}
	return;
}

void game_reset(uint8_t load) {
	
	camera_init();
	tsc_init();
	hud_create();
	// Default sprite sheets
	sheets_load_stage(255, TRUE, TRUE);
	
	gameFrozen = FALSE;
	if(load >= 4) {
		system_load_levelselect(load - 4);
	} else {
		system_load(sram_file);
	}
	const SpriteDefinition *wepSpr = weapon_info[playerWeapon[currentWeapon].type].sprite;
	if(wepSpr) TILES_QUEUE(SPR_TILES(wepSpr,0,0), TILE_WEAPONINDEX,6);
	
	SHEET_LOAD(&SPR_Bonk, 1, 1, 1, 1, 0,0);
	SHEET_LOAD(&SPR_QMark, 1, 1, TILE_QMARKINDEX, 1, 0,0);
	// Load up the main palettes
	VDP_setCachedPalette(PAL0, PAL_Main.data);
	VDP_setCachedPalette(PAL1, PAL_Sym.data);
	VDP_setPaletteColors(0, PAL_FadeOut, 64);
	
}

void draw_itemmenu(uint8_t resetCursor) {
	VDP_setEnable(FALSE);
	sprites_clear();
	uint8_t top = IS_PALSYSTEM ? 1 : 0;
	// Fill the top part
	uint8_t y = top;
	VDP_fillTileMap(VDP_PLAN_WINDOW, WINDOW_ATTR(0), (y<<6) + 1, 1);
	VDP_fillTileMap(VDP_PLAN_WINDOW, WINDOW_ATTR(1), (y<<6) + 2, 36);
	VDP_fillTileMap(VDP_PLAN_WINDOW, WINDOW_ATTR(2), (y<<6) + 38, 1);
	for(uint8_t i = 19; --i;) { // Body
		y++;
		VDP_fillTileMap(VDP_PLAN_WINDOW, WINDOW_ATTR(3), (y<<6) + 1, 1);
		VDP_fillTileMap(VDP_PLAN_WINDOW, WINDOW_ATTR(4), (y<<6) + 2, 36);
		VDP_fillTileMap(VDP_PLAN_WINDOW, WINDOW_ATTR(5), (y<<6) + 38, 1);
	}
	// Bottom
	y++;
	VDP_fillTileMap(VDP_PLAN_WINDOW, WINDOW_ATTR(6), (y<<6) + 1, 1);
	VDP_fillTileMap(VDP_PLAN_WINDOW, WINDOW_ATTR(7), (y<<6) + 2, 36);
	VDP_fillTileMap(VDP_PLAN_WINDOW, WINDOW_ATTR(8), (y<<6) + 38, 1);
	// Load the 4 tiles for the selection box. Since the menu can never be brought up
	// during scripts we overwrite the face image
	VDP_loadTileSet(&TS_ItemSel, TILE_FACEINDEX, TRUE);
	// Redraw message box at the bottom of the screen
	window_open(FALSE);
	// Load tiles for the font letters
#define LOAD_LETTER(c,in) (VDP_loadTileData(TS_MsgFont.tiles+((c-0x20)<<3),                    \
						   TILE_HUDINDEX+in,1,1))
#define DRAW_LETTER(in,xx,yy) (VDP_setTileMapXY(PLAN_WINDOW,                                   \
							TILE_ATTR_FULL(PAL0,1,0,0,TILE_HUDINDEX+in),xx,yy))
	LOAD_LETTER('0', 0);
	LOAD_LETTER('1', 1);
	LOAD_LETTER('2', 2);
	LOAD_LETTER('3', 3);
	LOAD_LETTER('4', 4);
	LOAD_LETTER('5', 5);
	LOAD_LETTER('6', 6);
	LOAD_LETTER('7', 7);
	LOAD_LETTER('8', 8);
	LOAD_LETTER('9', 9);
	LOAD_LETTER('A', 10);
	LOAD_LETTER('R', 11);
	LOAD_LETTER('M', 12);
	LOAD_LETTER('S', 13);
	LOAD_LETTER('L', 14);
	LOAD_LETTER('v', 15);
	LOAD_LETTER('/', 16);
	LOAD_LETTER('I', 17);
	LOAD_LETTER('T', 18);
	LOAD_LETTER('E', 19);
	LOAD_LETTER('-', 20);
	// Weapons
	y = top + 3;
	//VDP_drawTextWindow("--ARMS--", 4, y++);
	DRAW_LETTER(20,4,y);
	DRAW_LETTER(20,5,y);
	DRAW_LETTER(10,6,y);
	DRAW_LETTER(11,7,y);
	DRAW_LETTER(12,8,y);
	DRAW_LETTER(13,9,y);
	DRAW_LETTER(20,10,y);
	DRAW_LETTER(20,11,y);
	y++;
	for(uint16_t i = 0; i < MAX_WEAPONS; i++) {
		Weapon *w = &playerWeapon[i];
		if(!w->type) continue;
		// X tile pos and VRAM index to put the ArmsImage tiles
		uint16_t x = 4 + i*6;
		uint16_t index = TILE_FACEINDEX + 16 + i*4;
		VDP_loadTileData(SPR_TILES(&SPR_ArmsImageM, 0, w->type), index, 4, TRUE);
		// 4 mappings for ArmsImage icon
		VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,index),   x,   y);
		VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,index+2), x+1, y);
		VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,index+1), x,   y+1);
		VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0,1,0,0,index+3), x+1, y+1);
		// Level
		//char str[6];
		//sprintf(str, "Lv %hu", w->level);
		//VDP_drawTextWindow(str, x, y+2);
		DRAW_LETTER(14,			x,	y+2);
		DRAW_LETTER(15,			x+1,y+2);
		DRAW_LETTER(w->level,	x+3,y+2);
		
		// Ammo & Max Ammo
		if(w->maxammo) {
			//sprintf(str, " %3hu", w->ammo);
			//VDP_drawTextWindow(str, x, y+3);
			//sprintf(str, "/%3hu", w->maxammo);
			//VDP_drawTextWindow(str, x, y+4);
			uint8_t ammo = w->ammo;
			DRAW_LETTER(ammo % 10, 			x+3, y+3);
			DRAW_LETTER((ammo / 10) % 10, 	x+2, y+3);
			if(ammo >= 100) DRAW_LETTER(1, 	x+1, y+3);
			ammo = w->maxammo;
			DRAW_LETTER(ammo % 10, 			x+3, y+4);
			DRAW_LETTER((ammo / 10) % 10, 	x+2, y+4);
			if(ammo >= 100) DRAW_LETTER(1, 	x+1, y+4);
			DRAW_LETTER(16,	x,	y+4);
		} else {
			//VDP_drawTextWindow("  --", x, y+3);
			//VDP_drawTextWindow("/ --", x, y+4);
			DRAW_LETTER(20,	x+2,y+3);
			DRAW_LETTER(20,	x+3,y+3);
			
			DRAW_LETTER(16,	x,	y+4);
			DRAW_LETTER(20,	x+2,y+4);
			DRAW_LETTER(20,	x+3,y+4);
		}
	}
	// Items
	y = top + 10;
	//VDP_drawTextWindow("--ITEM--", 4, y);
	DRAW_LETTER(20,4,y);
	DRAW_LETTER(20,5,y);
	DRAW_LETTER(17,6,y);
	DRAW_LETTER(18,7,y);
	DRAW_LETTER(19,8,y);
	DRAW_LETTER(12,9,y);
	DRAW_LETTER(20,10,y);
	DRAW_LETTER(20,11,y);
	uint8_t held = 0;
	for(uint16_t i = 0; i < MAX_ITEMS; i++) {
		//playerInventory[i] = 35; // :^)
		uint16_t item = playerInventory[i];
		if(item > 0) {
			// Wonky workaround to use either PAL_Sym or PAL_Main
			const SpriteDefinition *sprDef = &SPR_ItemImage;
			uint16_t pal = PAL1;
			if(ITEM_PAL[item]) {
				sprDef = &SPR_ItemImageG;
				pal = PAL0;
			}
			// Clobber the entity/bullet shared sheets
			SHEET_LOAD(sprDef, 1, 6, TILE_SHEETINDEX+held*6, TRUE, item,0);
			itemSprite[i] = (VDPSprite){
				.x = 36 + (i % 6) * 32 + 128, 
				.y = 88 + (i / 6) * 16 + 128 + (top * 8), 
				.size = SPRITE_SIZE(3, 2),
				.attribut = TILE_ATTR_FULL(pal,1,0,0,TILE_SHEETINDEX+held*6)
			};
			held++;
		} else {
			itemSprite[i] = (VDPSprite){};
		}
	}
	// Draw item cursor at first index (default selection)
	if(resetCursor) {
		selectedItem = -6 + currentWeapon;
	}
	itemcursor_move(0, selectedItem);
	tsc_call_event(5000 + playerInventory[selectedItem]);
	// Make the window plane fully overlap the game
	VDP_setWindowPos(0, IS_PALSYSTEM ? 30 : 28);
	// Handle 0 items - if we don't draw any sprites at all, the non-menu sprites
	// will keep drawing. Draw a blank sprite in the upper left corner to work around this
	if(!held) {
		spr_num = 0;
		sprite_add(((VDPSprite) { .x = 128, .y = 128, .size = SPRITE_SIZE(1, 1) }));
	}
	VDP_setEnable(TRUE);
}

uint8_t update_pause() {
	// Start or B will close the menu and resume the game
	// Pressing C over a weapon will too, and switch to that weapon
	if((joy_pressed(btn[cfg_btn_pause]) || joy_pressed(btn[cfg_btn_shoot]) ||
		(selectedItem < 0 && joy_pressed(btn[cfg_btn_jump]))) && !tscState) {
		VDP_setEnable(FALSE);
		// Change weapon
		if((selectedItem < 0 && joy_pressed(btn[cfg_btn_jump])) &&
				playerWeapon[selectedItem + 6].type > 0) { // Weapon
			currentWeapon = selectedItem + 6;
			sound_play(SND_SWITCH_WEAPON, 5);
			if(weapon_info[playerWeapon[currentWeapon].type].sprite) {
				TILES_QUEUE(
					SPR_TILES(weapon_info[playerWeapon[currentWeapon].type].sprite,0,0),
					TILE_WEAPONINDEX,6);
			}
		}
		// Fix HUD since we clobbered it
		hud_show();
		hud_force_redraw();
		sprites_clear();
		VDP_loadTileData(TILE_BLANK,TILE_HUDINDEX+8,1,1);
		VDP_loadTileData(TILE_BLANK,TILE_HUDINDEX+9,1,1);
		VDP_loadTileData(TILE_BLANK,TILE_HUDINDEX+12,1,1);
		VDP_loadTileData(TILE_BLANK,TILE_HUDINDEX+13,1,1);
		aftervsync();
		// Reload shared sheets we clobbered
		sheets_load_stage(stageID, TRUE, FALSE);
		selectedItem = 0;
		aftervsync();
		// Reload TSC Events for the current stage
		tsc_load_stage(stageID);
		// Put the sprites for player/entities/HUD back
		player_unpause();
		player_draw();
		entities_draw();
		
		controlsLocked = FALSE;
		gameFrozen = FALSE;
		VDP_setWindowPos(0, 0);
		window_close();
		VDP_setEnable(TRUE);
		return FALSE;
	} else {
		// Every cursor move and item selection runs a script
		// Weapons are 1000 + ID
		// Items are 5000 + ID
		// Item descriptions are 6000 + ID
		if(tscState) {
			if(selectedItem >= 0) { // Item
				uint8_t overid = playerInventory[selectedItem];
				tsc_update();
				// Item was comsumed, have to adjust the icons
				if(playerInventory[selectedItem] != overid) {
					draw_itemmenu(FALSE);
				}
			} else { // Weapon
				tsc_update();
			}
		} else if(joy_pressed(btn[cfg_btn_jump])) {
			if(selectedItem >= 0) { // Item
				if(playerInventory[selectedItem] > 0) {
					tsc_call_event(6000 + playerInventory[selectedItem]);
				}
			}
		} else if(joy_pressed(BUTTON_LEFT)) {
			int8_t newsel = selectedItem % 6 != 0 ? selectedItem - 1 : selectedItem + 5;
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
			int8_t newsel = selectedItem >= 0 ? selectedItem - 6 : selectedItem + 24;
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
			int8_t newsel = selectedItem % 6 != 5 ? selectedItem + 1 : selectedItem - 5;
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
			int8_t newsel = selectedItem < MAX_ITEMS - 6 ? selectedItem + 6 : selectedItem - 24;
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
		for(uint8_t i = MAX_ITEMS; i--; ) if(itemSprite[i].y) sprite_add(itemSprite[i]);
	}
	return TRUE;
}

void itemcursor_move(int8_t oldindex, int8_t index) {
	uint8_t top = IS_PALSYSTEM ? 1 : 0;
	// Erase old position
	uint16_t x, y, w, h;
	if(oldindex >= 0) {
		x = 4 + (oldindex % 6) * 4;
		y = 11 + (oldindex / 6) * 2 + top;
		w = 3; 
		h = 1;
	} else {
		x = 3 + (oldindex + 6) * 6;
		y = 4 + top;
		w = 5;
		h = 4;
	}
	VDP_setTileMapXY(PLAN_WINDOW, TILE_WINDOWINDEX+4, x,   y);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_WINDOWINDEX+4, x+w, y);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_WINDOWINDEX+4, x,   y+h);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_WINDOWINDEX+4, x+w, y+h);
	// Draw new position
	if(index >= 0) {
		x = 4 + (index % 6) * 4;
		y = 11 + (index / 6) * 2 + top;
		w = 3; 
		h = 1;
	} else {
		x = 3 + (index + 6) * 6;
		y = 4 + top;
		w = 5;
		h = 4;
	}
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FACEINDEX,   x,   y);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FACEINDEX+1, x+w, y);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FACEINDEX+2, x,   y+h);
	VDP_setTileMapXY(PLAN_WINDOW, TILE_FACEINDEX+3, x+w, y+h);
}

void do_map() {
	sprites_clear();
	
	uint16_t mapx = (SCREEN_HALF_W - stageWidth / 2) / 8;
	uint16_t mapy = (SCREEN_HALF_H - stageHeight / 2) / 8;
	
	uint16_t index = TILE_SHEETINDEX;
	
	// Upload a completely blank & completely solid tile
	static const uint32_t blank[8] = {
		0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
	};
	DMA_doDma(DMA_VRAM, (uint32_t)blank, index*32, 16, 2);
	index++;
	static const uint32_t solid[8] = {
		0xBBBBBBBB,0xBBBBBBBB,0xBBBBBBBB,0xBBBBBBBB,0xBBBBBBBB,0xBBBBBBBB,0xBBBBBBBB,0xBBBBBBBB,
	};
	DMA_doDma(DMA_VRAM, (uint32_t)solid, index*32, 16, 2);
	index++;
	
	for(uint16_t y = 0; y < (stageHeight / 8) + (stageHeight % 8 > 0); y++) {
		
		for(uint16_t x = 0; x < (stageWidth / 8) + (stageWidth % 8 > 0); x++) {
			uint8_t result = gen_maptile(x*8, y*8, index);
			if(!result) {
				VDP_setTileMapXY(PLAN_WINDOW, 
						TILE_ATTR_FULL(PAL0,1,0,0,index), mapx+x, mapy+y);
				index++;
			} else {
				VDP_setTileMapXY(PLAN_WINDOW, 
						TILE_ATTR_FULL(PAL0,1,0,0,TILE_SHEETINDEX+(result-1)), mapx+x, mapy+y);
			}
		}
		
		ready = TRUE;
		vsync(); aftervsync();
	}
	
	VDPSprite whereami = (VDPSprite) {
		.x = mapx * 8 + sub_to_block(player.x) - 4 + 128,
		.y = mapy * 8 + sub_to_block(player.y) - 4 + 128,
		.size = SPRITE_SIZE(1,1),
		.attribut = TILE_ATTR_FULL(PAL0,1,0,0,1)
	};
	uint16_t blinkTimer = 0;
	
	input_update();
	while(!joy_pressed(btn[cfg_btn_shoot]) && !joy_pressed(btn[cfg_btn_jump]) 
			&& !joy_pressed(btn[cfg_btn_map])) {
		input_update();
		system_update();
		// Alternate between the small plus and transparency
		// We can't simply "not draw" the sprite because the VDP will draw it anyway
		//whereami.attribut &= ~1;
		if((++blinkTimer & 15) == 0) whereami.attribut ^= 1;
		sprite_add(whereami);
		ready = TRUE;
		vsync(); aftervsync();
	}
	draw_itemmenu(FALSE);
}

uint8_t gen_maptile(uint16_t bx, uint16_t by, uint16_t index) {
	static const uint32_t blank = 0x11111111;
	static const uint32_t solid = 0xBBBBBBBB;
	
	uint32_t tile[8];
	for(uint16_t y = 0; y < 8; y++) {
		if(by+y >= stageHeight) {
			tile[y] = 0x11111111;
		} else {
			tile[y] = 0;
			for(uint16_t x = 0; x < 8; x++) {
				tile[y] |= bx+x >= stageWidth ? (1 << ((7-x)*4))
						:  stage_get_block_type(bx+x, by+y) == 0x41 ? (11 << ((7-x)*4)) 
						:  stage_get_block_type(bx+x, by+y) == 0x43 ? (10 << ((7-x)*4))
						:  stage_get_block_type(bx+x, by+y) == 0x01 ? (9 << ((7-x)*4)) 
						:  (1 << ((7-x)*4));
			}
		}
	}
	// Check if completely blank or solid area, do not duplicate
	uint8_t blank_c = 0, solid_c = 0;
	for(uint8_t y = 0; y < 8; y++) {
		if(tile[y] == blank) blank_c++;
		if(tile[y] == solid) solid_c++;
	}
	if(blank_c == 8) return 1;
	if(solid_c == 8) return 2;
	// Otherwise upload tile
	DMA_doDma(DMA_VRAM, (uint32_t)tile, index*32, 16, 2);
	return 0;
}
