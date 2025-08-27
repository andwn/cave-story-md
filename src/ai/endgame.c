#include "ai_common.h"
#include "gamemode.h"
#include "sheet.h"

void onspawn_cloud_spawner(Entity *e) {
	e->alwaysActive = TRUE;
	if(e->flags & NPC_OPTION2) e->dir = 1;
}

// makes the clouds from the falling scene (good ending)
void ai_cloud_spawner(Entity *e) {
	if(!e->timer) {
		e->timer = TIME_8(e->dir ? 100 : 50);
		e->timer2 = rand() & 3;
		//if(e->timer2 == 0) e->timer2++;
		
		//Entity *cloud = entity_create(0, 0, OBJ_CLOUD, 
		//	((e->timer2 & 1) ? NPC_OPTION1 : 0) | ((e->timer2 & 2) ? NPC_OPTION2 : 0));
		Entity *cloud = entity_create(0, 0, OBJ_CLOUD + e->timer2, 0);
		
		// vertical clouds (falling)
		if(!e->dir)	{
			cloud->x = e->x + block_to_sub(-7 + (rand() & 15));
			cloud->y = e->y;
			cloud->y_speed = -SPEED_12(0xFFF >> e->timer2);	// each type half as fast as the last
		} else { // horizontal clouds (flying with Kazuma)
			cloud->x = e->x;
			cloud->y = e->y + block_to_sub(-7 + (rand() & 15));
			cloud->x_speed = -SPEED_10(0x3FF >> e->timer2);
		}
		
		// cut down on the amount of time Kazuma is flying
		// against plain blue when he appears in the credits
		if(gamemode == GM_CREDITS && e->state < 10) {
			cloud->x -= pixel_to_sub(128);
			e->state++;
		}
	} else {
		e->timer--;
	}

}

void onspawn_cloud(Entity *e) {
	e->alwaysActive = TRUE;
	e->hidden = TRUE;
	// Which cloud sprite/size to use
	uint8_t ssize = SPRITE_SIZE(4,4);
	uint8_t num_tiles = 16;
	switch(e->type) {
		case OBJ_CLOUD: 
		e->display_box = (bounding_box) {{ 96, 32, 96, 32 }};
		e->vramindex = TILE_CLOUDINDEX;
		break;
		case OBJ_CLOUD2: 
		e->display_box = (bounding_box) {{ 48, 16, 48, 16 }};
		e->vramindex = TILE_CLOUD2INDEX;
		break;
		case OBJ_CLOUD3: 
		e->display_box = (bounding_box) {{ 36, 12, 36, 12 }};
		e->vramindex = TILE_CLOUD3INDEX;
		ssize = SPRITE_SIZE(3,3);
		num_tiles = 9;
		break;
		case OBJ_CLOUD4: 
		e->display_box = (bounding_box) {{ 16, 8, 16, 8 }};
		e->vramindex = TILE_CLOUD4INDEX;
		ssize = SPRITE_SIZE(4,2);
		num_tiles = 8;
		break;
	}
	uint16_t index = e->vramindex;
	int16_t xoff = -e->display_box.left;
	int16_t yoff = -e->display_box.top;
	for(uint16_t i = 0; i < e->sprite_count; i++) {
		e->sprite[i] = (Sprite) {
			.x = 0x80 + (e->x>>CSF) - camera.x_shifted + xoff,
			.y = 0x80 + (e->y>>CSF) - camera.y_shifted + yoff,
			.size = ssize,
			.attr = TILE_ATTR(PAL2,0,0,0,index),
		};
		index += num_tiles;
		xoff += (e->type == OBJ_CLOUD3) ? 24 : 32;
		if(xoff >= e->display_box.right) {
			xoff = -e->display_box.left;
			yoff += (e->type == OBJ_CLOUD3) ? 24 : 32;
		}
	}
}

