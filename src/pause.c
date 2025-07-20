#include "common.h"

#include "audio.h"
#include "res/system.h"
#include "camera.h"
#include "md/dma.h"
#include "effect.h"
#include "entity.h"
#include "res/local.h"
#include "gamemode.h"
#include "hud.h"
#include "md/joy.h"
#include "math.h"
#include "player.h"
#include "resources.h"
#include "sheet.h"
#include "stage.h"
#include "system.h"
#include "tables.h"
#include "md/comp.h"
#include "tsc.h"
#include "md/vdp.h"
#include "weapon.h"
#include "window.h"
#include "md/sys.h"
#include "res/tiles.h"

#include "pause.h"

// Load tiles for the font letters
#define LOAD_LETTER(c,in) (vdp_tiles_load_uftc(*TS_MSGTEXT,      \
						   TILE_HUDINDEX+in,(c)-0x20,1))
#define DRAW_LETTER(in,xx,yy) (vdp_map_xy(VDP_PLANE_W,                                   \
							TILE_ATTR(PAL0,1,0,0,TILE_HUDINDEX+in),xx,yy))

// Item menu stuff
Sprite itemSprite[MAX_ITEMS];
int8_t selectedItem;

void draw_weapons(uint8_t y) {
    for(uint16_t i = 0; i < MAX_WEAPONS; i++) {
        Weapon *w = &playerWeapon[i];
        if(!w->type) continue;
        // X tile pos and VRAM index to put the ArmsImage tiles
        uint16_t x = 4 + i*6;
        uint16_t index = TILE_FACEINDEX + 16 + i*4;
        vdp_tiles_load(SPR_TILES(&SPR_ArmsImageM, w->type), index, 4);
        // 4 mappings for ArmsImage icon
        vdp_map_xy(VDP_PLANE_W, TILE_ATTR(PAL0, 1, 0, 0, index), x, y);
        vdp_map_xy(VDP_PLANE_W, TILE_ATTR(PAL0, 1, 0, 0, index + 2), x + 1, y);
        vdp_map_xy(VDP_PLANE_W, TILE_ATTR(PAL0, 1, 0, 0, index + 1), x, y + 1);
        vdp_map_xy(VDP_PLANE_W, TILE_ATTR(PAL0, 1, 0, 0, index + 3), x + 1, y + 1);
        // Lv
        DRAW_LETTER(26,			x,	y+2);
        DRAW_LETTER(27,			x+1,y+2);
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
            DRAW_LETTER(28,	x,	y+4);
        } else {
            //   --
            DRAW_LETTER(29,	x+2,y+3);
            DRAW_LETTER(29,	x+3,y+3);
            // / --
            DRAW_LETTER(28,	x,	y+4);
            DRAW_LETTER(29,	x+2,y+4);
            DRAW_LETTER(29,	x+3,y+4);
        }
    }
}

void draw_item(uint8_t sel) {
    uint16_t item = playerInventory[sel];
    if(item > 0) {
        // Wonky workaround to use either PAL_Sym or PAL_Main
        const SpriteDef *sprDef = &SPR_ItemImage;
        uint16_t pal = PAL1;
        if(ITEM_PAL[item]) {
            sprDef = &SPR_ItemImageG;
            pal = PAL0;
        }
        // Clobber the entity/bullet shared sheets
        vdp_tiles_load(SPR_TILES(sprDef, item), TILE_SHEETINDEX+sel*6, 6);
        //SHEET_LOAD(sprDef, 1, 6, TILE_SHEETINDEX+held*6, TRUE, item,0);
        itemSprite[sel] = (Sprite){
                .x = 36 + (sel % 6) * 32 + 128,
                .y = 88 + (sel / 6) * 16 + 128 + (pal_mode * 8),
                .size = SPRITE_SIZE(3, 2),
                .attr = TILE_ATTR(pal,1,0,0,TILE_SHEETINDEX+sel*6)
        };
    } else {
        itemSprite[sel] = (Sprite) {};
    }
}

