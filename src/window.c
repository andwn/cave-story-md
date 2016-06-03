#include "window.h"

#include <genesis.h>
#include "vdp_ext.h"
#include "input.h"
#include "audio.h"
#include "system.h"
#include "tables.h"
#include "resources.h"

#define WINDOW_ATTR(x) TILE_ATTR_FULL(PAL0, 1, 0, 0, TILE_WINDOWINDEX+(x))

// Window location
#define WINDOW_X1 2
#define WINDOW_X2 37
#define WINDOW_Y1 20
#define WINDOW_Y2 27
// Text area location within window
#define TEXT_X1 (WINDOW_X1 + 1)
#define TEXT_X2 (WINDOW_X2 - 1)
#define TEXT_Y1 (WINDOW_Y1 + 1)
#define TEXT_Y2 (WINDOW_Y2 - 1)
#define TEXT_X1_FACE (WINDOW_X1 + 8)
// Prompt window location
#define PROMPT_X 27
#define PROMPT_Y 18

bool windowOpen = false;
u16 showingFace = 0;

u8 textMode = TM_NORMAL;

u8 windowText[3][34];
u8 textRow, textColumn;
u8 windowTextTick = 0;

bool promptShowing = false;
bool promptAnswer = true;
Sprite *promptSpr = NULL, *handSpr = NULL;

u16 showingItem = 0;
Sprite *itemSpr = NULL, *itemWinSpr = NULL;

void window_clear_text();
void window_draw_face();

void window_open(u8 mode) {
	window_clear_text();
	textRow = 0;
	textColumn = 0;
	VDP_setTileMapXY(PLAN_WINDOW, WINDOW_ATTR(0), WINDOW_X1, WINDOW_Y1);
	for(u8 x = TEXT_X1; x <= TEXT_X2; x++)
		VDP_setTileMapXY(PLAN_WINDOW, WINDOW_ATTR(1), x, WINDOW_Y1);
	VDP_setTileMapXY(PLAN_WINDOW, WINDOW_ATTR(2), WINDOW_X2, WINDOW_Y1);
	for(u8 y = TEXT_Y1; y <= TEXT_Y2; y++) {
		VDP_setTileMapXY(PLAN_WINDOW, WINDOW_ATTR(3), WINDOW_X1, y);
		for(u8 x = TEXT_X1; x <= TEXT_X2; x++) {
			VDP_setTileMapXY(PLAN_WINDOW, WINDOW_ATTR(4), x, y);
		}
		VDP_setTileMapXY(PLAN_WINDOW, WINDOW_ATTR(5), WINDOW_X2, y);
	}
	VDP_setTileMapXY(PLAN_WINDOW, WINDOW_ATTR(6), WINDOW_X1, WINDOW_Y2);
	for(u8 x = TEXT_X1; x <= TEXT_X2; x++)
		VDP_setTileMapXY(PLAN_WINDOW, WINDOW_ATTR(7), x, WINDOW_Y2);
	VDP_setTileMapXY(PLAN_WINDOW, WINDOW_ATTR(8), WINDOW_X2, WINDOW_Y2);
	if(showingFace > 0) {
		window_draw_face(showingFace);
	}
	VDP_setWindowPos(0, 244);
	windowOpen = true;
}

bool window_is_open() {
	return windowOpen;
}

void window_clear() {
	u8 x1 = showingFace ? TEXT_X1_FACE : TEXT_X1;
	for(u8 y = TEXT_Y1; y <= TEXT_Y2; y++) {
		for(u8 x = x1; x <= TEXT_X2; x++) {
			VDP_setTileMapXY(PLAN_WINDOW, WINDOW_ATTR(4), x, y);
		}
	}
	window_clear_text();
	textMode = TM_NORMAL;
}

void window_clear_text() {
	textRow = 0;
	textColumn = 0;
	for(u8 row = 0; row < 3; row++) {
		for(u8 col = 0; col < 34; col++) {
			windowText[row][col] = ' ';
		}
	}
}

void window_close() {
	VDP_setWindowPos(0, 0);
	showingItem = 0;
	SPR_SAFERELEASE(itemWinSpr);
	SPR_SAFERELEASE(itemSpr);
	windowOpen = false;
}

void window_set_face(u16 face, bool open) {
	if(open && !windowOpen) window_open(0);
	showingFace = face;
	if(face > 0) window_draw_face();
}

void window_draw_char(u8 c) {
	if(c == '\n') {
		textRow++;
		textColumn = 0;
		if(textRow > 2) {
			if(textMode == TM_ALL) textMode = TM_NORMAL;
			window_scroll_text();
		} else if(textMode == TM_LINE) {
			textMode = TM_NORMAL;
		}
	} else {
		windowText[textRow][textColumn] = c;
		if(textColumn >= 34 - (showingFace > 0) * 8) return;
		u8 msgTextX = showingFace ? TEXT_X1_FACE : TEXT_X1;
		msgTextX += textColumn;
		u8 msgTextY = TEXT_Y1 + textRow * 2;
		VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0, 1, 0, 0,
				TILE_FONTINDEX + c - 0x20), msgTextX, msgTextY);
		textColumn++;
	}
}

