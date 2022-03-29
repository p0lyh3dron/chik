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
 *    Calculate the color differential.
 *
 *    @param color32_t *    The first color.
 *    @param color32_t *    The second color.
 * 
 *    @param f32            The divisor.
 * 
 *    @return vec4_t        The incremental color.
 */
vec4_t color_diff( color32_t *spColor1, color32_t *spColor2, f32 sDivisor ) {
    vec4_t sColor;
    
    sColor.x = ( spColor1->r - spColor2->r ) / sDivisor;
    sColor.y = ( spColor1->g - spColor2->g ) / sDivisor;
    sColor.z = ( spColor1->b - spColor2->b ) / sDivisor;
    sColor.w = ( spColor1->a - spColor2->a ) / sDivisor;
    
    return sColor;
}

/*
 *    Calculate an arbitrary color between two colors.
 *
 *    @param color32_t *    The final color.
 *    @param f32            The variable.
 *    @param vec4_t *    The differential.
 * 
 *    @return color32_t     The calculated color.
 */
color32_t color_lerp( color32_t *spColor, f32 sVariable, vec4_t *spColorDiff ) {
    color32_t sColor;
    
    sColor.r = spColor->r + ( spColorDiff->x ) * ( spColor->r - sVariable );
    sColor.g = spColor->g + ( spColorDiff->y ) * ( spColor->g - sVariable );
    sColor.b = spColor->b + ( spColorDiff->z ) * ( spColor->b - sVariable );
    sColor.a = spColor->a + ( spColorDiff->w ) * ( spColor->a - sVariable );
    
    return sColor;
}

/*
 *    Draw a scanline.
 *
 *    @param chik_vertex_t       The starting x position of the scanline.
 *    @param chik_vertex_t       The ending x position of the scanline.
 *    @param int                 The starting y position of the scanline.
 */
void draw_scanline( chik_vertex_t x1, chik_vertex_t x2, int y ) {
    /*
     *    Early out if the scanline is outside the render target,
     *    or if the line is a degenerate.
     */
    if ( y < 0 || y >= gpBackBuffer->apTarget->aHeight || x1.aRastPos.x == x2.aRastPos.x ) {
        return;
    }
    /*
     *    Rasterize the scanline.
     */
    chik_vertex_t x = x1;
                  x.aRastPos.x = MIN( x1.aRastPos.x, x2.aRastPos.x );
                  x.aRastPos.x = MAX( x.aRastPos.x, 0 );

    chik_vertex_t xEnd = x2;
                  xEnd.aRastPos.x = MAX( x1.aRastPos.x, x2.aRastPos.x );
                  xEnd.aRastPos.x = MIN( xEnd.aRastPos.x, ( s32 )gpBackBuffer->apTarget->aWidth );

    /*
     *    Interpolate color.
     */
    float dr = ( float )( xEnd.aColor.r - x.aColor.r ) / ( xEnd.aRastPos.x - x.aRastPos.x );
    float dg = ( float )( xEnd.aColor.g - x.aColor.g ) / ( xEnd.aRastPos.x - x.aRastPos.x );
    float db = ( float )( xEnd.aColor.b - x.aColor.b ) / ( xEnd.aRastPos.x - x.aRastPos.x );
    float da = ( float )( xEnd.aColor.a - x.aColor.a ) / ( xEnd.aRastPos.x - x.aRastPos.x );

    color32_t color = { 0 };

    while ( x.aRastPos.x < xEnd.aRastPos.x ) {
        /*
         *    Don't draw outside the render target.
         */
        if ( x.aRastPos.x >= 0 && x.aRastPos.x < gpBackBuffer->apTarget->aWidth && y >= 0 && y < gpBackBuffer->apTarget->aHeight ) {
            /*
             *    Color the pixel.
             */
            color.r = ( xEnd.aRastPos.x - x.aRastPos.x ) * dr + xEnd.aColor.r;
            color.g = ( xEnd.aRastPos.x - x.aRastPos.x ) * dg + xEnd.aColor.g;
            color.b = ( xEnd.aRastPos.x - x.aRastPos.x ) * db + xEnd.aColor.b;
            color.a = ( xEnd.aRastPos.x - x.aRastPos.x ) * da + xEnd.aColor.a;

            gpBackBuffer->apTarget->apData[ x.aRastPos.x + y * gpBackBuffer->apTarget->aWidth     ] = *( u32* )&color;
        }
        x.aRastPos.x++;
    }
}

/*
 *    Draws a triangle.
 *
 *    @param vertex_t    The first point of the triangle.
 *    @param vertex_t    The second point of the triangle.
 *    @param vertex_t    The third point of the triangle.
 */
