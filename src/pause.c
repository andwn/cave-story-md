#include "common.h"

#include "audio.h"
#include "camera.h"
#include "dma.h"
#include "effect.h"
#include "entity.h"
#include "error.h"
#include "gamemode.h"
#include "hud.h"
#include "joy.h"
#include "player.h"
#include "resources.h"
#include "sheet.h"
#include "stage.h"
#include "string.h"
#include "system.h"
#include "tables.h"
#include "tools.h"
#include "tsc.h"
#include "vdp.h"
#include "weapon.h"
#include "window.h"
#include "xgm.h"

#include "pause.h"

// Item menu stuff
VDPSprite itemSprite[MAX_ITEMS];
int8_t selectedItem = 0;

void draw_itemmenu(uint8_t resetCursor) {
    vdp_set_display(FALSE);
    vdp_sprites_clear();
    uint8_t top = pal_mode ? 1 : 0;
    // Fill the top part
    uint16_t y = top;
    vdp_map_xy(VDP_PLAN_W, 0, 0, y);
    vdp_map_xy(VDP_PLAN_W, WINDOW_ATTR(0), 1, y);
    vdp_map_fill_rect(VDP_PLAN_W, WINDOW_ATTR(1), 2, y, 36, 1, 0);
    vdp_map_xy(VDP_PLAN_W, WINDOW_ATTR(2), 38, y);
    vdp_map_xy(VDP_PLAN_W, 0, 39, y);
    for(uint16_t i = 19; --i;) { // Body
        y++;
        vdp_map_xy(VDP_PLAN_W, 0, 0, y);
        vdp_map_xy(VDP_PLAN_W, WINDOW_ATTR(3), 1, y);
        vdp_map_fill_rect(VDP_PLAN_W, WINDOW_ATTR(4), 2, y, 36, 1, 0);
        vdp_map_xy(VDP_PLAN_W, WINDOW_ATTR(5), 38, y);
        vdp_map_xy(VDP_PLAN_W, 0, 39, y);
    }
    // Bottom
    y++;
    vdp_map_xy(VDP_PLAN_W, 0, 0, y);
    vdp_map_xy(VDP_PLAN_W, WINDOW_ATTR(6), 1, y);
    vdp_map_fill_rect(VDP_PLAN_W, WINDOW_ATTR(7), 2, y, 36, 1, 0);
    vdp_map_xy(VDP_PLAN_W, WINDOW_ATTR(8), 38, y);
    vdp_map_xy(VDP_PLAN_W, 0, 39, y);

    // Load the 4 tiles for the selection box. Since the menu can never be brought up
    // during scripts we overwrite the face image
    vdp_tiles_load_from_rom(TS_ItemSel.tiles, TILE_FACEINDEX, TS_ItemSel.numTile);
    // Redraw message box at the bottom of the screen
    window_open(FALSE);
    // Load tiles for the font letters
#define LOAD_LETTER(c,in) (vdp_tiles_load_from_rom(TS_MsgFont.tiles+((c-0x20)<<3),      \
						   TILE_HUDINDEX+in,1))
