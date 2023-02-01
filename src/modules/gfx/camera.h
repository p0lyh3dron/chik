/*
 *    camera.h    --    header for camera functions
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on March 20, 2022
 *
 *    This file is part of the Chik engine.
 *
 *    Included in the Chik engine is the camera, which is used to
 *    render the scene.
 */
#pragma once

#include "libchik.h"

typedef struct {
    vec3_t pos;
    vec2_t ang;
    float fov;
    float near;
    float far;
    float aspect;
} camera_t;

extern camera_t *_camera;

/*
 *    Creates a view matrix for the camera.
 *
 *    @param camera_t *camera    The camera.
 *
 *    @return mat4_t       The view matrix.
 */
mat4_t camera_view(camera_t *camera);

/*
 *    Creates a projection matrix for the camera.
 *
 *    @param camera_t *camera    The camera.
 *
 *    @return mat4_t       The projection matrix.
 */
mat4_t camera_projection(camera_t *camera);