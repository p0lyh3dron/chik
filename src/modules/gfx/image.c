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

#include "gfx.h"

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
image_t *image_create(unsigned int width, unsigned int height, image_fmt_e format) {
    image_t      *image = (image_t *)malloc(sizeof(image_t));

    if (image == NULL) {
        LOGF_ERR("Could not allocate memory for image.");
        return 0;
    }

    image->width  = width;
    image->height = height;
    image->fmt    = format;
    /*
     *    In the future, image data may not be just width * height * format.
     *    For now, we just allocate the amount of memory we need.
     */
    image->buf = (unsigned int *)malloc(width * height * _pixel_sizes[format]);

    if (image->buf == NULL) {
        LOGF_ERR("Could not allocate memory for image buffer.");
        return 0;
    }

    return image;
}

/*
 *    Deduces a file format from a file.
 *
 *    @param const char *file  The file to deduce the format from.
 *
 *    @return file_type_e The file format.
 *                        FILE_TYPE_UNKNOWN if the file could not be deduced.
 */
file_type_e file_type(const char *file) {
    /*
     *    We'll be lazy and just check the file extension.
     *    In the future, we'll need to check the file header.
     */
    if (strstr(file, ".bmp") != NULL) {
        return FILE_TYPE_BMP;
    } else if (strstr(file, ".png") != NULL) {
        return FILE_TYPE_PNG;
    } else if (strstr(file, ".jpg") != NULL) {
        return FILE_TYPE_JPG;
    } else {
        return FILE_TYPE_UNSUPPORTED;
    }
}

/*
 *    Loads a bmp image from a file.
 *
 *    @param const char *file  The file to load the image from.
 *
 *    @return image_t *  The image.
 *                       NULL if the image could not be loaded.
 *                       The image should be freed with image_free().
 */
image_t *image_load_bmp(const char *file) {
    unsigned int   len;
    unsigned int   padding = 0;
    unsigned long  i;
    bmp_header_t   header;
    image_t       *image;
    unsigned char *data;

    unsigned char *buf = file_read(file, &len);

    if (buf == nullptr) {
        VLOGF_ERR("Could not read file %s.\n", file);
        return nullptr;
    }

    /*
     *    Read the header.
     */
    header = *(bmp_header_t *)buf;

    header.width  = *(unsigned int *)(buf + 0x12);
    header.height = *(unsigned int *)(buf + 0x16);

    header.offset = *(unsigned int *)(buf + 0x0A);

    if (*(unsigned short *)header.magic != 0x4D42) {
        VLOGF_ERR("File %s is not a bmp file.", file);
        free(buf);
        return NULL;
    }

    /*
     *    Read the image data.
     */
    data = (unsigned char *)(buf + header.offset);

    /*
     *    Create the image.
     */
    image = image_create(header.width, header.height, IMAGE_FMT_RGBA8);

    if (image == NULL) {
        LOGF_ERR("Could not create image.");
        free(buf);
        return NULL;
    }

    /*
     *    Copy the data into the image without the padding.
     */
    for (i = 0; i < header.height; i++) {
        /*
         *    Don't touch.
         */
        memcpy((unsigned char *)image->buf + (header.width * i) * 4,
               data + (header.width * i) * 4 + i * padding, header.width * 4);
        /*
         *    Literal magic.
         */
        padding += ((header.width * 4) + padding) % 4;
    }

    /*
     *    Free the buffer.
     */
    free(buf);

    return image;
}

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
image_t *image_create_from_file(char *file, unsigned int format) {
    file_type_e type = file_type(file);

    if (type == FILE_TYPE_UNSUPPORTED) {
        LOGF_ERR("Could not determine file type of image file.");
        return NULL;
    } else if (type == FILE_TYPE_BMP) {
        return image_load_bmp(file);
    }
}

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
unsigned int image_set_pixel(image_t *image, unsigned int x, unsigned int y, unsigned int color) {
    if (x >= image->width || y >= image->height) {
        return 0;
    }

    image->buf[y * image->width + x] = color;

    return 1;
}

/*
 *    Clears an image.
 *
 *    @param  image_t *image     The image.
 *    @param  unsigned int color          The color to clear the image with.
 *
 *    @return unsigned int           1 if the image was cleared, 0 if the image could not
 * be cleared.
 */
unsigned int image_clear(image_t *image, unsigned int color) {
    if (image == NULL) {
        LOGF_ERR("Tried to clear a NULL image.");
        return 0;
    }

    /*
     *    Fastest way to clear is to memset the entire buffer.
     */
    memset(image->buf, color, image->width * image->height * _pixel_sizes[image->fmt]);

    return 1;
}

/*
 *    Frees an image.
 *
 *    @param image_t *image     The image to free.
 */
void image_free(image_t *image) {
    if (image == NULL) {
        LOGF_ERR("Tried to free a NULL image.");
        return;
    }

    free(image->buf);
    free(image);
}