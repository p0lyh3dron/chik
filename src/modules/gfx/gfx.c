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

u32 graphics_init(void);
u32 graphics_update(f32);
u32 graphics_exit(void);

CHIK_MODULE(graphics_init, graphics_update, graphics_exit)

#include <string.h>

#include "gfx.h"

#include "cull.h"
#include "drawable.h"
#include "raster.h"
#include "rendertarget.h"
#include "vertexasm.h"

u32 (*platform_draw_image)(image_t *)     = 0;
vec2u_t (*platform_get_screen_size)(void) = 0;

extern rendertarget_t *_back_buffer;

resource_t *_handles;

/*
 *    Creates the graphics context.
 */
u32 graphics_init(void) {
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
 *    @param f32 dt    The time since the last update.
 *
 *    @return u32   The return code.
 */
u32 graphics_update(f32 dt) { return 1; }

/*
 *    Cleans up the graphics subsystem.
 */
u32 graphics_exit(void) { return 1; }

/*
 *    Creates a camera.
 *
 *    @return trap_t          The handle to the camera.
 */
trap_t create_camera(void) {
    camera_t cam;
    trap_t   handle;

    cam.pos.x = 0.0f;
    cam.pos.y = 0.0f;
    cam.pos.z = 0.0f;

    cam.ang.x = 0.0f;
    cam.ang.y = 0.0f;

    cam.near = 0.1f;
    cam.far  = 1000.f;

    cam.fov = 90.0f;

    cam.aspect = (float)_back_buffer->target->width /
                 (float)_back_buffer->target->height;

    handle = resource_add(_handles, &cam, sizeof(camera_t));

    if (BAD_TRAP(handle)) {
        LOGF_ERR("Failed to add camera resource.\n");
        return INVALID_TRAP;
    }

    return handle;
}

/*
 *    Sets camera position.
 *
 *    @param trap_t camera            The handle to the camera.
 *    @param vec3_t pos               The position of the camera.
 */
void set_camera_position(trap_t camera, vec3_t pos) {
    camera_t *cam = resource_get(_handles, camera);

    if (cam == NULL) {
        LOGF_ERR("Failed to get camera resource.\n");
        return;
    }

    cam->pos = pos;
}

/*
 *    Sets camera direction.
 *
 *    @param trap_t camera          The handle to the camera.
 *    @param vec2_t dir             The direction of the camera.
 */
void set_camera_direction(trap_t dir, vec2_t sDirection) {
    camera_t *camera = resource_get(_handles, dir);

    if (camera == NULL) {
        LOGF_ERR("Failed to get camera resource.\n");
        return;
    }

    camera->ang = sDirection;
}

/*
 *    Sets camera FOV.
 *
 *    @param trap_t camera          The handle to the camera.
 *    @param float  fov             The FOV of the camera.
 */
void set_camera_fov(trap_t fov, float sFov) {
    camera_t *cam = resource_get(_handles, fov);

    if (cam == NULL) {
        LOGF_ERR("Failed to get camera resource.\n");
        return;
    }

    cam->fov = sFov;
}

/*
 *    Sets the global camera.
 *
 *    @param trap_t cam         The handle to the camera.
 */
void set_camera(trap_t cam) {
    _camera = resource_get(_handles, cam);

    if (_camera == NULL)
        LOGF_ERR("Failed to get camera resource.\n");
}

/*
 *    Returns the camera's view matrix.
 *
 *    @param trap_t camera     The handle to the camera.
 *
 *    @return mat4_t           The view matrix.
 */
mat4_t get_camera_view(trap_t camera) {
    camera_t *cam = resource_get(_handles, camera);

    if (cam == NULL) {
        LOGF_ERR("Failed to get camera resource.\n");
        return m4_identity();
    }

    return camera_view(cam);
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
    image_clear(_back_buffer->target, 0xFF000000);
    raster_clear_depth();
}