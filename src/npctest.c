#include "npctest.h"

#include <genesis.h>

#include "resources.h"
#include "input.h"
#include "tables.h"
#include "audio.h"
#include "vdp_ext.h"

void draw_npc_template() {
	VDP_drawText("NPC Test", 16, 2); // Title
	VDP_drawText("No.", 2, 4); // ID
	// Stats
	VDP_drawText("Health:", 2, 6);
	VDP_drawText("Energy:", 2, 7);
	VDP_drawText("Attack:", 2, 8);
	// Flags
	u16 fl_y = 10;
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
	// Controls
	VDP_drawText("C-Hurt B-Kill A-Anim Start-Exit", 2, 25);
}

void draw_npc_info(u16 id) {
	SYS_disableInts();
	// ID
	VDP_drawInt(id, 5, 4); 
	// Name
	VDP_clearText(10, 4, 25);
	VDP_drawText(npc_info[id].name, 10, 4);
	// Stats
	VDP_clearText(10, 6, 5);
	VDP_clearText(10, 7, 5);
	VDP_clearText(10, 8, 5);
	VDP_drawInt(npc_health(id), 10, 6);
	VDP_drawInt(npc_experience(id), 10, 7);
	VDP_drawInt(npc_attack(id), 10, 8);
	// NPC Flags
	u16 fl = npc_flags(id);
	u16 fl_y = 10;
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
	SYS_enableInts();
}

Sprite* change_npc_sprite(Sprite *sprite, u16 id) {
	SPR_SAFERELEASE(sprite);
	const SpriteDefinition *def = npc_info[id].sprite;
	return def != NULL
		? SPR_addSprite(def, 304 - npc_displayBox(id).right - npc_displayBox(id).left, 
			32 + npc_displayBox(id).top + npc_displayBox(id).bottom,
			TILE_ATTR(npc_info[id].palette, 1, 0, 0))
		: NULL;
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
	VDP_setPalette(PAL2, PAL_Cave.data);
	VDP_setPalette(PAL3, PAL_Regu.data);
	draw_npc_template();
	draw_npc_info(npcId);
	npcSprite = change_npc_sprite(npcSprite, npcId);
	VDP_setEnable(true);
	SYS_enableInts();
	song_play(2);
    while(true) {
		input_update();
		// Scroll between NPCs
		if(joy_pressed(BUTTON_LEFT)) {
			if(npcId == 0) npcId = NPC_COUNT - 1;
			else npcId--;
			draw_npc_info(npcId);
			npcSprite = change_npc_sprite(npcSprite, npcId);
		} else if(joy_pressed(BUTTON_RIGHT)) {
			if(npcId == NPC_COUNT - 1) npcId = 0;
			else npcId++;
			draw_npc_info(npcId);
			npcSprite = change_npc_sprite(npcSprite, npcId);
		}
		// Play Hurt/Death sound effects
		if(joy_pressed(BUTTON_C)) {
			sound_play(npc_hurtSound(npcId), 5);
		} else if(joy_pressed(BUTTON_B)) {
			sound_play(npc_deathSound(npcId), 5);
		}
		// Cycle animations
		if(joy_pressed(BUTTON_A) && npcSprite != NULL) {
			if(npcSprite->animInd >= npcSprite->definition->numAnimation - 1) {
				SPR_setAnim(npcSprite, 0);
			} else {
				SPR_setAnim(npcSprite, npcSprite->animInd + 1);
			}
		}
		// Return to Title
		if(joy_pressed(BUTTON_START)) {
			oldstate |= BUTTON_START;
			break;
		}
		SPR_update();
		VDP_waitVSync();
    }
    song_stop();
}
