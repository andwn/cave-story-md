#include "common.h"
#include "ai.h"
#include "entity.h"
#include "player.h"

static const uint32_t tan_table[33] = {
	0,
	402,
	806,
	1215,
	1629,
	2051,
	2485,
	2931,
	3393,
	3874,
	4378,
	4910,
	5473,
	6075,
	6723,
	7424,
	8192,
	9038,
	9981,
	11045,
	12260,
	13667,
	15326,
	17320,
	19777,
	22895,
	27005,
	32704,
	41183,
	55225,
	83174,
	166752,
	// It is impossible for this to be less than anything, guaranteed to break the while loop
	0xFFFFFFFF, 
};

// My brain hurts... don't read this
uint8_t get_angle(int32_t curx, int32_t cury, int32_t tgtx, int32_t tgty) {
	uint8_t angle = 0;
	int32_t xdist = (tgtx - curx);
	int32_t ydist = (tgty - cury);
	if (!xdist) return (tgty > cury) ? 0x40 : 0xC0; // Undefined
	uint32_t ratio = (abs(ydist) << 13) / abs(xdist);
	while(tan_table[angle++] < ratio);
	//angle += 1;
	angle <<= 1;
	if (curx > tgtx) angle = 0x80 - angle;
	if (cury > tgty) angle = 0x100 - angle;
	return angle;
}

uint8_t mddir(uint8_t dir) {
	switch(dir) {
		case DIR_LEFT: 		return LEFT;
		case DIR_UP: 		return UP;
		case DIR_RIGHT: 	return RIGHT;
		case DIR_DOWN: 		return DOWN;
		case DIR_CENTER: 	return CENTER;
		default: 			return LEFT;
	}
}

uint8_t PLAYER_DIST_X(Entity *e, int32_t dist) {
	return player.x > e->x - dist && player.x < e->x + dist;
}

uint8_t PLAYER_DIST_Y(Entity *e, int32_t dist) {
	return player.y > e->y - dist && player.y < e->y + dist;
}

uint8_t PLAYER_DIST_X2(Entity *e, int32_t dist1, int32_t dist2) {
	return player.x > e->x - dist1 && player.x < e->x + dist2;
}

uint8_t PLAYER_DIST_Y2(Entity *e, int32_t dist1, int32_t dist2) {
	return player.y > e->y - dist1 && player.y < e->y + dist2;
}
