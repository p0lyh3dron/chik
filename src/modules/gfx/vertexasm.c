/*
 *    vertexasm.c    --    source for the vertex assembler
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on June 12, 2022.
 *
 *    This file is part of the Chik engine.
 *
 *    The definitions for the vertex assembler are in here.
 */
#include "vertexasm.h"

#include <string.h>

#include "cull.h"


#ifdef _WIN32
    #define THREAD_LOCAL __declspec( thread )
#else
    #define THREAD_LOCAL __thread
#endif


v_layout_t _layout  = {.attributes = {0}, .count = 0};
void      *_uniform = nullptr;

/*
 *    Sets the vertex assembler's vertex layout.
 *
 *    @param v_layout_t layout   The layout of the vertex data.
 */
void vertexasm_set_layout(v_layout_t layout) {
    _layout = layout;

    cull_set_vertex_size(_layout.stride);
}

/*
 *    Extracts the position from a vertex.
 *
 *    @param void *v        The raw vertex data.
 *
 *    @return vec4_t       The position of the vertex.
 */
vec4_t vertex_get_position(void *v) {
    size_t i;

    for (i = 0; i < _layout.count; i++) {
        if (_layout.attributes[i].usage == V_POS)
            break;
    }

    if (i == _layout.count)
        return (vec4_t){0, 0, 0, 0};

    return *(vec4_t *)(v + _layout.attributes[i].offset);
}

/*
 *    Sets the position of a vertex.
 *
 *    @param void *v          The raw vertex data.
 *    @param vec4_t pos       The position of the vertex.
 */
void vertex_set_position(void *v, vec4_t pos) {
    size_t i;

    for (i = 0; i < _layout.count; i++) {
        if (_layout.attributes[i].usage == V_POS)
            break;
    }

    if (i == _layout.count)
        return;

    *(vec4_t *)(v + _layout.attributes[i].offset) = pos;
}

/*
 *    Performs a perspective divide on a vertex.
 *
 *    @param void *v          The raw vertex data.
 */
void vertex_perspective_divide(void *v) {
    vec4_t pos = vertex_get_position(v);

    pos.x /= pos.w;
    pos.y /= pos.w;

    vertex_set_position(v, pos);
}

/*
 *    Creates a vertex differential.
 *
 *    @param void *vd          The destination raw vertex data.
 *    @param void *v0          The raw vertex data of the first vertex.
 *    @param void *v1          The raw vertex data of the second vertex.
 *    @param float dist        The "distance" between the two.
 */
void vertex_build_differential(void *vd, void *v0, void *v1, float dist) {
    size_t                 i;
    static THREAD_LOCAL unsigned char buf[VERTEX_ASM_MAX_VERTEX_SIZE];

    for (i = 0; i < _layout.count; i++) {
        vec_sub((vec_t *)(vd + _layout.attributes[i].offset),
                v1 + _layout.attributes[i].offset,
                v0 + _layout.attributes[i].offset, _layout.attributes[i].fmt);
            
        vec_scale((vec_t *)(vd + _layout.attributes[i].offset),
                  vd + _layout.attributes[i].offset, dist,
                  _layout.attributes[i].fmt);
    }
}

/*
 *    Adds two vertices together.
 *
 *    @param void *vd          The destination raw vertex data.
 *    @param void *v0          The raw vertex data of the first vertex.
 *    @param void *v1          The raw vertex data of the second vertex.
 */
void vertex_add(void *vd, void *v0, void *v1) {
    _layout.v_add(vd, v0, v1);
}

/*
 *    Builds a new vertex given two vertices and a normalized difference.
 *
 *    @param void *v0          The raw vertex data of the first vertex.
 *    @param void *v1          The raw vertex data of the second vertex.
 *    @param float   diff        The normalized difference between the two
 * vertices.
 *
 *    @return void *       The raw vertex data of the new vertex.
 */
void *vertex_build_interpolated(void *v0, void *v1, float diff) {
    size_t                 i;
    static THREAD_LOCAL unsigned char buf[VERTEX_ASM_MAX_VERTEX_SIZE];

    for (i = 0; i < _layout.count; i++) {
        vec_interp((vec_t *)(buf + _layout.attributes[i].offset),
                   v0 + _layout.attributes[i].offset,
                   v1 + _layout.attributes[i].offset, diff,
                   _layout.attributes[i].fmt);
    }

    return buf;
}

/*
 *    Scales a vertex by a scalar.
 *
 *    @param void *vd               The return vertex.
 *    @param void *v                The raw vertex data.
 *    @param float   scale          The scalar to scale the vertex by.
 *    @param unsigned int   flags   A usage flag that determines how to scale the vertex.
 */
void vertex_scale(void *vd, void *v, float scale, unsigned int flags) {
    _layout.v_scale(vd, v, scale);
}

/*
 *    Binds the fragment shader's uniform data to the vertex assembler.
 *
 *    @param void *uniform        The raw uniform data.
 */
void vertexasm_bind_uniform(void *uniform) { _uniform = uniform; }

/*
 *    Applies the fragments of a fragment shader to a pixel.
 *
 *    @param void *f          The raw fragment data.
 *    @param fragment_t *p    The pixel to apply the fragment to.
 */
void fragment_apply(void *f, fragment_t *p, void *assets, material_t* mat) { _layout.f_fun(p, f, assets, mat); }