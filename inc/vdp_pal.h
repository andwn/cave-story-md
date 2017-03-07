/**
 *  \file vdp_pal.h
 *  \brief VDP Palette support
 *  \author Stephane Dallongeville
 *  \date 08/2011
 *
 * This unit provides methods to manipulate the VDP Color Palette.<br>
 * The Sega Genesis VDP has 4 palettes of 16 colors.<br>
 * Color is defined with 3 bits for each component : xxxxBBBxGGGxRRRx
 */

#define VDPPALETTE_REDSFT           1
#define VDPPALETTE_GREENSFT         5
#define VDPPALETTE_BLUESFT          9

#define VDPPALETTE_REDMASK          0x000E
#define VDPPALETTE_GREENMASK        0x00E0
#define VDPPALETTE_BLUEMASK         0x0E00
#define VDPPALETTE_COLORMASK        0x0EEE

extern const uint16_t palette_grey[16];

int16_t fading_cnt;

/**
 *  \brief
 *      Returns RGB color of specified palette entry.
 *
 *  \param index
 *      Color index (0-63).
 *  \return RGB intensity for the specified color index.
 */
uint16_t  VDP_getPaletteColor(uint16_t index);
/**
 *  \brief
 *      Read count RGB colors from specified index and store them in specified palette.
 *
 *  \param index
 *      Color index where start to read (0-63).
 *  \param dest
 *      Destination palette where to write read RGB intensities.
 *  \param count
 *      Number of color to get.
 */
void  VDP_getPaletteColors(uint16_t index, uint16_t* dest, uint16_t count);
/**
 *  \brief
 *      Get palette.
 *
 *  \param index
 *      Palette index (0-3).
 *  \param pal
 *      Destination where to copy palette (should be 16 words long at least)
 */
void VDP_getPalette(uint16_t index, uint16_t *pal);

/**
 *  \brief
 *      Set RGB color to specified palette entry.
 *
 *  \param index
 *      Color index to set (0-63).
 *  \param value
 *      RGB intensity to set in the specified color index.
 */
void VDP_setPaletteColor(uint16_t index, uint16_t value);
/**
 *  \brief
 *      Set RGB colors to specified palette entries.
 *
 *  \param index
 *      Color index where to start to write (0-63).
 *  \param values
 *      RGB intensities to set.
 *  \param count
 *      Number of color to set.
 */
void VDP_setPaletteColors(uint16_t index, const uint16_t* values, uint16_t count);
/**
 *  \brief
 *      Set palette.
 *
 *  \param num
 *      Palette number (0-3).
 *  \param pal
 *      Source palette.
 */
void VDP_setPalette(uint16_t num, const uint16_t *pal);


// these functions should be private as they are called by VDP_fadeXXX functions internally
// but they can be useful sometime for better control on the fading processus
uint16_t  VDP_doStepFading();
uint16_t  VDP_initFading(uint16_t fromcol, uint16_t tocol, const uint16_t *palsrc, const uint16_t *paldst, uint16_t numframe);

/**
 *  \brief
 *      General palette fading effect.
 *
 *  \param fromcol
 *      Start color index for the fade effect (0-63).
 *  \param tocol
 *      End color index for the fade effect (0-63 and >= fromcol).
 *  \param palsrc
 *      Fade departure palette.
 *  \param paldst
 *      Fade arrival palette.
 *  \param numframe
 *      Duration of palette fading in number of frame.
 *  \param async
 *      Async process.<br>
 *      If set the function return immediatly else the function wait for fading to complete.
 *
 *  This function does general palette fading effect.<br>
 *  The fade operation is done to all palette entries between 'fromcol' and 'tocol'.<br>
 *  Example: fading to all palette entries --> fromcol = 0  and  tocol = 63
 */
void VDP_fade(uint16_t fromcol, uint16_t tocol, const uint16_t *palsrc, const uint16_t *paldst, uint16_t numframe, uint8_t async);
/**
 *  \brief
 *      Fade current color palette to specified one.
 *
 *  \param fromcol
 *      Start color index for the fade operation (0-63).
 *  \param tocol
 *      End color index for the fade operation (0-63 and >= fromcol).
 *  \param pal
 *      Fade arrival palette.
 *  \param numframe
 *      Duration of palette fading in number of frame.
 *  \param async
 *      Async process.<br>
 *      If set the function return immediatly else the function wait for fading to complete.
 *
 *  See VDP_fade() for more informations.
 */
void VDP_fadeTo(uint16_t fromcol, uint16_t tocol, const uint16_t *pal, uint16_t numframe, uint8_t async);
