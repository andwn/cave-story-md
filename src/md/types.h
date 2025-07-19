#ifndef MD_TYPES_H
#define MD_TYPES_H

// bool and stdint types
#define FALSE   0
#define TRUE    1
#define NULL    0

typedef signed char		int8_t;
typedef signed short	int16_t;
typedef signed long		int32_t;

typedef unsigned char	uint8_t;
typedef unsigned short	uint16_t;
typedef unsigned long	uint32_t;

typedef struct {
    uint16_t width, height, frames, fsize;
    uint32_t tiles[0];
} LocSprite;

typedef struct {
    int16_t y;
    union {
        struct {
            uint8_t size;
            uint8_t link;
        };
        uint16_t size_link;
    };
    uint16_t attr;
    int16_t x;
} Sprite;

typedef struct {
	uint16_t numTile;
	uint32_t *tiles;
} TileSet;

typedef struct {
	int16_t y;
	uint16_t size;
	int16_t x;
	uint16_t numTile;
} VDPSpriteInf;

typedef struct {
	uint16_t 		numFrame;
	TileSet 		**tilesets;
	uint16_t 		numSprite;
	VDPSpriteInf 	**sprites;
	int16_t			w;
	int16_t			h;
} SpriteDef;

#endif //MD_TYPES_H
