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

extern v_layout_t _layout;

/*
 *    Sets up the rasterization stage.
 */
void raster_setup(void) {
    unsigned int width;
    unsigned int height;

    if (args_has("-w") && args_has("-h")) {
        width  = args_get_int("-w");
        height = args_get_int("-h");
    } else {
        width  = 1152;
        height = 864;
    }

    _z_buffer = rendertarget_create(width, height, IMAGE_FMT_RGBA8);

    if (!_z_buffer) {
        LOGF_FAT("Could not create Z buffer.");
        return;
    }
}

/*
 *    Sets the vertex layout.
 *
 *    @param v_layout_t    The vertex layout.
 */
void raster_set_vertex_layout(v_layout_t layout) {

}

/*
 *    Sets the rasterization stage's bitmap.
 *
 *    @param    rendertarget_t *target    The rendertarget to use for
 * rasterization.
 */
void raster_set_rendertarget(rendertarget_t *target) {
    _raster_target = target;
}

/*
 *    Clears the depth buffer.
 */
void raster_clear_depth(void) {
    size_t i;
    float        *pDepth = (float *)_z_buffer->target->buf;

    for (i = 0; i < _z_buffer->target->width * _z_buffer->target->height; i++) {
        pDepth[i] = 1000.f;
    }
}

/*
 *    Draw a scanline.
 *
 *    @param int x1          The screen x coordinate of the start of the scanline.
 *    @param int x2          The screen x coordinate of the end of the scanline.
 *    @param int y           The screen y coordinate of the scanline.
 *    @param void *v1        The first vertex of the scanline.
 *    @param void *v2        The second vertex of the scanline.
 */
