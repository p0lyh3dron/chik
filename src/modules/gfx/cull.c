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

/*
 *    Clips a pair of vertices.
 *
 *    @param plane_t       *    The plane to clip against.
 *    @param chik_vertex_t *    The first vertex.
 *    @param chik_vertex_t *    The second vertex.
 *    @param chik_vertex_t *    The clipped vertex.
 *    @param u32                True if this is the first vertex. 
 * 
 *    @return u32               Bitmask of the clip flags.
 *                              0x0 = keep the first vertex.
 *                              0x1 = modify first vertex with the clipped.
 *
 */
u32 cull_clip_vertex( plane_t *spP, chik_vertex_t *spV0, chik_vertex_t *spV1, chik_vertex_t *spRet, u32 sFirst ) {
    f32                  outside     = plane_distance( spP, &spV0->aPos );
    f32                  nextOutside = plane_distance( spP, &spV1->aPos );

    //log_note( "outside = %f, next = %f\n", outside, nextOutside );
    /*
    *    Check if the triangle is outside the frustum.
    */
    if ( outside > 0.f ^ nextOutside > 0.f ) {
        f32 t = outside / ( outside - nextOutside );
        /*
        *    Generate a new vertex.
        */
        *spRet = *spV0;
        spRet->aPos.x += t * ( spV1->aPos.x - spV0->aPos.x );
        spRet->aPos.y += t * ( spV1->aPos.y - spV0->aPos.y );
        spRet->aPos.z += t * ( spV1->aPos.z - spV0->aPos.z );
        spRet->aPos.w += t * ( spV1->aPos.w - spV0->aPos.w );
        /*
        *    If our initial vertex is inside, append the new vertex.
        */
        if ( outside > 0.f || sFirst ) {
            return 0b00000011;
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
        if ( outside > 0.f ) {
            /*
             *    Keep.
             */
            return 0b00000001;
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
 *    @param chik_vertex_t *     The vertex to insert.
 *    @param chik_vertex_t **    The list of vertices.
 *    @param u32                 The target index.
 *    @param u32                 The number of vertices in the list.
 *    @param u32                 The list size.
 */
void cull_insert_vertex( chik_vertex_t *spV, chik_vertex_t *spList, u32 sIndex, u32 sCount, u32 sSize ) {
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
        spList[ i ] = spList[ i - 1 ];
    }
    /*
     *    Insert the vertex.
     */
    spList[ sIndex ] = *spV;
}

/*
 *    Removes a vertex from a clipped vertex list.
 *
 *    @param u32                 The index to remove.
 *    @param chik_vertex_t **    The list of vertices.
 *    @param u32                 The number of vertices in the list.
 *    @param u32                 The list size.
 */
void cull_remove_vertex( u32 sIndex, chik_vertex_t *spList, u32 sCount, u32 sSize ) {
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
        spList[ i ] = spList[ i + 1 ];
    }
}

/*
 *    Creates the view frustum.
 */
void cull_create_frustum() {
    f32 tanFov = 1;

    f32 n = 0.01f;
    f32 f = 100.f;

    vec2_t nsw = { -n * tanFov, -n * tanFov };
    vec2_t nse = {  n * tanFov, -n * tanFov };
    vec2_t nne = {  n * tanFov,  n * tanFov };
    vec2_t nnw = { -n * tanFov,  n * tanFov };

    vec2_t fsw = { -f  * tanFov, -f  * tanFov };
    vec2_t fse = {  f  * tanFov, -f  * tanFov };
    vec2_t fne = {  f  * tanFov,  f  * tanFov };
    vec2_t fnw = { -f  * tanFov,  f  * tanFov };

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
 *    @param chik_vertex_t *     The first vertex.
 *    @param chik_vertex_t *     The second vertex.
 *    @param chik_vertex_t *     The third vertex.
 *    @param s32 *               The number of new vertices.
 *
 *    @return chik_vertex_t *    The new vertices.
 */
chik_vertex_t *cull_clip_triangle( chik_vertex_t *spV0, chik_vertex_t *spV1, chik_vertex_t *spV2, s32 *spNumVertices ) {
    static chik_vertex_t vertices[ 8 ];
    u32                  numVertices   = 3;

    vertices[ 0 ] = *spV0;
    vertices[ 1 ] = *spV1;
    vertices[ 2 ] = *spV2;

    for ( u64 i = 0; i < ARR_LEN( gFrustum.aPlanes ); ++i ) {
        //log_note( " plane %i\n", i );
        for ( u64 j = 0; j < numVertices; ++j ) {
            chik_vertex_t v;
            u32 ret = cull_clip_vertex( &gFrustum.aPlanes[ i ], &vertices[ j ], &vertices[ ( j + 1 ) % numVertices ], &v, j == 0 );

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
            }
            else if ( ret & 0b00000010 ) {
                /*
                 *    Replace the first vertex.
                 */
                vertices[ j ] = v;
            }
            else {
                /*
                 *    Erase the first vertex.
                 */
                cull_remove_vertex( j--, &vertices, numVertices--, ARR_LEN( vertices ) );
            }
        }
    }
    *spNumVertices = numVertices;

    //log_note( "Clipped %d vertices.\n", numVertices );

    return vertices;
}