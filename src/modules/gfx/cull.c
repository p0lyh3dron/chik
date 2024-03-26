/*
 *    cull.h    --    header for culling/clipping routines
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on June 12, 2022.
 *
 *    This file is part of the Chik engine.
 *
 *    This file defines the culling/clipping routines.
 */
#include "cull.h"

#include <string.h>

#include "camera.h"
#include "vertexasm.h"

frustum_t    _frustum;
unsigned int _vert_size = 0;

/*
 *    Sets the current vertex size.
 *
 *    @param unsigned int size           The size of the vertex data.
 */
void cull_set_vertex_size(unsigned int size) { _vert_size = size; }

/*
 *    Clips a pair of vertices.
 *
 *    @param plane_t       *plane    The plane to clip against.
 *    @param void          *v0       The first vertex.
 *    @param void          *v1       The second vertex.
 *    @param void          *ret      The clipped vertex.
 *    @param unsigned int            first    True if this is the first vertex.
 *
 *    @return unsigned int               Bitmask of the clip flags.
 *                              0x0 = keep the first vertex.
 *                              0x1 = modify first vertex with the clipped.
 *                              0x2 = modify the first vertex of the array.
 *
 */
unsigned int cull_clip_vertex(plane_t *plane, void *v0, void *v1, void *ret, unsigned int first) {
    float t;

    vec4_t p0 = vertex_get_position(v0);
    vec4_t p1 = vertex_get_position(v1);

    float outside      = plane_distance(plane, (vec3_t *)&p0);
    float next_outside = plane_distance(plane, (vec3_t *)&p1);
    /*
     *    Check if the triangle is outside the frustum.
     */
    if (outside > 0.f ^ next_outside > 0.f) {
        t = outside / (outside - next_outside);
        /*
         *    Generate a new vertex.
         */
        memcpy(ret, vertex_build_interpolated(v0, v1, t), _vert_size);
        /*
         *    If our initial vertex is inside, append the new vertex.
         */
        if (outside >= 0.f) {
            return 0b00000011;
        }
        /*
         *    Remove the first vertex.
         */
        else if (first) {
            return 0b00000111;
        }
        /*
         *    If our initial vertex is outside, replace the first vertex.
         */
        else {
            return 0b00000010;
        }
    }
    /*
     *    If both points are inside, keep the vertices, otherwise
     *    discard them.
     */
    else {
        if (outside >= 0.f) {
            /*
             *    Keep.
             */
            return 0b00000001;
        }
        /*
         *    Discard the first vertex.
         */
        else if (first) {
            return 0b00000101;
        } else {
            /*
             *    Discard.
             */
            return 0b00000000;
        }
    }
}

/*
 *    Inserts a vertex into a clipped vertex list.
 *
 *    @param void           *v        The vertex to insert.
 *    @param void          **list     The list of vertices.
 *    @param unsigned int             idx      The target index.
 *    @param unsigned int             count    The number of vertices in the list.
 *    @param unsigned int             len      The list size.
 */
void cull_insert_vertex(void *v, void **list, unsigned int idx, unsigned int count, unsigned int len) {
    size_t i;

    if (idx >= len) {
        LOGF_ERR("Index out of bounds.\n");
        return;
    }
    if (count >= len) {
        LOGF_ERR("List is full.\n");
        return;
    }

    /*
     *    Shift the vertices down.
     */
    for (i = count; i > idx; i--) {
        memcpy((unsigned char *)list + i * VERTEX_ASM_MAX_VERTEX_SIZE,
               (unsigned char *)list + (i - 1) * VERTEX_ASM_MAX_VERTEX_SIZE, _vert_size);
    }

    /*
     *    Insert the vertex.
     */
    memcpy((unsigned char *)list + idx * VERTEX_ASM_MAX_VERTEX_SIZE, v, _vert_size);
}

