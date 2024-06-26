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
    void* v1;
    void* v2;
    void* assets;
    material_t* material;
} triangle_t;

/*
 *    Sets up the rasterization stage.
 */
void raster_setup(void);

/*
 *    Sets the vertex layout.
 *
 *    @param v_layout_t    The vertex layout.
 */
void raster_set_vertex_layout(v_layout_t layout);

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
 *    @param    unsigned int              The x coordinate of the pixel.
 *    @param    unsigned int              The y coordinate of the pixel.
 *    @param    float              The depth of the pixel.
 *
 *    @return   unsigned int              Whether the pixel should be drawn.
 */
unsigned int raster_check_depth(unsigned int sX, unsigned int sY, float sDepth);

/*
 *    Draw a scanline.
 *
 *    @param int           The screen x coordinate of the start of the scanline.
 *    @param int           The screen x coordinate of the end of the scanline.
 *    @param int           The screen y coordinate of the scanline.
 *    @param void *        The first vertex of the scanline.
 *    @param void *        The second vertex of the scanline.
 */
void raster_draw_scanline(int sX1, int sX2, int sY, void *spV1, void *spV2, void *assets, material_t* mat);

/*
 *    Rasterizes a single triangle.
 *
 *    @param void *        The raw vertex data for the first vertex.
 *    @param void *        The raw vertex data for the second vertex.
 *    @param void *        The raw vertex data for the third vertex.
 */
void raster_rasterize_triangle(void *spV1, void *spV2, void *spV3, void *assets, material_t* mat);

/*
 *    Uses threads to rasterize a triangle.
 *
 *    @param void *     The parameters for the rasterization.
 */
void *raster_rasterize_triangle_thread(void *spParams);

#endif /* CHIK_GFX_RASTER_H  */