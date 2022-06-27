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
#include <string.h>

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
image_t *image_create( u32 sWidth, u32 sHeight, u32 sFormat ) {
    image_t *pImage = malloc( sizeof( image_t ) );
    if( pImage == NULL ) {
        log_error( "Could not allocate memory for image." );
    }

    pImage->aWidth  = sWidth;
    pImage->aHeight = sHeight;
    pImage->aFormat = sFormat;
    /*
     *    In the future, image data may not be just width * height * format.
     *    For now, we just allocate the amount of memory we need.
     */
    pImage->apData  = malloc( sWidth * sHeight * sizeof( u32 ) );
    if( pImage->apData == NULL ) {
        log_error( "Could not allocate memory for image buffer." );
    }

    return pImage;
}

/*
 *    Deduces a file format from a file.
 *
 *    @param const s8 *  The file to deduce the format from.
 * 
 *    @return file_type_e The file format.
 *                        FILE_TYPE_UNKNOWN if the file could not be deduced.
 */
file_type_e file_type( const s8 *spFile ) {
    /*
     *    We'll be lazy and just check the file extension.
     *    In the future, we'll need to check the file header.
     */
    if( strstr( spFile, ".bmp" ) != NULL ) {
        return FILE_TYPE_BMP;
    } else if( strstr( spFile, ".png" ) != NULL ) {
        return FILE_TYPE_PNG;
    } else if( strstr( spFile, ".jpg" ) != NULL ) {
        return FILE_TYPE_JPG;
    } else {
        return FILE_TYPE_UNKNOWN;
    }
}

/*
 *    Loads a bmp image from a file.
 *
 *    @param const s8 *  The file to load the image from.
 *
 *    @return image_t *  The image.
 *                       NULL if the image could not be loaded.
 *                       The image should be freed with image_free().
 */
image_t *image_load_bmp( const s8 *spFile ) {
    u32 len;
    u8 *pBuffer = file_read( spFile, &len );

    if( pBuffer == nullptr ) {
        return nullptr;
    }
    /*
     *    Read the header.
     */
    bmp_header_t header = *( bmp_header_t * )pBuffer;

    header.aWidth  = *( u32 * )( pBuffer + 0x12 );
    header.aHeight = *( u32 * )( pBuffer + 0x16 );

    header.aOffset = *( u32 * )( pBuffer + 0x0A );

    if ( *( u16* )header.aMagic != 0x4D42 ) {
        log_error( "File %s is not a bmp file.", spFile );
        free( pBuffer );
        return NULL;
    }

    /*
     *    Read the image data.
     */
    u8 *pData = ( u8 * )( pBuffer + header.aOffset );

    /*
     *    Create the image.
     */
    image_t *pImage = image_create( header.aWidth, header.aHeight, 32 );

    if( pImage == NULL ) {
        log_error( "Could not create image." );
        free( pBuffer );
        return NULL;
    }

    /*
     *    Copy the data into the image without the padding.
     */
    u32 padding = 0;
    s64 i;
    for ( i = 0; i < header.aHeight; i++ ) {
        /*
         *    Don't touch.
         */
        memcpy( ( u8 * )pImage->apData + ( header.aWidth * i ) * 3, pData + ( header.aWidth * i ) * 3 + i * padding, header.aWidth * 3 );
        /*
         *    Literal magic.
         */
        padding += ( ( header.aWidth * 3 ) + padding ) % 4;
    }

    /*
     *    Free the buffer.
     */
    free( pBuffer );

    return pImage;
}

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
image_t *image_create_from_file( s8 *spPath, u32 sFormat ) {
    file_type_e type = file_type( spPath );
    if( type == FILE_TYPE_UNKNOWN ) {
        log_error( "Could not determine file type of image file." );
        return NULL;
    }
    else if ( type == FILE_TYPE_BMP ) {
        return image_load_bmp( spPath );
    }
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
 *    Clears an image.
 *
 *    @param  image_t *     The image.
 *    @param  u32           The color to clear the image with.
 *
 *    @return u32           1 if the image was cleared, 0 if the image could not be cleared.
 */
u32 image_clear( image_t *spImage, u32 sColor ) {
    if ( spImage == NULL ) {
        return 0;
    }
    
    /*
     *    Fastest way to clear is to memset the entire buffer.
     */
    memset( spImage->apData, sColor, spImage->aWidth * spImage->aHeight * sizeof( u32 ) );

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