void ai_cloud(Entity *e) {
	e->x += e->x_speed;
	e->y += e->y_speed;
	if(e->x < -(pixel_to_sub(e->display_box.right)) 
			|| e->y < -(pixel_to_sub(e->display_box.bottom))) {
		e->state = STATE_DELETE;
	} else {
		if(e->type == OBJ_CLOUD) moveMeToFront = TRUE;
		// Sprite positions
		int16_t xoff = -e->display_box.left;
		int16_t yoff = -e->display_box.top;
		for(uint16_t i = 0; i < e->sprite_count; i++) {
			e->sprite[i].x = 0x80 + (e->x>>CSF) - camera.x_shifted + xoff;
			e->sprite[i].y = 0x80 + (e->y>>CSF) - camera.y_shifted + yoff;
			xoff += (e->type == OBJ_CLOUD3) ? 24 : 32;
			if(xoff >= e->display_box.right) {
				xoff = -e->display_box.left;
				yoff += (e->type == OBJ_CLOUD3) ? 24 : 32;
			}
		}
		vdp_sprites_add(e->sprite, e->sprite_count);
	}
}

// Balrog flying in clouds with player and Curly in best-ending.
void ai_balrog_flying(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->state++;
			e->animtime = 0;
			
			e->y_mark = e->y - 0x2000;
			e->x_mark = e->x - 0xC00;
			e->y_speed = 0;
			
			Entity *curly = entity_create(e->x, e->y, OBJ_BALROG_PASSENGER, 0);
			curly->linkedEntity = e;
			curly->state = 1;
			Entity *quote = entity_create(e->x, e->y, OBJ_BALROG_PASSENGER, NPC_OPTION2);
			quote->linkedEntity = e;
			quote->state = 1;
		} /* fallthrough */
		case 1:
		{
			if((++e->animtime & 3) == 0) e->frame ^= 1;
			e->x_speed += (e->x < e->x_mark) ? 0x08 : -0x08;
			e->y_speed += (e->y < e->y_mark) ? 0x08 : -0x08;
		}
		break;
		case 20:	// fly away
		{
			e->state = 21;
			e->x_speed = -SPEED_10(0x3FF);
			e->y_speed = SPEED_10(0x200);
		} /* fallthrough */
		case 21:
		{
			if((++e->animtime & 1) == 0) e->frame ^= 1;
			
			e->x_speed += 0x10;
			e->y_speed -= 0x08;
			
			if (e->x > 0x78000) {
				e->x_speed = 0;
				e->y_speed = 0;
				e->state = 22;
			}
		}
		break;
	}
	moveMeToFront = TRUE;
	e->x += e->x_speed;
	e->y += e->y_speed;
}

// player/curly when rescued by Balrog during best-ending
void ai_balrog_passenger(Entity *e) {
	if(!e->linkedEntity) {
		e->state = STATE_DELETE;
		return;
	}
	e->dir = 1;
	moveMeToFront = TRUE;
	switch(e->state) {
		case 0:		// being rescued from Seal Chamber
		{
			if (e->flags & NPC_OPTION2) {
				//e->sprite = player->sprite;
				//e->frame = 4;
				e->frame = 0;
				e->x = e->linkedEntity->x - (12<<CSF);
			} else {
				//e->sprite = SPR_CURLY;
				//e->frame = 7;
				e->frame = 2;
				e->x = e->linkedEntity->x + (8<<CSF);
			}
			e->y = e->linkedEntity->y - (4<<CSF);
		}
		break;
		case 1:		// flying in clouds
		{
			if (e->flags & NPC_OPTION2) {
				//e->sprite = SPR_MYCHAR;
				//e->frame = 12;
				e->frame = 1;
				e->x = e->linkedEntity->x - (8<<CSF);
			} else {
				//e->sprite = SPR_CURLY;
				//e->frame = 18;
				e->frame = 3;
				e->x = e->linkedEntity->x + (4<<CSF);
			}
			e->y = e->linkedEntity->y - (18<<CSF);
		}
		break;
	}
}

// seen in credits
void onspawn_balrog_medic(Entity *e) {
	e->y += 12 << CSF;

	e->y -= 8 << CSF;
	e->x += 8 << CSF;
}

