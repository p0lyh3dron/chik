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

#include "cull.h"

v_layout_t  gLayout   = { .aAttribs = { 0 }, .aCount = 0 };
void       *gpUniform = nullptr;

/*
 *    Sets the vertex assembler's vertex layout.
 *
 *    @param v_layout_t    The layout of the vertex data.
 */
void vertexasm_set_layout( v_layout_t sLayout ) {
    gLayout = sLayout;

    cull_set_vertex_size( gLayout.aStride );
}

/*
 *    Extracts the position from a vertex.
 *
 *    @param void *        The raw vertex data.
 * 
 *    @return vec4_t       The position of the vertex.
 */
vec4_t vertex_get_position( void *spV ) {
    s64 i;
    for ( i = 0; i < gLayout.aCount; i++ ) {
        if ( gLayout.aAttribs[ i ].aUsage == V_POS ) {
            break;
        }
    }
    if ( i == gLayout.aCount ) {
        return ( vec4_t ){ 0, 0, 0, 0 };
    }
    return *( vec4_t * )( ( u8 * )spV + gLayout.aAttribs[ i ].aOffset );
}

/*
 *    Builds a new vertex given two vertices and a normalized difference.
 *
 *    @param void *        The raw vertex data of the first vertex.
 *    @param void *        The raw vertex data of the second vertex.
 *    @param f32           The normalized difference between the two vertices.
 * 
 *    @return void *       The raw vertex data of the new vertex.
 */
void *vertex_build_interpolated( void *spV1, void *spV2, f32 sDiff ) {
    static u8 buf[ VERTEX_ASM_MAX_VERTEX_SIZE ];
    s64       i;

    for ( i = 0; i < gLayout.aCount; i++ ) {
        vec_interp( 
            buf  + gLayout.aAttribs[ i ].aOffset, 
            spV1 + gLayout.aAttribs[ i ].aOffset, 
            spV2 + gLayout.aAttribs[ i ].aOffset, 
            sDiff, 
            gLayout.aAttribs[ i ].aFormat 
        );
    }

    return buf;
}

/*
 *    Scales a vertex by a scalar.
 *
 *    @param void *        The raw vertex data.
 *    @param f32           The scalar to scale the vertex by.
 *    @param u32           A usage flag that determines how to scale the vertex.
 * 
 *    @return void *       The raw vertex data of the scaled vertex.
 */
void *vertex_scale( void *spV, f32 sScale, u32 sFlags ) {
    static u8 buf[ VERTEX_ASM_MAX_VERTEX_SIZE ];
    s64       i;

    for ( i = 0; i < gLayout.aCount; i++ ) {
        if ( !( gLayout.aAttribs[ i ].aUsage & sFlags ) ) {
            vec_scale( 
                buf  + gLayout.aAttribs[ i ].aOffset, 
                spV  + gLayout.aAttribs[ i ].aOffset, 
                sScale, 
                gLayout.aAttribs[ i ].aFormat 
            );
        }
        else {
            memcpy( buf + gLayout.aAttribs[ i ].aOffset, spV + gLayout.aAttribs[ i ].aOffset, gLayout.aAttribs[ i ].aStride );
        }
    }

    return buf;
}

/*
 *    Binds the fragment shader's uniform data to the vertex assembler.
 *
 *    @param void *        The raw uniform data.
 */
void vertexasm_bind_uniform( void *spUniform ) {
    gpUniform = spUniform;
}

/*
 *    Applies the fragments of a fragment shader to a pixel.
 *
 *    @param void *          The raw fragment data.
 *    @param fragment_t *    The pixel to apply the fragment to.
 */
void fragment_apply( void *spF, fragment_t *spP ) {
    s64 i;
    for ( i = 0; i < gLayout.aCount; i++ ) {
        if ( gLayout.aAttribs[ i ].apFunc ) {
            gLayout.aAttribs[ i ].apFunc( spP, spF + gLayout.aAttribs[ i ].aOffset, gpUniform );
        }
    }
}