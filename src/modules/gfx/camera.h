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
    chik_vec3_t aPosition;
    chik_vec3_t aDirection;
    float       aFOV;
} camera_t;