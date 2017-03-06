/**
 *  \file vdp_spr.h
 *  \brief VDP Sprite support
 *  \author Stephane Dallongeville
 *  \date 08/2011
 *
 * This unit provides methods to allocate and manipulate VDP Sprite at low level.<br>
 * The Sega Genesis VDP can handle up to 80 simultanous sprites of 4x4 tiles (32x32 pixels).
 */

#ifndef _VDP_SPR_H_
#define _VDP_SPR_H_

/**
 *  \brief
 *      Maximum number of hardware sprite
 */
#define MAX_VDP_SPRITE          80

/**
 *  \brief
 *      Helper to define sprite size in sprite definition structure.
 *
 *  \param w
 *      sprite width (in tile).
 *  \param h
 *      sprite height (in tile).
 */
#define SPRITE_SIZE(w, h)   ((((w) - 1) << 2) | ((h) - 1))

/**
 *  \brief
 *      VDP sprite definition structure replicating VDP hardware sprite.
 *
 *  \param y
 *      Y position - 0x80 (0x80 = 0 on screen)
 *  \param size
 *      sprite size (see SPRITE_SIZE macro)
 *  \param link
 *      sprite link, this information is used to define sprite drawing order (use 0 to force end of list)
 *  \param attr
 *      tile index and sprite attribut (priority, palette, H/V flip), see TILE_ATTR_FULL macro
 *  \param x
 *      X position - 0x80 (0x80 = 0 on screen)
 */
typedef struct
{
    s16 y;
    union
    {
        struct {
            u8 size;
            u8 link;
        };
        u16 size_link;
    };
    u16 attribut;
    s16 x;
}  VDPSprite;

#endif // _VDP_SPR_H_
