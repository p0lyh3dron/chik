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

extern rendertarget_t *gpRenderTarget;

resource_t *gpVBuffer = NULL;

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
    if ( y < 0 || y >= gpRenderTarget->apTarget->aHeight ) {
        return;
    }
    /*
     *    Rasterize the scanline.
     */
    int x    = MIN( x1, x2 );
        x    = MAX( x, 0 );
    int endx = MAX( x1, x2 );
        endx = MIN( endx, ( s32 )gpRenderTarget->apTarget->aWidth );
    while ( x < endx ) {
        /*
         *    Don't draw outside the render target.
         */
        if ( x >= 0 && x < gpRenderTarget->apTarget->aWidth && y >= 0 && y < gpRenderTarget->apTarget->aHeight ) {
            gpRenderTarget->apTarget->apData[ x + y * gpRenderTarget->apTarget->aWidth ] = 0xFFFFFFFF;
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
    int y0 = ( a.y + 1 ) * gpRenderTarget->apTarget->aHeight / 2;
    int y1 = ( b.y + 1 ) * gpRenderTarget->apTarget->aHeight / 2;
    int y2 = ( c.y + 1 ) * gpRenderTarget->apTarget->aHeight / 2;

    int x0 = ( a.x + 1 ) * gpRenderTarget->apTarget->aWidth / 2;
    int x1 = ( b.x + 1 ) * gpRenderTarget->apTarget->aWidth / 2;
    int x2 = ( c.x + 1 ) * gpRenderTarget->apTarget->aWidth / 2;

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
        return NULL;
    }
    handle_t handle = resource_add( gpVBuffer, spVertices, sCount * sizeof( chik_vertex_t ) );
    if ( handle == NULL ) {
        log_error( "Failed to add vertex resource.\n" );
        resource_delete( gpVBuffer );
        return NULL;
    }
    return handle;
}

static float theta = 0.f;

/*
 *    Draws a vertex buffer.
 *
 *    @param handle_t          The handle to the vertex buffer.
 */
void draw_vertex_buffer( handle_t sBuffer ) {
    if ( gpRenderTarget == NULL ) {
        log_error( "No render target.\n" );
        gpRenderTarget = rendertarget_get_backbuffer();
        return;
    }

    chik_vertex_t *spVertices = resource_get( gpVBuffer, sBuffer );
    if ( spVertices == NULL ) {
        log_error( "Failed to get vertex resource.\n" );
        return;
    }
    theta += .01f;
    log_note( "Theta: %f\n", theta );
    //rendertarget_get_backbuffer()->aCamera.aFOV = 179.f;
    rendertarget_get_backbuffer()->aCamera.aDirection.y = sin( theta * 1.f ) * .4f;
    rendertarget_get_backbuffer()->aCamera.aDirection.x = sin( theta * 1.f ) * .9f;
    //rendertarget_get_backbuffer()->aCamera.aDirection.y = theta;
    camera_t camera = rendertarget_get_backbuffer()->aCamera;
    mat4_t   view   = camera_view( &camera );
    log_note( "View:\n" );
    log_note( "{ %f, %f, %f, %f\n", view.v[ 0 ], view.v[ 1 ], view.v[ 2 ], view.v[ 3 ] );
    log_note( "{ %f, %f, %f, %f\n", view.v[ 4 ], view.v[ 5 ], view.v[ 6 ], view.v[ 7 ] );
    log_note( "{ %f, %f, %f, %f\n", view.v[ 8 ], view.v[ 9 ], view.v[ 10 ], view.v[ 11 ] );
    log_note( "{ %f, %f, %f, %f\n", view.v[ 12 ], view.v[ 13 ], view.v[ 14 ], view.v[ 15 ] );
    log_note( "}\n" );
    for ( u32 i = 0; i < HANDLE_GET_SIZE( sBuffer ) / sizeof( chik_vertex_t ); i += 3 ) {
        chik_vertex_t a = spVertices[ i ];
        chik_vertex_t b = spVertices[ i + 1 ];
        chik_vertex_t c = spVertices[ i + 2 ];

        /*mat4_t        ma = m4_mul_v4( m4_translate( ( vec3_t ){ 0, 0, 0 } ), ( vec4_t ){ a.aPos.x, a.aPos.y, a.aPos.z, 1 } );//m4_mul_m4( view, m4_translate( a.aPos ) );
        mat4_t        mb = m4_mul_v4( m4_identity(), ( vec4_t ){ b.aPos.x, b.aPos.y, b.aPos.z, 1 } );//m4_mul_m4( view, m4_translate( b.aPos ) );
        mat4_t        mc = m4_mul_v4( m4_identity(), ( vec4_t ){ c.aPos.x, c.aPos.y, c.aPos.z, 1 } );//m4_mul_m4( view, m4_translate( c.aPos ) );
*/
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
 *    Cleans up the graphics subsystem.
 */
__attribute__( ( destructor ) )
void cleanup_graphics( void ) {
    if ( gpVBuffer != NULL ) {
        resource_destroy( gpVBuffer );
        gpVBuffer = NULL;
    }
}