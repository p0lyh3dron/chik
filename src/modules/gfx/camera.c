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

/*
 *    Creates a view matrix for the camera.
 *
 *    @param camera_t *    The camera.
 * 
 *    @return mat4_t       The view matrix.
 */
mat4_t camera_view( camera_t *spCamera ) {
    mat4_t view = m4_identity();

    view = m4_mul_m4( view, camera_projection( spCamera ) );
    view = m4_mul_m4( view, m4_rotate( spCamera->aDirection.x, ( vec3_t ){ 1, 0, 0 } ) );
    view = m4_mul_m4( view, m4_rotate( spCamera->aDirection.y, ( vec3_t ){ 0, 1, 0 } ) );
    view = m4_mul_m4( view, m4_translate( ( vec3_t ){ -spCamera->aPosition.x, -spCamera->aPosition.y, -spCamera->aPosition.z } ) );

    return view;
}

/*
 *    Creates a projection matrix for the camera.
 *
 *    @param camera_t *    The camera.
 *
 *    @return mat4_t       The projection matrix.
 */
mat4_t camera_projection( camera_t *spCamera ) {
    float scale = 1.0f / tan( spCamera->aFOV * 0.5f * 3.14159265358979323846f / 180.0f );

    mat4_t projection = {
        scale, 0, 0, 0,
        0, scale, 0, 0,
        0, 0, -spCamera->aFar / ( spCamera->aFar - spCamera->aNear ), -1,
        0, 0, -( spCamera->aFar * spCamera->aNear ) / ( ( spCamera->aFar - spCamera->aNear ) ), 0
    };

    return projection;
}