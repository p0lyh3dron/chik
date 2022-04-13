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

#include <string.h>

#include "abstract.h"
#include "rendertarget.h"

extern rendertarget_t *gpBackBuffer;

resource_t *gpVBuffer = NULL;
resource_t *gpGResources = NULL;

/*
 *    Draws vertex color to the fragment.
 *
 *    @param fragment_t *    The fragment to draw to.
 *    @param vec_t *         The color to draw.
 */
void draw_vertex_color( fragment_t *spFrag, vec_t *spColor ) {
    spFrag->aColor.r = spColor->v4.x * 255.0f;
    spFrag->aColor.g = spColor->v4.y * 255.0f;
    spFrag->aColor.b = spColor->v4.z * 255.0f;
    spFrag->aColor.a = spColor->v4.w * 255.0f;
}

v_layout_t gVLayout = {
    .aAttribs = { 
        { V_POS, V_R32G32B32_F,     sizeof( vec3_t ), 0,  NULL               },
        { 0,     V_R32G32B32A32_F,  sizeof( vec4_t ), 12, &draw_vertex_color },
    },
    .aCount   = 2,
};

/*
 *    Draw a scanline.
 *
 *    @param s32                  The initial x coordinate.
 *    @param s32                  The final x coordinate.
 *    @param vec_t *              A pointer to initial interpolated vector data.
 *    @param vec_t *              A pointer to the final interpolated vector data.
 *    @param int                  The starting y position of the scanline.
 */
void draw_scanline( s32 x0, s32 x1, vec_t *v0, vec_t *v1, int y ) {
    /*
     *    Early out if the scanline is outside the render target,
     *    or if the line is a degenerate.
     */
    if ( y < 0 || y >= gpBackBuffer->apTarget->aHeight || ( x0 < 0 && x1 < 0 ) ) {
        return;
    }
    s32 x = MIN( x0, x1 );
        x = MAX( x, 0 );

    s32 endX = MAX( x0, x1 );
        endX = MIN( endX, gpBackBuffer->apTarget->aWidth );

    vec_t v[ MAX_VECTOR_ATTRIBUTES ];
    while ( x < endX ) {
        fragment_t f = {
            .aPos = { x, y },
        };
        /*
         *    Interpolate the vector values, and apply to the fragment.
         */
        for ( u32 i = 0; i < gVLayout.aCount; i++ ) {
            v[ i ] = vec_interp( ( void* )&v0[ i ], ( void* )&v1[ i ], ( f32 )( x - x0 ) / ( endX - x0 ), gVLayout.aAttribs[ i ].aFormat );

            if ( gVLayout.aAttribs[ i ].apFunc ) {
                gVLayout.aAttribs[ i ].apFunc( &f, &v[ i ] );
            }
        }

        /*
         *    Draw the vertex.
         */
        memcpy( gpBackBuffer->apTarget->apData + ( y * gpBackBuffer->apTarget->aWidth + x ), &f.aColor, sizeof( f.aColor ) );

        x++;
    }
}

/*
 *    Draws a triangle.
 *
 *    @param void *     The data for the first point of the triangle.
 *    @param void *     The data for the second point of the triangle.
 *    @param void *     The data for the third point of the triangle.
 */
