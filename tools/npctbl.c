#define NPC_COUNT 361

// NPC Flags
#define NPC_SOLID 			0
#define NPC_IGNORE44 		1
#define NPC_INVINCIBLE 		2
#define NPC_IGNORESOLID 	3
#define NPC_BOUNCYTOP 		4
#define NPC_SHOOTABLE 		5
#define NPC_SPECIALSOLID 	6
#define NPC_FRONTATKONLY 	7
#define NPC_EVENT_ON_TOUCH 	8
#define NPC_EVENTONDEATH 	9
#define NPC_DROPPOWERUP 	10
#define NPC_ENABLEONFLAG 	11
#define NPC_FACE_RIGHT 		12
#define NPC_INTERACTIVE 	13
#define NPC_DISABLEONFLAG 	14
#define NPC_SHOWDAMAGE 		15

// Locations in npc.tbl where data for each property begins
#define NPC_FLAGIND 0
#define NPC_HPIND   (NPC_COUNT * 2)
#define NPC_PALIND  (NPC_COUNT * 4)
#define NPC_DSFXIND (NPC_COUNT * 5)
#define NPC_HSFXIND (NPC_COUNT * 6)
#define NPC_SMOKIND (NPC_COUNT * 7)
#define NPC_XPIND   (NPC_COUNT * 8)
#define NPC_ATKIND  (NPC_COUNT * 12)
#define NPC_HBOXIND (NPC_COUNT * 16)
#define NPC_DBOXIND (NPC_COUNT * 20)

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char flag_name[16][16] = {
	"Solid",
	"No44",
	"Invul",
	"NoSolid",
	"Bouncy",
	"Shootable",
	"SpSolid",
	"FrontAtk",
	"OP1",
	"DeathEv",
	"PowUp",
	"Flag+",
	"FaceRight",
	"Interact",
	"Flag-",
	"ShowDmg",
};

void* fslurp(const char *fn, size_t *size) {
	FILE *f = fopen(fn, "rb");
	if(!f) return NULL;
	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	fseek(f, 0, SEEK_SET);
	uint8_t *data = malloc(*size);
	if(!data) return NULL;
	fread(data, 1, *size, f);
	fclose(f);
	return data;
}

int main(int argc, char *argv[]) {
	if(argc != 2) {
		puts("Usage: npctbl <in file>");
		return EXIT_FAILURE;
	}
	size_t src_size = 0;
	void *src = fslurp(argv[1], &src_size);
	if(!src) {
		printf("Failed to read %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	const uint16_t *flags   = (uint16_t*)(src + NPC_FLAGIND);
	const uint16_t *hp      = (uint16_t*)(src + NPC_HPIND);
	const uint8_t *diesfx   = (uint8_t*) (src + NPC_DSFXIND);
	const uint8_t *hurtsfx  = (uint8_t*) (src + NPC_HSFXIND);
	const int *xp           = (int*)     (src + NPC_XPIND);
	const int *attack       = (int*)     (src + NPC_ATKIND);
	const uint8_t *hitbox   = (uint8_t*) (src + NPC_HBOXIND);
	const uint8_t *dispbox  = (uint8_t*) (src + NPC_DBOXIND);

	for(int i = 0; i < NPC_COUNT; i++) {
		printf("{\t// %03d\n", i);

		printf("\t\t.num_sprites\t= 0,\n");
		printf("\t\t.sheet_id\t\t= NUM_SHEETS,\n");
		printf("\t\t.tiloc_def\t\t= NULL,\n");
		printf("\t\t.palette\t\t= PAL0,\n");
		printf("\t\t.persistent\t\t= false,\n");
		printf("\t\t.act_func\t\t= &ai_null,\n");
//		printf("\t\t.draw_func\t\t= &draw_null,\n");

		printf("\t\t.flags\t\t\t= 0x%04X, //", flags[i]);
		for(int f = 0; f < 16; f++) {
			if(flags[i] & (1 << f)) printf(" %s", flag_name[f]);
		}
		printf("\n\t\t.health\t\t\t= %hu,\n", hp[i]);
		printf("\t\t.deathSound\t\t= %hu,\n", diesfx[i]);
		printf("\t\t.hurtSound\t\t= %hu,\n", hurtsfx[i]);
		printf("\t\t.exp\t\t\t= %d,\n", xp[i]);
		printf("\t\t.attack\t\t\t= %d,\n", attack[i]);
		printf("\t\t.hit_box\t\t= {%hu,%hu,%hu,%hu},\n",
			   hitbox[i*4+0],hitbox[i*4+1],hitbox[i*4+2],hitbox[i*4+3]);
		printf("\t\t.origin_x\t\t= %hu,\n", dispbox[i*4+0]);
		printf("\t\t.origin_y\t\t= %hu,\n", dispbox[i*4+1]);

		printf("\t},");
	}

	return EXIT_SUCCESS;
}