void draw_itemmenu(uint8_t resetCursor) {
    vdp_set_display(FALSE);
    vdp_sprites_clear();
    uint8_t top = pal_mode ? 1 : 0;
    // Fill the top part
    uint16_t y = top;
    vdp_map_xy(VDP_PLANE_W, 0, 0, y);
    vdp_map_xy(VDP_PLANE_W, WINDOW_ATTR(0), 1, y);
    vdp_map_fill_rect(VDP_PLANE_W, WINDOW_ATTR(1), 2, y, 36, 1, 0);
    vdp_map_xy(VDP_PLANE_W, WINDOW_ATTR(2), 38, y);
    vdp_map_xy(VDP_PLANE_W, 0, 39, y);
    for(uint16_t i = 19; --i;) { // Body
        y++;
        vdp_map_xy(VDP_PLANE_W, 0, 0, y);
        vdp_map_xy(VDP_PLANE_W, WINDOW_ATTR(3), 1, y);
        vdp_map_fill_rect(VDP_PLANE_W, WINDOW_ATTR(4), 2, y, 36, 1, 0);
        vdp_map_xy(VDP_PLANE_W, WINDOW_ATTR(5), 38, y);
        vdp_map_xy(VDP_PLANE_W, 0, 39, y);
    }
    // Bottom
    y++;
    vdp_map_xy(VDP_PLANE_W, 0, 0, y);
    vdp_map_xy(VDP_PLANE_W, WINDOW_ATTR(6), 1, y);
    vdp_map_fill_rect(VDP_PLANE_W, WINDOW_ATTR(7), 2, y, 36, 1, 0);
    vdp_map_xy(VDP_PLANE_W, WINDOW_ATTR(8), 38, y);
    vdp_map_xy(VDP_PLANE_W, 0, 39, y);

    //disable_ints();
    //z80_pause_fast();
    // Load the 4 tiles for the selection box. Since the menu can never be brought up
    // during scripts we overwrite the face image
    vdp_tiles_load_uftc(UFTC_ItemSel, TILE_FACEINDEX, 0, 4);
    // Redraw message box at the bottom of the screen
    window_open(FALSE);

    // Load 8x8 numbers
    vdp_tiles_load_uftc(*TS_MSGTEXT,TILE_HUDINDEX,'0'-0x20,10);
    // Lv, slash for weapon display
    LOAD_LETTER('L', 26);
    LOAD_LETTER('v', 27);
    LOAD_LETTER('/', 28);
    LOAD_LETTER('-', 29);
    // ARMSITEM or ぶきもちもの
    //const uint32_t *ts = cfg_language == LANG_JA ? TS_MenuTextJ.tiles : TS_MenuTextE.tiles;
    vdp_tiles_load_uftc(*TS_MENUTEXT, TILE_HUDINDEX + 10, 0, 8);
    vdp_tiles_load_uftc(*TS_MENUTEXT, TILE_HUDINDEX + 18, 8, 8);
    // Weapons
    y = top + 3;
    // --ARMS-- or --ぶき--
    DRAW_LETTER(10,4,y);
    DRAW_LETTER(11,5,y);
    DRAW_LETTER(12,6,y);
    DRAW_LETTER(13,7,y);
    DRAW_LETTER(14,8,y);
    DRAW_LETTER(15,9,y);
    DRAW_LETTER(16,10,y);
    DRAW_LETTER(17,11,y);
    y++;

    draw_weapons(y);

    // Items
    y = top + 10;
    // --ITEM-- or --もちもの--
    DRAW_LETTER(18,4,y);
    DRAW_LETTER(19,5,y);
    DRAW_LETTER(20,6,y);
    DRAW_LETTER(21,7,y);
    DRAW_LETTER(22,8,y);
    DRAW_LETTER(23,9,y);
    DRAW_LETTER(24,10,y);
    DRAW_LETTER(25,11,y);
    for(uint16_t i = 0; i < MAX_ITEMS; i++) {
        //playerInventory[i] = 35; // :^)
        draw_item(i);
    }
    //z80_resume();
    //enable_ints();
    // Draw the item cursor at first index (default selection)
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
    // No items? clear sprites so we don't draw garbage
    if(!playerInventory[0]) vdp_sprites_clear();

    vdp_set_display(TRUE);
}

