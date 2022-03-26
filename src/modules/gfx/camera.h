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
    vec3_t aPosition;
    vec2_t aDirection;
    float       aFOV;
    float       aNear;
    float       aFar;
} camera_t;

/*
 *    Creates a view matrix for the camera.
 *
 *    @param camera_t *    The camera.
 * 
 *    @return mat4_t       The view matrix.
 */
mat4_t camera_view( camera_t *spCamera );

/*
 *    Creates a projection matrix for the camera.
 *
 *    @param camera_t *    The camera.
 *
 *    @return mat4_t       The projection matrix.
 */
mat4_t camera_projection( camera_t *spCamera );