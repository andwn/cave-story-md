#include "window.h"

#include <genesis.h>
#include "sprite.h"
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
#define PROMPT_X1 26
#define PROMPT_X2 36
#define PROMPT_Y1 20
#define PROMPT_Y2 22

bool windowOpen = false;
//u8 msgTextX = 0;
//u8 msgTextY = 0;
u16 showingFace = 0;
u8 textMode = TM_NORMAL;

u8 windowText[3][34];
u8 textRow, textColumn;
u8 windowTextTick = 0;

bool promptShowing = false;
bool promptAnswer = true;
u8 handSpr = SPRITE_NONE;

void window_clear_text();
void window_draw_face();

void window_open(u8 mode) {
	window_clear_text();
	textRow = 0;
	textColumn = 0;
	VDP_setTileMapXY(WINDOW, WINDOW_ATTR(0), WINDOW_X1, WINDOW_Y1);
	for(u8 x = TEXT_X1; x <= TEXT_X2; x++)
		VDP_setTileMapXY(WINDOW, WINDOW_ATTR(1), x, WINDOW_Y1);
	VDP_setTileMapXY(WINDOW, WINDOW_ATTR(2), WINDOW_X2, WINDOW_Y1);
	for(u8 y = TEXT_Y1; y <= TEXT_Y2; y++) {
		VDP_setTileMapXY(WINDOW, WINDOW_ATTR(6), WINDOW_X1, y);
		for(u8 x = TEXT_X1; x <= TEXT_X2; x++) {
			VDP_setTileMapXY(WINDOW, WINDOW_ATTR(7), x, y);
		}
		VDP_setTileMapXY(WINDOW, WINDOW_ATTR(8), WINDOW_X2, y);
	}
	VDP_setTileMapXY(WINDOW, WINDOW_ATTR(12), WINDOW_X1, WINDOW_Y2);
	for(u8 x = TEXT_X1; x <= TEXT_X2; x++)
		VDP_setTileMapXY(WINDOW, WINDOW_ATTR(13), x, WINDOW_Y2);
	VDP_setTileMapXY(WINDOW, WINDOW_ATTR(14), WINDOW_X2, WINDOW_Y2);
	//msgTextX = TEXT_X1;
	//msgTextY = TEXT_Y1;
	if(showingFace > 0) {
		window_draw_face(showingFace);
		//msgTextX = TEXT_X1_FACE;
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
			VDP_setTileMapXY(WINDOW, WINDOW_ATTR(7), x, y);
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
	VDP_setWindowPos(0, 251 * debuggingEnabled);
	showingFace = 0;
	windowOpen = false;
}

void window_set_face(u16 face) {
	if(!windowOpen) window_open(0);
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
		VDP_setTileMapXY(WINDOW, TILE_ATTR_FULL(PAL0, 1, 0, 0,
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
			VDP_setTileMapXY(WINDOW, TILE_ATTR_FULL(PAL0, 1, 0, 0,
					TILE_FONTINDEX + windowText[row][col] - 0x20), msgTextX, msgTextY);
			msgTextX++;
		}
	}
	// Clear third row
	u8 msgTextX = showingFace ? TEXT_X1_FACE : TEXT_X1;
	u8 msgTextY = TEXT_Y1 + 4;
	for(u8 col = 0; col < 34 - (showingFace > 0) * 8; col++) {
		windowText[2][col] = ' ';
		VDP_setTileMapXY(WINDOW, TILE_ATTR_FULL(PAL0, 1, 0, 0,
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
	// Top of window
	VDP_setTileMapXY(WINDOW, WINDOW_ATTR(3), PROMPT_X1, PROMPT_Y1);
	for(u8 x = PROMPT_X1 + 1; x < PROMPT_X2; x++)
		VDP_setTileMapXY(WINDOW, WINDOW_ATTR(4), x, PROMPT_Y1);
	VDP_setTileMapXY(WINDOW, WINDOW_ATTR(5), 36, PROMPT_Y1);
	// Text area of window
	VDP_setTileMapXY(WINDOW, WINDOW_ATTR(9), PROMPT_X1, PROMPT_Y1 + 1);
	for(u8 x = PROMPT_X1 + 1; x < PROMPT_X2; x++)
		VDP_setTileMapXY(WINDOW, WINDOW_ATTR(10), x, PROMPT_Y1 + 1);
	VDP_setTileMapXY(WINDOW, WINDOW_ATTR(11), PROMPT_X2, PROMPT_Y1 + 1);
	// Bottom of window
	VDP_setTileMapXY(WINDOW, WINDOW_ATTR(15), PROMPT_X1, PROMPT_Y2);
	for(u8 x = PROMPT_X1 + 1; x < PROMPT_X2; x++)
		VDP_setTileMapXY(WINDOW, WINDOW_ATTR(16), x, PROMPT_Y2);
	VDP_setTileMapXY(WINDOW, WINDOW_ATTR(17), PROMPT_X2, PROMPT_Y2);
	VDP_drawTextWindow("Yes / No", PROMPT_X1 + 2, PROMPT_Y1 + 1);
	sound_play(SOUND_PROMPT, 5);
	// Load hand sprite and move next to yes
	handSpr = sprite_create(&SPR_Pointer, PAL0, true);
	sprite_set_position(handSpr, tile_to_pixel(PROMPT_X1 + 1)-4, tile_to_pixel(PROMPT_Y1 + 1) - 4);
	promptAnswer = true; // Yes is default
}

void window_prompt_close() {
	sprite_delete(handSpr);
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
		sprite_set_position(handSpr,
				tile_to_pixel(33-(promptAnswer*6))-4, tile_to_pixel(PROMPT_Y1+1)-4);
	}
	return false;
}

void window_draw_face() {
	SYS_disableInts();
	VDP_loadTileSet(face_info[showingFace].tiles, TILE_FACEINDEX, false);
	VDP_fillTileMapRectInc(WINDOW, TILE_ATTR_FULL(face_info[showingFace].palette,
			1, 0, 0, TILE_FACEINDEX), TEXT_X1, TEXT_Y1, 6, 6);
	SYS_enableInts();
}
