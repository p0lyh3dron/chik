/*
 *    drawable.h    --    header for general drawable functionality
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on June 7, 2022.
 *
 *    This file is part of the Chik engine.
 *
 *    The drawables refer to objects such as vertex buffers, textures,
 *    shaders, and so on. In it's primitive state, we'll allow for loading
 *    vertex buffers, and drawing a texture on to them.
 */
#pragma once

#define CHIK_GFX_DRAWABLE_MESH_MAX_ASSETS 16

#include "libchik.h"

#include "image.h"

typedef struct {
    void      *buf;
    u32        stride;
    u32        size;
    v_layout_t layout;
} vbuffer_t;

typedef struct {
    vbuffer_t    *vbuf;
    void         *assets;
    unsigned long assets_size;
    unsigned long assets_count;
} mesh_t;

/*
 *    Creates a vertex buffer.
 *
 *    @param void *v              The vertex data.
 *    @param u32 size             The size of the vertex data.
 *    @param u32 stride           The stride of the vertex data.
 *    @param v_layout_t layout    The layout of the vertex data.
 *
 *    @return void *              The vertex buffer.
 */
void *vbuffer_create(void *v, u32 size, u32 stride, v_layout_t layout);

/*
 *    Frees a vertex buffer.
 *
 *    @param void *buf   The vertex buffer to free.
 */
void vbuffer_free(void *buf);

/*
 *    Creates a mesh.
 *
 *    @param void *v    The vertex buffer.
 *
 *    @return void *    The mesh.
 */
void *mesh_create(void *v);

/*
 *    Sets the vertex buffer of a mesh.
 *
 *    @param void *m    The mesh.
 *    @param void *v    The vertex buffer.
 */
void mesh_set_vbuffer(void *m, void *v);

/*
 *    Appends an asset to a mesh.
 *
 *    @param void *m              The mesh.
 *    @param void *a              The asset.
 *    @param unsigned long size   The size of the asset.
 */
void mesh_append_asset(void *m, void *a, unsigned long size);

/*
 *    Sets the assets of a mesh.
 *
 *    @param void *m              The mesh.
 *    @param void *a              The asset.
 *    @param unsigned long size   The size of the asset.
 *    @param unsigned long i      The index of the asset.
 */
void mesh_set_asset(void *m, void *a, unsigned long size, unsigned long i);

/*
 *    Returns the data of an asset.
 *
 *    @param void *a            The assets.
 *    @param unsigned long i    The index of the asset.
 * 
 *    @return void *            The asset data.
 */
void *mesh_get_asset(void *a, unsigned long i);

/*
 *    Draws a mesh.
 *
 *    @param void *m    The mesh.
 */
void mesh_draw(void *m);

/*
 *    Frees a mesh.
 *
 *    @param void *m    The mesh.
 */
void mesh_free(void *m);