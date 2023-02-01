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

#include "vertexasm.h"

rendertarget_t *_raster_target;

rendertarget_t *_z_buffer;

/*
 *    Sets up the rasterization stage.
 */
void raster_setup(void) {
    u32 width;
    u32 height;

    if (args_has("-w") && args_has("-h")) {
        width = args_get_int("-w");
        height = args_get_int("-h");
    } else {
        width = 1152;
        height = 764;
    }

    _z_buffer = rendertarget_create(width, height, 32);

    if (!_z_buffer) {
        LOGF_FAT("Could not create Z buffer.");
        return;
    }
}

/*
 *    Sets the rasterization stage's bitmap.
 *
 *    @param    rendertarget_t *target    The rendertarget to use for rasterization.
 */
void raster_set_rendertarget(rendertarget_t *target) {
    _raster_target = target;
}

/*
 *    Clears the depth buffer.
 */
void raster_clear_depth(void) {
    u64 i;
    f32 *pDepth = (f32 *)_z_buffer->target->buf;

    for (i = 0; i < _z_buffer->target->width * _z_buffer->target->height;
         i++) {
        pDepth[i] = 1000.f;
    }
}

/*
 *    Check a pixel against the depth buffer.
 *
 *    @param    u32 x              The x coordinate of the pixel.
 *    @param    u32 y              The y coordinate of the pixel.
 *    @param    f32 d              The depth of the pixel.
 *
 *    @return   u32              Whether the pixel should be drawn.
 */
u32 raster_check_depth(u32 x, u32 y, f32 d) {
    f32 *pDepth = (f32 *)_z_buffer->target->buf;
    u32 i = y * _z_buffer->target->width + x;

    if (d < pDepth[i]) {
        pDepth[i] = d;
        return 1;
    }

    return 0;
}

/*
 *    Draw a scanline.
 *
 *    @param s32 x1          The screen x coordinate of the start of the scanline.
 *    @param s32 x2          The screen x coordinate of the end of the scanline.
 *    @param s32 y           The screen y coordinate of the scanline.
 *    @param void *v1        The first vertex of the scanline.
 *    @param void *v2        The second vertex of the scanline.
 */
void raster_draw_scanline(s32 x1, s32 x2, s32 y, void *v1, void *v2) {
    s32 x;
    s32 end_x;
    s32 temp;
    f32 z;
    f32 iz1;
    f32 iz2;
    vec_t *tempv;
    vec4_t p1;
    vec4_t p2;
    vec_t v[MAX_VECTOR_ATTRIBUTES];
    vec_t *new_v;
    fragment_t f;

    /*
     *    Early out if the scanline is outside the render target,
     *    or if the line is a degenerate.
     */
    if (y < 0 || y >= _raster_target->target->height ||
        (x1 < 0 && x2 < 0)) {
        return;
    }

    if (x1 > x2) {
        temp = x1;
        x1 = x2;
        x2 = temp;

        tempv = v1;
        v1 = v2;
        v2 = tempv;
    }

    x = MAX(x1, 0);
    end_x = x2;

    p1 = vertex_get_position(v1);
    p2 = vertex_get_position(v2);

    if (p1.z == 0.0f || p2.z == 0.0f) {
        return;
    }

    /*
     *    Make a temporary because there's a coefficient that doesn't
     *    need to be recalculated.
     */
    iz1 = p1.z / (x2 - x1);
    iz2 = p2.z / (x2 - x1);

    f.pos.x = x;
    f.pos.y = y;

    while (x < end_x && x < (s32)_raster_target->target->width) {
        /*
         *    Linearly interpolate between inverted z coordinates, and invert.
         */
        z = 1 / ((p1.z - iz1 * (f32)(x - x1)) + iz2 * (f32)(x - x1));
        if (!raster_check_depth(x, y, z)) {
            x++;
            continue;
        }

        /*
         *    Interpolate the vector values, and apply to the fragment.
         */
        new_v = vertex_build_interpolated(v1, v2, (f32)(x - x1) / (x2 - x1));
        new_v = vertex_scale(new_v, z, V_POS);

        fragment_apply(new_v, &f);

        /*
         *    Draw the vertex.
         */
        memcpy(_raster_target->target->buf +
                   (y * _raster_target->target->width + x),
               &f.color, sizeof(f.color));

        x++;
    }
}

/*
 *    Rasterizes a single triangle.
 *
 *    @param void *v0        The raw vertex data for the first vertex.
 *    @param void *v1        The raw vertex data for the second vertex.
 *    @param void *v2        The raw vertex data for the third vertex.
 */
