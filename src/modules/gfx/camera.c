/*
 *    camera.c   --    source for camera functions
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on March 25, 2022
 *
 *    This file is part of the Chik engine.
 *
 *    This file defines the camera functionality.
 */
#include "camera.h"

#include <math.h>

camera_t *_camera = NULL;

/*
 *    Creates a view matrix for the camera.
 *
 *    @param camera_t *camera    The camera.
 *
 *    @return mat4_t       The view matrix.
 */
mat4_t camera_view(camera_t *camera) {
    mat4_t view = m4_identity();

    view = m4_mul_m4(view, camera_projection(camera));
    view =
        m4_mul_m4(view, m4_rotate(camera->ang.x, (vec3_t){1, 0, 0}));
    view =
        m4_mul_m4(view, m4_rotate(camera->ang.y, (vec3_t){0, 1, 0}));
    view = m4_mul_m4(view, m4_translate(camera->pos));

    return view;
}

/*
 *    Creates a projection matrix for the camera.
 *
 *    @param camera_t *camera    The camera.
 *
 *    @return mat4_t       The projection matrix.
 */
mat4_t camera_projection(camera_t *camera) {
    float fov =
        0.5f / tanf(camera->fov * 0.5f * 3.14159265358979323846f / 180.0f);

    /*
     *    We'll be using the Vulkan projection matrix.
     *
     *    This matrix uses reverse z, so we'll hardcode the
     *    farz to 0, and nearz to 1.
     *
     *    Stolen from:
     * https://vincent-p.github.io/posts/vulkan_perspective_matrix/#infinite-perspective
     */
    mat4_t projection = {fov / camera->aspect,
                         0.f,
                         0.f,
                         0.f,
                         0.f,
                         fov,
                         0.f,
                         0.f,
                         0.f,
                         0.f,
                         -1.f,
                         0.f,
                         0.f,
                         0.f,
                         1.f,
                         0.f};

    return projection;
}