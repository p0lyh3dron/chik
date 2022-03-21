/*
 *    image.c    --    source for image functionality
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on March 20, 2022
 * 
 *    This file is part of the Chik engine.
 *
 *    This file declares the functions used for loading and manipulating images.
 */
#include "image.h"

#include <malloc.h>

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
image_t *image_create( u32 width, u32 height, u32 format ) {
    image_t *pImage = malloc( sizeof( image_t ) );
    if( pImage == NULL ) {
        log_error( "Could not allocate memory for image." );
    }

    pImage->aWidth  = width;
    pImage->aHeight = height;
    pImage->aFormat = format;
    /*
     *    In the future, image data may not be just width * height * format.
     *    For now, we just allocate the amount of memory we need.
     */
    pImage->apData  = malloc( width * height * sizeof( u32 ) );
    if( pImage->apData == NULL ) {
        log_error( "Could not allocate memory for image buffer." );
    }

    return pImage;
}

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
u32 image_set_pixel( image_t *image, u32 x, u32 y, u32 color ) {
    if( x >= image->aWidth || y >= image->aHeight ) {
        return 0;
    }

    image->apData[ y * image->aWidth + x ] = color;

    return 1;
}

/*
 *    Frees an image.
 *
 *    @param image_t *     The image to free.
 */
void image_free( image_t *pImage ) {
    if ( pImage == NULL ) {
        log_error( "Tried to free a NULL image." );
        return;
    }
    free( pImage->apData );
    free( pImage );
}