void ai_balrog_medic(Entity *e) {
	e->frame = 0;
	if(e->dir) { // Script turns Balrog right in best ending to change to Pooh Black sprite
		e->dir = 0;
		e->type = OBJ_POOH_BLACK_MEDIC;
		sprite_pal(&e->sprite[0], PAL0);
		sprite_pal(&e->sprite[1], PAL0);
		e->oframe = 255;
	} else {
		RANDBLINK(e, 1, 200);
	}
}
void onspawn_gaudi_patient(Entity *e) {
	e->y += pixel_to_sub(10);

	e->y -= 8 << CSF;
	e->x += 8 << CSF;
}

void ai_gaudi_patient(Entity *e) {
	switch(e->state) {
		case 0:		// sitting
		{
			e->frame = 0;
		}
		break;
		
		case 10:	// lying
		{
			e->frame = 1;
		}
		break;
		
		case 20:	// hurting
		{
			e->state = 21;
			e->frame = 2;
		} /* fallthrough */
		case 21:
		{
			ANIMATE(e, 16, 2,3);
		}
		break;
	}
}

void onspawn_baby_puppy(Entity *e) {
	e->y -= 8 << CSF;
}

void ai_baby_puppy(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->animtime = rand() & 7;	// desync with other puppies
			e->state++;
		} /* fallthrough */
		case 1:
		{
			ANIMATE(e, 8, 0,1);
		}
		break;
	}
}

void ai_turning_human(Entity *e) {
	// LEFT = Itoh
	// RIGHT = Sue
	switch(e->state) {
		case 0:
		{
			e->frame = (e->flags & NPC_OPTION2) ? 8 : 0;
			e->x += (16<<CSF);
			e->y -= (8<<CSF);
			e->state++;
		} /* fallthrough */
		case 1:
		{
			if(++e->timer > TIME_8(80)) {
				e->state = 10;
				e->frame += 2;
				e->timer = 0;
			}
			
			// before machine turns on they both blink,
			// at slightly different times
			if(e->flags & NPC_OPTION2) {
				if(e->timer == 30) e->frame++;
				if(e->timer == 40) e->frame--;
			} else {
				if(e->timer == 50) e->frame++;
				if(e->timer == 60) e->frame--;
			}
		}
		break;
		
		case 10:	// being transformed
		{
			e->timer++;
			if(e->timer & 1) e->frame ^= 1;
			
			if (e->timer > TIME_8(50)) {
				e->state = 20;
				e->frame = (e->flags & NPC_OPTION2) ? 12 : 4;
				// wait for slightly different times before falling
				e->timer = (e->flags & NPC_OPTION2) ? 40 : 60;
			}
		}
		break;
		
		case 20:	// waiting after transformation
		{
			if(--e->timer == 0) e->state = 30;
		}
		break;
		
		case 30:	// falling
		{
			if(e->y_speed < SPEED_12(0x5C0)) e->y_speed += SPEED_8(0x40);
			// sneeze
			if(++e->timer > TIME_8(50)) {
				e->state = 40;
				e->timer = 0;
				e->frame++;	// head-back to sneeze
				
				// create sneeze. Itoh is taller.
				int yoffs = (e->flags & NPC_OPTION2) ? (8<<CSF) : (16<<CSF);
				Entity *ahchoo = entity_create(e->x, e->y - yoffs, OBJ_AHCHOO, 0);
				ahchoo->linkedEntity = e;
			}
		}
		break;
		
		case 40:	// sneezing
		{
			e->timer++;
			if(e->timer == TIME_8(30)) e->frame++;
			else if(e->timer > TIME_8(40)) e->state = 50;
		}
		break;
		
		case 50:	// turn back to mimiga...
		{
			e->state = 41;
			e->timer = 0;
			e->frame = (e->flags & NPC_OPTION2) ? 8 : 0;
		} /* fallthrough */
		case 51:	// ..and blink
		{
			e->timer++;
			if(e->timer == TIME_8(30)) e->frame++;
			else if(e->timer == TIME_8(40)) e->frame--;
		}
		break;
	}
}