#define DRAW_LETTER(in,xx,yy) (vdp_map_xy(VDP_PLAN_W,                                   \
							TILE_ATTR(PAL0,1,0,0,TILE_HUDINDEX+in),xx,yy))
    // Load 8x8 numbers
    vdp_tiles_load_from_rom(TS_MsgFont.tiles+(('0'-0x20)<<3),TILE_HUDINDEX,10);
    // Lv, slash for weapon display
    LOAD_LETTER('L', 14);
    LOAD_LETTER('v', 15);
    LOAD_LETTER('/', 16);
    LOAD_LETTER('-', 17);
    // ARMSITEM or ぶきもちもの
    const uint32_t *ts = cfg_language == LANG_JA ? TS_MenuTextJ.tiles : TS_MenuTextE.tiles;
    vdp_tiles_load_from_rom(ts + (2<<3), TILE_HUDINDEX + 10, 4);
    vdp_tiles_load_from_rom(ts + (10<<3), TILE_HUDINDEX + 18, 4);
    // Weapons
    y = top + 3;
    // --ARMS-- or --ぶき--
    DRAW_LETTER(17,4,y);
    DRAW_LETTER(17,5,y);
    DRAW_LETTER(10,6,y);
    DRAW_LETTER(11,7,y);
    DRAW_LETTER(12,8,y);
    DRAW_LETTER(13,9,y);
    DRAW_LETTER(17,10,y);
    DRAW_LETTER(17,11,y);
    y++;
    for(uint16_t i = 0; i < MAX_WEAPONS; i++) {
        Weapon *w = &playerWeapon[i];
        if(!w->type) continue;
        // X tile pos and VRAM index to put the ArmsImage tiles
        uint16_t x = 4 + i*6;
        uint16_t index = TILE_FACEINDEX + 16 + i*4;
        vdp_tiles_load_from_rom(SPR_TILES(&SPR_ArmsImageM, 0, w->type), index, 4);
        // 4 mappings for ArmsImage icon
        vdp_map_xy(VDP_PLAN_W, TILE_ATTR(PAL0,1,0,0,index),   x,   y);
        vdp_map_xy(VDP_PLAN_W, TILE_ATTR(PAL0,1,0,0,index+2), x+1, y);
        vdp_map_xy(VDP_PLAN_W, TILE_ATTR(PAL0,1,0,0,index+1), x,   y+1);
        vdp_map_xy(VDP_PLAN_W, TILE_ATTR(PAL0,1,0,0,index+3), x+1, y+1);
        // Lv
        DRAW_LETTER(14,			x,	y+2);
        DRAW_LETTER(15,			x+1,y+2);
        DRAW_LETTER(w->level,	x+3,y+2);

        // Ammo & Max Ammo
        if(w->maxammo) {
            uint8_t ammo = w->ammo;
            DRAW_LETTER(mod10[ammo], 		x+3, y+3);
            DRAW_LETTER(mod10[div10[ammo]], x+2, y+3);
            if(ammo >= 100) DRAW_LETTER(1, 	x+1, y+3);
            ammo = w->maxammo;
            DRAW_LETTER(mod10[ammo], 		x+3, y+4);
            DRAW_LETTER(mod10[div10[ammo]], x+2, y+4);
            if(ammo >= 100) DRAW_LETTER(1, 	x+1, y+4);
            DRAW_LETTER(16,	x,	y+4);
        } else {
            //   --
            DRAW_LETTER(17,	x+2,y+3);
            DRAW_LETTER(17,	x+3,y+3);
            // / --
            DRAW_LETTER(16,	x,	y+4);
            DRAW_LETTER(17,	x+2,y+4);
            DRAW_LETTER(17,	x+3,y+4);
        }
    }
    // Items
    y = top + 10;
    // --ITEM-- or --もちもの--
    DRAW_LETTER(17,4,y);
    DRAW_LETTER(17,5,y);
    DRAW_LETTER(18,6,y);
    DRAW_LETTER(19,7,y);
    DRAW_LETTER(20,8,y);
    DRAW_LETTER(21,9,y);
    DRAW_LETTER(17,10,y);
    DRAW_LETTER(17,11,y);
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
            vdp_tiles_load_from_rom(SPR_TILES(sprDef, item, 0), TILE_SHEETINDEX+held*6, 6);
            //SHEET_LOAD(sprDef, 1, 6, TILE_SHEETINDEX+held*6, TRUE, item,0);
            itemSprite[i] = (VDPSprite){
                    .x = 36 + (i % 6) * 32 + 128,
                    .y = 88 + (i / 6) * 16 + 128 + (top * 8),
                    .size = SPRITE_SIZE(3, 2),
                    .attr = TILE_ATTR(pal,1,0,0,TILE_SHEETINDEX+held*6)
            };
            held++;
        } else {
            itemSprite[i] = (VDPSprite) {};
        }
    }
    // Draw item cursor at first index (default selection)
    if(resetCursor) {
        selectedItem = -6 + currentWeapon;
    }
    itemcursor_move(0, selectedItem);
    if(selectedItem < 0) {
        tsc_call_event(1000 + playerWeapon[selectedItem + 6].type);
    } else {
        tsc_call_event(5000 + playerInventory[selectedItem]);
    }
    // Make the window plane fully overlap the game
    vdp_set_window(0, pal_mode ? 30 : 28);
    // Handle 0 items - if we don't draw any sprites at all, the non-menu sprites
    // will keep drawing. Draw a blank sprite in the upper left corner to work around this
    if(!held) {
        vdp_sprites_clear();
        //spr_num = 0;
        //vdp_sprite_add(((VDPSprite) { .x = 128, .y = 128, .size = SPRITE_SIZE(1, 1) }));
    }
    vdp_set_display(TRUE);
}

