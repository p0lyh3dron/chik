/*
 *    gfx.c    --    source file for the main graphics functions.
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on March 20, 2022
 *
 *    This file is part of the Chik Engine.
 *
 *    This file defines the main graphics functions. These functions
 *    don't require a header file, as they will compiled into the dll
 *    and exported to the game.
 */
#include "libchik.h"

unsigned int graphics_init(void);
unsigned int graphics_update(float);
unsigned int graphics_exit(void);

CHIK_MODULE(graphics_init, graphics_update, graphics_exit)

#include <string.h>

#include "gfx.h"

#include "cull.h"
#include "drawable.h"
#include "raster.h"
#include "rendertarget.h"
#include "vertexasm.h"

unsigned int (*platform_draw_image)(image_t *) = 0;
vec2u_t (*platform_get_screen_size)(void)      = 0;

extern rendertarget_t *_back_buffer;

resource_t *_handles;

/*
 *    Creates the graphics context.
 */
unsigned int graphics_init(void) {
    _handles                 = resource_new(64 * 1024 * 1024);
    platform_draw_image      = engine_load_function("platform_draw_image");
    platform_get_screen_size = engine_load_function("platform_get_screen_size");

    if (_handles == nullptr) {
        LOGF_ERR("Failed to create graphics resource.\n");
        return 0;
    }

    if (platform_draw_image == nullptr) {
        LOGF_ERR("Failed to load platform_draw_image.\n");
        return 0;
    }

    if (platform_get_screen_size == nullptr) {
        LOGF_ERR("Failed to load "
                 "platform_get_screen_size.\n");
        return 0;
    }

    raster_setup();
    cull_create_frustum();
    rendertarget_create_backbuffer();
    raster_set_rendertarget(_back_buffer);

    return 1;
}

/*
 *    Updates the graphics context.
 *
 *    @param float dt    The time since the last update.
 *
 *    @return unsigned int   The return code.
 */
unsigned int graphics_update(float dt) { return 1; }

/*
 *    Cleans up the graphics subsystem.
 */
unsigned int graphics_exit(void) { return 1; }

/*
 *    Creates a camera.
 *
 *    @return void *         The camera pointer.
 */
void *create_camera(void) {
    camera_t *cam = malloc(sizeof(camera_t));

    if (cam == (camera_t*)0x0) {
        LOGF_ERR("Failed to allocate camera.\n");

        return (void*)0x0;
    }

    cam->pos.x = 0.0f;
    cam->pos.y = 0.0f;
    cam->pos.z = 0.0f;

    cam->ang.x = 0.0f;
    cam->ang.y = 0.0f;

    cam->near  = 0.1f;
    cam->far   = 1000.f;

    cam->fov   = 90.0f;

    cam->aspect = (float)_back_buffer->target->width /
                  (float)_back_buffer->target->height;

    return cam;
}

/*
 *    Sets camera position.
 *
 *    @param void  *camera            The handle to the camera.
 *    @param vec3_t pos               The position of the camera.
 */
void set_camera_position(void *camera, vec3_t pos) {
    if (camera == (void*)0x0) {
        LOGF_ERR("Failed to get camera resource.\n");

        return;
    }

    camera_t *cam = (camera_t*)camera;

    cam->pos = pos;
}

/*
 *    Sets camera direction.
 *
 *    @param void  *camera          The handle to the camera.
 *    @param vec2_t dir             The direction of the camera.
 */
void set_camera_direction(void *camera, vec2_t dir) {
    if (camera == (void*)0x0) {
        LOGF_ERR("Failed to get camera resource.\n");

        return;
    }

    camera_t *cam = (camera_t*)camera;

    cam->ang = dir;
}

/*
 *    Sets camera FOV.
 *
 *    @param void  *camera          The handle to the camera.
 *    @param float  fov             The FOV of the camera.
 */
void set_camera_fov(void *camera, float fov) {
    if (camera == (void*)0x0) {
        LOGF_ERR("Failed to get camera resource.\n");

        return;
    }

    camera_t *cam = (camera_t*)camera;

    cam->fov = fov;
}

/*
 *    Sets the global camera.
 *
 *    @param void *cam         The handle to the camera.
 */
void set_camera(void *cam) {
    _camera = cam;
}

/*
 *    Returns the camera's view matrix.
 *
 *    @param void *cam         The handle to the camera.
 *
 *    @return mat4_t           The view matrix.
 */
mat4_t get_camera_view(void *cam) {
    if (cam == (void*)0x0) {
        LOGF_ERR("Failed to get camera resource.\n");

        return m4_identity();
    }

    camera_t *camera = (camera_t*)cam;

    return camera_view(cam);
}

/*
 *    Begins a new render group.
 */
void begin_render_group(void) {
    raster_clear_depth();
}

/*
 *    Returns the width and height of the screen.
 *
 *    @return vec2_t             The width and height of the screen.
 */
vec2_t get_screen_size(void) {
    vec2_t size;

    size.x = _back_buffer->target->width;
    size.y = _back_buffer->target->height;

    return size;
}

/*
 *    Draws the current frame.
 */
void draw_frame(void) {
    platform_draw_image(_back_buffer->target);
    image_clear(_back_buffer->target, 0xFF202020);
    raster_clear_depth();
}