void ai_ahchoo(Entity *e) {
	if (!e->linkedEntity) {
		e->state = STATE_DELETE;
		return;
	}
	
	e->timer++;
	switch(e->state) {
		case 0:		// rise up "ah..."
		{
			if(e->timer < 4) e->y -= (2 << CSF);
			// sneezing frame
			if(e->timer > TIME_8(30)) {
				e->frame = 1;	// "choo!"
				e->timer = 0;
				e->state = 1;
				e->x_mark = e->x;
				e->y_mark = e->y;
			}
		}
		break;
		
		case 1:		// shaking "choo!"
		{
			if(e->timer < TIME_8(48)) {	// shake
				e->x = e->x_mark + (-1 + (rand() & 3));
				e->y = e->y_mark + (-1 + (rand() & 3));
			} else {	// return to original pos
				e->x_mark = e->x;
				e->y_mark = e->y;
			}
			
			if(e->timer > TIME_8(70)) e->state = STATE_DELETE;
		}
		break;
	}
}

void ai_misery_wind(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->x -= 8 << CSF;
			e->y -= (pal_mode ? 16 : 24) << CSF;
			entity_create(0,0,OBJ_END_BALCONY,0);
			e->state = 1;
		}
		// Fallthrough
		case 1:
		{
			ANIMATE(e, 8, 0,1);
		}
		break;
		
		case 10:	// look at screen
		{
			ANIMATE(e, 8, 3,4);
		}
		break;
	}
}

// 000 - King
// 100 - Toroko
// 200 - Kazuma
// 300 - Sue
// 400 - Momorin
// 500 - Booster
// 600 - Jenka
//
// 700 - Nurse Hasumi
// 800 - Dr Gero
// 900 - Balrog
// 1000 - Curly
// 1100 - Misery
// 1200 - Malco
// 1300 - Hermit Gunsmith
static const struct {
	uint16_t obj;
	uint8_t fallframe, standframe;
	uint8_t dir;
	uint8_t tall;
} cast_data[] = {
	{ OBJ_KING,				1, 0, RIGHT, FALSE, },
	{ OBJ_TOROKO,			1, 0, RIGHT, FALSE, },
	{ OBJ_KAZUMA,			1, 0, RIGHT, TRUE, },
	{ OBJ_SUE, 				1, 0, RIGHT, FALSE, },
	{ OBJ_MOMORIN, 			2, 0, LEFT, TRUE, },
	{ OBJ_PROFESSOR_BOOSTER,1, 0, LEFT, FALSE, },
	{ OBJ_JENKA,			0, 0, LEFT, FALSE, },
	
	{ OBJ_NURSE_HASUMI,	 	1, 0, RIGHT, FALSE, },
	{ OBJ_DR_GERO,		 	1, 0, RIGHT, FALSE, },
	{ OBJ_BALROG_MEDIC,	 	1, 0, RIGHT, TRUE, },
	{ OBJ_CURLY,			1, 0, RIGHT, FALSE, },
	{ OBJ_MISERY_STAND,	 	0, 2, LEFT, FALSE, },
	{ OBJ_MALCO_BROKEN,	 	7, 6, LEFT, TRUE, },
	{ OBJ_HERMIT_GUNSMITH, 	0, 0, LEFT, FALSE, },
};

