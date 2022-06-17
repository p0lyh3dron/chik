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

camera_t *gpCamera = NULL;

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
    view = m4_mul_m4( view, m4_translate( spCamera->aPosition ) );

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
    float fov   = 0.5f / tanf( spCamera->aFOV * 0.5f * 3.14159265358979323846f / 180.0f );

    /*
     *    We'll be using the Vulkan projection matrix.
     *
     *    Whatever the fuck this is, the z-coordinate linearly
     *    interpolates between 0 and near as the camera moves from
     *    far to near.
     * 
     *    Stolen from: https://vincent-p.github.io/posts/vulkan_perspective_matrix/#infinite-perspective
     */
    mat4_t projection = {
        fov / spCamera->aAspect, 0, 0, 0,
        0, -fov, 0, 0,
        0, 0, ( spCamera->aNear ) / ( spCamera->aFar - spCamera->aNear ), ( spCamera->aNear * spCamera->aFar ) / ( spCamera->aFar - spCamera->aNear ),
        0, 0, 1, 0
    };
    /*mat4_t projection = {
        fov / spCamera->aAspect, 0, 0, 0,
        0, -fov, 0, 0,
        0, 0, spCamera->aFar / ( spCamera->aFar - spCamera->aNear ), -spCamera->aFar * spCamera->aNear / ( spCamera->aFar - spCamera->aNear ),
        0, 0, 1, 0
    };*/
    /*mat4_t projection = {
        fov / spCamera->aAspect, 0, 0, 0,
        0, fov, 0, 0,
        0, 0, -spCamera->aFar / ( spCamera->aFar - spCamera->aNear ), -spCamera->aFar * spCamera->aNear / ( spCamera->aFar - spCamera->aNear ),
        0, 0, -1, 0
    };*/
    /*mat4_t projection = {
        fov / spCamera->aAspect, 0, 0, 0,
        0, fov, 0, 0,
        0, 0, ( spCamera->aFar + spCamera->aNear ) / ( spCamera->aNear - spCamera->aFar ), 2 * spCamera->aFar * spCamera->aNear / ( spCamera->aNear - spCamera->aFar ),
        0, 0, -1, 0
    };*/

    return projection;
}