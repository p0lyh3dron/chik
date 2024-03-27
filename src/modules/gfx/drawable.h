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
    char        *buf;
    unsigned int stride;
    unsigned int size;
    v_layout_t   layout;
} vbuffer_t;

typedef struct {
    u32        offset;
    u32        size;
    material_t material;
} mesh_surface_t;

typedef struct {
    vbuffer_t      *vbuf;
    mesh_surface_t *surfaces;
    u32             surface_count;
    char           *assets;
    u64             assets_size;
    u64             assets_count;
} mesh_t;

/*
 *    Creates a vertex buffer.
 *
 *    @param void *v              The vertex data.
 *    @param unsigned int size             The size of the vertex data.
 *    @param unsigned int stride           The stride of the vertex data.
 *    @param v_layout_t layout    The layout of the vertex data.
 *
 *    @return void *              The vertex buffer.
 */
void *vbuffer_create(void *v, unsigned int size, unsigned int stride, v_layout_t layout);

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
 *    Gets the amount of surfaces a mesh has
 *
 *    @param void *m              The mesh.
 *
 *    @return u32                 The amount of surfaces the mesh has
 */
u32 mesh_get_surface_count(void* m);

/*
 *    Sets the amount of surfaces a mesh has
 *
 *    @param void *m              The mesh.
 *    @param u32                  Set the amount of surfaces for the mesh
 *
 *    @return bool                Returns if it succeeded or failed
 */
bool mesh_set_surface_count(void *m, u32 count);

/*
 *    Sets the buffer data for a surfaces a mesh has
 *
 *    @param void *m              The mesh.
 *    @param u32                  The surface to set buffer data for
 *    @param u32                  How many vertices offset from the start this surface uses
 *    @param u32                  How many vertices this surfaces uses
 */
void mesh_set_surface_buffer_data(void *m, u32 surface, u32 offset, u32 size);

/*
 *    Gets a material on a mesh surface
 *
 *    @param void *m              The mesh.
 *    @param u32                  The surface to set buffer data for
 *
 *    @return material_t*         The material for this surface
 */
material_t* mesh_get_material(void *m, u32 surface);

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

/*
 *    Initializes the mesh system.
 */
void mesh_init();