void onspawn_the_cast(Entity *e) {
	//vdp_color(0, 0xEEE);
	e->y -= pixel_to_sub(64);
	e->alwaysActive = TRUE;
	e->id /= 100;
	if(e->id >= 14) e->id = 0;

	if(e->id <= 3) e->x -= block_to_sub(1);
	if(e->id >= 7 && e->id <= 10) e->x -= block_to_sub(1);

	e->dir = cast_data[e->id].dir;
	
	// Manual sprite VRAM allocation
	uint16_t obj = cast_data[e->id].obj;
	e->sprite_count--;
	e->frame = e->oframe = cast_data[e->id].fallframe;
	const SpriteDef *f = npc_info[obj].sprite;
	if(obj == OBJ_HERMIT_GUNSMITH) f = &SPR_Gunsmith2;
	e->framesize = f->tilesets[0]->numTile;
	e->tiloc = tiloc_add(e->framesize);
	if(e->tiloc != NOTILOC) {
		e->vramindex = tiloc_index + e->tiloc * 4;
		e->sprite[0] = (Sprite) {
			.size = f->sprites[0]->size,
			.attr = TILE_ATTR(npc_info[obj].palette,0,0,e->dir,e->vramindex),
		};
		if(e->id == 9) {
			e->sprite[1] = (Sprite) { // Balrog
				.size = f->sprites[1]->size,
				.attr = TILE_ATTR(npc_info[obj].palette, 0, 0, e->dir, e->vramindex + 12)
			};
		}
		TILES_QUEUE(f->tilesets[e->frame]->tiles, e->vramindex, e->framesize);
	}

	if(cast_data[e->id].tall) {
		e->y -= (4<<CSF);
		e->hit_box.bottom += 7;
	}
	
	// create King's sword
	//if(cast_data[e->id].obj == OBJ_KING) {
	//	Entity *sword = entity_create(e->x, e->y, OBJ_KINGS_SWORD, 0);
	//	sword->linkedEntity = e;
		//sword->carry.flip = TRUE;
	//}
}

void ai_the_cast(Entity *e) {
	if(!e->state) {
		e->dir = cast_data[e->id].dir;
		if(e->y_speed < SPEED_12(0x5C0)) e->y_speed += SPEED_8(0x40);
		e->y += e->y_speed;
		if((e->y >> CSF) + e->hit_box.bottom >= 200) {
			if(e->tiloc != NOTILOC) {
				uint16_t obj = cast_data[e->id].obj;
				e->frame = e->oframe = cast_data[e->id].standframe;
				const SpriteDef *f = npc_info[obj].sprite;
				if(obj == OBJ_HERMIT_GUNSMITH) f = &SPR_Gunsmith2;
				e->sprite[0].size = f->sprites[0]->size;
				e->sprite[0].attr = TILE_ATTR(npc_info[obj].palette, 0, 0, e->dir, e->vramindex);
				if(e->id == 9) {
					e->sprite[1] = (Sprite) { // Balrog
						.size = f->sprites[1]->size,
						.attr = TILE_ATTR(npc_info[obj].palette, 0, 0, e->dir, e->vramindex + 12)
					};
				}
				TILES_QUEUE(f->tilesets[e->frame]->tiles, e->vramindex, e->framesize);
			}
			e->state++;
		}
	}

	if(e->id == 9) {
		e->sprite[1].x = e->sprite[0].x - 8; // Balrog is separated for some reason
		e->sprite[1].y = e->sprite[0].y;
		vdp_sprite_add(&e->sprite[1]);
	}
	if(e->id == 7 || e->id == 10) moveMeToFront = TRUE; // Nurse and Curly over Balrog
}

#define REAL_SPRITE_COUNT 	(2+9+5+2+6)

#define BLOCK_INDEX			(2)
#define BLOCK_COUNT			(9)
#define BOTTOM_INDEX		(BLOCK_INDEX + BLOCK_COUNT)
#define BOTTOM_COUNT		(5)
#define DOOR_INDEX			(BOTTOM_INDEX + BOTTOM_COUNT)
#define DOOR_COUNT			(2)
#define GRASS_INDEX			(DOOR_INDEX + DOOR_COUNT)
#define GRASS_COUNT			(3)
#define REST_INDEX			(GRASS_INDEX + GRASS_COUNT)
#define REST_COUNT			(3)

#define sheet_block			deathSound
#define sheet_bottom		jump_time
#define sheet_door			hurtSound
#define sheet_grass			experience
#define sheet_leftmid		enableSlopes
#define sheet_lefttop		attack
#define sheet_righttop		animtime

#define STG_X				(128 + 160)
#define STG_Y				(128 + (pal_mode ? 80 : 72))