void draw_triangle( void *pA, void *pB, void *pC ) {
    /*
     *    Extract the data from the vertices.
     */
    u32 offset = 0;
    for ( u32 i = 0; i < gVLayout.aCount; i++ ) {
        if ( gVLayout.aAttribs[ i ].aUsage & V_POS ) {
            break;
        }
        else {
            offset += get_vertex_component_size( gVLayout.aAttribs[ i ].aFormat );
        }
    }

    /*
     *    Extract position data, and rasterize it.
     */
    int y0 = ( *( float* )( pA + offset + sizeof( float ) * 1 ) + 1 ) * gpBackBuffer->apTarget->aHeight / 2;
    int y1 = ( *( float* )( pB + offset + sizeof( float ) * 1 ) + 1 ) * gpBackBuffer->apTarget->aHeight / 2;
    int y2 = ( *( float* )( pC + offset + sizeof( float ) * 1 ) + 1 ) * gpBackBuffer->apTarget->aHeight / 2;

    int x0 = ( *( float* )( pA + offset + sizeof( float ) * 0 ) + 1 ) * gpBackBuffer->apTarget->aWidth / 2;
    int x1 = ( *( float* )( pB + offset + sizeof( float ) * 0 ) + 1 ) * gpBackBuffer->apTarget->aWidth / 2;
    int x2 = ( *( float* )( pC + offset + sizeof( float ) * 0 ) + 1 ) * gpBackBuffer->apTarget->aWidth / 2;


    /*
     *    Sort the vertices by y-coordinate.
     *
     *    y0 at the top, y1 at the middle, y2 at the bottom.
     */
    if ( y0 < y1 ) {
        void *pTemp = pA;
        pA = pB;
        pB = pTemp;

        int temp = y0;
        y0 = y1;
        y1 = temp;

        temp = x0;
        x0 = x1;
        x1 = temp;
    }
    if ( y1 < y2 ) {
        void *pTemp = pB;
        pB = pC;
        pC = pTemp;

        int temp = y1;
        y1 = y2;
        y2 = temp;

        temp = x1;
        x1 = x2;
        x2 = temp;
    }
    if ( y0 < y1 ) {
        void *pTemp = pA;
        pA = pB;
        pB = pTemp;

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
     *    Rasterize the starting y position.
     */
    int y = MAX( y0, 0 );
        y = MIN( y, gpBackBuffer->apTarget->aHeight );

    /*
     *    Calculate the slopes of the lines.
     */
    float dy0 = ( ( float )x1 - x0 ) / ( y1 - y0 );
    float dy1 = ( ( float )x2 - x0 ) / ( y2 - y0 );
    float dy2 = ( ( float )x2 - x1 ) / ( y2 - y1 );

    y2 = MAX( y2, 0 );

    vec_t v0[ MAX_VECTOR_ATTRIBUTES ];
    vec_t v [ MAX_VECTOR_ATTRIBUTES ];

    /*
     *    Check for flat top.
     */
    if ( y0 == y1 ) {
        /*
         *    Sort top two vertices by x-coordinate, 
         *    y0 at the left, y1 at the right.
         */
        if ( x0 < x1 ) {
            void *pTemp = pA;
            pA = pB;
            pB = pTemp;

            int temp = y0;
            y0 = y1;
            y1 = temp;

            temp = x0;
            x0 = x1;
            x1 = temp;
        }
        while ( y >= y2 ) {
            for ( u32 i = 0; i < gVLayout.aCount; ++i ) {
                v0[ i ] = vec_interp( 
                    pB + gVLayout.aAttribs[ i ].aOffset, 
                    pC + gVLayout.aAttribs[ i ].aOffset, 
                    ( f32 )( y0 - y ) / ( y0 - y2 ), 
                    gVLayout.aAttribs[ i ].aFormat
                );
                v[ i ] = vec_interp( 
                    pA + gVLayout.aAttribs[ i ].aOffset, 
                    pC + gVLayout.aAttribs[ i ].aOffset, 
                    ( f32 )( y0 - y ) / ( y0 - y2 ), 
                    gVLayout.aAttribs[ i ].aFormat
                );
            }
            draw_scanline( x1 + ( y - y0 ) * dy2, x0 + ( y - y0 ) * dy1, v0, v, y );
            y--;
        }
        return;
    }

    /*
     *    Check for flat bottom.
     */
    else if ( y1 == y2 ) {
        /*
         *    Sort bottom two vertices by x-coordinate, 
         *    y1 at the left, y2 at the right.
         */
        if ( x1 < x2 ) {
            void *pTemp = pB;
            pB = pC;
            pC = pTemp;

            int temp = y1;
            y1 = y2;
            y2 = temp;

            temp = x1;
            x1 = x2;
            x2 = temp;
        }
        while ( y >= y2 ) {
            for ( u32 i = 0; i < gVLayout.aCount; ++i ) {
                v0[ i ] = vec_interp( 
                    pA + gVLayout.aAttribs[ i ].aOffset, 
                    pC + gVLayout.aAttribs[ i ].aOffset, 
                    ( f32 )( y0 - y ) / ( y0 - y2 ), 
                    gVLayout.aAttribs[ i ].aFormat
                );
                v[ i ] = vec_interp( 
                    pA + gVLayout.aAttribs[ i ].aOffset, 
                    pB + gVLayout.aAttribs[ i ].aOffset, 
                    ( f32 )( y0 - y ) / ( y0 - y2 ), 
                    gVLayout.aAttribs[ i ].aFormat
                );
            }
            draw_scanline( x0 + ( y - y0 ) * dy0, x0 + ( y - y0 ) * dy1, v0, v, y );
            y--;
        }
        return;
    }

    while ( y >= y2 ) {
        /*
         *    Bend is on the left.
         */
        if ( x1 < x2 ) {
            for ( u32 i = 0; i < gVLayout.aCount; ++i ) {
                v[ i ] = vec_interp( 
                    pA + gVLayout.aAttribs[ i ].aOffset, 
                    pC + gVLayout.aAttribs[ i ].aOffset, 
                    ( f32 )( y0 - y ) / ( y0 - y2 ), 
                    gVLayout.aAttribs[ i ].aFormat
                );
            }
            if ( y >= y1 ) {
                for ( u32 i = 0; i < gVLayout.aCount; ++i ) {
                    v0[ i ] = vec_interp( 
                        pA + gVLayout.aAttribs[ i ].aOffset, 
                        pB + gVLayout.aAttribs[ i ].aOffset, 
                        ( f32 )( y0 - y ) / ( y0 - y1 ), 
                        gVLayout.aAttribs[ i ].aFormat
                    );
                }
                draw_scanline( x0 + ( y - y0 ) * dy0, x0 + ( y - y0 ) * dy1, v0, v, y );
            }
            else {
                for ( u32 i = 0; i < gVLayout.aCount; ++i ) {
                    v0[ i ] = vec_interp( 
                        pB + gVLayout.aAttribs[ i ].aOffset, 
                        pC + gVLayout.aAttribs[ i ].aOffset, 
                        ( f32 )( y1 - y ) / ( y1 - y2 ), 
                        gVLayout.aAttribs[ i ].aFormat
                    );
                }
                draw_scanline( x1 + ( y - y1 ) * dy2, x0 + ( y - y0 ) * dy1, v0, v, y );
            }
        }
        /*
         *    Bend is on the right.
         */
        else {
            for ( u32 i = 0; i < gVLayout.aCount; ++i ) {
                v0[ i ] = vec_interp( 
                    pA + gVLayout.aAttribs[ i ].aOffset, 
                    pC + gVLayout.aAttribs[ i ].aOffset, 
                    ( f32 )( y0 - y ) / ( y0 - y2 ), 
                    gVLayout.aAttribs[ i ].aFormat
                );
            }
            if ( y >= y1 ) {
                for ( u32 i = 0; i < gVLayout.aCount; ++i ) {
                    v[ i ] = vec_interp( 
                        pA + gVLayout.aAttribs[ i ].aOffset, 
                        pB + gVLayout.aAttribs[ i ].aOffset, 
                        ( f32 )( y0 - y ) / ( y0 - y1 ), 
                        gVLayout.aAttribs[ i ].aFormat
                    );
                }
                draw_scanline( x0 + ( y - y0 ) * dy1, x0 + ( y - y0 ) * dy0, v0, v, y );
            }
            else {
                for ( u32 i = 0; i < gVLayout.aCount; ++i ) {
                    v[ i ] = vec_interp( 
                        pB + gVLayout.aAttribs[ i ].aOffset, 
                        pC + gVLayout.aAttribs[ i ].aOffset, 
                        ( f32 )( y1 - y ) / ( y1 - y2 ), 
                        gVLayout.aAttribs[ i ].aFormat
                    );
                }
                draw_scanline( x0 + ( y - y0 ) * dy1, x1 + ( y - y1 ) * dy2, v0, v, y );
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

    cam.aNear = 0.1f;
    cam.aFar = 100.0f;

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
        chik_vertex_t a0 = spVertices[ i ];
        chik_vertex_t b0 = spVertices[ i + 1 ];
        chik_vertex_t c0 = spVertices[ i + 2 ];

        mat4_t        ma = m4_mul_v4( view, ( vec4_t ){ -a0.aPos.x, -a0.aPos.y, -a0.aPos.z, 1 } );
        mat4_t        mb = m4_mul_v4( view, ( vec4_t ){ -b0.aPos.x, -b0.aPos.y, -b0.aPos.z, 1 } );
        mat4_t        mc = m4_mul_v4( view, ( vec4_t ){ -c0.aPos.x, -c0.aPos.y, -c0.aPos.z, 1 } );

        int j = 0;

        /*
         *    If the triangle is facing away from the camera, skip it.
         */
        /*if ( ma.v[ j + 8 ] > gpCamera->aNear && mb.v[ j + 8 ] > gpCamera->aNear && mc.v[ j + 8 ] > gpCamera->aNear ) {
            continue;
        }*/

        chik_vertex_t a = a0;
        a.aPos = ( vec3_t ){ ma.v[ j ] / ma.v[ j + 12 ], ma.v[ j + 4 ] / ma.v[ j + 12 ], ma.v[ j + 8 ] / ma.v[ j + 12 ] };
        
        chik_vertex_t b = b0;
        b.aPos = ( vec3_t ){ mb.v[ j ] / mb.v[ j + 12 ], mb.v[ j + 4 ] / mb.v[ j + 12 ], mb.v[ j + 8 ] / mb.v[ j + 12 ] };

        chik_vertex_t c = c0;
        c.aPos = ( vec3_t ){ mc.v[ j ] / mc.v[ j + 12 ], mc.v[ j + 4 ] / mc.v[ j + 12 ], mc.v[ j + 8 ] / mc.v[ j + 12 ] };

        draw_triangle( ( void* )&a, ( void* )&b, ( void* )&c );
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