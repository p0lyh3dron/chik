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
    char         magic[2];
    unsigned int size;
    unsigned int reserved;
    unsigned int offset;

    unsigned int   header_size;
    unsigned int   width;
    unsigned int   height;
    unsigned short planes;
    unsigned short bpp;
    unsigned int   compression;
    unsigned int   image_size;
    unsigned int   pixels_per_m_x;
    unsigned int   pxiels_per_m_y;
    unsigned int   colors_used;
    unsigned int   colors_important;

    unsigned int red_mask;
    unsigned int green_mask;
    unsigned int blue_mask;
    unsigned int alpha_mask;
} bmp_header_t;

/*
 *    Creates an image.
 *
 *    @param unsigned int width           The width of the image.
 *    @param unsigned int height          The height of the image.
 *    @param image_fmt_e  format          The format of the image.
 *
 *    @return image_t *    The image.
 *                         NULL if the image could not be created.
 *                         The image should be freed with image_free().
 */
image_t *image_create(unsigned int width, unsigned int height, image_fmt_e format);

/*
 *    Deduces a file format from a file.
 *
 *    @param const char *file  The file to deduce the format from.
 *
 *    @return file_type_e The file format.
 *                        FILE_TYPE_UNKNOWN if the file could not be deduced.
 */
file_type_e file_type(const char *file);

/*
 *    Loads a bmp image from a file.
 *
 *    @param const char *file  The file to load the image from.
 *
 *    @return image_t *  The image.
 *                       NULL if the image could not be loaded.
 *                       The image should be freed with image_free().
 */
image_t *image_load_bmp(const char *file);

/*
 *    Creates an image from a file.
 *
 *    @param char *file          The path to the image file.
 *    @param unsigned int format        The format of the image.
 *
 *    @return image_t *  The image.
 *                       NULL if the image could not be created.
 *                       The image should be freed with image_free().
 */
image_t *image_create_from_file(char *file, unsigned int format);

/*
 *    Sets a pixel in an image.
 *
 *    @param image_t *image     The image.
 *    @param unsigned int                The x coordinate of the pixel.
 *    @param unsigned int                The y coordinate of the pixel.
 *    @param unsigned int                The color of the pixel.
 *
 *    @return unsigned int          1 if the pixel was set, 0 if the pixel could not be
 * set.
 */
unsigned int image_set_pixel(image_t *image, unsigned int x, unsigned int y, unsigned int color);

/*
 *    Clears an image.
 *
 *    @param  image_t *image     The image.
 *    @param  unsigned int color          The color to clear the image with.
 *
 *    @return unsigned int           1 if the image was cleared, 0 if the image could not
 * be cleared.
 */
unsigned int image_clear(image_t *image, unsigned int color);

/*
 *    Frees an image.
 *
 *    @param image_t *image     The image to free.
 */
void image_free(image_t *image);