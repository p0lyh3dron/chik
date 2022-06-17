/*
 *    vertexasm.h    --    header for the vertex assembler
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on June 12, 2022.
 * 
 *    This file is part of the Chik engine.
 * 
 *    The declarations in this file are for the vertex assembler.
 *    The vertex assembler is used to manage raw vertex data, such as
 *    position for rasterization. It will also create new vertex data for
 *    interpolation during rasterization.
 */
#ifndef CHIK_GFX_VERTEXASM_H
#define CHIK_GFX_VERTEXASM_H

#include "libchik.h"

#define VERTEX_ASM_MAX_VERTEX_SIZE    ( 1024 )

/*
 *    Sets the vertex assembler's vertex layout.
 *
 *    @param v_layout_t    The layout of the vertex data.
 */
void vertexasm_set_layout( v_layout_t sLayout );

/*
 *    Extracts the position from a vertex.
 *
 *    @param void *        The raw vertex data.
 * 
 *    @return vec4_t       The position of the vertex.
 */
vec4_t vertex_get_position( void *spV );

/*
 *    Sets the position of a vertex.
 *
 *    @param void *        The raw vertex data.
 *    @param vec4_t       The position of the vertex.
 */
void vertex_set_position( void *spV, vec4_t sPosition );

/*
 *    Builds a new vertex given two vertices and a normalized difference.
 *
 *    @param void *        The raw vertex data of the first vertex.
 *    @param void *        The raw vertex data of the second vertex.
 *    @param f32           The normalized difference between the two vertices.
 * 
 *    @return void *       The raw vertex data of the new vertex.
 */
void *vertex_build_interpolated( void *spV1, void *spV2, f32 sDiff );

/*
 *    Scales a vertex by a scalar.
 *
 *    @param void *        The raw vertex data.
 *    @param f32           The scalar to scale the vertex by.
 *    @param u32           A usage flag that determines how to scale the vertex.
 * 
 *    @return void *       The raw vertex data of the scaled vertex.
 */
void *vertex_scale( void *spV, f32 sScale, u32 sFlags );

/*
 *    Binds the fragment shader's uniform data to the vertex assembler.
 *
 *    @param void *        The raw uniform data.
 */
void vertexasm_bind_uniform( void *spUniform );

/*
 *    Applies the fragments of a fragment shader to a pixel.
 *
 *    @param void *          The raw fragment data.
 *    @param fragment_t *    The pixel to apply the fragment to.
 */
void fragment_apply( void *spF, fragment_t *spP );

#endif /* CHIK_GFX_VERTEXASM_H  */