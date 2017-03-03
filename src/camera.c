#include "camera.h"

#include <genesis.h>
#include "input.h"
#include "player.h"
#include "stage.h"
#include "tables.h"

// Since only one row or column of tiles is drawn at a time
#define CAMERA_MAX_SPEED 	0xFFF
#define LIMIT 				8
#define FOCUS_SPEED 		5
// While following the player focus on a variable point left/right of the player
// The idea is to move slowly along this line relative to the player, but not globally
// limit the speed that way or we will fall behind
#define PAN_SPEED	0x200
#define PAN_SIZE	0x6000
// When cameraShake is nonzero the camera will shake, and decrement this value
// each frame until it becomes zero again
u16 cameraShake;
// Tile attr buffer to draw offscreen during map scroll
u16 mapcol[32], maprow[64];

void camera_init() {
	camera.target = &player;
	camera.x = pixel_to_sub(SCREEN_HALF_W);
	camera.y = pixel_to_sub(SCREEN_HALF_H + 8);
	camera.x_offset = 0;
	cameraShake = 0;
}

void camera_set_position(s32 x, s32 y) {
	// Don't let the camera leave the stage
	if(x > block_to_sub(stageWidth) - (SCREEN_HALF_W<<CSF))
		x = block_to_sub(stageWidth) - (SCREEN_HALF_W<<CSF);
	if(y > block_to_sub(stageHeight) - ((SCREEN_HALF_H+LIMIT)<<CSF))
		y = block_to_sub(stageHeight) - ((SCREEN_HALF_H+LIMIT)<<CSF);
	if(x < SCREEN_HALF_W<<CSF) 
		x = SCREEN_HALF_W<<CSF;
	if(y < (SCREEN_HALF_H+LIMIT)<<CSF) 
		y = (SCREEN_HALF_H+LIMIT)<<CSF;
	// Apply
	camera.x = x;
	camera.y = y;
	// Update quick fetch cutoff values
	camera_xmin = camera.x - pixel_to_sub(SCREEN_HALF_W + 32);
	camera_xsize = pixel_to_sub(SCREEN_WIDTH + 64);
	camera_ymin = camera.y - pixel_to_sub(SCREEN_HALF_H + 32);
	camera_ysize = pixel_to_sub(SCREEN_HEIGHT + 64);
}

void camera_shake(u16 time) {
	cameraShake = time;
}

