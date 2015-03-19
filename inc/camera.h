#ifndef INC_CAMERA_H_
#define INC_CAMERA_H_

#include "common.h"
#include "entity.h"

struct {
	s32 x, y; // Units
	Entity *target; // Player, boss, NPC, or NULL
	// Offset is the point relative to the target's position
	// that the camera tries to center on
	s16 x_offset, y_offset;
} camera;

// Initialize the camera with default values (upper left, no target)
void camera_init();
// Center camera directly on a specific point
// This does not redraw or morph the stage, call stage_draw_area() manually after
void camera_set_position(s32 x, s32 y);

void camera_shake(u16 time);
// Per frame update for camera, moves toward the target entity
// Will "morph" the stage (draw areas surrounding the screen) when necessary
void camera_update();

// Functions used by TSC Scripts
void camera_focus_on_player(u16 speed);
void camera_focus_on_entity(Entity *e, u16 speed);
void camera_focus_on_boss(u16 speed);

#endif /* INC_CAMERA_H_ */
