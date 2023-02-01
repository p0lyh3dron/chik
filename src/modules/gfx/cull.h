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
 *    @param u32 size           The size of the vertex data.
 */
void cull_set_vertex_size(u32 size);

/*
 *    Clips a pair of vertices.
 *
 *    @param plane_t       *plane    The plane to clip against.
 *    @param void          *v0       The first vertex.
 *    @param void          *v1       The second vertex.
 *    @param void          *ret      The clipped vertex.
 *    @param u32            first    True if this is the first vertex.
 *
 *    @return u32               Bitmask of the clip flags.
 *                              0x0 = keep the first vertex.
 *                              0x1 = modify first vertex with the clipped.
 *                              0x2 = modify the first vertex of the array.
 *
 */
u32 cull_clip_vertex(plane_t *plane, void *v0, void *v1, void *ret, u32 first);

/*
 *    Inserts a vertex into a clipped vertex list.
 *
 *    @param void           *v        The vertex to insert.
 *    @param void          **list     The list of vertices.
 *    @param u32             idx      The target index.
 *    @param u32             count    The number of vertices in the list.
 *    @param u32             len      The list size.
 */
void cull_insert_vertex(void *v, void **list, u32 idx, u32 count,
                        u32 len);

/*
 *    Removes a vertex from a clipped vertex list.
 *
 *    @param u32             idx      The index to remove.
 *    @param void          **list     The list of vertices.
 *    @param u32             count    The number of vertices in the list.
 *    @param u32             len     The list size.
 */
void cull_remove_vertex(u32 idx, void **list, u32 len, u32 sSize);

/*
 *    Creates the view frustum.
 */
void cull_create_frustum();

/*
 *    Clips a triangle.
 *
 *    Reference: https://youtu.be/hxOw_p0kLfI
 *
 *    @param void *v0             The first vertex.
 *    @param void *v1             The second vertex.
 *    @param void *v2             The third vertex.
 *    @param s32  *num_verts      The number of new vertices.
 *    @param u32   is_clipped     Whether or not to clip the triangle.
 *
 *    @return void *    The new vertices.
 */
void *cull_clip_triangle(void *v0, void *v1, void *v2, s32 *num_verts,
                         u32 is_clipped);

#endif /* CHIK_GFX_CULL_H  */