uint8_t update_pause(void) {
    // Start or B will close the menu and resume the game.
    // Pressing C over a weapon will too, and switch to that weapon.
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
                        SPR_TILES(weapon_info[playerWeapon[currentWeapon].type].sprite,0),
                        TILE_WEAPONINDEX,6);
            }
        }
        // Fix HUD since we clobbered it
        hud_show();
        hud_force_redraw();
        vdp_sprites_clear();

        //disable_ints();
        //z80_pause_fast();
        vdp_tiles_load(BlankData,TILE_HUDINDEX+8,1);
        vdp_tiles_load(BlankData,TILE_HUDINDEX+9,1);
        vdp_tiles_load(BlankData,TILE_HUDINDEX+12,1);
        vdp_tiles_load(BlankData,TILE_HUDINDEX+13,1);
        //z80_resume();
        //enable_ints();
        aftervsync();
        // Reload shared sheets we clobbered
        //disable_ints();
        //z80_pause_fast();
        sheets_load_stage(g_stage.id, TRUE, FALSE);
        //z80_resume();
        //enable_ints();

        selectedItem = 0;
        aftervsync();

        // Reload TSC Events for the current stage
        tsc_load_stage(g_stage.id);
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
        // Every cursor move and item selection runs a script.
        // Weapons are 1000 + ID
        // Items are 5000 + ID
        // Item descriptions are 6000 + ID
        if(tscState) {
            if(selectedItem >= 0) { // Item
                uint8_t overid = playerInventory[selectedItem];
                tsc_update();
                // Item was consumed, have to adjust the icons
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
        if(!tscState && lastRunEvent != 6015 && lastRunEvent != 6018 && lastRunEvent != 6023
           && lastRunEvent != 6026 && lastRunEvent != 6038) {
            if (joy_pressed(JOY_A)) { // Debug
                if(selectedItem < 0) { // Weapon changer
                    // Cycle the selected weapon out for the next one in the list,
                    // but skip invalids and duplicates, and wrap the list.
                    uint8_t wep = selectedItem + 6;
                    for(uint8_t i = playerWeapon[wep].type + 1; i != playerWeapon[wep].type; i++) {
                        if(i >= WEAPON_COUNT) i = 0;
                        if(weapon_info[i].sprite) { // Has a sprite -- valid weapon
                            // Make sure it is not a duplicate of one the player already has
                            uint8_t j = 0;
                            for(; j < MAX_WEAPONS; j++) {
                                if(j == wep) continue; // Skip weapon index we are changing
                                if(playerWeapon[j].type == i) break; // It's a dupe
                            }
                            // j will complete the loop if weapon is unique
                            if(j == MAX_WEAPONS) {
                                playerWeapon[wep].type = i;
                                //disable_ints();
                                //z80_pause_fast();
                                draw_weapons(pal_mode ? 5 : 4);
                                //z80_resume();
                                //enable_ints();
                                sound_play(SND_SWITCH_WEAPON, 5);
                                tsc_call_event(1000 + playerWeapon[selectedItem + 6].type);
                                break;
                            }
                        }
                    }
                } else { // Item changer
                    if(++playerInventory[selectedItem] >= 40) {
                        playerInventory[selectedItem] = 0;
                    }
                    //disable_ints();
                    //z80_pause_fast();
                    draw_item(selectedItem);
                    //z80_resume();
                    //enable_ints();
                    sound_play(SND_SWITCH_WEAPON, 5);
                    tsc_call_event(5000 + playerInventory[selectedItem]);
                }
            } else if (joy_pressed(JOY_LEFT)) {
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
            } else if (joy_pressed(JOY_UP)) {
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
            } else if (joy_pressed(JOY_RIGHT)) {
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
            } else if (joy_pressed(JOY_DOWN)) {
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
    vdp_map_xy(VDP_PLANE_W, TILE_WINDOWINDEX + 4, x, y);
    vdp_map_xy(VDP_PLANE_W, TILE_WINDOWINDEX + 4, x + w, y);
    vdp_map_xy(VDP_PLANE_W, TILE_WINDOWINDEX + 4, x, y + h);
    vdp_map_xy(VDP_PLANE_W, TILE_WINDOWINDEX + 4, x + w, y + h);
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
    vdp_map_xy(VDP_PLANE_W, TILE_FACEINDEX, x, y);
    vdp_map_xy(VDP_PLANE_W, TILE_FACEINDEX + 1, x + w, y);
    vdp_map_xy(VDP_PLANE_W, TILE_FACEINDEX + 2, x, y + h);
    vdp_map_xy(VDP_PLANE_W, TILE_FACEINDEX + 3, x + w, y + h);
}

void do_map(void) {
    vdp_sprites_clear();

    uint16_t mapx = (ScreenHalfW - g_stage.pxm.width / 2) / 8;
    uint16_t mapy = (ScreenHalfH - g_stage.pxm.height / 2) / 8;

    uint16_t index = TILE_SHEETINDEX;

    //disable_ints();
    //z80_pause_fast();
    // Upload a completely blank & completely solid tile
    static const uint32_t blank[8] = {
            0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
    };
    dma_now(DmaVRAM, (uint32_t)blank, index << 5, 16, 2);
    index++;
    static const uint32_t solid[8] = {
            0xBBBBBBBB,0xBBBBBBBB,0xBBBBBBBB,0xBBBBBBBB,0xBBBBBBBB,0xBBBBBBBB,0xBBBBBBBB,0xBBBBBBBB,
    };
    dma_now(DmaVRAM, (uint32_t)solid, index << 5, 16, 2);
    index++;

    //z80_resume();
    //enable_ints();

    for(uint16_t y = 0; y < ((g_stage.pxm.height+2) / 8) + ((g_stage.pxm.height) % 8 > 0); y++) {
        for(uint16_t x = 0; x < ((g_stage.pxm.width+2) / 8) + ((g_stage.pxm.width) % 8 > 0); x++) {
            //disable_ints();
            //z80_pause_fast();
            uint8_t result = gen_maptile(x*8, y*8, index);
            if(!result) {
                vdp_map_xy(VDP_PLANE_W, TILE_ATTR(PAL0, 1, 0, 0, index), mapx + x, mapy + y);
                index++;
            } else {
                vdp_map_xy(VDP_PLANE_W, TILE_ATTR(PAL0, 1, 0, 0, TILE_SHEETINDEX + (result - 1)), mapx + x, mapy + y);
            }
            //z80_resume();
            //enable_ints();
        }
        ready = TRUE;
        sys_wait_vblank(); aftervsync();
    }

    Sprite whereami = (Sprite) {
            .x = (mapx << 3) + sub_to_block(player.x) - 2 + 128,
            .y = (mapy << 3) + sub_to_block(player.y) - 3 + 128,
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
        sys_wait_vblank(); aftervsync();
    }
    if(paused) draw_itemmenu(FALSE);
}

uint8_t gen_maptile(uint16_t bx, uint16_t by, uint16_t index) {
    static const uint32_t blankLine = 0x11111111;
    static const uint32_t solidLine = 0xBBBBBBBB;
    static const uint32_t colors[4] = {9, 11, 10, 1};

    uint32_t tile[8];
    for(uint16_t y = 0; y < 8; y++) {
        if(by+y == 0 || by+y+1 == g_stage.pxm.height) { // Top / Bottom borders
            tile[y] = blankLine;
        } else if(by+y+1 > g_stage.pxm.height) { // Below bottom border
            tile[y] = paused ? 0x22222222 : 0x00000000;
        } else {
            tile[y] = 0;
            uint16_t stg_y = by + y - 1;
            for(uint16_t x = 0; x < 8; x++) {
                if(bx+x == 0) { // Left border
                    tile[y] |= 0x10000000LU;
                } else if(bx+x+1 == g_stage.pxm.width) { // Right border
                    tile[y] |= 1LU << ((7 - x) << 2);
                } else if(bx+x+1 > g_stage.pxm.width) { // After right border
                    tile[y] |= (paused ? 2LU : 0LU) << ((7 - x) << 2);
                } else {
                    uint16_t stg_x = bx + x - 1;
                    uint8_t block = stage_get_block_type(stg_x, stg_y);
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
        if(tile[y] == blankLine) blank_c++;
        if(tile[y] == solidLine) solid_c++;
    }
    if(blank_c == 8) return 1;
    if(solid_c == 8) return 2;
    // Otherwise upload tile
    dma_now(DmaVRAM, (uint32_t)tile, index << 5, 16, 2);
    return 0;
}