void raster_rasterize_triangle(void *r0, void *r1, void *r2) {
    /*
     *    Extract position data and rasterize it.
     */
    vec4_t p1 = vertex_get_position(r0);
    vec4_t p2 = vertex_get_position(r1);
    vec4_t p3 = vertex_get_position(r2);

    /*
     *    Map the normalized coordinates to screen coordinates.
     */
    vec2u_t v1 = {
        .x = (u32)((p1.x + 1.0f) * _raster_target->target->width / 2),
        .y = (u32)((p1.y + 1.0f) * _raster_target->target->height / 2),
    };
    vec2u_t v2 = {
        .x = (u32)((p2.x + 1.0f) * _raster_target->target->width / 2),
        .y = (u32)((p2.y + 1.0f) * _raster_target->target->height / 2),
    };
    vec2u_t v3 = {
        .x = (u32)((p3.x + 1.0f) * _raster_target->target->width / 2),
        .y = (u32)((p3.y + 1.0f) * _raster_target->target->height / 2),
    };

    u8 v0[VERTEX_ASM_MAX_VERTEX_SIZE];
    u8 v[VERTEX_ASM_MAX_VERTEX_SIZE];

    u8 pIA[VERTEX_ASM_MAX_VERTEX_SIZE];
    u8 pIB[VERTEX_ASM_MAX_VERTEX_SIZE];
    u8 pIC[VERTEX_ASM_MAX_VERTEX_SIZE];

    u8 swap[VERTEX_ASM_MAX_VERTEX_SIZE];

    vec4_t pa;
    vec4_t pb;
    vec4_t pc;

    /*
     *    In order to perform perspective correction, we need to know the
     *    z-coordinates of the vertices.
     */
    f32 z1 = p1.z;
    f32 z2 = p2.z;
    f32 z3 = p3.z;

    vec2u_t temp;
    f32 tempf;
    void *pTemp;

    /*
     *    Sort the vertices by y-coordinate.
     *
     *    y0 at the top, y1 at the middle, y2 at the bottom.
     */
    if (v1.y < v2.y) {
        temp = v1;
        v1 = v2;
        v2 = temp;

        tempf = z1;
        z1 = z2;
        z2 = tempf;

        pTemp = r0;
        r0 = r1;
        r1 = pTemp;
    }
    if (v2.y < v3.y) {
        temp = v2;
        v2 = v3;
        v3 = temp;

        tempf = z2;
        z2 = z3;
        z3 = tempf;

        pTemp = r1;
        r1 = r2;
        r2 = pTemp;
    }
    if (v1.y < v2.y) {
        temp = v1;
        v1 = v2;
        v2 = temp;

        tempf = z1;
        z1 = z2;
        z2 = tempf;

        pTemp = r0;
        r0 = r1;
        r1 = pTemp;
    }

    /*
     *    If the triangle is flat, return.
     */
    if (v1.y == v3.y) {
        return;
    }

    /*
     *    Rasterize the starting y position.
     */
    int y = MAX(v1.y, 0);
    y = MIN(y, _raster_target->target->height);

    /*
     *    Calculate the slopes of the lines.
     */
    float dy0 = ((float)v2.x - v1.x) / (v2.y - v1.y);
    float dy1 = ((float)v3.x - v1.x) / (v3.y - v1.y);
    float dy2 = ((float)v3.x - v2.x) / (v3.y - v2.y);

    /*
     *    Scale vertex attributes by their inverse z-coordinates.
     *    In the drawing process, we will then divide by the inverse
     *    z-coordinate again to perform perspective correction.
     */
    memcpy(pIA, vertex_scale(r0, 1 / z1, V_POS), VERTEX_ASM_MAX_VERTEX_SIZE);
    memcpy(pIB, vertex_scale(r1, 1 / z2, V_POS), VERTEX_ASM_MAX_VERTEX_SIZE);
    memcpy(pIC, vertex_scale(r2, 1 / z3, V_POS), VERTEX_ASM_MAX_VERTEX_SIZE);

    /*
     *    We'll also interpolate the z coordinate by the inverse z-coordinates.
     */
    pa = vertex_get_position(pIA);
    pb = vertex_get_position(pIB);
    pc = vertex_get_position(pIC);

    pa.z = 1 / pa.z;
    pb.z = 1 / pb.z;
    pc.z = 1 / pc.z;

    vertex_set_position(pIA, pa);
    vertex_set_position(pIB, pb);
    vertex_set_position(pIC, pc);

    /*
     *    Check for flat top.
     */
    if (v1.y == v2.y) {
        /*
         *    Sort top two vertices by x-coordinate,
         *    y0 at the left, y1 at the right.
         */
        if (v1.x < v2.x) {
            temp = v1;
            v1 = v2;
            v2 = temp;

            tempf = z1;
            z1 = z2;
            z2 = tempf;

            tempf = dy1;
            dy1 = dy2;
            dy2 = tempf;

            memcpy(swap, pIA, VERTEX_ASM_MAX_VERTEX_SIZE);
            memcpy(pIA, pIB, VERTEX_ASM_MAX_VERTEX_SIZE);
            memcpy(pIB, swap, VERTEX_ASM_MAX_VERTEX_SIZE);
        }
        while (y >= v3.y) {
            memcpy(v0,
                   vertex_build_interpolated(pIB, pIC,
                                             (f32)(v1.y - y) / (v1.y - v3.y)),
                   VERTEX_ASM_MAX_VERTEX_SIZE);
            memcpy(v,
                   vertex_build_interpolated(pIA, pIC,
                                             (f32)(v1.y - y) / (v1.y - v3.y)),
                   VERTEX_ASM_MAX_VERTEX_SIZE);
            raster_draw_scanline(v2.x + (y - v1.y) * dy2,
                                 v1.x + (y - v1.y) * dy1, y, v0, v);
            y--;
        }
        return;
    }

    /*
     *    Check for flat bottom.
     */
    else if (v2.y == v3.y) {
        /*
         *    Sort bottom two vertices by x-coordinate,
         *    y1 at the left, y2 at the right.
         */
        if (v2.x < v3.x) {
            temp = v2;
            v2 = v3;
            v3 = temp;

            tempf = z2;
            z2 = z3;
            z3 = tempf;

            tempf = dy1;
            dy1 = dy0;
            dy0 = tempf;

            memcpy(swap, pIB, VERTEX_ASM_MAX_VERTEX_SIZE);
            memcpy(pIB, pIC, VERTEX_ASM_MAX_VERTEX_SIZE);
            memcpy(pIC, swap, VERTEX_ASM_MAX_VERTEX_SIZE);
        }
        while (y >= v3.y) {
            memcpy(v0,
                   vertex_build_interpolated(pIA, pIB,
                                             (f32)(v1.y - y) / (v1.y - v3.y)),
                   VERTEX_ASM_MAX_VERTEX_SIZE);
            memcpy(v,
                   vertex_build_interpolated(pIA, pIC,
                                             (f32)(v1.y - y) / (v1.y - v3.y)),
                   VERTEX_ASM_MAX_VERTEX_SIZE);
            raster_draw_scanline(v1.x + (y - v1.y) * dy0,
                                 v1.x + (y - v1.y) * dy1, y, v0, v);
            y--;
        }
        return;
    }

    while (y >= v3.y) {
        /*
         *    Bend is on the left.
         */
        if (v2.x < v3.x) {
            memcpy(v,
                   vertex_build_interpolated(pIA, pIC,
                                             (f32)(v1.y - y) / (v1.y - v3.y)),
                   VERTEX_ASM_MAX_VERTEX_SIZE);
            if (y >= v2.y) {
                memcpy(v0,
                       vertex_build_interpolated(
                           pIA, pIB, (f32)(v1.y - y) / (v1.y - v2.y)),
                       VERTEX_ASM_MAX_VERTEX_SIZE);
                raster_draw_scanline(v1.x + (y - v1.y) * dy0,
                                     v1.x + (y - v1.y) * dy1, y, v0, v);
            } else {
                memcpy(v0,
                       vertex_build_interpolated(
                           pIB, pIC, (f32)(v2.y - y) / (v2.y - v3.y)),
                       VERTEX_ASM_MAX_VERTEX_SIZE);
                raster_draw_scanline(v2.x + (y - v2.y) * dy2,
                                     v1.x + (y - v1.y) * dy1, y, v0, v);
            }
        }
        /*
         *    Bend is on the right.
         */
        else {
            memcpy(v0,
                   vertex_build_interpolated(pIA, pIC,
                                             (f32)(v1.y - y) / (v1.y - v3.y)),
                   VERTEX_ASM_MAX_VERTEX_SIZE);
            if (y >= v2.y) {
                memcpy(v,
                       vertex_build_interpolated(
                           pIA, pIB, (f32)(v1.y - y) / (v1.y - v2.y)),
                       VERTEX_ASM_MAX_VERTEX_SIZE);
                raster_draw_scanline(v1.x + (y - v1.y) * dy1,
                                     v1.x + (y - v1.y) * dy0, y, v0, v);
            } else {
                memcpy(v,
                       vertex_build_interpolated(
                           pIB, pIC, (f32)(v2.y - y) / (v2.y - v3.y)),
                       VERTEX_ASM_MAX_VERTEX_SIZE);
                raster_draw_scanline(v1.x + (y - v1.y) * dy1,
                                     v2.x + (y - v2.y) * dy2, y, v0, v);
            }
        }
        y--;
    }
}

/*
 *    Uses threads to rasterize a triangle.
 *
 *    @param void *params     The parameters for the rasterization.
 */
void *raster_rasterize_triangle_thread(void *params) {
    triangle_t *tri = (triangle_t *)params;

    raster_rasterize_triangle(tri->v0, tri->v1, tri->v2);

    free(tri->v0);
    free(tri->v1);
    free(tri->v2);
    free(tri);
}