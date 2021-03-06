/*
 *    raster.c    --    source for graphics rasterization stage
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on June 9, 2022
 *
 *    This file is part of the Chik Engine.
 * 
 *    The rasterization stage is defined here.
 */
#include "raster.h"

#include "surface.h"
#include "vertexasm.h"

rendertarget_t *gpRasterTarget;

rendertarget_t *gpZBuffer;

/*
 *    Sets up the rasterization stage.
 */
void raster_setup( void ) {
    u32 width;
    u32 height;
    if ( args_has( "-w" ) && args_has( "-h" ) ) {
        width  = args_get_int( "-w" );
        height = args_get_int( "-h" );
    } else {
        width  = DEFAULT_WIDTH;
        height = DEFAULT_HEIGHT;
    }

    gpZBuffer  = rendertarget_create( width, height, 32 );

    if ( !gpZBuffer ) {
        log_fatal( "Could not create Z buffer." );
        return;
    }
}

/*
 *    Sets the rasterization stage's bitmap.
 *
 *    @param    rendertarget_t *    The rendertarget to use for rasterization.
 */
void raster_set_rendertarget( rendertarget_t *spTarget ) {
    gpRasterTarget = spTarget;
}

/*
 *    Clears the depth buffer.
 */
void raster_clear_depth( void ) {
    u64 i;
    f32 *pDepth = ( f32 * )gpZBuffer->apTarget->apData;
    for ( i = 0; i < gpZBuffer->apTarget->aWidth * gpZBuffer->apTarget->aHeight; i++ ) {
        pDepth[ i ] = 1000.f;
    }
}

/*
 *    Check a pixel against the depth buffer.
 *
 *    @param    u32              The x coordinate of the pixel.
 *    @param    u32              The y coordinate of the pixel.
 *    @param    f32              The depth of the pixel.
 * 
 *    @return   u32              Whether the pixel should be drawn.
 */
u32 raster_check_depth( u32 sX, u32 sY, f32 sDepth ) {
    f32 *pDepth = ( f32 * )gpZBuffer->apTarget->apData;
    u32 i       = sY * gpZBuffer->apTarget->aWidth + sX;
    if ( sDepth < pDepth[ i ] ) {
        pDepth[ i ] = sDepth;
        return 1;
    }
    return 0;
}

/*
 *    Draw a scanline.
 *
 *    @param s32           The screen x coordinate of the start of the scanline.
 *    @param s32           The screen x coordinate of the end of the scanline.
 *    @param s32           The screen y coordinate of the scanline.
 *    @param void *        The first vertex of the scanline.
 *    @param void *        The second vertex of the scanline.
 */
void raster_draw_scanline( s32 sX1, s32 sX2, s32 sY, void *spV1, void *spV2 ) {
    /*
     *    Early out if the scanline is outside the render target,
     *    or if the line is a degenerate.
     */
    if ( sY < 0 || sY >= gpRasterTarget->apTarget->aHeight || ( sX1 < 0 && sX2 < 0 ) ) {
        return;
    }

    if ( sX1 > sX2 ) {
        s32 temp     = sX1;
        sX1          = sX2;
        sX2          = temp;

        vec_t *tempv = spV1;
        spV1         = spV2;
        spV2         = tempv;
    }
    
    s32 x     = MAX( sX1, 0 );

    s32 endX  = sX2;

    vec4_t p1 = vertex_get_position( spV1 );
    vec4_t p2 = vertex_get_position( spV2 );

    f32 iz1   = p1.z;
    f32 iz2   = p2.z;

    if ( p1.z == 0.0f || p2.z == 0.0f ) {
        return;
    }

    vec_t v[ MAX_VECTOR_ATTRIBUTES ];

    /*
     *    Make a temporary because there's a coefficient that doesn't
     *    need to be recalculated.
     */
    f32 izs1 = iz1 / ( sX2 - sX1 );
    f32 izs2 = iz2 / ( sX2 - sX1 );

    fragment_t f = {
        .aPos = { x, sY },
    };

    while ( x < endX && x < ( s32 )gpRasterTarget->apTarget->aWidth ) {
        /*
         *    Linearly interpolate between inverted z coordinates, and invert.
         */
        f32 z = 1 / ( ( iz1 - izs1 * ( f32 )( x - sX1 ) ) + izs2 * ( f32 )( x - sX1 ) );
        if ( !raster_check_depth( x, sY, z ) ) {
            x++;
            continue;
        }

        /*
         *    Interpolate the vector values, and apply to the fragment.
         */
        vec_t *pV = vertex_build_interpolated( spV1, spV2, ( f32 )( x - sX1 ) / ( sX2 - sX1 ) );
        pV        = vertex_scale( pV, z, V_POS );

        fragment_apply( pV, &f );

        /*
         *    Draw the vertex.
         */
        memcpy( gpRasterTarget->apTarget->apData + ( sY * gpRasterTarget->apTarget->aWidth + x ), &f.aColor, sizeof( f.aColor ) );

        x++;
    }
}

/*
 *    Rasterizes a single triangle.
 *
 *    @param void *        The raw vertex data for the first vertex.
 *    @param void *        The raw vertex data for the second vertex.
 *    @param void *        The raw vertex data for the third vertex.
 */
