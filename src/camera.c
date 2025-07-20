#include "common.h"

#include "gamemode.h"
#include "md/dma.h"
#include "entity.h"
#include "md/joy.h"
#include "math.h"
#include "player.h"
#include "stage.h"
#include "tables.h"
#include "md/stdlib.h"
#include "md/vdp.h"

#include "camera.h"

// Since only one row or column of tiles is drawn at a time
#define CAMERA_MAX_SPEED 	0xFFF
#define FOCUS_SPEED 		5
// While following the player focus on a variable point left/right of the player
// The idea is to move slowly along this line relative to the player, but not globally
// limit the speed that way or we will fall behind
#define PAN_SPEED	0x200
#define PAN_SIZE	0x6000

int32_t camera_xmin, camera_ymin;
uint32_t camera_xsize, camera_ysize;
Camera camera;

// When cameraShake is nonzero the camera will shake, and decrement this value
// each frame until it becomes zero again
uint16_t cameraShake;
// Tile attr buffer to draw offscreen during map scroll
uint16_t mapbuf[64+32];

void camera_init(void) {
	camera.target = &player;
    camera.x_offset = 0;
    cameraShake = 0;
    camera_set_position(0, 0);
}

void camera_set_position_force(int32_t x, int32_t y) {
	camera.x = camera.x_mark = x;
	camera.y = camera.y_mark = y;
	camera.x_shifted = (x >> CSF) - ScreenHalfW;
	camera.y_shifted = (y >> CSF) - ScreenHalfH;
}

void camera_set_position(int32_t x, int32_t y) {
	// Don't let the camera leave the stage
	if(x > block_to_sub(g_stage.pxm.width) - pixel_to_sub(ScreenHalfW + 8))
		x = block_to_sub(g_stage.pxm.width) - pixel_to_sub(ScreenHalfW + 8);
	if(y > block_to_sub(g_stage.pxm.height) - pixel_to_sub(ScreenHalfH + 8))
		y = block_to_sub(g_stage.pxm.height) - pixel_to_sub(ScreenHalfH + 8);
	if(x < pixel_to_sub(ScreenHalfW + 8)) x = pixel_to_sub(ScreenHalfW + 8);
	if(y < pixel_to_sub(ScreenHalfH + 8)) y = pixel_to_sub(ScreenHalfH + 8);
	// Apply
	camera.x = camera.x_mark = x;
	camera.y = camera.y_mark = y;
	camera.x_shifted = (x >> CSF) - ScreenHalfW;
	camera.y_shifted = (y >> CSF) - ScreenHalfH;
	// Update quick fetch cutoff values
	camera_xmin = camera.x - pixel_to_sub(ScreenHalfW + 32);
	camera_xsize = pixel_to_sub(ScreenWidth + 64);
	camera_ymin = camera.y - pixel_to_sub(ScreenHalfH + 32);
	camera_ysize = pixel_to_sub(ScreenHeight + 64);
}

void camera_shake(uint16_t time) {
	cameraShake = time;
}