void raster_draw_scanline(int x1, int x2, int y, void *v1, void *v2, void *assets, material_t *mat) {
    int        x = 0;
    int        end_x = 0;
    int        temp = 0;
    int        width = 0;
    float      iz = 0.f;
    float      z = 0.f;
    float      dz = 0.f;
    float     *depth = nullptr;
    char      *raster = nullptr;
    vec_t     *tempv = nullptr;
    vec4_t     p1;
    vec4_t     p2;
    vec_t      v[MAX_VECTOR_ATTRIBUTES];
    vec_t      scaled_v[MAX_VECTOR_ATTRIBUTES];
    vec_t      diff[MAX_VECTOR_ATTRIBUTES];
    fragment_t f;
    void (*f_fun)(fragment_t *, void *, void *, material_t *) = _layout.f_fun;
    void (*v_scale)(void *, void *, float) = _layout.v_scale;
    void (*v_add)(void *, void *, void *) = _layout.v_add;

    /*
     *    Early out if the scanline is outside the render target,
     *    or if the line is a degenerate.
     */
    if (y < 0 || y >= _raster_target->target->height || (x1 < 0 && x2 < 0)) {
        return;
    }

    if (x1 > x2) {
        temp = x1;
        x1   = x2;
        x2   = temp;

        tempv = v1;
        v1    = v2;
        v2    = tempv;
    }

    p1 = vertex_get_position(v1);
    p2 = vertex_get_position(v2);

    if (p1.z == 0.0f || p2.z == 0.0f) {
        return;
    }

    f.pos.x = x;
    f.pos.y = y;

    x      = MAX(x1, 0);
    z      = p1.z;
    dz     = (p2.z - p1.z) / (x2 - x1);
    width  = _raster_target->target->width;
    depth  = (float *)_z_buffer->target->buf + x + y * width;
    raster = _raster_target->target->buf + (y * width + x) * 3;
    end_x  = MIN(x2, width);

    /*
     *    Build the differential.
     */
    vertex_build_differential(diff, v1, v2, 1.0 / (x2 - x1));
    memcpy(&v, v1, sizeof(v));

    while (x < end_x) {
        iz = 1.0f / z;

        if (*depth <= iz) {
            /*
             *    Interpolate the vector values, and apply to the fragment.
             */
            v_add(&v, &v, diff);
            
            z      += dz;
            raster += 3;
            depth++;
            x++;
            continue;   
        }

        *depth = iz;

        v_scale(&scaled_v, &v, iz);
        f_fun(&f, &scaled_v, assets, mat);

        /*
         *    Draw the vertex.
         */
        memcpy(raster, &f.color, 3);

        /*
         *    Interpolate the vector values, and apply to the fragment.
         */
        v_add(&v, &v, diff);

        z      += dz;
        raster += 3;
        depth++;
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
void raster_rasterize_triangle(void *r0, void *r1, void *r2, void *assets, material_t *mat) {
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
        .x = (unsigned int)((p1.x + 1.0f) * _raster_target->target->width / 2),
        .y = (unsigned int)((p1.y + 1.0f) * _raster_target->target->height / 2),
    };
    vec2u_t v2 = {
        .x = (unsigned int)((p2.x + 1.0f) * _raster_target->target->width / 2),
        .y = (unsigned int)((p2.y + 1.0f) * _raster_target->target->height / 2),
    };
    vec2u_t v3 = {
        .x = (unsigned int)((p3.x + 1.0f) * _raster_target->target->width / 2),
        .y = (unsigned int)((p3.y + 1.0f) * _raster_target->target->height / 2),
    };

    unsigned char v0[VERTEX_ASM_MAX_VERTEX_SIZE];
    unsigned char v[VERTEX_ASM_MAX_VERTEX_SIZE];

    unsigned char pIA[VERTEX_ASM_MAX_VERTEX_SIZE];
    unsigned char pIB[VERTEX_ASM_MAX_VERTEX_SIZE];
    unsigned char pIC[VERTEX_ASM_MAX_VERTEX_SIZE];

    unsigned char swap[VERTEX_ASM_MAX_VERTEX_SIZE];

    vec4_t pa;
    vec4_t pb;
    vec4_t pc;

    /*
     *    In order to perform perspective correction, we need to know the
     *    z-coordinates of the vertices.
     */
    float z1 = p1.z;
    float z2 = p2.z;
    float z3 = p3.z;

    vec2u_t temp;
    float   tempf;
    void   *pTemp;

    /*
     *    Sort the vertices by y-coordinate.
     *
     *    y0 at the top, y1 at the middle, y2 at the bottom.
     */
    if (v1.y < v2.y) {
        temp = v1;
        v1   = v2;
        v2   = temp;

        tempf = z1;
        z1    = z2;
        z2    = tempf;

        pTemp = r0;
        r0    = r1;
        r1    = pTemp;
    }
    if (v2.y < v3.y) {
        temp = v2;
        v2   = v3;
        v3   = temp;

        tempf = z2;
        z2    = z3;
        z3    = tempf;

        pTemp = r1;
        r1    = r2;
        r2    = pTemp;
    }
    if (v1.y < v2.y) {
        temp = v1;
        v1   = v2;
        v2   = temp;

        tempf = z1;
        z1    = z2;
        z2    = tempf;

        pTemp = r0;
        r0    = r1;
        r1    = pTemp;
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
    y     = MIN(y, _raster_target->target->height);

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
    vertex_scale(pIA, r0, 1 / z1, V_POS);
    vertex_scale(pIB, r1, 1 / z2, V_POS);
    vertex_scale(pIC, r2, 1 / z3, V_POS);

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
            v1   = v2;
            v2   = temp;

            tempf = z1;
            z1    = z2;
            z2    = tempf;

            tempf = dy1;
            dy1   = dy2;
            dy2   = tempf;

            memcpy(swap, pIA, VERTEX_ASM_MAX_VERTEX_SIZE);
            memcpy(pIA, pIB, VERTEX_ASM_MAX_VERTEX_SIZE);
            memcpy(pIB, swap, VERTEX_ASM_MAX_VERTEX_SIZE);
        }
        while (y >= v3.y) {
            memcpy(v0,
                   vertex_build_interpolated(pIB, pIC,
                                             (float)(v1.y - y) / (v1.y - v3.y)),
                   VERTEX_ASM_MAX_VERTEX_SIZE);
            memcpy(v,
                   vertex_build_interpolated(pIA, pIC,
                                             (float)(v1.y - y) / (v1.y - v3.y)),
                   VERTEX_ASM_MAX_VERTEX_SIZE);
            raster_draw_scanline(v2.x + (y - v1.y) * dy2,
                                 v1.x + (y - v1.y) * dy1, y, v0, v, assets, mat);
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
            v2   = v3;
            v3   = temp;

            tempf = z2;
            z2    = z3;
            z3    = tempf;

            tempf = dy1;
            dy1   = dy0;
            dy0   = tempf;

            memcpy(swap, pIB, VERTEX_ASM_MAX_VERTEX_SIZE);
            memcpy(pIB, pIC, VERTEX_ASM_MAX_VERTEX_SIZE);
            memcpy(pIC, swap, VERTEX_ASM_MAX_VERTEX_SIZE);
        }
        while (y >= v3.y) {
            memcpy(v0,
                   vertex_build_interpolated(pIA, pIB,
                                             (float)(v1.y - y) / (v1.y - v3.y)),
                   VERTEX_ASM_MAX_VERTEX_SIZE);
            memcpy(v,
                   vertex_build_interpolated(pIA, pIC,
                                             (float)(v1.y - y) / (v1.y - v3.y)),
                   VERTEX_ASM_MAX_VERTEX_SIZE);
            raster_draw_scanline(v1.x + (y - v1.y) * dy0,
                                 v1.x + (y - v1.y) * dy1, y, v0, v, assets, mat);
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
                                             (float)(v1.y - y) / (v1.y - v3.y)),
                   VERTEX_ASM_MAX_VERTEX_SIZE);
            if (y >= v2.y) {
                memcpy(v0,
                       vertex_build_interpolated(
                           pIA, pIB, (float)(v1.y - y) / (v1.y - v2.y)),
                       VERTEX_ASM_MAX_VERTEX_SIZE);
                raster_draw_scanline(v1.x + (y - v1.y) * dy0,
                                     v1.x + (y - v1.y) * dy1, y, v0, v, assets, mat);
            } else {
                memcpy(v0,
                       vertex_build_interpolated(
                           pIB, pIC, (float)(v2.y - y) / (v2.y - v3.y)),
                       VERTEX_ASM_MAX_VERTEX_SIZE);
                raster_draw_scanline(v2.x + (y - v2.y) * dy2,
                                     v1.x + (y - v1.y) * dy1, y, v0, v, assets, mat);
            }
        }
        /*
         *    Bend is on the right.
         */
        else {
            memcpy(v0,
                   vertex_build_interpolated(pIA, pIC,
                                             (float)(v1.y - y) / (v1.y - v3.y)),
                   VERTEX_ASM_MAX_VERTEX_SIZE);
            if (y >= v2.y) {
                memcpy(v,
                       vertex_build_interpolated(
                           pIA, pIB, (float)(v1.y - y) / (v1.y - v2.y)),
                       VERTEX_ASM_MAX_VERTEX_SIZE);
                raster_draw_scanline(v1.x + (y - v1.y) * dy1,
                                     v1.x + (y - v1.y) * dy0, y, v0, v, assets, mat);
            } else {
                memcpy(v,
                       vertex_build_interpolated(
                           pIB, pIC, (float)(v2.y - y) / (v2.y - v3.y)),
                       VERTEX_ASM_MAX_VERTEX_SIZE);
                raster_draw_scanline(v1.x + (y - v1.y) * dy1,
                                     v2.x + (y - v2.y) * dy2, y, v0, v, assets, mat);
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

    raster_rasterize_triangle(tri->v0, tri->v1, tri->v2, tri->assets, tri->material);

    free(tri->v0);
    free(tri->v1);
    free(tri->v2);
    free(tri);
}