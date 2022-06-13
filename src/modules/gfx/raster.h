/*
 *    raster.h    --    header for graphics rasterization stage
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on June 9, 2022
 *
 *    This file is part of the Chik Engine.
 * 
 *    The rasterization stage is responsible for rasterizing the
 *    geometry into a 2D bitmap. Currently, the rasterization stage
 *    clips the geometry to the viewing frustum, and then rasterizes
 *    the geometry into a bitmap by drawing scanlines. The clipping
 *    algorithm should probably be put into another stage/file,
 *    but it is currently implemented here.
 */
#ifndef CHIK_GFX_RASTER_H
#define CHIK_GFX_RASTER_H

#include "libchik.h"

#include "rendertarget.h"

/*
 *    Sets the rasterization stage's bitmap.
 *
 *    @param    rendertarget_t *    The rendertarget to use for rasterization.
 */
void raster_set_rendertarget( rendertarget_t *spTarget );

/*
 *    Draw a scanline.
 *
 *    @param s32           The screen x coordinate of the start of the scanline.
 *    @param s32           The screen x coordinate of the end of the scanline.
 *    @param s32           The screen y coordinate of the scanline.
 *    @param void *        The first vertex of the scanline.
 *    @param void *        The second vertex of the scanline.
 */
void raster_draw_scanline( s32 sX1, s32 sX2, s32 sY, void *spV1, void *spV2 );

/*
 *    Rasterizes a single triangle.
 *
 *    @param void *        The raw vertex data for the first vertex.
 *    @param void *        The raw vertex data for the second vertex.
 *    @param void *        The raw vertex data for the third vertex.
 */
void raster_rasterize_triangle( void *spV1, void *spV2, void *spV3 );

#endif /* CHIK_GFX_RASTER_H  */