/**
 *  \file tools.h
 *  \brief Misc tools methods
 *  \author Stephane Dallongeville
 *  \date 08/2011
 *
 * This unit provides some misc tools methods as getFPS(), unpack()...
 */

/**
 *  \brief
 *      No compression.
 */
#define COMPRESSION_NONE        0
/**
 *  \brief
 *      Use LZ4W compression scheme.
 */
#define COMPRESSION_LZ4W        2

/**
 *  \brief
 *      Set the randomizer seed (to allow reproductible value if we are lucky with HV counter :p)
 */
void setRandomSeed(uint16_t seed);
/**
 *  \brief
 *      Return a random uint16_t integer.
 */
uint16_t random();

/**
 *  \brief
 *      Returns number of Frame Per Second.
 *
 * This function actually returns the number of time it was called in the last second.<br>
 * i.e: for benchmarking you should call this method only once per frame update.
 */
uint32_t getFPS();

/**
 *  \brief
 *      Allocate TileSet structure which can receive unpacked tiles data of the specified TileSet.<br>
 *      If source is not packed the function only allocate space for simple shallow copy of the source.
 *
 *  \param tileset
 *      Source TileSet we want to allocate the unpacked TileSet object.
 *  \return
 *      The new allocated TileSet object which can receive the unpacked TileSet, note that returned tile set
 *      is allocated in a single bloc and can be released with Mem_Free(tb).<br>
 *      <i>NULL</i> is returned if there is not enough memory to store the unpacked tiles.
 *      If the source TileSet is not packed then returned TileSet allocate only memory to do <i>NULL</i> is returned if there is not enough memory to store the unpacked tiles.
 */
TileSet *allocateTileSet(const TileSet *tileset);
/**
 *  \brief
 *      Allocate a new TileSet structure which can receive the data for the specified number of tile.
 *
 *  \param numTile
 *      Number of tile this tileset can contain
 *  \return
 *      The new allocated TileSet object which can receive the specified number of tile.<br>
 *      Note that returned tileset is allocated in a single bloc and can be released with Mem_Free(tileset).<br>
 *      <i>NULL</i> is returned if there is not enough memory to allocatee the tileset.
 */
TileSet *allocateTileSetEx(uint16_t numTile);
/**
 *  \brief
 *      Unpack the specified TileSet structure and return result in a new allocated TileSet.
 *
 *  \param src
 *      tiles to unpack.
 *  \param dest
 *      Destination TileSet structure where to store unpacked data, be sure to allocate enough space in tiles and tilemap buffer.<br>
 *      If set to NULL then a dynamic allocated TileSet is returned.
 *  \return
 *      The unpacked TileSet.<br>
 *      If <i>dest</i> was set to NULL then the returned tiles base is allocated in a single bloc and can be released with Mem_Free(tb).<br>
 *      <i>NULL</i> is returned if there is not enough memory to store the unpacked tiles.
 */
TileSet *unpackTileSet(const TileSet *src, TileSet *dest);

/**
 *  \brief
 *      Unpack the specified source data buffer in the specified destination buffer.<br>
 *      if source is not packed then nothing is done.
 *
 *  \param compression
 *      compression type, accepted values:<br>
 *      <b>COMPRESSION_APLIB</b><br>
 *      <b>COMPRESSION_LZ4W</b><br>
 *  \param src
 *      Source data buffer containing the packed data to unpack.
 *  \param dest
 *      Destination buffer where to store unpacked data, be sure to allocate enough space.
 *  \return
 *      Unpacked size.
 */
uint32_t unpack(uint16_t compression, uint8_t *src, uint8_t *dest);
/**
 *  \brief
 *      Unpack (LZ4W) the specified source data buffer in the specified destination buffer.
 *
 *  \param src
 *      Source data buffer containing the packed data (LZ4W packed) to unpack.
 *  \param dest
 *      Destination buffer where to store unpacked data, be sure to allocate enough space.<br>
 *      The size of unpacked data is contained in the first 4 bytes of 'src'.
 *  \return
 *      Unpacked size.
 */
uint32_t lz4w_unpack(const uint8_t *src, uint8_t *dest);