void camera_update() {
	// Just stick to the target object
	s32 x_next, y_next;
	if(camera.target) {
		// If following the player focus on where they are walking/looking
		if(camera.target == &player) {
			if(player.dir == LEFT) {
				//camera.x_offset = -pixel_to_sub(48);
				camera.x_offset -= PAN_SPEED;
				if(camera.x_offset < -PAN_SIZE) camera.x_offset = -PAN_SIZE;
			} else {
				//camera.x_offset = pixel_to_sub(48);
				camera.x_offset += PAN_SPEED;
				if(camera.x_offset > PAN_SIZE) camera.x_offset = PAN_SIZE;
			}
			if(!controlsLocked && joy_down(BUTTON_UP) && player.y_speed <= 0) {
				camera.y_offset -= PAN_SPEED;
				if(camera.y_offset < -PAN_SIZE) camera.y_offset = -PAN_SIZE;
			} else {
				camera.y_offset += PAN_SPEED;
				if(camera.y_offset > 0) camera.y_offset = 0;
			}
			// Calculate where camera will be next frame
			x_next = camera.x +
					(((floor(camera.target->x) + camera.x_offset) - camera.x) >> 4);
			y_next = camera.y +
					(((floor(camera.target->y) + camera.y_offset) - camera.y) >> 4);
		} else {
			x_next = camera.x +
					(((floor(camera.target->x) + camera.x_offset) - camera.x) >> 5);
			y_next = camera.y +
					(((floor(camera.target->y) + camera.y_offset) - camera.y) >> 5);
		}
		// Camera shaking
		if(cameraShake) {
			x_next += (random() % 0x800) - 0x400;
			y_next += (random() % 0x800) - 0x400;
			if(cameraShake != 9999) cameraShake--;
		}
	} else { // Camera isn't following anything
		return;
	}
	// Enforce max camera speed
	if(x_next - camera.x < -CAMERA_MAX_SPEED) x_next = camera.x - CAMERA_MAX_SPEED;
	if(y_next - camera.y < -CAMERA_MAX_SPEED) y_next = camera.y - CAMERA_MAX_SPEED;
	if(x_next - camera.x > CAMERA_MAX_SPEED) x_next = camera.x + CAMERA_MAX_SPEED;
	if(y_next - camera.y > CAMERA_MAX_SPEED) y_next = camera.y + CAMERA_MAX_SPEED;
	// Don't let the camera leave the stage
	if(stageID == 18 && !IS_PALSYSTEM) { // Special case for shelter
		x_next = pixel_to_sub(SCREEN_HALF_W + LIMIT);
		y_next = pixel_to_sub(SCREEN_HALF_H + 16);
	} else {
		if(x_next < pixel_to_sub(SCREEN_HALF_W + LIMIT))
			x_next = pixel_to_sub(SCREEN_HALF_W + LIMIT);
		else if(x_next > block_to_sub(stageWidth) - pixel_to_sub(SCREEN_HALF_W + LIMIT))
			x_next = block_to_sub(stageWidth) - pixel_to_sub(SCREEN_HALF_W + LIMIT);
		if(y_next < pixel_to_sub(SCREEN_HALF_H + LIMIT))
			y_next = pixel_to_sub(SCREEN_HALF_H + LIMIT);
		else if(y_next > block_to_sub(stageHeight) - pixel_to_sub(SCREEN_HALF_H + LIMIT))
			y_next = block_to_sub(stageHeight) - pixel_to_sub(SCREEN_HALF_H + LIMIT);
	}
	// Update quick fetch cutoff values
	camera_xmin = camera.x - pixel_to_sub(SCREEN_HALF_W + 32);
	camera_xsize = pixel_to_sub(SCREEN_WIDTH + 64);
	camera_ymin = camera.y - pixel_to_sub(SCREEN_HALF_H + 32);
	camera_ysize = pixel_to_sub(SCREEN_HEIGHT + 64);
	// Morph the stage if the camera is moving
	morphingColumn = sub_to_tile(x_next) - sub_to_tile(camera.x);
	morphingRow = sub_to_tile(y_next) - sub_to_tile(camera.y);
	if(morphingColumn | morphingRow) {
		// Reactivate any entities that are approaching the screen
		entities_update_inactive();
		// Queue row and/or column mapping
		if(morphingColumn) {
			s16 x = sub_to_tile(x_next) + (morphingColumn == 1 ? 30 : -30);
			s16 y = sub_to_tile(y_next) - 16 + morphingRow;
			if(x >= 0 && x < stageWidth * 2) {
				for(u8 i = 32; i--; ) {
					if(y >= stageHeight * 2) break;
					if(y >= 0) {
						// Fuck math tbh
						u8 b = stage_get_block(x/2, y/2);
						u16 t = (b%16) * 2 + (b/16) * 64;
						u8 ta = stage_get_block_type(x/2, y/2);
						mapcol[y%32] = TILE_ATTR_FULL(ta == 0x43 ? PAL1 : PAL2, (ta&0x40) > 0, 
								0, 0, TILE_TSINDEX + t + (x&1) + ((y&1)*32));
					}
					y++;
				}
				DMA_queueDma(DMA_VRAM, (u32)mapcol, VDP_getAPlanAddress() + (x%64)*2, 32, 128);
			}
		}
		if(morphingRow) {
			s16 y = sub_to_tile(y_next) + (morphingRow == 1 ? 15 : -15);
			s16 x = sub_to_tile(x_next) - 32 + morphingColumn;
			if(y >= 0 && y < stageHeight * 2) {
				for(u8 i = 64; i--; ) {
					if(x >= stageWidth * 2) break;
					if(x >= 0) {
						u8 b = stage_get_block(x/2, y/2);
						u16 t = (b%16) * 2 + (b/16) * 64;
						u8 ta = stage_get_block_type(x/2, y/2);
						maprow[x%64] = TILE_ATTR_FULL(ta == 0x43 ? PAL1 : PAL2, (ta&0x40) > 0, 
								0, 0, TILE_TSINDEX + t + (x&1) + ((y&1)*32));
					}
					x++;
				}
				DMA_queueDma(DMA_VRAM, (u32)maprow, VDP_getAPlanAddress() + (y%32)*64*2, 64, 2);
			}
		}
	}
	// Apply camera position
	camera.x = x_next;
	camera.y = y_next;
}
