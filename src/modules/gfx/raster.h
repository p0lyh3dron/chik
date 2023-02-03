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

typedef struct {
    void *v0;
    void *v1;
    void *v2;
    void *assets;
} triangle_t;

/*
 *    Sets up the rasterization stage.
 */
void raster_setup(void);

/*
 *    Sets the rasterization stage's bitmap.
 *
 *    @param    rendertarget_t *    The rendertarget to use for rasterization.
 */
void raster_set_rendertarget(rendertarget_t *spTarget);

/*
 *    Clears the depth buffer.
 */
void raster_clear_depth(void);

/*
 *    Check a pixel against the depth buffer.
 *
 *    @param    u32              The x coordinate of the pixel.
 *    @param    u32              The y coordinate of the pixel.
 *    @param    f32              The depth of the pixel.
 *
 *    @return   u32              Whether the pixel should be drawn.
 */
u32 raster_check_depth(u32 sX, u32 sY, f32 sDepth);

/*
 *    Draw a scanline.
 *
 *    @param s32           The screen x coordinate of the start of the scanline.
 *    @param s32           The screen x coordinate of the end of the scanline.
 *    @param s32           The screen y coordinate of the scanline.
 *    @param void *        The first vertex of the scanline.
 *    @param void *        The second vertex of the scanline.
 */
void raster_draw_scanline(s32 sX1, s32 sX2, s32 sY, void *spV1, void *spV2, void *assets);

/*
 *    Rasterizes a single triangle.
 *
 *    @param void *        The raw vertex data for the first vertex.
 *    @param void *        The raw vertex data for the second vertex.
 *    @param void *        The raw vertex data for the third vertex.
 */
void raster_rasterize_triangle(void *spV1, void *spV2, void *spV3, void *assets);

/*
 *    Uses threads to rasterize a triangle.
 *
 *    @param void *     The parameters for the rasterization.
 */
void *raster_rasterize_triangle_thread(void *spParams);

#endif /* CHIK_GFX_RASTER_H  */