void draw_triangle( chik_vertex_t a, chik_vertex_t b, chik_vertex_t c ) {
    /*
     *    Sort the vertices by y-coordinate.
     *
     *    a at the top, b at the middle, c at the bottom.
     */
    if ( a.aPos.y < b.aPos.y ) {
        chik_vertex_t temp = a;
        a = b;
        b = temp;
    }
    if ( b.aPos.y < c.aPos.y ) {
        chik_vertex_t temp = b;
        b = c;
        c = temp;
    }
    if ( a.aPos.y < b.aPos.y ) {
        chik_vertex_t temp = a;
        a = b;
        b = temp;
    }

    int y0 = ( a.aPos.y + 1 ) * gpBackBuffer->apTarget->aHeight / 2;
    int y1 = ( b.aPos.y + 1 ) * gpBackBuffer->apTarget->aHeight / 2;
    int y2 = ( c.aPos.y + 1 ) * gpBackBuffer->apTarget->aHeight / 2;

    int x0 = ( a.aPos.x + 1 ) * gpBackBuffer->apTarget->aWidth / 2;
    int x1 = ( b.aPos.x + 1 ) * gpBackBuffer->apTarget->aWidth / 2;
    int x2 = ( c.aPos.x + 1 ) * gpBackBuffer->apTarget->aWidth / 2;

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
     *    Calculate the slopes of the colors.
     */
    vec4_t dcolor0 = color_diff( &a.aColor, &b.aColor, y1 - y0 );
    vec4_t dcolor1 = color_diff( &a.aColor, &c.aColor, y2 - y0 );
    vec4_t dcolor2 = color_diff( &b.aColor, &c.aColor, y2 - y1 );
    
    /*
     *    Rasterize the starting y position.
     */
    int y = MAX( y0, 0 );
        y = MIN( y, gpBackBuffer->apTarget->aHeight );

    y2    = MIN( y2, gpBackBuffer->apTarget->aHeight );
    y1    = MIN( y1, gpBackBuffer->apTarget->aHeight );

    /*
     *    Check for flat top.
     */
    if ( y0 == y1 ) {
        while ( y >= y2 ) {
            chik_vertex_t v0 = a;
            v0.aRastPos.x = x0 + ( y - y0 ) * dy1;
            v0.aColor     = color_lerp( &b.aColor, y, &dcolor1 );
            chik_vertex_t v  = b;
            v.aRastPos.x  = x1 + ( y - y0 ) * dy2;
            v.aColor      = color_lerp( &c.aColor, y, &dcolor2 );

            draw_scanline( v0, v, y );
            y--;
        }
        return;
    }

    /*
     *    Check for flat bottom.
     */
    else if ( y1 == y2 ) {
        while ( y >= y2 ) {
            chik_vertex_t v0 = a;
            v0.aRastPos.x = x0 + ( y - y0 ) * dy0;
            v0.aColor     = color_lerp( &b.aColor, y, &dcolor0 );
            chik_vertex_t v  = b;
            v.aRastPos.x  = x0 + ( y - y0 ) * dy1;
            v.aColor      = color_lerp( &c.aColor, y, &dcolor1 );

            draw_scanline( v0, v, y );
            y--;
        }
        return;
    }

    while ( y >= y2 ) {
        /*
         *    Bend is on the left.
         */
        if ( x1 < x2 ) {
            chik_vertex_t v0 = a;
            chik_vertex_t v  = b;
            v.aRastPos.x  = x0 + ( y - y0 ) * dy1;
            v.aColor      = color_lerp( &c.aColor, y, &dcolor1 );
            if ( y >= y1 ) {
                v0.aRastPos.x  = x0 + ( y - y0 ) * dy0;
                v0.aColor      = color_lerp( &b.aColor, y, &dcolor0 );
                draw_scanline( v0, v, y );
            }
            else {
                v0.aRastPos.x = x1 + ( y - y1 ) * dy2;
                v0.aColor     = color_lerp( &c.aColor, y, &dcolor2 );
                draw_scanline( v0, v, y );
            }
        }
        /*
         *    Bend is on the right.
         */
        else {
            chik_vertex_t v0 = a;
            v0.aRastPos.x = x0 + ( y - y0 ) * dy1;
            v0.aColor     = color_lerp( &b.aColor, y, &dcolor1 );
            chik_vertex_t v  = b;
            if ( y >= y1 ) {
                v.aRastPos.x = x0 + ( y - y0 ) * dy0;
                v.aColor     = color_lerp( &b.aColor, y, &dcolor0 );
                draw_scanline( v0, v, y );
            }
            else {
                v.aRastPos.x = x1 + ( y - y1 ) * dy2;
                v.aColor     = color_lerp( &c.aColor, y, &dcolor2 );
                draw_scanline( v0, v, y );
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
        if ( ma.v[ j + 8 ] > gpCamera->aNear && mb.v[ j + 8 ] > gpCamera->aNear && mc.v[ j + 8 ] > gpCamera->aNear ) {
            continue;
        }

        chik_vertex_t a = a0;
        a.aPos = ( vec3_t ){ ma.v[ j ] / ma.v[ j + 12 ], ma.v[ j + 4 ] / ma.v[ j + 12 ], ma.v[ j + 8 ] / ma.v[ j + 12 ] };
        
        chik_vertex_t b = b0;
        b.aPos = ( vec3_t ){ mb.v[ j ] / mb.v[ j + 12 ], mb.v[ j + 4 ] / mb.v[ j + 12 ], mb.v[ j + 8 ] / mb.v[ j + 12 ] };

        chik_vertex_t c = c0;
        c.aPos = ( vec3_t ){ mc.v[ j ] / mc.v[ j + 12 ], mc.v[ j + 4 ] / mc.v[ j + 12 ], mc.v[ j + 8 ] / mc.v[ j + 12 ] };

        draw_triangle( a, b, c );
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