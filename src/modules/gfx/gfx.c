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

#include "abstract.h"
#include "rendertarget.h"

extern rendertarget_t *gpBackBuffer;

resource_t *gpVBuffer = NULL;
resource_t *gpGResources = NULL;

/*
 *    Draw a scanline.
 *
 *    @param int       The starting x position of the scanline.
 *    @param int       The ending x position of the scanline.
 *    @param int       The starting y position of the scanline.
 */
void draw_scanline( int x1, int x2, int y ) {
    /*
     *    Early out if the scanline is outside the render target.
     */
    if ( y < 0 || y >= gpBackBuffer->apTarget->aHeight ) {
        return;
    }
    /*
     *    Rasterize the scanline.
     */
    int x    = MIN( x1, x2 );
        x    = MAX( x, 0 );
    int endx = MAX( x1, x2 );
        endx = MIN( endx, ( s32 )gpBackBuffer->apTarget->aWidth );
    while ( x < endx ) {
        /*
         *    Don't draw outside the render target.
         */
        if ( x >= 0 && x < gpBackBuffer->apTarget->aWidth && y >= 0 && y < gpBackBuffer->apTarget->aHeight ) {
            gpBackBuffer->apTarget->apData[ x + y * gpBackBuffer->apTarget->aWidth ] = 0xFFFFFFFF;
        }
        x++;
    }
}

/*
 *    Draws a triangle.
 *
 *    @param vec3_t    The first point of the triangle.
 *    @param vec3_t    The second point of the triangle.
 *    @param vec3_t    The third point of the triangle.
 */
void draw_triangle( vec3_t a, vec3_t b, vec3_t c ) {
    int y0 = ( a.y + 1 ) * gpBackBuffer->apTarget->aHeight / 2;
    int y1 = ( b.y + 1 ) * gpBackBuffer->apTarget->aHeight / 2;
    int y2 = ( c.y + 1 ) * gpBackBuffer->apTarget->aHeight / 2;

    int x0 = ( a.x + 1 ) * gpBackBuffer->apTarget->aWidth / 2;
    int x1 = ( b.x + 1 ) * gpBackBuffer->apTarget->aWidth / 2;
    int x2 = ( c.x + 1 ) * gpBackBuffer->apTarget->aWidth / 2;

    /*
     *    Sort the points by y-coordinate.
     *
     *    y0 at the top, y2 at the bottom.
     */
    if ( y0 < y1 ) {
        int temp = y0;
        y0 = y1;
        y1 = temp;

        temp = x0;
        x0 = x1;
        x1 = temp;
    }
    if ( y1 < y2 ) {
        int temp = y1;
        y1 = y2;
        y2 = temp;

        temp = x1;
        x1 = x2;
        x2 = temp;
    }
    if ( y0 < y1 ) {
        int temp = y0;
        y0 = y1;
        y1 = temp;

        temp = x0;
        x0 = x1;
        x1 = temp;
    }

    /*
     *    If the triangle is flat, return.
     */
    if ( y0 == y2 ) {
        return;
    }

    /*
     *    Calculate the slopes of the lines.
     */
    float dy0 = ( ( float )x1 - x0 ) / ( y1 - y0 );
    float dy1 = ( ( float )x2 - x0 ) / ( y2 - y0 );
    float dy2 = ( ( float )x2 - x1 ) / ( y2 - y1 );
    
    /*
     *    Rasterize the starting y position.
     */
    int y = y0;

    /*
     *    Check for flat top.
     */
    if ( y0 == y1 ) {
        while ( y >= y2 ) {
            draw_scanline( x0 + dy1 * ( y - y0 ), x1 + dy2 * ( y - y0 ), y );
            y--;
        }
        return;
    }

    /*
     *    Check for flat bottom.
     */
    else if ( y1 == y2 ) {
        while ( y >= y2 ) {
            draw_scanline( x0 + dy0 * ( y - y0 ), x0 + dy1 * ( y - y0 ), y );
            y--;
        }
        return;
    }

    while ( y >= y2 ) {
        /*
         *    Bend is on the left.
         */
        if ( x1 < x2 ) {
            if ( y >= y1 ) {
                draw_scanline( x0 + dy0 * ( y - y0 ), x0 + dy1 * ( y - y0 ), y );
            }
            else {
                draw_scanline( x1 + dy2 * ( y - y1 ), x0 + dy1 * ( y - y0 ), y );
            }
        }
        /*
         *    Bend is on the right.
         */
        else {
            if ( y >= y1 ) {
                draw_scanline( x0 + dy1 * ( y - y0 ), x0 + dy0 * ( y - y0 ), y );
            }
            else {
                draw_scanline( x0 + dy1 * ( y - y0 ), x1 + dy2 * ( y - y1 ), y );
            }
        }
        y--;
    }
}

