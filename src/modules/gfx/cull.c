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

#include "vertexasm.h"

frustum_t gFrustum;
u32       gVertexSize = 0;

/*
 *    Sets the current vertex size.
 *
 *    @param u32           The size of the vertex data.
 */
void cull_set_vertex_size( u32 sSize ) {
    gVertexSize = sSize;
}

/*
 *    Clips a pair of vertices.
 *
 *    @param plane_t       *    The plane to clip against.
 *    @param void          *    The first vertex.
 *    @param void          *    The second vertex.
 *    @param void          *    The clipped vertex.
 *    @param u32                True if this is the first vertex. 
 * 
 *    @return u32               Bitmask of the clip flags.
 *                              0x0 = keep the first vertex.
 *                              0x1 = modify first vertex with the clipped.
 *                              0x2 = modify the first vertex of the array.
 *
 */
u32 cull_clip_vertex( plane_t *spP, void *spV0, void *spV1, void *spRet, u32 sFirst ) {
    vec4_t p0 = vertex_get_position( spV0 );
    vec4_t p1 = vertex_get_position( spV1 );

    f32                  outside     = plane_distance( spP, &p0 );
    f32                  nextOutside = plane_distance( spP, &p1 );

    log_note( "outside = %f, next = %f\n", outside, nextOutside );
    /*
    *    Check if the triangle is outside the frustum.
    */
    if ( outside > 0.f ^ nextOutside > 0.f ) {
        f32 t = outside / ( outside - nextOutside );
        /*
        *    Generate a new vertex.
        */
        memcpy( spRet, vertex_build_interpolated( spV0, spV1, t ), gVertexSize );
        /*
        *    If our initial vertex is inside, append the new vertex.
        */
        if ( outside >= 0.f ) {
            return 0b00000011;
        }
        
        else if ( sFirst ) {
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
        if ( outside >= 0.f ) {
            /*
             *    Keep.
             */
            return 0b00000001;
        }
        else if ( sFirst ) {
            return 0b00000101;
        }
        else {
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
 *    @param void           *    The vertex to insert.
 *    @param void          **    The list of vertices.
 *    @param u32                 The target index.
 *    @param u32                 The number of vertices in the list.
 *    @param u32                 The list size.
 */
void cull_insert_vertex( void *spV, void **spList, u32 sIndex, u32 sCount, u32 sSize ) {
    if ( sIndex >= sSize ) {
        log_error( "Index out of bounds.\n" );
        return;
    }
    if ( sCount >= sSize ) {
        log_error( "List is full.\n" );
        return;
    }
    /*
     *    Shift the vertices down.
     */
    for ( u32 i = sCount; i > sIndex; i-- ) {
        memcpy( ( u8 * )spList + i * VERTEX_ASM_MAX_VERTEX_SIZE, ( u8 * )spList + ( i - 1 ) * VERTEX_ASM_MAX_VERTEX_SIZE, gVertexSize );
    }
    /*
     *    Insert the vertex.
     */
    memcpy( ( u8 * )spList + sIndex * VERTEX_ASM_MAX_VERTEX_SIZE, spV, gVertexSize );
}

/*
 *    Removes a vertex from a clipped vertex list.
 *
 *    @param u32                 The index to remove.
 *    @param void          **    The list of vertices.
 *    @param u32                 The number of vertices in the list.
 *    @param u32                 The list size.
 */
void cull_remove_vertex( u32 sIndex, void **spList, u32 sCount, u32 sSize ) {
    if ( sIndex >= sSize ) {
        log_error( "Index out of bounds.\n" );
        return;
    }
    if ( sCount <= 0 ) {
        log_error( "List is empty.\n" );
        return;
    }
    /*
     *    Shift the vertices up.
     */
    for ( u32 i = sIndex; i < sCount - 1; i++ ) {
        /*
         *    The scope doesn't seem to know that void **spList is a u8[][], so we'll
         *    use direct memory access. I hope this works on other platforms.
         */
        memcpy( ( u8 * )spList + i * VERTEX_ASM_MAX_VERTEX_SIZE, ( u8 * )spList + ( i + 1 ) * VERTEX_ASM_MAX_VERTEX_SIZE, gVertexSize );
    }
}

/*
 *    Creates the view frustum.
 */
void cull_create_frustum() {
    f32 n = 0.1f;
    f32 f = 100.f;

    vec2_t nsw = { -n, -n };
    vec2_t nse = {  n, -n };
    vec2_t nne = {  n,  n };
    vec2_t nnw = { -n,  n };

    vec2_t fsw = { -f, -f  };
    vec2_t fse = {  f, -f  };
    vec2_t fne = {  f,  f  };
    vec2_t fnw = { -f,  f  };

    vec3_t nearTop   = { nnw.x,  nnw.y, n };
    vec3_t nearBot   = { nsw.x,  nsw.y, n };
    vec3_t nearRight = { nne.x,  nne.y, n };
    plane_from_points( &gFrustum.aPlanes[ 0 ], &nearTop, &nearBot, &nearRight );

    vec3_t leftCloseBottom = { nsw.x,  nsw.y, n };
    vec3_t leftCloseTop    = { nnw.x,  nnw.y,  n };
    vec3_t leftFarTop      = { fnw.x,  fnw.y,  f };
    plane_from_points( &gFrustum.aPlanes[ 1 ], &leftCloseBottom, &leftCloseTop, &leftFarTop );

    /*
     *    2 Far points, close point is on the bottom.
     */
    vec3_t rightCloseBottom  = { nse.x,  nse.y,  n };
    vec3_t rightFarBottom    = { fse.x,  fse.y,  f };
    vec3_t rightFarTop       = { fne.x,  fne.y,  f };
    plane_from_points( &gFrustum.aPlanes[ 2 ], &rightCloseBottom, &rightFarBottom, &rightFarTop );

    vec3_t topCloseLeft  = { nnw.x, nnw.y,  n }; 
    vec3_t topCloseRight = { nne.x, nne.y,  n };
    vec3_t topFarLeft    = { fnw.x, fnw.y,  f };
    plane_from_points( &gFrustum.aPlanes[ 3 ], &topCloseLeft, &topCloseRight, &topFarLeft );

    vec3_t bottomCloseLeft = { nsw.x,  nsw.y, n };
    vec3_t bottomFarLeft   = { fsw.x,  fsw.y, f };
    vec3_t bottomFarRight  = { fse.x,  fse.y, f };
    plane_from_points( &gFrustum.aPlanes[ 4 ], &bottomFarRight, &bottomCloseLeft, &bottomFarLeft );

    /*
     *    Explain this later.
     */
    vec3_t farTop   = { fnw.x, fnw.y,  f };
    vec3_t farBot   = { fsw.x, fsw.y,  f };
    vec3_t farRight = { fne.x, fne.y,  f };
    plane_from_points( &gFrustum.aPlanes[ 5 ], &farTop, &farRight, &farBot );
}

/*
 *    Clips a triangle.
 *
 *    Reference: https://youtu.be/hxOw_p0kLfI
 *
 *    @param void *     The first vertex.
 *    @param void *     The second vertex.
 *    @param void *     The third vertex.
 *    @param s32  *     The number of new vertices.
 *
 *    @return chik_vertex_t *    The new vertices.
 */
chik_vertex_t *cull_clip_triangle( void *spV0, void *spV1, void *spV2, s32 *spNumVertices ) {
    static u8 vertices[ 8 * VERTEX_ASM_MAX_VERTEX_SIZE ];
    u32                  numVertices   = 3;

    /*
     *    Copy the vertices into the array.
     */
    memcpy( vertices + 0 * VERTEX_ASM_MAX_VERTEX_SIZE, spV0, gVertexSize );
    memcpy( vertices + 1 * VERTEX_ASM_MAX_VERTEX_SIZE, spV1, gVertexSize );
    memcpy( vertices + 2 * VERTEX_ASM_MAX_VERTEX_SIZE, spV2, gVertexSize );

    for ( u64 i = 0; i < ARR_LEN( gFrustum.aPlanes ); ++i ) {
        log_note( " plane %i\n", i );
        u32 removeFirst = 0;
        for ( u64 j = 0; j < numVertices; ) {
            u8  v[ VERTEX_ASM_MAX_VERTEX_SIZE ];
            u32 ret = cull_clip_vertex( 
                &gFrustum.aPlanes[ i ], 
                &vertices[ j * VERTEX_ASM_MAX_VERTEX_SIZE ], 
                &vertices[ ( ( j + 1 ) % numVertices ) * VERTEX_ASM_MAX_VERTEX_SIZE ], 
                &v, 
                j == 0 
            );

            /*
             *    Remove the first vertex once we are at the end of the loop.
             */
            if ( ret & 0b00000100 ) {
                removeFirst = 1;
            }

            /*
             *    Keep the first vertex.
             */
            if ( ret & 0b00000001 ) {
                /*
                 *    Insert the new clipped vertex.
                 */
                if ( ret & 0b00000010 ) {
                    cull_insert_vertex( &v, &vertices, ++j, numVertices, ARR_LEN( vertices ) );
                    numVertices++;
                }
                /*
                 *    No need to insert the new vertex.
                 */
                ++j;
            }
            else if ( ret & 0b00000010 ) {
                /*
                 *    Replace the first vertex.
                 */
                memcpy( vertices + j * VERTEX_ASM_MAX_VERTEX_SIZE, &v, gVertexSize );
                ++j;
            }
            else {
                /*
                 *    Erase the first vertex.
                 */
                cull_remove_vertex( j, &vertices, numVertices--, ARR_LEN( vertices ) );
            }
        }
        if ( removeFirst ) {
            cull_remove_vertex( 0, &vertices, numVertices--, ARR_LEN( vertices ) );
        }
    }
    *spNumVertices = numVertices;

    log_note( "Clipped %d vertices.\n", numVertices );

    for ( s64 i = 0; i < numVertices; ++i ) {
        vec3_t v = *( vec3_t* )( vertices + i * VERTEX_ASM_MAX_VERTEX_SIZE );
        vec2u_t v1 = {
        .x = ( u32 )( ( v.x + 1.0f ) * 640  / 2 ),
        .y = ( u32 )( ( v.y + 1.0f ) * 480 / 2 ),
        };
        log_note( "  %i: %d %d\n", i, v1.x, v1.y );
    }

    return vertices;
}