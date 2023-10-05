#include "ai_common.h"
#include "gamemode.h"

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
}

void ai_balrog_medic(Entity *e) {
	e->frame = 0;
	RANDBLINK(e, 1, 200);
}
void onspawn_gaudi_patient(Entity *e) {
	e->y += pixel_to_sub(10);
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
	{ OBJ_HERMIT_GUNSMITH, 	2, 1, LEFT, FALSE, },
};

void onspawn_the_cast(Entity *e) {
	//vdp_color(0, 0xEEE);
	e->y -= pixel_to_sub(64);
	e->alwaysActive = TRUE;
	e->id /= 100;
	if(e->id >= 14) e->id = 0;
	
	// Manual sprite VRAM allocation
	uint16_t obj = cast_data[e->id].obj;
	if(obj != OBJ_BALROG_MEDIC) e->sprite_count--;
	e->frame = e->oframe = cast_data[e->id].fallframe;
	const AnimationFrame *f = npc_info[obj].sprite->animations[0]->frames[e->frame];
	e->framesize = f->tileset->numTile;
	TILOC_ADD(e->tiloc, e->framesize);
	if(e->tiloc != NOTILOC) {
		e->vramindex = tiloc_index + e->tiloc * 4;
		uint16_t tile_offset = 0;
		for(uint8_t i = 0; i < e->sprite_count; i++) {
			e->sprite[i] = (Sprite) {
				.size = f->vdpSpritesInf[i]->size,
				.attr = TILE_ATTR(npc_info[obj].palette,
						0,0,0,e->vramindex + tile_offset)
			};
			TILES_QUEUE(f->tileset->tiles, e->vramindex, f->tileset->numTile);
			tile_offset += f->vdpSpritesInf[i]->numTile;
		}
	}
	
	e->dir = cast_data[e->id].dir;
	if(cast_data[e->id].tall) {
		e->y -= (4<<CSF);
		e->hit_box.bottom += 8;
	}
	
	// create King's sword
	//if(cast_data[e->id].obj == OBJ_KING) {
	//	Entity *sword = entity_create(e->x, e->y, OBJ_KINGS_SWORD, 0);
	//	sword->linkedEntity = e;
		//sword->carry.flip = TRUE;
	//}
}

void ai_the_cast(Entity *e) {
	if(e->id == 9) e->sprite[1].x = e->sprite[0].x - 8; // Balrog is separated for some reason
	if(e->id == 10) moveMeToFront = TRUE; // Curly over balrog
	
	if(!e->state) {
		e->dir = cast_data[e->id].dir;
		if(e->y_speed < SPEED_12(0x5C0)) e->y_speed += SPEED_8(0x40);
		e->y += e->y_speed;
		if((e->y >> CSF) + e->hit_box.bottom >= 200) {
			if(e->tiloc != NOTILOC) {
				uint16_t obj = cast_data[e->id].obj;
				e->frame = e->oframe = cast_data[e->id].standframe;
				const AnimationFrame *f = npc_info[obj].sprite->animations[0]->frames[e->frame];
				uint16_t tile_offset = 0;
				for(uint8_t i = 0; i < e->sprite_count; i++) {
					e->sprite[i] = (Sprite) {
							.size = f->vdpSpritesInf[i]->size,
							.attr = TILE_ATTR(npc_info[obj].palette,
							        0, 0, 0, e->vramindex + tile_offset)
					};
					TILES_QUEUE(f->tileset->tiles, e->vramindex, f->tileset->numTile);
					tile_offset += f->vdpSpritesInf[i]->numTile;
				}
			}
			e->state++;
		}
	}
}