uint8_t update_pause() {
    // Start or B will close the menu and resume the game
    // Pressing C over a weapon will too, and switch to that weapon
    if((joy_pressed(btn[cfg_btn_pause]) || joy_pressed(btn[cfg_btn_shoot]) ||
        (selectedItem < 0 && joy_pressed(btn[cfg_btn_jump]))) /*&& !tscState*/) {
        tscState = TSC_IDLE;
        vdp_set_display(FALSE);
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
        vdp_sprites_clear();
        vdp_tiles_load_from_rom(TILE_BLANK,TILE_HUDINDEX+8,1);
        vdp_tiles_load_from_rom(TILE_BLANK,TILE_HUDINDEX+9,1);
        vdp_tiles_load_from_rom(TILE_BLANK,TILE_HUDINDEX+12,1);
        vdp_tiles_load_from_rom(TILE_BLANK,TILE_HUDINDEX+13,1);
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
        vdp_set_window(0, 0);
        window_close();
        vdp_set_display(TRUE);
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
            if (selectedItem >= 0) { // Item
                if (playerInventory[selectedItem] > 0) {
                    tsc_call_event(6000 + playerInventory[selectedItem]);
                }
            }
        }
        if(!tscState || lastRunEvent != 6015 || lastRunEvent != 6018 || lastRunEvent != 6023
           || lastRunEvent != 6026 || lastRunEvent != 6038) {
            if (joy_pressed(BUTTON_LEFT)) {
                int8_t newsel = selectedItem % 6 != 0 ? selectedItem - 1 : selectedItem + 5;
                if (newsel == -1) newsel = -2;
                itemcursor_move(selectedItem, newsel);
                sound_play(SND_MENU_MOVE, 5);
                selectedItem = newsel;
                if (selectedItem >= 0) { // Item
                    tsc_call_event(5000 + playerInventory[selectedItem]);
                } else { // Weapon
                    tsc_call_event(1000 + playerWeapon[selectedItem + 6].type);
                }
            } else if (joy_pressed(BUTTON_UP)) {
                int8_t newsel = selectedItem >= 0 ? selectedItem - 6 : selectedItem + 24;
                if (newsel == -1) newsel = -2;
                itemcursor_move(selectedItem, newsel);
                sound_play(SND_MENU_MOVE, 5);
                selectedItem = newsel;
                if (selectedItem >= 0) { // Item
                    tsc_call_event(5000 + playerInventory[selectedItem]);
                } else { // Weapon
                    tsc_call_event(1000 + playerWeapon[selectedItem + 6].type);
                }
            } else if (joy_pressed(BUTTON_RIGHT)) {
                int8_t newsel = selectedItem % 6 != 5 ? selectedItem + 1 : selectedItem - 5;
                if (newsel == -1) newsel = -6;
                itemcursor_move(selectedItem, newsel);
                sound_play(SND_MENU_MOVE, 5);
                selectedItem = newsel;
                if (selectedItem >= 0) { // Item
                    tsc_call_event(5000 + playerInventory[selectedItem]);
                } else { // Weapon
                    tsc_call_event(1000 + playerWeapon[selectedItem + 6].type);
                }
            } else if (joy_pressed(BUTTON_DOWN)) {
                int8_t newsel = selectedItem < MAX_ITEMS - 6 ? selectedItem + 6 : selectedItem - 24;
                if (newsel == -1) newsel = -2;
                itemcursor_move(selectedItem, newsel);
                sound_play(SND_MENU_MOVE, 5);
                selectedItem = newsel;
                if (selectedItem >= 0) { // Item
                    tsc_call_event(5000 + playerInventory[selectedItem]);
                } else { // Weapon
                    tsc_call_event(1000 + playerWeapon[selectedItem + 6].type);
                }
            }
        }
        for(uint8_t i = MAX_ITEMS; i--; ) if(itemSprite[i].y) vdp_sprite_add(&itemSprite[i]);
    }
    return TRUE;
}

void itemcursor_move(int8_t oldindex, int8_t index) {
    uint8_t top = pal_mode ? 1 : 0;
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
    vdp_map_xy(VDP_PLAN_W, TILE_WINDOWINDEX+4, x,   y);
    vdp_map_xy(VDP_PLAN_W, TILE_WINDOWINDEX+4, x+w, y);
    vdp_map_xy(VDP_PLAN_W, TILE_WINDOWINDEX+4, x,   y+h);
    vdp_map_xy(VDP_PLAN_W, TILE_WINDOWINDEX+4, x+w, y+h);
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
    vdp_map_xy(VDP_PLAN_W, TILE_FACEINDEX,   x,   y);
    vdp_map_xy(VDP_PLAN_W, TILE_FACEINDEX+1, x+w, y);
    vdp_map_xy(VDP_PLAN_W, TILE_FACEINDEX+2, x,   y+h);
    vdp_map_xy(VDP_PLAN_W, TILE_FACEINDEX+3, x+w, y+h);
}

