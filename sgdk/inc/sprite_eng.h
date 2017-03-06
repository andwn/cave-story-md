/**
 *  \file sprite_eng.h
 *  \brief Sprite engine
 *  \author Stephane Dallongeville
 *  \date 10/2013
 *
 * Sprite engine providing advanced sprites manipulation and operations.<br>
 * This unit use both the tile cache engine (see tilecache.h file for more info)<br>
 * and the Sega Genesis VDP sprite capabilities (see vdp_spr.h file for more info).
 */

#ifndef _SPRITE_ENG_H_
#define _SPRITE_ENG_H_

#include "vdp_pal.h"
#include "vdp_tile.h"
#include "vdp_spr.h"

/**
 *  \struct Collision
 *  \brief
 *      Collision definition union.
 *
 *  \param type
 *      Collision type:<br>
 *      Allowed values are #COLLISION_TYPE_BOX or #COLLISION_TYPE_CIRCLE.
 *  \param box
 *      Box definition if type = #COLLISION_TYPE_BOX
 *  \param circle
 *      Circle definition if type = #COLLISION_TYPE_CIRCLE
 */
typedef struct
{
    u16 type;
    union
    {
        Box box;
        Circle circle;
    };
} Collision;

/**
 *  \brief
 *      VDP sprite info structure for sprite resource definition.
 *
 *  \param y
 *      Y offset for this VDP sprite relative to global Sprite position plus 0x80 (0x80 = 0 = no offset)
 *  \param size
 *      sprite size (see SPRITE_SIZE macro)
 *  \param numTile
 *      number of tile for this VDP sprite (should be coherent with the given size field)
 *  \param x
 *      X offset for this VDP sprite relative to global Sprite position plus 0x80 (0x80 = 0 = no offset)
 */
typedef struct
{
    s16 y;          // respect VDP sprite field order
    u16 size;
    s16 x;
    u16 numTile;
}  VDPSpriteInf;


/**
 *  \brief
 *      Sprite animation frame structure.
 *
 *  \param numSprite
 *      number of VDP sprite which compose this frame
 *  \param vdpSprites
 *      VDP sprites composing the frame
 *  \param numCollision
 *      number of collision structure for this frame
 *  \param collisions
 *      collisions structures (can be either Box or Circle)
 *  \param tileset
 *      tileset containing tiles for this animation frame (ordered for sprite)
 *  \param w
 *      frame width in pixel
 *  \param h
 *      frame height in pixel
 *  \param timer
 *      active time for this frame (in 1/60 of second)
 */
typedef struct
{
    u16 numSprite;
    VDPSpriteInf **vdpSpritesInf;
    u16 numCollision;
    Collision **collisions;
    TileSet *tileset;
    s16 w;
    s16 h;
    u16 timer;
} AnimationFrame;

/**
 *  \brief
 *      Sprite animation structure.
 *
 *  \param numFrame
 *      number of different frame for this animation
 *  \param frames
 *      frames composing the animation
 *  \param length
 *      animation sequence length
 *  \param sequence
 *      frame sequence animation (for instance: 0-1-2-2-1-2-3-4..)
 *  \param loop
 *      frame sequence index for loop (last index if no loop)
 */
typedef struct
{
    u16 numFrame;
    AnimationFrame **frames;
    u16 length;
    u8 *sequence;
    s16 loop;
} Animation;

/**
 *  \brief
 *      Sprite definition structure.
 *
 *  \param palette
 *      Default palette data
 *  \param numAnimation
 *      number of animation for this sprite
 *  \param animations
 *      animation definitions
 *  \param maxNumTile
 *      maximum number of tile used by a single animation frame (used for VRAM tile space allocation)
 *  \param maxNumSprite
 *      maximum number of VDP sprite used by a single animation frame (used for VDP sprite allocation)
 *
 *  Contains all animations for a Sprite and internal informations.
 */
typedef struct
{
    Palette *palette;
    u16 numAnimation;
    Animation **animations;
    u16 maxNumTile;
    u16 maxNumSprite;
} SpriteDefinition;

#endif // _SPRITE_ENG_H_