/*
 *    Removes a vertex from a clipped vertex list.
 *
 *    @param unsigned int             idx      The index to remove.
 *    @param void          **list     The list of vertices.
 *    @param unsigned int             count    The number of vertices in the list.
 *    @param unsigned int             len     The list size.
 */
void cull_remove_vertex(unsigned int idx, void **list, unsigned int len, unsigned int sSize) {
    size_t i;

    if (idx >= sSize) {
        LOGF_ERR("Index out of bounds.\n");
        return;
    }
    if (len <= 0) {
        LOGF_ERR("List is empty.\n");
        return;
    }
    /*
     *    Shift the vertices up.
     */
    for (i = idx; i < len - 1; i++) {
        /*
         *    The scope doesn't seem to know that void **list is a unsigned char[][], so
         * we'll use direct memory access. I hope this works on other platforms.
         */
        memcpy((unsigned char *)list + i * VERTEX_ASM_MAX_VERTEX_SIZE,
               (unsigned char *)list + (i + 1) * VERTEX_ASM_MAX_VERTEX_SIZE, _vert_size);
    }
}

/*
 *    Creates the view frustum.
 */
void cull_create_frustum() {
    float n = 0.1;
    float f = 100;

    vec2_t nsw = {-n, -n};
    vec2_t nse = {n, -n};
    vec2_t nne = {n, n};
    vec2_t nnw = {-n, n};

    vec2_t fsw = {-f, -f};
    vec2_t fse = {f, -f};
    vec2_t fne = {f, f};
    vec2_t fnw = {-f, f};

    vec3_t nearTop   = {nnw.x, nnw.y, n};
    vec3_t nearBot   = {nsw.x, nsw.y, n};
    vec3_t nearRight = {nne.x, nne.y, n};

    vec3_t leftCloseBottom = {nsw.x, nsw.y, n};
    vec3_t leftCloseTop    = {nnw.x, nnw.y, n};
    vec3_t leftFarTop      = {fnw.x, fnw.y, f};

    vec3_t rightCloseBottom = {nse.x, nse.y, n};
    vec3_t rightFarBottom   = {fse.x, fse.y, f};
    vec3_t rightFarTop      = {fne.x, fne.y, f};

    vec3_t topCloseLeft  = {nnw.x, nnw.y, n};
    vec3_t topCloseRight = {nne.x, nne.y, n};
    vec3_t topFarLeft    = {fnw.x, fnw.y, f};

    vec3_t bottomFarRight  = {fse.x, fse.y, f};
    vec3_t bottomCloseLeft = {nsw.x, nsw.y, n};
    vec3_t bottomFarLeft   = {fsw.x, fsw.y, f};

    vec3_t farTop   = {fnw.x, fnw.y, f};
    vec3_t farRight = {fne.x, fne.y, f};
    vec3_t farBot   = {fsw.x, fsw.y, f};

    if (!_camera) {
        n = 0.1f;
        f = 100.f;
    } else {
        n = _camera->near;
        f = _camera->far;
    }

    /*
     *    Plane 0:    Near.
     *    This plane consists of the three points:
     *    the top-left, bottom-left, and bottom-right.
     */
    plane_from_points(&_frustum.planes[0], &nearTop, &nearBot, &nearRight);

    /*
     *    Plane 1:    Left.
     *    This plane consists of the three points ( when directly facing this
     * plane from outside the frustum as if looking through the game camera ):
     *    the bottom-right, top-right, and top-left.
     */
    plane_from_points(&_frustum.planes[1], &leftCloseBottom, &leftCloseTop,
                      &leftFarTop);

    /*
     *    Plane 2:    Right.
     *    This plane consists of the three points ( same conditions as above,
     * same for below ): the bottom-left, bottom-right, and top-right.
     */
    plane_from_points(&_frustum.planes[2], &rightCloseBottom, &rightFarBottom,
                      &rightFarTop);

    /*
     *    Plane 3:    Top.
     *    This plane consists of the three points:
     *    the bottom-left, bottom-right, and top-left.
     */
    plane_from_points(&_frustum.planes[3], &topCloseLeft, &topCloseRight,
                      &topFarLeft);

    /*
     *    Plane 4:    Bottom.
     *    This plane consists of the three points:
     *    the bottom-right, bottom-left, and top-left.
     */
    plane_from_points(&_frustum.planes[4], &bottomFarRight, &bottomCloseLeft,
                      &bottomFarLeft);

    /*
     *    Plane 5:    Far.
     *    This plane consists of the three points:
     *    the top-left, top-right, bottom-left.
     */
    plane_from_points(&_frustum.planes[5], &farTop, &farRight, &farBot);
}

