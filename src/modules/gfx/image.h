/*
 *    image.h    --    header for declaring images
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on March 20, 2022
 * 
 *    This file is part of the Chik engine.
 *
 *    Included is functionality for creating images to be drawn to.
 */
#pragma once

#include "libchik.h"

typedef enum {
    FILE_TYPE_UNSUPPORTED = 0,
    FILE_TYPE_BMP,
    FILE_TYPE_PNG,
    FILE_TYPE_JPG,
} file_type_e;

typedef struct {
    s8  aMagic[ 2 ];
    u32 aSize;
    u32 aReserved;
    u32 aOffset;

    u32 aHeaderSize;
    u32 aWidth;
    u32 aHeight;
    u16 aPlanes;
    u16 aBitsPerPixel;
    u32 aCompression;
    u32 aImageSize;
    u32 aXPixelsPerMeter;
    u32 aYPixelsPerMeter;
    u32 aColorsUsed;
    u32 aColorsImportant;

    u32 aRedMask;
    u32 aGreenMask;
    u32 aBlueMask;
    u32 aAlphaMask;
} bmp_header_t;

/*
 *    Creates an image.
 *
 *    @param u32           The width of the image.
 *    @param u32           The height of the image.
 *    @param u32           The format of the image.
 *
 *    @return image_t *    The image.
 *                         NULL if the image could not be created.
 *                         The image should be freed with image_free().
 */
image_t *image_create( u32 sWidth, u32 sHeight, u32 sFormat );

/*
 *    Deduces a file format from a file.
 *
 *    @param const s8 *  The file to deduce the format from.
 * 
 *    @return file_type_e The file format.
 *                        FILE_TYPE_UNKNOWN if the file could not be deduced.
 */
file_type_e file_type( const s8 *spFile );

/*
 *    Loads a bmp image from a file.
 *
 *    @param const s8 *  The file to load the image from.
 *
 *    @return image_t *  The image.
 *                       NULL if the image could not be loaded.
 *                       The image should be freed with image_free().
 */
image_t *image_load_bmp( const s8 *spFile );

/*
 *    Creates an image from a file.
 *
 *    @param s8 *        The path to the image file.
 *    @param u32         The format of the image.
 * 
 *    @return image_t *  The image.
 *                       NULL if the image could not be created.
 *                       The image should be freed with image_free().
 */
image_t *image_create_from_file( s8 *spPath, u32 sFormat );

/*
 *    Sets a pixel in an image.
 *
 *    @param image_t *     The image.
 *    @param u32           The x coordinate of the pixel.
 *    @param u32           The y coordinate of the pixel.
 *    @param u32           The color of the pixel.
 *
 *    @return u32          1 if the pixel was set, 0 if the pixel could not be set.
 */
u32 image_set_pixel( image_t *spImage, u32 sX, u32 sY, u32 sColor );

/*
 *    Clears an image.
 *
 *    @param image_t *     The image.
 *    @param u32           The color to clear the image with.
 *
 *    @return u32          1 if the image was cleared, 0 if the image could not be cleared.
 */
u32 image_clear( image_t *spImage, u32 sColor );

/*
 *    Frees an image.
 *
 *    @param image_t *     The image to free.
 */
void image_free( image_t *pImage );