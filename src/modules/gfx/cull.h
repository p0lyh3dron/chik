/*
 *    cull.h    --    header for culling/clipping routines
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on June 12, 2022.
 * 
 *    This file is part of the Chik engine.
 * 
 *    As of now, the culling routines consist of just frustum culling,
 *    which tests a line segment against a plane, and interpolates a new
 *    point on the line segment if it intersects the plane.
 */
#ifndef CHIK_GFX_CULL_H
#define CHIK_GFX_CULL_H

#include "libchik.h"

/*
 *    Sets the current vertex size.
 *
 *    @param u32           The size of the vertex data.
 */
void cull_set_vertex_size( u32 sSize );

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
 *
 */
u32 cull_clip_vertex( plane_t *spP, void *spV0, void *spV1, void *spRet, u32 sFirst );

/*
 *    Inserts a vertex into a clipped vertex list.
 *
 *    @param void           *    The vertex to insert.
 *    @param void          **    The list of vertices.
 *    @param u32                 The target index.
 *    @param u32                 The number of vertices in the list.
 *    @param u32                 The list size.
 */
void cull_insert_vertex( void *spV, void **spList, u32 sIndex, u32 sCount, u32 sSize );

/*
 *    Removes a vertex from a clipped vertex list.
 *
 *    @param u32                 The index to remove.
 *    @param void          **    The list of vertices.
 *    @param u32                 The number of vertices in the list.
 *    @param u32                 The list size.
 */
void cull_remove_vertex( u32 sIndex, void **spList, u32 sCount, u32 sSize );

/*
 *    Creates the view frustum.
 */
void cull_create_frustum();

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
 *    @return void *    The new vertices.
 */
void *cull_clip_triangle( void *spV0, void *spV1, void *spV2, s32 *spNumVertices );

#endif /* CHIK_GFX_CULL_H  */