void do_map() {
    vdp_sprites_clear();

    uint16_t mapx = (SCREEN_HALF_W - stageWidth / 2) / 8;
    uint16_t mapy = (SCREEN_HALF_H - stageHeight / 2) / 8;

    uint16_t index = TILE_SHEETINDEX;

    disable_ints;
    z80_request();
    // Upload a completely blank & completely solid tile
    static const uint32_t blank[8] = {
            0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
    };
    DMA_doDma(DMA_VRAM, (uint32_t)blank, index << 5, 16, 2);
    index++;
    static const uint32_t solid[8] = {
            0xBBBBBBBB,0xBBBBBBBB,0xBBBBBBBB,0xBBBBBBBB,0xBBBBBBBB,0xBBBBBBBB,0xBBBBBBBB,0xBBBBBBBB,
    };
    DMA_doDma(DMA_VRAM, (uint32_t)solid, index << 5, 16, 2);
    index++;

    z80_release();
    enable_ints;

    for(uint16_t y = 0; y < (stageHeight / 8) + (stageHeight % 8 > 0); y++) {
        for(uint16_t x = 0; x < (stageWidth / 8) + (stageWidth % 8 > 0); x++) {
            disable_ints;
            z80_request();
            uint8_t result = gen_maptile(x*8, y*8, index);
            if(!result) {
                vdp_map_xy(VDP_PLAN_W, TILE_ATTR(PAL0,1,0,0,index), mapx+x, mapy+y);
                index++;
            } else {
                vdp_map_xy(VDP_PLAN_W, TILE_ATTR(PAL0,1,0,0,TILE_SHEETINDEX+(result-1)), mapx+x, mapy+y);
            }
            z80_release();
            enable_ints;
        }
        ready = TRUE;
        vdp_vsync(); aftervsync();
    }

    VDPSprite whereami = (VDPSprite) {
            .x = (mapx << 3) + sub_to_block(player.x) - 4 + 128,
            .y = (mapy << 3) + sub_to_block(player.y) - 4 + 128,
            .size = SPRITE_SIZE(1,1),
            .attr = TILE_ATTR(PAL0,1,0,0,1)
    };
    uint16_t blinkTimer = 0;

    while(!joy_pressed(btn[cfg_btn_shoot]) && !joy_pressed(btn[cfg_btn_jump])
          && !joy_pressed(btn[cfg_btn_map])) {
        system_update();
        // Alternate between the small plus and transparency
        // We can't simply "not draw" the sprite because the VDP will draw it anyway
        if((++blinkTimer & 15) == 0) whereami.attr ^= 1;
        vdp_sprite_add(&whereami);
        ready = TRUE;
        vdp_vsync(); aftervsync();
    }
    if(paused) draw_itemmenu(FALSE);
}

uint8_t gen_maptile(uint16_t bx, uint16_t by, uint16_t index) {
    static const uint32_t blank = 0x11111111;
    static const uint32_t solid = 0xBBBBBBBB;
    static const uint32_t colors[6] = {9, 11, 10, 1, 0, 2};

    uint8_t borderColor = paused ? 5 : (stageBackgroundType == 4) ? 3 : 4;
    uint32_t borderLine = paused ? 0x22222222 : (stageBackgroundType == 4) ? 0x11111111 : 0x00000000;
    uint32_t tile[8];
    for(uint16_t y = 0; y < 8; y++) {
        if(by+y >= stageHeight) {
            tile[y] = borderLine;
        } else {
            tile[y] = 0;
            for(uint16_t x = 0; x < 8; x++) {
                if(bx+x >= stageWidth) {
                    tile[y] |= colors[borderColor] << ((7 - x) << 2);
                } else {
                    uint8_t block = stage_get_block_type(bx+x, by+y);
                    switch(block) {
                        case 0x01: tile[y] |= colors[0] << ((7 - x) << 2); break;
                        case 0x41: tile[y] |= colors[1] << ((7 - x) << 2); break;
                        case 0x43: tile[y] |= colors[2] << ((7 - x) << 2); break;
                        default:   tile[y] |= colors[3] << ((7 - x) << 2); break;
                    }
                }
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
    DMA_doDma(DMA_VRAM, (uint32_t)tile, index << 5, 16, 2);
    return 0;
}
