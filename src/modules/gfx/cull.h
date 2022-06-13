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
u32 cull_clip_vertex( plane_t *spP, chik_vertex_t *spV0, chik_vertex_t *spV1, chik_vertex_t *spRet, u32 sFirst );

/*
 *    Inserts a vertex into a clipped vertex list.
 *
 *    @param chik_vertex_t *     The vertex to insert.
 *    @param chik_vertex_t **    The list of vertices.
 *    @param u32                 The target index.
 *    @param u32                 The number of vertices in the list.
 *    @param u32                 The list size.
 */
void cull_insert_vertex( chik_vertex_t *spV, chik_vertex_t *spList, u32 sIndex, u32 sCount, u32 sSize );

/*
 *    Removes a vertex from a clipped vertex list.
 *
 *    @param u32                 The index to remove.
 *    @param chik_vertex_t **    The list of vertices.
 *    @param u32                 The number of vertices in the list.
 *    @param u32                 The list size.
 */
void cull_remove_vertex( u32 sIndex, chik_vertex_t *spList, u32 sCount, u32 sSize );

/*
 *    Creates the view frustum.
 */
void cull_create_frustum();

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
chik_vertex_t *cull_clip_triangle( chik_vertex_t *spV0, chik_vertex_t *spV1, chik_vertex_t *spV2, s32 *spNumVertices );

#endif /* CHIK_GFX_CULL_H  */