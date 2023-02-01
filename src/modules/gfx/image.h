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
    s8  magic[2];
    u32 size;
    u32 reserved;
    u32 offset;

    u32 header_size;
    u32 width;
    u32 height;
    u16 planes;
    u16 bpp;
    u32 compression;
    u32 image_size;
    u32 pixels_per_m_x;
    u32 pxiels_per_m_y;
    u32 colors_used;
    u32 colors_important;

    u32 red_mask;
    u32 green_mask;
    u32 blue_mask;
    u32 alpha_mask;
} bmp_header_t;

/*
 *    Creates an image.
 *
 *    @param u32 width           The width of the image.
 *    @param u32 height          The height of the image.
 *    @param u32 format          The format of the image.
 *
 *    @return image_t *    The image.
 *                         NULL if the image could not be created.
 *                         The image should be freed with image_free().
 */
image_t *image_create(u32 width, u32 height, u32 format);

/*
 *    Deduces a file format from a file.
 *
 *    @param const s8 *file  The file to deduce the format from.
 *
 *    @return file_type_e The file format.
 *                        FILE_TYPE_UNKNOWN if the file could not be deduced.
 */
file_type_e file_type(const s8 *file);

/*
 *    Loads a bmp image from a file.
 *
 *    @param const s8 *file  The file to load the image from.
 *
 *    @return image_t *  The image.
 *                       NULL if the image could not be loaded.
 *                       The image should be freed with image_free().
 */
image_t *image_load_bmp(const s8 *file);

/*
 *    Creates an image from a file.
 *
 *    @param s8 *file          The path to the image file.
 *    @param u32 format        The format of the image.
 *
 *    @return image_t *  The image.
 *                       NULL if the image could not be created.
 *                       The image should be freed with image_free().
 */
image_t *image_create_from_file(s8 *file, u32 format);

/*
 *    Sets a pixel in an image.
 *
 *    @param image_t *image     The image.
 *    @param u32                The x coordinate of the pixel.
 *    @param u32                The y coordinate of the pixel.
 *    @param u32                The color of the pixel.
 *
 *    @return u32          1 if the pixel was set, 0 if the pixel could not be
 * set.
 */
u32 image_set_pixel(image_t *image, u32 x, u32 y, u32 color);

/*
 *    Clears an image.
 *
 *    @param  image_t *image     The image.
 *    @param  u32 color          The color to clear the image with.
 *
 *    @return u32           1 if the image was cleared, 0 if the image could not
 * be cleared.
 */
u32 image_clear(image_t *image, u32 color);

/*
 *    Frees an image.
 *
 *    @param image_t *image     The image to free.
 */
void image_free(image_t *image);