void raster_rasterize_triangle( void *spV1, void *spV2, void *spV3 ) {
    /*
     *    Extract position data and rasterize it.
     */
    vec4_t p1 = vertex_get_position( spV1 );
    vec4_t p2 = vertex_get_position( spV2 );
    vec4_t p3 = vertex_get_position( spV3 );

    /*
     *    Map the normalized coordinates to screen coordinates.
     */
    vec2u_t v1 = {
        .x = ( u32 )( ( p1.x + 1.0f ) * gpRasterTarget->apTarget->aWidth  / 2 ),
        .y = ( u32 )( ( p1.y + 1.0f ) * gpRasterTarget->apTarget->aHeight / 2 ),
    };
    vec2u_t v2 = {
        .x = ( u32 )( ( p2.x + 1.0f ) * gpRasterTarget->apTarget->aWidth  / 2 ),
        .y = ( u32 )( ( p2.y + 1.0f ) * gpRasterTarget->apTarget->aHeight / 2 ),
    };
    vec2u_t v3 = {
        .x = ( u32 )( ( p3.x + 1.0f ) * gpRasterTarget->apTarget->aWidth  / 2 ),
        .y = ( u32 )( ( p3.y + 1.0f ) * gpRasterTarget->apTarget->aHeight / 2 ),
    };

    /*
     *    In order to perform perspective correction, we need to know the
     *    z-coordinates of the vertices.
     */
    f32 z1 = p1.z;
    f32 z2 = p2.z;
    f32 z3 = p3.z;

    /*
     *    Sort the vertices by y-coordinate.
     *
     *    y0 at the top, y1 at the middle, y2 at the bottom.
     */
    if ( v1.y < v2.y ) {
        vec2u_t temp = v1;
        v1           = v2;
        v2           = temp;

        f32 tempf    = z1;
        z1           = z2;
        z2           = tempf;

        void *pTemp  = spV1;
        spV1         = spV2;
        spV2         = pTemp;
    }
    if ( v2.y < v3.y ) {
        vec2u_t temp = v2;
        v2           = v3;
        v3           = temp;

        f32 tempf    = z2;
        z2           = z3;
        z3           = tempf;

        void *pTemp  = spV2;
        spV2         = spV3;
        spV3         = pTemp;
    }
    if ( v1.y < v2.y ) {
        vec2u_t temp = v1;
        v1           = v2;
        v2           = temp;

        f32 tempf    = z1;
        z1           = z2;
        z2           = tempf;

        void *pTemp  = spV1;
        spV1         = spV2;
        spV2         = pTemp;
    }

    /*
     *    If the triangle is flat, return.
     */
    if ( v1.y == v3.y ) {
        return;
    }

    /*
     *    Rasterize the starting y position.
     */
    int y = MAX( v1.y, 0 );
        y = MIN( y, gpRasterTarget->apTarget->aHeight );

    /*
     *    Calculate the slopes of the lines.
     */
    float dy0 = ( ( float )v2.x - v1.x ) / ( v2.y - v1.y );
    float dy1 = ( ( float )v3.x - v1.x ) / ( v3.y - v1.y );
    float dy2 = ( ( float )v3.x - v2.x ) / ( v3.y - v2.y );

    u8 v0[ VERTEX_ASM_MAX_VERTEX_SIZE ];
    u8 v [ VERTEX_ASM_MAX_VERTEX_SIZE ];

    u8 pIA[ VERTEX_ASM_MAX_VERTEX_SIZE ];
    u8 pIB[ VERTEX_ASM_MAX_VERTEX_SIZE ];
    u8 pIC[ VERTEX_ASM_MAX_VERTEX_SIZE ];

    /*
     *    Scale vertex attributes by their inverse z-coordinates.
     *    In the drawing process, we will then divide by the inverse
     *    z-coordinate again to perform perspective correction.
     */
    memcpy( pIA, vertex_scale( spV1, 1 / z1, V_POS ), VERTEX_ASM_MAX_VERTEX_SIZE );
    memcpy( pIB, vertex_scale( spV2, 1 / z2, V_POS ), VERTEX_ASM_MAX_VERTEX_SIZE );
    memcpy( pIC, vertex_scale( spV3, 1 / z3, V_POS ), VERTEX_ASM_MAX_VERTEX_SIZE );

    /*
     *    We'll also interpolate the z coordinate by the inverse z-coordinates.
     */
    vec4_t pa = vertex_get_position( pIA );
    vec4_t pb = vertex_get_position( pIB );
    vec4_t pc = vertex_get_position( pIC );

    pa.z = 1 / pa.z;
    pb.z = 1 / pb.z;
    pc.z = 1 / pc.z;

    vertex_set_position( pIA, pa );
    vertex_set_position( pIB, pb );
    vertex_set_position( pIC, pc );

    /*
     *    Check for flat top.
     */
    if ( v1.y == v2.y ) {
        /*
         *    Sort top two vertices by x-coordinate, 
         *    y0 at the left, y1 at the right.
         */
        if ( v1.x < v2.x ) {
            vec2u_t temp = v1;
            v1           = v2;
            v2           = temp;

            f32 tempf    = z1;
            z1           = z2;
            z2           = tempf;

            tempf        = dy1;
            dy1          = dy2;
            dy2          = tempf;

            u8 pTemp[ VERTEX_ASM_MAX_VERTEX_SIZE ];
            memcpy( pTemp, pIA,   VERTEX_ASM_MAX_VERTEX_SIZE );
            memcpy( pIA,   pIB,   VERTEX_ASM_MAX_VERTEX_SIZE );
            memcpy( pIB,   pTemp, VERTEX_ASM_MAX_VERTEX_SIZE );
        }
        while ( y >= v3.y ) {
            memcpy( v0, vertex_build_interpolated( pIB, pIC, ( f32 )( v1.y - y ) / ( v1.y - v3.y ) ), VERTEX_ASM_MAX_VERTEX_SIZE );
            memcpy( v,  vertex_build_interpolated( pIA, pIC, ( f32 )( v1.y - y ) / ( v1.y - v3.y ) ), VERTEX_ASM_MAX_VERTEX_SIZE );
            raster_draw_scanline( v2.x + ( y - v1.y ) * dy2, v1.x + ( y - v1.y ) * dy1, y, v0, v );
            y--;
        }
        return;
    }

    /*
     *    Check for flat bottom.
     */
    else if ( v2.y == v3.y ) {
        /*
         *    Sort bottom two vertices by x-coordinate, 
         *    y1 at the left, y2 at the right.
         */
        if ( v2.x < v3.x ) {
            vec2u_t temp = v2;
            v2           = v3;
            v3           = temp;

            f32 tempf    = z2;
            z2           = z3;
            z3           = tempf;

            tempf        = dy1;
            dy1          = dy0;
            dy0          = tempf;

            u8 pTemp[ VERTEX_ASM_MAX_VERTEX_SIZE ];
            memcpy( pTemp, pIB,   VERTEX_ASM_MAX_VERTEX_SIZE );
            memcpy( pIB,   pIC,   VERTEX_ASM_MAX_VERTEX_SIZE );
            memcpy( pIC,   pTemp, VERTEX_ASM_MAX_VERTEX_SIZE );
        }
        while ( y >= v3.y ) {
            memcpy( v0, vertex_build_interpolated( pIA, pIB, ( f32 )( v1.y - y ) / ( v1.y - v3.y ) ), VERTEX_ASM_MAX_VERTEX_SIZE );
            memcpy( v,  vertex_build_interpolated( pIA, pIC, ( f32 )( v1.y - y ) / ( v1.y - v3.y ) ), VERTEX_ASM_MAX_VERTEX_SIZE );
            raster_draw_scanline( v1.x + ( y - v1.y ) * dy0, v1.x + ( y - v1.y ) * dy1, y, v0, v );
            y--;
        }
        return;
    }

    while ( y >= v3.y ) {
        /*
         *    Bend is on the left.
         */
        if ( v2.x < v3.x ) {
            memcpy( v, vertex_build_interpolated( pIA, pIC, ( f32 )( v1.y - y ) / ( v1.y - v3.y ) ), VERTEX_ASM_MAX_VERTEX_SIZE );
            if ( y >= v2.y ) {
                memcpy( v0, vertex_build_interpolated( pIA, pIB, ( f32 )( v1.y - y ) / ( v1.y - v2.y ) ), VERTEX_ASM_MAX_VERTEX_SIZE );
                raster_draw_scanline( v1.x + ( y - v1.y ) * dy0, v1.x + ( y - v1.y ) * dy1, y, v0, v );
            }
            else {
                memcpy( v0, vertex_build_interpolated( pIB, pIC, ( f32 )( v2.y - y ) / ( v2.y - v3.y ) ), VERTEX_ASM_MAX_VERTEX_SIZE );
                raster_draw_scanline( v2.x + ( y - v2.y ) * dy2, v1.x + ( y - v1.y ) * dy1, y, v0, v );
            }
        }
        /*
         *    Bend is on the right.
         */
        else {
            memcpy( v0, vertex_build_interpolated( pIA, pIC, ( f32 )( v1.y - y ) / ( v1.y - v3.y ) ), VERTEX_ASM_MAX_VERTEX_SIZE );
            if ( y >= v2.y ) {
                memcpy( v, vertex_build_interpolated( pIA, pIB, ( f32 )( v1.y - y ) / ( v1.y - v2.y ) ), VERTEX_ASM_MAX_VERTEX_SIZE );
                raster_draw_scanline( v1.x + ( y - v1.y ) * dy1, v1.x + ( y - v1.y ) * dy0, y, v0, v );
            }
            else {
                memcpy( v, vertex_build_interpolated( pIB, pIC, ( f32 )( v2.y - y ) / ( v2.y - v3.y ) ), VERTEX_ASM_MAX_VERTEX_SIZE );
                raster_draw_scanline( v1.x + ( y - v1.y ) * dy1, v2.x + ( y - v2.y ) * dy2, y, v0, v );
            }
        }
        y--;
    }
}