/*
 *    Creates a vertex buffer.
 *
 *    @param chik_vertex_t *    The array of vertices to store in the buffer.
 *    @param u32                The number of vertices in the array.
 * 
 *    @return handle_t          The handle to the vertex buffer.
 */
handle_t create_vertex_buffer( chik_vertex_t *spVertices, u32 sCount ) {
    gpVBuffer       = resource_new( 1024 * 1024 * sizeof( chik_vertex_t ) );
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

    cam.aNear = 0.1f;
    cam.aFar = 100.0f;

    cam.aFOV = 160.0f;

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
 *    Draws a vertex buffer.
 *
 *    @param handle_t          The handle to the vertex buffer.
 */
void draw_vertex_buffer( handle_t sBuffer ) {
    if ( gpBackBuffer == NULL ) {
        log_error( "No render target.\n" );
        return;
    }

    if ( gpCamera == NULL ) {
        log_error( "No camera.\n" );
        return;
    }

    chik_vertex_t *spVertices = resource_get( gpVBuffer, sBuffer );
    if ( spVertices == NULL ) {
        log_error( "Failed to get vertex resource.\n" );
        return;
    }
    mat4_t   view   = camera_view( gpCamera );
    for ( u32 i = 0; i < HANDLE_GET_SIZE( sBuffer ) / sizeof( chik_vertex_t ); i += 3 ) {
        chik_vertex_t a = spVertices[ i ];
        chik_vertex_t b = spVertices[ i + 1 ];
        chik_vertex_t c = spVertices[ i + 2 ];

        mat4_t        ma = m4_mul_v4( view, ( vec4_t ){ a.aPos.x, a.aPos.y, a.aPos.z, 1 } );
        mat4_t        mb = m4_mul_v4( view, ( vec4_t ){ b.aPos.x, b.aPos.y, b.aPos.z, 1 } );
        mat4_t        mc = m4_mul_v4( view, ( vec4_t ){ c.aPos.x, c.aPos.y, c.aPos.z, 1 } );

        int j = 0;

        draw_triangle( ( vec3_t ){ ma.v[ j ] / ma.v[ j + 12 ], ma.v[ j + 4 ] / ma.v[ j + 12 ], ma.v[ j + 8 ] / ma.v[ j + 12 ] },
                       ( vec3_t ){ mb.v[ j ] / mb.v[ j + 12 ], mb.v[ j + 4 ] / mb.v[ j + 12 ], mb.v[ j + 8 ] / mb.v[ j + 12 ] },
                       ( vec3_t ){ mc.v[ j ] / mc.v[ j + 12 ], mc.v[ j + 4 ] / mc.v[ j + 12 ], mc.v[ j + 8 ] / mc.v[ j + 12 ] } );
    }
}

/*
 *    Draws the current frame.
 */
void draw_frame( void ) {
    platform_draw_frame();
}

/*
 *    Creates the graphics context.
 */
__attribute( ( constructor ) ) 
void graphics_init( void ) {
    gpGResources = resource_new( 1024 * 1024 );
    if ( gpGResources == NULL ) {
        log_error( "Failed to create graphics resource.\n" );
        return;
    }
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