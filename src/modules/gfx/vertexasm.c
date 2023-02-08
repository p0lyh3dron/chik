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
    unsigned long i;

    for (i = 0; i < _layout.count; i++) {
        if (_layout.attributes[i].usage == V_POS)
            break;
    }

    if (i == _layout.count)
        return (vec4_t){0, 0, 0, 0};

    return *(vec4_t *)((u8 *)v + _layout.attributes[i].offset);
}

/*
 *    Sets the position of a vertex.
 *
 *    @param void *v          The raw vertex data.
 *    @param vec4_t pos       The position of the vertex.
 */
void vertex_set_position(void *v, vec4_t pos) {
    unsigned long i;

    for (i = 0; i < _layout.count; i++) {
        if (_layout.attributes[i].usage == V_POS)
            break;
    }

    if (i == _layout.count)
        return;

    *(vec4_t *)((u8 *)v + _layout.attributes[i].offset) = pos;
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
 *    Builds a new vertex given two vertices and a normalized difference.
 *
 *    @param void *v0          The raw vertex data of the first vertex.
 *    @param void *v1          The raw vertex data of the second vertex.
 *    @param f32   diff        The normalized difference between the two
 * vertices.
 *
 *    @return void *       The raw vertex data of the new vertex.
 */
void *vertex_build_interpolated(void *v0, void *v1, f32 diff) {
    unsigned long      i;
    static __thread u8 buf[VERTEX_ASM_MAX_VERTEX_SIZE];

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
 *    @param void *v            The raw vertex data.
 *    @param f32   scale        The scalar to scale the vertex by.
 *    @param u32   flags        A usage flag that determines how to scale the
 * vertex.
 *
 *    @return void *       The raw vertex data of the scaled vertex.
 */
void *vertex_scale(void *v, f32 scale, u32 flags) {
    unsigned long      i;
    static __thread u8 buf[VERTEX_ASM_MAX_VERTEX_SIZE];

    for (i = 0; i < _layout.count; i++) {
        if (!(_layout.attributes[i].usage & flags)) {
            vec_scale((vec_t *)(buf + _layout.attributes[i].offset),
                      v + _layout.attributes[i].offset, scale,
                      _layout.attributes[i].fmt);
        } else {
            memcpy(buf + _layout.attributes[i].offset,
                   v + _layout.attributes[i].offset,
                   _layout.attributes[i].stride);
        }
    }

    return buf;
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
void fragment_apply(void *f, fragment_t *p, void *assets) { _layout.f_fun(p, f, assets); }