/*
 *    Clips a triangle.
 *
 *    Reference: https://youtu.be/hxOw_p0kLfI
 *
 *    @param void *v0             The first vertex.
 *    @param void *v1             The second vertex.
 *    @param void *v2             The third vertex.
 *    @param int  *num_verts      The number of new vertices.
 *    @param unsigned int   is_clipped     Whether or not to clip the triangle.
 *
 *    @return void *    The new vertices.
 */
void *cull_clip_triangle(void *v0, void *v1, void *v2, int *num_verts,
                         unsigned int is_clipped) {
    size_t i;
    size_t j;
    unsigned int  remove_first;
    unsigned int  ret;

    /*
     *    TODO:    This used to be eight, but I changed it to sixteen
     *             because for some reason, eleven clipped vertices
     *             were being returned, overwriting platform_draw_image
     */
    static unsigned char vertices[16 * VERTEX_ASM_MAX_VERTEX_SIZE];
    unsigned char        v[VERTEX_ASM_MAX_VERTEX_SIZE];

    *num_verts = 3;

    /*
     *    Copy the vertices into the array.
     */
    memcpy(vertices + 0 * VERTEX_ASM_MAX_VERTEX_SIZE, v0, _vert_size);
    memcpy(vertices + 1 * VERTEX_ASM_MAX_VERTEX_SIZE, v1, _vert_size);
    memcpy(vertices + 2 * VERTEX_ASM_MAX_VERTEX_SIZE, v2, _vert_size);

    if (!is_clipped) {
        return vertices;
    }

    for (i = 0; i < ARR_LEN(_frustum.planes); ++i) {
        remove_first = 0;
        for (j = 0; j < *num_verts;) {
            ret = cull_clip_vertex(
                &_frustum.planes[i], &vertices[j * VERTEX_ASM_MAX_VERTEX_SIZE],
                &vertices[(j + 1) % (*num_verts) * VERTEX_ASM_MAX_VERTEX_SIZE],
                &v, j == 0);

            /*
             *    Remove the first vertex once we are at the end of the loop.
             */
            if (ret & 0b00000100) {
                remove_first = 1;
            }

            /*
             *    Keep the first vertex.
             */
            if (ret & 0b00000001) {
                /*
                 *    Insert the new clipped vertex.
                 */
                if (ret & 0b00000010) {
                    cull_insert_vertex(&v, (void **)&vertices, ++j, *num_verts,
                                       ARR_LEN(vertices));
                    (*num_verts)++;
                }
                /*
                 *    No need to insert the new vertex.
                 */
                ++j;
            } else if (ret & 0b00000010) {
                /*
                 *    Replace the first vertex.
                 */
                memcpy(vertices + j * VERTEX_ASM_MAX_VERTEX_SIZE, &v,
                       _vert_size);
                ++j;
            } else {
                /*
                 *    Erase the first vertex.
                 */
                cull_remove_vertex(j, (void **)&vertices, (*num_verts)--,
                                   ARR_LEN(vertices));
            }
        }
        /*
         *    Since we are doing this in place, we need to remove the first
         * vertex occasionally.
         */
        if (remove_first) {
            cull_remove_vertex(0, (void **)&vertices, (*num_verts)--,
                               ARR_LEN(vertices));
        }
    }

    return (void *)vertices;
}