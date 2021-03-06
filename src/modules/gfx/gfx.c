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

u32 graphics_init( void );
u32 graphics_update( f32 );
u32 graphics_exit( void );

CHIK_MODULE( graphics_init, graphics_update, graphics_exit )

#include <string.h>

#include "rendertarget.h"
#include "drawable.h"
#include "raster.h"
#include "vertexasm.h"
#include "cull.h"

u32     ( *platform_draw_image )     ( image_t * ) = 0;
vec2u_t ( *platform_get_screen_size )( void )      = 0;

extern rendertarget_t *gpBackBuffer;

resource_t *gpVBuffer    = NULL;
resource_t *gpGResources = NULL;

/*
 *    Creates the graphics context.
 */
u32 graphics_init( void ) {
    gpGResources = resource_new( 1024 * 1024 );
    if ( gpGResources == nullptr ) {
        log_error( "u32 graphics_init( void ): Failed to create graphics resource.\n" );
        return 0;
    }
    platform_draw_image = engine_load_function( "platform_draw_image" );
    if ( platform_draw_image == nullptr ) {
        log_error( "u32 graphics_init( void ): Failed to load platform_draw_image.\n" );
        return 0;
    }
    platform_get_screen_size = engine_load_function( "platform_get_screen_size" );
    if ( platform_get_screen_size == nullptr ) {
        log_error( "u32 graphics_init( void ): Failed to load platform_get_screen_size.\n" );
        return 0;
    }

    raster_setup();
    cull_create_frustum();
    init_drawable_resources();
    rendertarget_create_backbuffer();
    raster_set_rendertarget( gpBackBuffer );

    return 1;
}

/*
 *    Updates the graphics context.
 *
 *    @param f32    The time since the last update.
 *
 *    @return u32   The return code.
 */
u32 graphics_update( f32 sDT ) {
    return 1;
}

/*
 *    Cleans up the graphics subsystem.
 */ 
u32 graphics_exit( void ) {
    return 1;
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

    cam.aNear = 0.1f;
    cam.aFar  = 1000.f;

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
 *    Returns the camera's view matrix.
 *
 *    @param handle_t           The handle to the camera.
 * 
 *    @return mat4_t           The view matrix.
 */
mat4_t get_camera_view( handle_t sCamera ) {
    camera_t *pCamera = resource_get( gpGResources, sCamera );
    if ( pCamera == NULL ) {
        log_error( "Failed to get camera resource.\n" );
        return m4_identity();
    }
    return camera_view( pCamera );
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
    platform_draw_image( gpBackBuffer->apTarget );
    image_clear( gpBackBuffer->apTarget, 0xFF000000 );
    raster_clear_depth();
}