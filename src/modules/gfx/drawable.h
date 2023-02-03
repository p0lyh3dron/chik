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

#include "libchik.h"

#include "image.h"

typedef struct {
    void      *buf;
    u32        stride;
    u32        size;
    v_layout_t layout;
} vbuffer_t;

typedef enum {
    MESHFLAGS_NONE           = 0,
    MESHFLAGS_DRAW_WIREFRAME = 1 << 0,
    /*
     *    Useful for a static mesh / font / etc.
     */
    MESHFLAGS_SKIP_PROJECTION = 1 << 1,
    /*
     *    Useful for a static mesh / font / etc.
     */
    MESHFLAGS_SKIP_CLIPPING = 1 << 2,
} meshflags_e;

typedef struct {
    meshflags_e aFlags;
    trap_t      aVBuf;
    /*
     *    Will be replaced with material_t *
     */
    trap_t aTex;
} mesh_t;

/*
 *    Initializes resources for below functions.
 *
 *    @return u32    Whether or not the initialization was successful.
 *                   1 = success, 0 = failure.
 */
u32 init_drawable_resources();

/*
 *    Creates a vertex buffer.
 *
 *    @param void *         The vertex data.
 *    @param u32            The size of the vertex data.
 *    @param u32            The stride of the vertex data.
 *    @param v_layout_t     The layout of the vertex data.
 *
 *    @return trap_t      The vertex buffer.
 *                          INVALID_TRAP if the vertex buffer could not be
 * created. The mesh should be freed with vbuffer_free().
 */
trap_t vbuffer_create(void *spVerts, u32 sSize, u32 sVStride,
                      v_layout_t sLayout);

/*
 *    Frees a vertex buffer.
 *
 *    @param trap_t    The vertex buffer to free.
 */
void vbuffer_free(trap_t sVBuffer);

/*
 *    Creates a texture from a file.
 *
 *    @param s8 *          The path to the texture file.
 *    @param u32           The format of the texture.
 *
 *    @return trap_t     The texture.
 */
trap_t texture_create_from_file(s8 *spPath, u32 sFormat);

/*
 *    Creates a texture from raw data ( must be in BRGA ).
 *
 *    @param void *        The raw data.
 *    @param u32           The width of the texture.
 *    @param u32           The height of the texture.
 *
 *    @return trap_t     The texture.
 */
trap_t texture_create_raw(void *spData, u32 sWidth, u32 sHeight);

/*
 *    Frees a texture.
 *
 *    @param trap_t    The texture to free.
 */
void texture_free(trap_t sTex);

/*
 *    Creates a mesh.
 *
 *    @param trap_t    The vertex buffer.
 *    @param trap_t    The texture.
 *
 *    @return trap_t      The mesh.
 */
trap_t mesh_create(trap_t sVBuffer, trap_t sTex);

/*
 *    Sets a mesh to not use the projection matrix.
 *
 *    @param trap_t    The mesh.
 */
void mesh_set_skip_projection(trap_t sMesh);

/*
 *    Sets a mesh to not be clipped.
 *
 *    @param trap_t    The mesh.
 */
void mesh_set_skip_clipping(trap_t sMesh);

/*
 *    Sets the vertex buffer of a mesh.
 *
 *    @param trap_t    The mesh.
 *    @param trap_t    The vertex buffer.
 */
void mesh_set_vertex_buffer(trap_t sMesh, trap_t sVBuffer);

/*
 *    Sets the texture of a mesh.
 *
 *    @param trap_t    The mesh.
 *    @param trap_t    The texture.
 */
void mesh_set_texture(trap_t sMesh, trap_t sTex);

/*
 *    Translates a mesh.
 *
 *    @param vec3_t      The translation vector.
 */
void mesh_translate(vec3_t sTranslation);

/*
 *    Rotates a mesh.
 *
 *    @param vec3_t      The rotation vector.
 */
void mesh_rotate(vec3_t sRotation);

/*
 *    Scales a mesh.
 *
 *    @param vec3_t      The scale vector.
 */
void mesh_scale(vec3_t sScale);

/*
 *    Draws a vertex buffer.
 *
 *    @param trap_t          The handle to the vertex buffer.
 *    @param meshflags_e       The flags to use for the mesh.
 */
void vbuffer_draw(trap_t sBuffer, meshflags_e sFlags);

/*
 *    Draws a mesh.
 *
 *    @param trap_t    The mesh.
 */
void mesh_draw(trap_t sMesh);

/*
 *    Frees a mesh.
 *
 *    @param trap_t      The mesh to free.
 */
void mesh_free(trap_t sMesh);