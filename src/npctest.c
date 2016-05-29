#include "npctest.h"

#include <genesis.h>

#include "resources.h"
#include "input.h"
#include "tables.h"
#include "audio.h"
#include "vdp_ext.h"

void redraw_text(const char *txt, u16 x, u16 y, u8 len) {
	VDP_clearText(x, y, len);
	VDP_drawText(txt, x, y);
}

void draw_npc_template() {
	VDP_drawText("NPC Test", 16, 2); // Title
	VDP_drawText("No.", 2, 4); // ID
	// Flags
	u16 fl_y = 8;
	VDP_drawText("[ ] Solid", 2, fl_y++);
	VDP_drawText("[ ] Sp.Solid", 2, fl_y++);
	VDP_drawText("[ ] Bouncy Top", 2, fl_y++);
	VDP_drawText("[ ] Ignore Solid", 2, fl_y++);
	VDP_drawText("[ ] Interactive", 2, fl_y++);
	VDP_drawText("[ ] Shootable", 2, fl_y++);
	VDP_drawText("[ ] Drop PowerUp", 2, fl_y++);
	VDP_drawText("[ ] Invincible", 2, fl_y++);
	VDP_drawText("[ ] Show Damage", 2, fl_y++);
	VDP_drawText("[ ] Misc 1", 2, fl_y++);
	VDP_drawText("[ ] Misc 2", 2, fl_y++);
	VDP_drawText("[ ] Event on Death", 2, fl_y++);
	VDP_drawText("[ ] Enable on Flag", 2, fl_y++);
	VDP_drawText("[ ] Disable on Flag", 2, fl_y++);
	VDP_drawText("[ ] Ignore NPC Blockade", 2, fl_y++);
	VDP_drawText("[ ] Only Front Vulnerable", 2, fl_y++);
	// Stats
	
	// Collision and display box
}

void draw_npc_info(u16 id) {
	draw_word(id, 5, 4); // ID
	VDP_drawText("???", 10, 4); // Name
	// NPC Flags
	u16 fl = npc_flags(id);
	u16 fl_y = 8;
	VDP_drawText(fl&NPC_SOLID ? "X":" ", 3, fl_y++);
	VDP_drawText(fl&NPC_SPECIALSOLID ? "X":" ", 3, fl_y++);
	VDP_drawText(fl&NPC_BOUNCYTOP ? "X":" ", 3, fl_y++);
	VDP_drawText(fl&NPC_IGNORESOLID ? "X":" ", 3, fl_y++);
	VDP_drawText(fl&NPC_INTERACTIVE ? "X":" ", 3, fl_y++);
	VDP_drawText(fl&NPC_SHOOTABLE ? "X":" ", 3, fl_y++);
	VDP_drawText(fl&NPC_DROPPOWERUP ? "X":" ", 3, fl_y++);
	VDP_drawText(fl&NPC_INVINCIBLE ? "X":" ", 3, fl_y++);
	VDP_drawText(fl&NPC_SHOWDAMAGE ? "X":" ", 3, fl_y++);
	VDP_drawText(fl&NPC_OPTION1 ? "X":" ", 3, fl_y++);
	VDP_drawText(fl&NPC_OPTION2 ? "X":" ", 3, fl_y++);
	VDP_drawText(fl&NPC_EVENTONDEATH ? "X":" ", 3, fl_y++);
	VDP_drawText(fl&NPC_ENABLEONFLAG ? "X":" ", 3, fl_y++);
	VDP_drawText(fl&NPC_DISABLEONFLAG ? "X":" ", 3, fl_y++);
	VDP_drawText(fl&NPC_FRONTATKONLY ? "X":" ", 3, fl_y++);
	// Stats
	
	// Collision and display box
}

void npctest_main() {
	u16 npcId = 1;
	Sprite *npcSprite = NULL;
	SYS_disableInts();
	VDP_setEnable(false);
	SPR_reset();
	VDP_clearPlan(PLAN_A, true);
	VDP_clearPlan(PLAN_B, true);
	VDP_setPalette(PAL0, PAL_Main.data);
	VDP_setPalette(PAL1, PAL_Sym.data);
	VDP_setPalette(PAL2, PAL_Sym.data);
	VDP_setPalette(PAL3, PAL_Sym.data);
	draw_npc_template();
	VDP_setEnable(true);
	SYS_enableInts();
	song_play(1);
    while(true) {
		input_update();
		if(joy_pressed(BUTTON_LEFT)) {
			if(npcId == 0) npcId = NPC_COUNT - 1;
			else npcId--;
			draw_npc_info(npcId);
		} else if(joy_pressed(BUTTON_RIGHT)) {
			if(npcId == NPC_COUNT - 1) npcId = 0;
			else npcId++;
			draw_npc_info(npcId);
		}
		if(joy_pressed(BUTTON_START)) {
			oldstate |= BUTTON_START;
			break;
		}
		VDP_waitVSync();
    }
    song_stop();
}