void camera_update(void) {
	int32_t x_next, y_next;
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
			if(!controlsLocked && joy_down(JOY_UP)) {
				camera.y_offset -= PAN_SPEED;
				if(camera.y_offset < -PAN_SIZE) camera.y_offset = -PAN_SIZE;
			} else if(!controlsLocked && joy_down(JOY_DOWN)) {
				camera.y_offset += PAN_SPEED;
				if(camera.y_offset > PAN_SIZE) camera.y_offset = PAN_SIZE;
			} else {
				if(camera.y_offset > 0)
					camera.y_offset -= PAN_SPEED;
				else if(camera.y_offset < 0)
					camera.y_offset += PAN_SPEED;
			}
			// Calculate where camera will be next frame
			x_next = camera.x +
					(((floor(camera.target->x) + camera.x_offset) - camera.x) >> 4);
			y_next = camera.y +
					(((floor(camera.target->y) + camera.y_offset) - camera.y) >> 4);
		} else {
			// Just stick to the target object without any offset
			camera.x_offset = 0;
			camera.y_offset = 0;
			x_next = camera.x +
					(((floor(camera.target->x) + camera.x_offset) - camera.x) >> 5);
			y_next = camera.y +
					(((floor(camera.target->y) + camera.y_offset) - camera.y) >> 5);
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
	uint16_t bounds = cameraShake ? 2 : 8;
	if(g_stage.id == 18 && !pal_mode) { // Special case for shelter
		x_next = pixel_to_sub(ScreenHalfW + 8);
		y_next = pixel_to_sub(ScreenHalfH + 16);
	} else {
		if(x_next < pixel_to_sub(ScreenHalfW + bounds)) {
			x_next = pixel_to_sub(ScreenHalfW + bounds);
		} else if(x_next > block_to_sub(g_stage.pxm.width) - pixel_to_sub(ScreenHalfW + bounds)) {
			x_next = block_to_sub(g_stage.pxm.width) - pixel_to_sub(ScreenHalfW + bounds);
		}
		if(y_next < pixel_to_sub(ScreenHalfH + bounds)) {
			y_next = pixel_to_sub(ScreenHalfH + bounds);
		} else if(y_next > block_to_sub(g_stage.pxm.height) - pixel_to_sub(ScreenHalfH + bounds)) {
			y_next = block_to_sub(g_stage.pxm.height) - pixel_to_sub(ScreenHalfH + bounds);
		}
	}
	// Camera shaking
	// stay within the same (pre-shake) 8x8 area to avoid pointless map redraw
	if(cameraShake && (--cameraShake & 1)) {
		int16_t x_shake = (rand() & 0x7FF) - 0x400;
		int16_t y_shake = (rand() & 0x7FF) - 0x400;
		if((x_next & 0xF000) == ((x_next + x_shake) & 0xF000)) x_next += x_shake;
		if((y_next & 0xF000) == ((y_next + y_shake) & 0xF000)) y_next += y_shake;
	}
	// Update quick fetch cutoff values
	camera_xmin = camera.x - pixel_to_sub(ScreenHalfW + 32);
	camera_xsize = pixel_to_sub(ScreenWidth + 64);
	camera_ymin = camera.y - pixel_to_sub(ScreenHalfH + 32);
	camera_ysize = pixel_to_sub(ScreenHeight + 64);
	// Morph the stage if the camera is moving
	g_stage.scrolling_column = sub_to_tile(x_next) - sub_to_tile(camera.x);
	g_stage.scrolling_row = sub_to_tile(y_next) - sub_to_tile(camera.y);
	if(g_stage.scrolling_column | g_stage.scrolling_row) {
		uint16_t *buf = mapbuf;
		// Queue row and/or column mapping
		if(g_stage.scrolling_column) {
			int16_t x = sub_to_tile(x_next) + (g_stage.scrolling_column == 1 ? 30 : -30);
			int16_t y = sub_to_tile(y_next) - 16 /*+ morphingRow*/;
			if(x >= 0 && x < (int16_t)(g_stage.pxm.width) << 1) {
				for(uint16_t i = 32; i--; ) {
					uint16_t b = stage_get_block(x>>1, y>>1);
					uint16_t t = b << 2; //((b&15) << 1) + ((b>>4) << 6);
					uint16_t ta = g_stage.pxa[b]; //stage_get_block_type(x>>1, y>>1);
					uint16_t pal = (ta == 0x43 || ta & 0x80) ? PAL1 : PAL2;
					buf[y&31] = TILE_ATTR(pal, (ta&0x40) > 0, 
							0, 0, TILE_TSINDEX + t + (x&1) + ((y&1)<<1));
					y++;
				}
				dma_queue(DmaVRAM, (uint32_t) buf, VDP_PLANE_A + ((x & 63) << 1), 32, 128);
				buf += 32;
			}
		}
		if(g_stage.scrolling_row) {
			int16_t y = sub_to_tile(y_next) + (g_stage.scrolling_row == 1 ? 15 : -15);
			int16_t x = sub_to_tile(x_next) - 32 /*+ morphingColumn*/;
			if(y >= 0 && y < (int16_t)(g_stage.pxm.height) << 1) {
				for(uint16_t i = 64; i--; ) {
					uint16_t b = stage_get_block(x>>1, y>>1);
					uint16_t t = b << 2; //((b&15) << 1) + ((b>>4) << 6);
					uint16_t ta = g_stage.pxa[b]; //stage_get_block_type(x>>1, y>>1);
					uint16_t pal = (ta == 0x43 || ta & 0x80) ? PAL1 : PAL2;
					buf[x&63] = TILE_ATTR(pal, (ta&0x40) > 0, 
							0, 0, TILE_TSINDEX + t + (x&1) + ((y&1)<<1));
					x++;
				}
				dma_queue(DmaVRAM, (uint32_t) buf, VDP_PLANE_A + ((y & 31) << 7), 64, 2);
			}
		}
	}
	if(!g_stage.scrolling_column && (abs(camera.x_mark - x_next) > 0x1FFF || abs(camera.y_mark - y_next) > 0x1FFF)) {
		camera.x_mark = x_next;
		camera.y_mark = y_next;
		entities_update_inactive();
	}
	// Apply camera position
	camera.x = x_next;
	camera.y = y_next;
	// Shifted values
	camera.x_shifted = (x_next >> CSF) - ScreenHalfW;
	camera.y_shifted = (y_next >> CSF) - ScreenHalfH;
    // Update scrolling
    if(gamemode == GM_GAME) stage_update();
}