void ai_e_blcn_stg(Entity *e) {
	switch(e->state) {
		case 0:
		{
			e->alwaysActive = TRUE;
			e->flags = 0;
			e->sprite_count = 2; // Drawing everything except the statue manually
			e->x = (int32_t)(160 + 104) << CSF;
			e->y = (int32_t)(pal_mode ? 80 : 72) << CSF;
			e->display_box = (bounding_box) {{ 0,0,0,0 }}; // 0,0 origin for statue
			// All sheets used
			SHEET_FIND(e->sheet_block, SHEET_BLOCK);
			SHEET_FIND(e->sheet_bottom, SHEET_EBLCN1);
			SHEET_FIND(e->sheet_door, SHEET_EBLCN2);
			SHEET_FIND(e->sheet_grass, SHEET_EBLCN3);
			SHEET_FIND(e->sheet_leftmid, SHEET_EBLCN4);
			SHEET_FIND(e->sheet_lefttop, SHEET_EBLCN5);
			SHEET_FIND(e->sheet_righttop, SHEET_EBLCN6);
			// Block layout
			e->sprite[BLOCK_INDEX+0] = (Sprite) {
				.x = STG_X + 56, .y = STG_Y + 48,
				.size = SPRITE_SIZE(4,4),
				.attr = TILE_ATTR(PAL2,0,0,0,sheets[e->sheet_block].index),
			};
			e->sprite[BLOCK_INDEX+1] = (Sprite) {
				.x = STG_X + 56+32, .y = STG_Y + 48,
				.size = SPRITE_SIZE(4,4),
				.attr = TILE_ATTR(PAL2,0,0,0,sheets[e->sheet_block].index),
			};
			e->sprite[BLOCK_INDEX+2] = (Sprite) {
				.x = STG_X + 56+64, .y = STG_Y + 48,
				.size = SPRITE_SIZE(4,4),
				.attr = TILE_ATTR(PAL2,0,0,0,sheets[e->sheet_block].index),
			};
			e->sprite[BLOCK_INDEX+3] = (Sprite) {
				.x = STG_X + 40, .y = STG_Y + 48+32,
				.size = SPRITE_SIZE(4,4),
				.attr = TILE_ATTR(PAL2,0,0,0,sheets[e->sheet_block].index),
			};
			e->sprite[BLOCK_INDEX+4] = (Sprite) {
				.x = STG_X + 8, .y = STG_Y + 48+64,
				.size = SPRITE_SIZE(4,4),
				.attr = TILE_ATTR(PAL2,0,0,0,sheets[e->sheet_block].index),
			};
			e->sprite[BLOCK_INDEX+5] = (Sprite) {
				.x = STG_X + 8+32, .y = STG_Y + 48+64,
				.size = SPRITE_SIZE(4,4),
				.attr = TILE_ATTR(PAL2,0,0,0,sheets[e->sheet_block].index),
			};
			e->sprite[BLOCK_INDEX+6] = (Sprite) {
				.x = STG_X + 8+64, .y = STG_Y + 48+64,
				.size = SPRITE_SIZE(4,4),
				.attr = TILE_ATTR(PAL2,0,0,0,sheets[e->sheet_block].index),
			};
			e->sprite[BLOCK_INDEX+7] = (Sprite) {
				.x = STG_X + 8+96, .y = STG_Y + 48+64,
				.size = SPRITE_SIZE(4,4),
				.attr = TILE_ATTR(PAL2,0,0,0,sheets[e->sheet_block].index),
			};
			e->sprite[BLOCK_INDEX+8] = (Sprite) {
				.x = STG_X + 8+128, .y = STG_Y + 48+64,
				.size = SPRITE_SIZE(3,4),
				.attr = TILE_ATTR(PAL2,0,0,0,sheets[e->sheet_block].index),
			};
			// Bottom layout
			e->sprite[BOTTOM_INDEX+0] = (Sprite) {
				.x = STG_X + 0, .y = STG_Y + 144,
				.size = SPRITE_SIZE(4,2),
				.attr = TILE_ATTR(PAL2,0,0,0,sheets[e->sheet_bottom].index),
			};
			e->sprite[BOTTOM_INDEX+1] = (Sprite) {
				.x = STG_X + 32, .y = STG_Y + 144,
				.size = SPRITE_SIZE(4,2),
				.attr = TILE_ATTR(PAL2,0,0,0,sheets[e->sheet_bottom].index+16),
			};
			e->sprite[BOTTOM_INDEX+2] = (Sprite) {
				.x = STG_X + 64, .y = STG_Y + 144,
				.size = SPRITE_SIZE(4,2),
				.attr = TILE_ATTR(PAL2,0,0,0,sheets[e->sheet_bottom].index+32),
			};
			e->sprite[BOTTOM_INDEX+3] = (Sprite) {
				.x = STG_X + 96, .y = STG_Y + 144,
				.size = SPRITE_SIZE(4,2),
				.attr = TILE_ATTR(PAL2,0,0,0,sheets[e->sheet_bottom].index),
			};
			e->sprite[BOTTOM_INDEX+4] = (Sprite) {
				.x = STG_X + 128, .y = STG_Y + 144,
				.size = SPRITE_SIZE(4,2),
				.attr = TILE_ATTR(PAL2,0,0,0,sheets[e->sheet_bottom].index+16),
			};
			// Door area layout
			e->sprite[DOOR_INDEX+0] = (Sprite) {
				.x = STG_X + 40+32, .y = STG_Y + 48+32,
				.size = SPRITE_SIZE(4,4),
				.attr = TILE_ATTR(PAL2,0,0,0,sheets[e->sheet_door].index),
			};
			e->sprite[DOOR_INDEX+1] = (Sprite) {
				.x = STG_X + 40+64, .y = STG_Y + 48+32,
				.size = SPRITE_SIZE(4,4),
				.attr = TILE_ATTR(PAL2,0,0,0,sheets[e->sheet_door].index+16),
			};
			// Patches of grass
			e->sprite[GRASS_INDEX+0] = (Sprite) {
				.x = STG_X + 56, .y = STG_Y + 40,
				.size = SPRITE_SIZE(2,1),
				.attr = TILE_ATTR(PAL2,0,0,0,sheets[e->sheet_grass].index),
			};
			e->sprite[GRASS_INDEX+1] = (Sprite) {
				.x = STG_X + 56+32, .y = STG_Y + 40,
				.size = SPRITE_SIZE(2,1),
				.attr = TILE_ATTR(PAL2,0,0,0,sheets[e->sheet_grass].index+2),
			};
			e->sprite[GRASS_INDEX+2] = (Sprite) {
				.x = STG_X + 144, .y = STG_Y + 104,
				.size = SPRITE_SIZE(2,1),
				.attr = TILE_ATTR(PAL2,0,0,0,sheets[e->sheet_grass].index+2),
			};
			// Slopes on the left side
			e->sprite[REST_INDEX+0] = (Sprite) {
				.x = STG_X + 8, .y = STG_Y + 48+40,
				.size = SPRITE_SIZE(4,3),
				.attr = TILE_ATTR(PAL2,0,0,0,sheets[e->sheet_leftmid].index),
			};
			e->sprite[REST_INDEX+1] = (Sprite) {
				.x = STG_X + 40, .y = STG_Y + 48,
				.size = SPRITE_SIZE(2,4),
				.attr = TILE_ATTR(PAL2,0,0,0,sheets[e->sheet_lefttop].index),
			};
			e->sprite[REST_INDEX+2] = (Sprite) {
				.x = STG_X + 152, .y = STG_Y + 48,
				.size = SPRITE_SIZE(1,4),
				.attr = TILE_ATTR(PAL2,0,0,0,sheets[e->sheet_righttop].index),
			};
			e->state = 1;
		}
		// Fallthrough
		case 1:
		{
			vdp_sprites_add(&e->sprite[BLOCK_INDEX], REAL_SPRITE_COUNT - BLOCK_INDEX);
		}
		break;
	}
}
