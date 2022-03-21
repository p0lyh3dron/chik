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

typedef struct {
    u32          aWidth;
    u32          aHeight;
    u32          aFormat;
    u32          aDataLen;
    u8          *apData;
} image_t;

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
image_t *image_create( u32 width, u32 height, u32 format );

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
u32 image_set_pixel( image_t *image, u32 x, u32 y, u32 color );

/*
 *    Frees an image.
 *
 *    @param image_t *     The image to free.
 */
void image_free( image_t *pImage );