void window_scroll_text() {
	// Push bottom 2 rows to top
	for(u8 row = 0; row < 2; row++) {
		u8 msgTextX = showingFace ? TEXT_X1_FACE : TEXT_X1;
		u8 msgTextY = TEXT_Y1 + row * 2;
		for(u8 col = 0; col < 34 - (showingFace > 0) * 8; col++) {
			windowText[row][col] = windowText[row + 1][col];
			VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0, 1, 0, 0,
					TILE_FONTINDEX + windowText[row][col] - 0x20), msgTextX, msgTextY);
			msgTextX++;
		}
	}
	// Clear third row
	u8 msgTextX = showingFace ? TEXT_X1_FACE : TEXT_X1;
	u8 msgTextY = TEXT_Y1 + 4;
	for(u8 col = 0; col < 34 - (showingFace > 0) * 8; col++) {
		windowText[2][col] = ' ';
		VDP_setTileMapXY(PLAN_WINDOW, TILE_ATTR_FULL(PAL0, 1, 0, 0,
				TILE_FONTINDEX), msgTextX, msgTextY);
		msgTextX++;
	}
	// Reset to beginning of third row
	textRow = 2;
	textColumn = 0;
}

void window_set_textmode(u8 mode) {
	textMode = mode;
}

bool window_tick() {
	if(textMode > 0) return true;
	windowTextTick++;
	if(windowTextTick > 2 || (windowTextTick > 1 && (joystate&BUTTON_C))) {
		windowTextTick = 0;
		return true;
	} else {
		return false;
	}
}

void window_prompt_open() {
	sound_play(SOUND_PROMPT, 5);
	// Load hand sprite and move next to yes
	handSpr = SPR_addSprite(&SPR_Pointer, 
		tile_to_pixel(PROMPT_X) - 4, tile_to_pixel(PROMPT_Y + 1) - 4, 
		TILE_ATTR(PAL0, 1, 0, 0));
	// Load prompt sprite
	promptSpr = SPR_addSprite(&SPR_Prompt, 
		tile_to_pixel(PROMPT_X), tile_to_pixel(PROMPT_Y), 
		TILE_ATTR(PAL0, 1, 0, 0));
	promptAnswer = true; // Yes is default
}

void window_prompt_close() {
	SPR_SAFERELEASE(promptSpr);
	SPR_SAFERELEASE(handSpr);
	window_clear();
}

bool window_prompt_answer() {
	return promptAnswer;
}

bool window_prompt_update() {
	if(joy_pressed(BUTTON_C)) {
		sound_play(SOUND_CONFIRM, 5);
		window_prompt_close();
		return true;
	} else if(joy_pressed(BUTTON_LEFT) | joy_pressed(BUTTON_RIGHT)) {
		promptAnswer = !promptAnswer;
		sound_play(SOUND_CURSOR, 5);
		SPR_setPosition(handSpr, 
			tile_to_pixel(31-(promptAnswer*4))-4, tile_to_pixel(PROMPT_Y+1)-4);
	}
	return false;
}

void window_draw_face() {
	SYS_disableInts();
	VDP_loadTileSet(face_info[showingFace].tiles, TILE_FACEINDEX, true);
	VDP_fillTileMapRectInc(PLAN_WINDOW, 
		TILE_ATTR_FULL(face_info[showingFace].palette, 1, 0, 0, TILE_FACEINDEX), 
		TEXT_X1, TEXT_Y1, 6, 6);
	SYS_enableInts();
}

void window_show_item(u16 item) {
	showingItem = item;
	if(item == 0) {
		SPR_SAFERELEASE(itemSpr);
		SPR_SAFERELEASE(itemWinSpr);
		return;
	}
	// Wonky workaround to use either PAL_Sym or PAL_Main
	const SpriteDefinition *sprDef = &SPR_ItemImage;
	u16 pal = PAL1;
	if(item == 2 || item == 13 || item == 18 || item == 19 || item == 23 || item == 25
		|| item == 32 || item == 35 || item == 37 || item == 38 || item == 39) {
		sprDef = &SPR_ItemImageG;
		pal = PAL0;
	}
	itemSpr = SPR_addSprite(sprDef, SCREEN_HALF_W - 16, SCREEN_HALF_H + 12,
		TILE_ATTR(pal, 1, 0, 0));
	itemWinSpr = SPR_addSprite(&SPR_ItemWin, SCREEN_HALF_W - 24, SCREEN_HALF_H + 8,
		TILE_ATTR(PAL0, 1, 0, 0));
	SPR_setAnimAndFrame(itemSpr, item / 8, item % 8);
}

void window_show_weapon(u16 item) {
	showingItem = item;
	if(item == 0) {
		SPR_SAFERELEASE(itemSpr);
		SPR_SAFERELEASE(itemWinSpr);
		return;
	}
	itemSpr = SPR_addSprite(&SPR_ArmsImage, SCREEN_HALF_W - 8, SCREEN_HALF_H + 12,
		TILE_ATTR(PAL0, 1, 0, 0));
	itemWinSpr = SPR_addSprite(&SPR_ItemWin, SCREEN_HALF_W - 24, SCREEN_HALF_H + 8,
		TILE_ATTR(PAL0, 1, 0, 0));
	SPR_setFrame(itemSpr, item);
}
