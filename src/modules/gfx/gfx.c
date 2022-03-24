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

/*
 *    Draw a scanline.
 *
 *    @param float       The starting x position of the scanline.
 *    @param float       The ending x position of the scanline.
 *    @param float       The starting y position of the scanline.
 */
void draw_scanline( float x1, float x2, float y ) {
    if ( gpRenderTarget == NULL ) {
        gpRenderTarget = rendertarget_get_backbuffer();
        return;
    }
    float x = x1;
    while ( x < x2 ) {
        int xRaster = ( int )( ( x + 1.f ) * gpRenderTarget->apTarget->aWidth / 2 );
        int yRaster = ( int )( ( y + 1.f ) * gpRenderTarget->apTarget->aHeight / 2 );
        gpRenderTarget->apTarget->apData[ yRaster * gpRenderTarget->apTarget->aWidth + xRaster ] = 0xFFFFFFFF;
        x += 2.f / gpRenderTarget->apTarget->aWidth;
    }
}

/*
 *    Draws a triangle.
 *
 *    @param chik_vec2_t    The first point of the triangle.
 *    @param chik_vec2_t    The second point of the triangle.
 *    @param chik_vec2_t    The third point of the triangle.
 */
void draw_triangle( chik_vec2_t a, chik_vec2_t b, chik_vec2_t c ) {
    float y0 = a.y;
    float y1 = b.y;
    float y2 = c.y;

    float x0 = a.x;
    float x1 = b.x;
    float x2 = c.x;

    /*
     *    Sort the points by y-coordinate.
     *
     *    y0 at the top, y2 at the bottom.
     */
    if ( y0 < y1 ) {
        float temp = y0;
        y0 = y1;
        y1 = temp;

        temp = x0;
        x0 = x1;
        x1 = temp;
    }
    if ( y1 < y2 ) {
        float temp = y1;
        y1 = y2;
        y2 = temp;

        temp = x1;
        x1 = x2;
        x2 = temp;
    }
    if ( y0 < y1 ) {
        float temp = y0;
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
    float dy0 = ( x1 - x0 ) / ( y1 - y0 );
    float dy1 = ( x2 - x0 ) / ( y2 - y0 );
    float dy2 = ( x2 - x1 ) / ( y2 - y1 );

    float y = y0;
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
        y -= 2.f / gpRenderTarget->apTarget->aHeight;
    }
}

/*
 *    Draws the current frame.
 */
void draw_frame( void ) {
    platform_draw_frame();
}