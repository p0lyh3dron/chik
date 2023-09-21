#include "libchik.h"

#include "presentation.h"

/*
 *    Creates an image.
 *
 *    @param unsigned int width           The width of the image.
 *    @param unsigned int height          The height of the image.
 *    @param unsigned int format          The format of the image.
 *
 *    @return image_t *    The image.
 *                         NULL if the image could not be created.
 *                         The image should be freed with image_free().
 */
image_t *image_create(unsigned int width, unsigned int height, unsigned int format) {
    image_t *image = (image_t *)malloc(sizeof(image_t));

    if (image == NULL) {
        LOGF_ERR("Could not allocate memory for image.");
        return 0;
    }

    image->width  = width;
    image->height = height;
    image->fmt    = format;
    image->size   = width * height * 4;
    /*
     *    In the future, image data may not be just width * height * format.
     *    For now, we just allocate the amount of memory we need.
     */
    image->buf = (unsigned int *)malloc(width * height * sizeof(unsigned int));

    if (image->buf == NULL) {
        LOGF_ERR("Could not allocate memory for image buffer.");
        return 0;
    }

    return image;
}

/*
 *    Loads a ppm image.
 *
 *    @param const char *file  The file to load.
 *
 *    @return image_t *    The image.
 */
image_t *image_load_ppm(const char *file) {
    unsigned long len;

    char *buf = (char *)file_read(file, &len);

    if (buf == NULL) {
        VLOGF_ERR("Could not read file %s.\n", file);
        return (image_t *)0x0;
    }

    if (buf[0] != 'P' || buf[1] != '6') {
        VLOGF_ERR("File %s is not a valid PPM file.\n", file);
        return (image_t *)0x0;
    }

    unsigned int width  = 0;
    unsigned int height = 0;

    unsigned int i = 3;

    while (buf[i] >= '0' && buf[i] <= '9') {
        width *= 10;
        width += buf[i] - '0';
        i++;
    }

    i++;

    while (buf[i] >= '0' && buf[i] <= '9') {
        height *= 10;
        height += buf[i] - '0';
        i++;
    }

    i++;

    unsigned int max = 0;

    while (buf[i] >= '0' && buf[i] <= '9') {
        max *= 10;
        max += buf[i] - '0';
        i++;
    }

    i++;

    image_t *image = image_create(width, height, 32);

    if (image == (image_t *)0x0) {
        LOGF_ERR("Could not create image.\n");
        return (image_t *)0x0;
    }

    unsigned int x = 0;
    unsigned int y = 0;

    while (i < len) {
        unsigned int r = buf[i]     & 0xFF;
        unsigned int g = buf[i + 1] & 0xFF;
        unsigned int b = buf[i + 2] & 0xFF;

        image->buf[x + y * width] = (b << 16) | (g << 8) | (r << 0);

        x++;

        if (x >= width) {
            x = 0;
            y++;
        }

        i += 3;
    }

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
    if (file == (char *)0x0) {
        LOGF_ERR("file is NULL.\n");
        return (image_t *)0x0;
    }

    if (strcmp(file + strlen(file) - 4, ".ppm") == 0) {
        return image_load_ppm(file);
    } else {
        LOGF_ERR("Unsupported image format.\n");
        return (image_t *)0x0;
    }
}

void text_create(void) {
}

mat4_t get_camera_view(trap_t sCamera) {
}

void draw_frame(void) {
    presentation_draw_frame();
}

trap_t create_camera(void) {
}

void set_camera_position(trap_t sCamera, vec3_t sPosition) {
}

void set_camera_direction(trap_t sCamera, vec2_t sDirection) {
}

void set_camera_fov(trap_t sCamera, float sFov) {
}

void set_camera(trap_t sCamera) {
}

vec2_t get_screen_size(void) {
}