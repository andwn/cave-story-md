#include "camera.h"

#include <genesis.h>
#include "stage.h"
#include "player.h"

// One less than BLOCK_SIZE in sub-pixels, because only one
// row/column is drawn per frame while the camera is moving
#define CAMERA_MAX_SPEED (block_to_sub(1) - 1)

u16 focusSpeed = 32;
// When cameraShake is nonzero the camera will shake, and decrement this value
// each frame until it becomes zero again
u16 cameraShake;

void camera_init() {
	camera.target = &player;
	camera.x = pixel_to_sub(SCREEN_HALF_W);
	camera.y = pixel_to_sub(SCREEN_HALF_H);
	cameraShake = 0;
}

void camera_set_position(s32 x, s32 y) {
	// Don't let the camera leave the stage
	if(x < pixel_to_sub(SCREEN_HALF_W)) x = pixel_to_sub(SCREEN_HALF_W);
	if(y < pixel_to_sub(SCREEN_HALF_H)) y = pixel_to_sub(SCREEN_HALF_H);
	if(x > block_to_sub(stageWidth) - pixel_to_sub(SCREEN_HALF_W))
		x = block_to_sub(stageWidth) - pixel_to_sub(SCREEN_HALF_W);
	if(y > block_to_sub(stageHeight) - pixel_to_sub(SCREEN_HALF_H))
		y = block_to_sub(stageHeight) - pixel_to_sub(SCREEN_HALF_H);
	// Apply
	camera.x = x;
	camera.y = y;
}

void camera_shake(u16 time) {
	cameraShake = time;
}

void camera_update() {
	// Just stick to the target object
	s32 x_next, y_next;
	if(camera.target != NULL) {
		camera.x_offset = 0;
		camera.y_offset = 0;
		// If following the player focus on where they are walking/looking
		if(camera.target == &player) {
			if(player.controller[0] & BUTTON_LEFT) {
				camera.x_offset = pixel_to_sub(-48);
			} else if(player.controller[0] & BUTTON_RIGHT) {
				camera.x_offset = pixel_to_sub(48);
			}
			if((player.controller[0] & BUTTON_UP) && player.y_speed <= 0) {
				camera.y_offset = pixel_to_sub(-48);
			}
		}
		// Camera shaking
		if(cameraShake > 0) {
			camera.x_offset += (random() % 0x2000) - 0x1000;
			camera.y_offset += (random() % 0x2000) - 0x1000;
			cameraShake--;
		}
		// Calculate where camera will be next frame
		x_next = camera.x +
				((floor(camera.target->x) + camera.x_offset) - camera.x) / focusSpeed;
		y_next = camera.y +
				((floor(camera.target->y) + camera.y_offset) - camera.y) / focusSpeed;
	} else { // Camera isn't following anything
		return;
	}
	// Enforce max camera speed
	if(x_next - camera.x < -CAMERA_MAX_SPEED) x_next = camera.x - CAMERA_MAX_SPEED;
	if(y_next - camera.y < -CAMERA_MAX_SPEED) y_next = camera.y - CAMERA_MAX_SPEED;
	if(x_next - camera.x > CAMERA_MAX_SPEED) x_next = camera.x + CAMERA_MAX_SPEED;
	if(y_next - camera.y > CAMERA_MAX_SPEED) y_next = camera.y + CAMERA_MAX_SPEED;
	// Don't let the camera leave the stage
	if(x_next < pixel_to_sub(SCREEN_HALF_W)) x_next = pixel_to_sub(SCREEN_HALF_W);
	if(y_next < pixel_to_sub(SCREEN_HALF_H)) y_next = pixel_to_sub(SCREEN_HALF_H);
	if(x_next > block_to_sub(stageWidth) - pixel_to_sub(SCREEN_HALF_W))
		x_next = block_to_sub(stageWidth) - pixel_to_sub(SCREEN_HALF_W);
	if(y_next > block_to_sub(stageHeight) - pixel_to_sub(SCREEN_HALF_H))
		y_next = block_to_sub(stageHeight) - pixel_to_sub(SCREEN_HALF_H);
	// Morph the stage if the camera is moving
	s16 x_block = sub_to_block(x_next);
	s16 y_block = sub_to_block(y_next);
	s16 x_dir = x_block - sub_to_block(camera.x);
	s16 y_dir = y_block - sub_to_block(camera.y);
	stage_morph(x_block, y_block, x_dir, y_dir);
	// Apply camera position
	camera.x = x_next;
	camera.y = y_next;
	// Reactivate any entities that are approaching the screen
	if(x_dir != 0 || y_dir != 0) {
		entities_update_inactive();
	}
}

void camera_focus_on_player(u16 speed) {
	camera.target = &player;
	focusSpeed = speed;
}

void camera_focus_on_entity(Entity *e, u16 speed) {
	camera.target = e;
	focusSpeed = speed;
}
// TODO: Focus on boss
void camera_focus_on_boss(u16 speed) {
	focusSpeed = speed;
}
