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

CHIK_MODULE( graphics_init )

#include <string.h>

#include "abstract.h"
#include "rendertarget.h"
#include "drawable.h"
#include "raster.h"
#include "vertexasm.h"
#include "cull.h"

extern rendertarget_t *gpBackBuffer;

resource_t *gpVBuffer    = NULL;
resource_t *gpGResources = NULL;

/*
 *    Draws vertex color to the fragment.
 *
 *    @param fragment_t *    The fragment to draw to.
 *    @param vec_t *         The color to draw.
 */
void draw_vertex_color( fragment_t *spFrag, vec_t *spColor, void *spData ) {
    spFrag->aColor.r = spColor->v4.x * 255.0f;
    spFrag->aColor.g = spColor->v4.y * 255.0f;
    spFrag->aColor.b = spColor->v4.z * 255.0f;
    spFrag->aColor.a = spColor->v4.w * 255.0f;
}

/*
 *    Samples a texture at the given UV coordinates.
 *
 *    @param fragment_t *    The fragment to draw to.
 *    @param vec_t *         The UV coordinates to sample.
 */
void sample_texture( fragment_t *spFrag, vec_t *spUV, void *spData ) {
    texture_t *pTex = ( texture_t* )spData;
    image_t   *pImage = pTex->apImage;

    if ( !spData || !pTex || !pImage ) {
        return;
    }
    spUV->v2u.x = fabs( fmod( fabs( spUV->v2.x ), pImage->aWidth - 1 ) );
    spUV->v2u.y = fabs( fmod( fabs( spUV->v2.y ), pImage->aHeight - 1 ) );
    spFrag->aColor.r = ( ( u8* )pImage->apData )[ ( spUV->v2u.x + spUV->v2u.y * pImage->aWidth ) * 3 + 0 ];
    spFrag->aColor.g = ( ( u8* )pImage->apData )[ ( spUV->v2u.x + spUV->v2u.y * pImage->aWidth ) * 3 + 1 ];
    spFrag->aColor.b = ( ( u8* )pImage->apData )[ ( spUV->v2u.x + spUV->v2u.y * pImage->aWidth ) * 3 + 2 ];
    /*spFrag->aColor.r = 255;
    spFrag->aColor.g = 255;
    spFrag->aColor.b = 255;*/
    spFrag->aColor.a = 1;
}

v_layout_t gVLayout = {
    .aAttribs = { 
        /* Usage, Type, Stride, Offset, Fragment callback.  */
        { V_POS, V_R32G32B32A32_F,     sizeof( vec4_t ),  0,        NULL           },
        { 0,     V_R32G32_F,           sizeof( vec2u_t ), 16 + 16, &sample_texture }
    },
    .aCount   = 2,
    .aStride  = sizeof( chik_vertex_t ),
};

/*
 *    Creates a vertex buffer.
 *
 *    @param chik_vertex_t *    The array of vertices to store in the buffer.
 *    @param u32                The number of vertices in the array.
 * 
 *    @return handle_t          The handle to the vertex buffer.
 */
handle_t create_vertex_buffer( chik_vertex_t *spVertices, u32 sCount ) {
    if ( !gpVBuffer )
        gpVBuffer = resource_new( 1024 * 1024 * sizeof( chik_vertex_t ) );
    if ( gpVBuffer == NULL ) {
        log_error( "Failed to create vertex resource.\n" );
        return INVALID_HANDLE;
    }
    handle_t handle = resource_add( gpVBuffer, spVertices, sCount * sizeof( chik_vertex_t ) );
    if ( handle == INVALID_HANDLE ) {
        log_error( "Failed to add vertex resource.\n" );
        resource_destroy( gpVBuffer );
        return INVALID_HANDLE;
    }
    return handle;
}

/*
 *    Creates a camera.
 *
 *    @return handle_t          The handle to the camera.
 */
handle_t create_camera( void ) {
    camera_t cam;
    cam.aPosition.x = 0.0f;
    cam.aPosition.y = 0.0f;
    cam.aPosition.z = 0.0f;

    cam.aDirection.x = 0.0f;
    cam.aDirection.y = 0.0f;

    cam.aNear = 1.0f;
    cam.aFar  = 0.f;

    /*cam.aNear = 0.1f;
    cam.aFar  = 100.0f;*/

    cam.aFOV = 90.0f;

    cam.aAspect = ( float )gpBackBuffer->apTarget->aWidth / ( float )gpBackBuffer->apTarget->aHeight;
    
    handle_t handle = resource_add( gpGResources, &cam, sizeof( camera_t ) );
    if ( handle == INVALID_HANDLE ) {
        log_error( "Failed to add camera resource.\n" );
        return INVALID_HANDLE;
    }
    return handle;
}

/*
 *    Sets camera position.
 *
 *    @param handle_t           The handle to the camera.
 *    @param vec3_t             The position of the camera.
 */
void set_camera_position( handle_t sCamera, vec3_t sPosition ) {
    camera_t *pCamera = resource_get( gpGResources, sCamera );
    if ( pCamera == NULL ) {
        log_error( "Failed to get camera resource.\n" );
        return;
    }
    pCamera->aPosition = sPosition;
}

/*
 *    Sets camera direction.
 *
 *    @param handle_t           The handle to the camera.
 *    @param vec2_t             The direction of the camera.
 */
void set_camera_direction( handle_t sCamera, vec2_t sDirection ) {
    camera_t *pCamera = resource_get( gpGResources, sCamera );
    if ( pCamera == NULL ) {
        log_error( "Failed to get camera resource.\n" );
        return;
    }
    pCamera->aDirection = sDirection;
}

/*
 *    Sets camera FOV.
 *
 *    @param handle_t           The handle to the camera.
 *    @param float              The FOV of the camera.
 */
void set_camera_fov( handle_t sCamera, float sFov ) {
    camera_t *pCamera = resource_get( gpGResources, sCamera );
    if ( pCamera == NULL ) {
        log_error( "Failed to get camera resource.\n" );
        return;
    }
    pCamera->aFOV = sFov;
}

/*
 *    Sets the global camera.
 *
 *    @param handle_t           The handle to the camera.
 */
void set_camera( handle_t sCamera ) {
    gpCamera = resource_get( gpGResources, sCamera );
    if ( gpCamera == NULL ) {
        log_error( "Failed to get camera resource.\n" );
        return;
    }
}

/*
 *    Returns the width and height of the screen.
 *
 *    @return vec2_t             The width and height of the screen.
 */
vec2_t get_screen_size( void ) {
    vec2_t size;
    size.x = gpBackBuffer->apTarget->aWidth;
    size.y = gpBackBuffer->apTarget->aHeight;
    return size;
}

static float theta = 0.f;

/*
 *    Draws the current frame.
 */
void draw_frame( void ) {
    platform_draw_frame();
}

/*
 *    Creates the graphics context.
 */
void graphics_init( void ) {
    gpGResources = resource_new( 1024 * 1024 );
    if ( gpGResources == NULL ) {
        log_error( "Failed to create graphics resource.\n" );
        return;
    }
    cull_create_frustum();
    init_drawable_resources();
    raster_set_rendertarget( gpBackBuffer );
    vertexasm_set_layout( gVLayout );
}

/*
 *    Cleans up the graphics subsystem.
 */
__attribute__( ( destructor ) )
void cleanup_graphics( void ) {
    if ( gpVBuffer != NULL ) {
        resource_destroy( gpVBuffer );
        gpVBuffer = NULL;
    }
}