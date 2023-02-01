/*
 *    drawable.c    --    source for general drawable functionality
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on June 7, 2022.
 *
 *    This file is part of the Chik engine.
 *
 *    The usage defined here is for drawables defined in the header.
 */
#include "drawable.h"

#include <string.h>

#include "gfx.h"

#include "camera.h"
#include "cull.h"
#include "raster.h"
#include "vertexasm.h"

/*
 *    Creates a vertex buffer.
 *
 *    @param void *v              The vertex data.
 *    @param u32 size             The size of the vertex data.
 *    @param u32 stride           The stride of the vertex data.
 *    @param v_layout_t layout    The layout of the vertex data.
 *
 *    @return trap_t      The vertex buffer.
 *                          INVALID_TRAP if the vertex buffer could not be
 * created. The mesh should be freed with vbuffer_free().
 */
trap_t vbuffer_create(void *v, u32 size, u32 stride,
                      v_layout_t layout) {
                        vbuffer_t buf;
                        trap_t h;
    if (v == nullptr) {
        LOGF_ERR("Vertex data is null.");
        return INVALID_TRAP;
    }
    if (size == 0) {
        LOGF_ERR("Vertex data size is zero.");
        return INVALID_TRAP;
    }
    if (stride == 0) {
        LOGF_ERR("Vertex data stride is zero.");
        return INVALID_TRAP;
    }

    buf.buf = malloc(size);
    buf.size = size;
    buf.stride = stride;
    buf.layout = layout;

    if (buf.buf == nullptr) {
        log_error("Could not allocate vertex buffer.");
        return INVALID_TRAP;
    }

    memcpy(buf.buf, v, size);

    h = resource_add(_handles, &buf, sizeof(vbuffer_t));

    if (BAD_TRAP(h)) {
        log_error("Could not add vertex buffer to resource list.");
        return INVALID_TRAP;
    }

    return h;
}

/*
 *    Frees a vertex buffer.
 *
 *    @param trap_t    The vertex buffer to free.
 */
void vbuffer_free(trap_t sVBuffer) {
    if (_handles == nullptr) {
        log_error("Resources not initialized.");
        return;
    }
    if (BAD_TRAP(sVBuffer)) {
        log_error("Vertex buffer is null.");
        return;
    }

    vbuffer_t *pVBuffer = resource_get(_handles, sVBuffer);
    if (pVBuffer == nullptr) {
        log_error("Vertex buffer is null.");
        return;
    }

    free(pVBuffer->buf);
}

/*
 *    Creates a texture from a file.
 *
 *    @param s8 *          The path to the texture file.
 *    @param u32           The format of the texture.
 *
 *    @return trap_t     The texture.
 */
trap_t texture_create_from_file(s8 *spPath, u32 sFormat) {
    if (_handles == nullptr) {
        log_error("Resources not initialized.");
        return INVALID_TRAP;
    }
    if (spPath == nullptr) {
        log_error("Texture path is null.");
        return INVALID_TRAP;
    }

    texture_t tex = {};
    tex.image = image_create_from_file(spPath, sFormat);

    if (tex.image == nullptr) {
        log_error("Could not create texture from file.");
        return INVALID_TRAP;
    }

    trap_t h = resource_add(_handles, &tex, sizeof(texture_t));

    if (BAD_TRAP(h)) {
        log_error("Could not add texture to resource list.");
        return INVALID_TRAP;
    }

    return h;
}

/*
 *    Creates a texture from raw data ( must be in BRGA ).
 *
 *    @param void *        The raw data.
 *    @param u32           The width of the texture.
 *    @param u32           The height of the texture.
 *
 *    @return trap_t     The texture.
 */
trap_t texture_create_raw(void *spData, u32 sWidth, u32 sHeight) {
    if (_handles == nullptr) {
        log_error("trap_t texture_create_raw( void*, u32, u32 ): Resources not "
                  "initialized.\n");
        return INVALID_TRAP;
    }
    if (spData == nullptr) {
        log_error("trap_t texture_create_raw( void*, u32, u32 ): Texture data "
                  "is null.\n");
        return INVALID_TRAP;
    }

    texture_t tex = {};
    tex.image = malloc(sizeof(image_t));

    if (tex.image == nullptr) {
        log_error("trap_t texture_create_raw( void*, u32, u32 ): Could not "
                  "create texture from raw data.\n");
        return INVALID_TRAP;
    }

    tex.image->buf = malloc(sWidth * sHeight * 4);

    if (tex.image->buf == nullptr) {
        log_error("trap_t texture_create_raw( void*, u32, u32 ): Could not "
                  "create texture from raw data.\n");
        return INVALID_TRAP;
    }

    memcpy(tex.image->buf, spData, sWidth * sHeight * 4);

    trap_t h = resource_add(_handles, &tex, sizeof(texture_t));

    if (BAD_TRAP(h)) {
        log_error("trap_t texture_create_raw( void*, u32, u32 ): Could not add "
                  "texture to resource list.\n");
        return INVALID_TRAP;
    }

    return h;
}

/*
 *    Frees a texture.
 *
 *    @param trap_t    The texture to free.
 */
void texture_free(trap_t sTex) {
    if (_handles == nullptr) {
        log_error("Resources not initialized.");
        return;
    }
    if (BAD_TRAP(sTex)) {
        log_error("Texture is null.");
        return;
    }

    texture_t *pTex = resource_get(_handles, sTex);
    if (pTex == nullptr) {
        log_error("Texture is null.");
        return;
    }

    image_free(pTex->image);
    resource_remove(_handles, sTex);
}

/*
 *    Creates a mesh.
 *
 *    @param trap_t    The vertex buffer.
 *    @param trap_t    The texture.
 *
 *    @return trap_t      The mesh.
 */
trap_t mesh_create(trap_t sVBuffer, trap_t sTex) {
    mesh_t mesh = {};
    mesh.aFlags = MESHFLAGS_NONE;
    mesh.aVBuf = sVBuffer;
    mesh.aTex = sTex;

    trap_t h = resource_add(_handles, &mesh, sizeof(mesh_t));

    if (BAD_TRAP(h)) {
        log_error("void mesh_create( trap_t, trap_t ): Could not add mesh to "
                  "resource list.\n");
        return INVALID_TRAP;
    }

    return h;
}

/*
 *    Sets a mesh to not use the projection matrix.
 *
 *    @param trap_t    The mesh.
 */
void mesh_set_skip_projection(trap_t sMesh) {
    if (_handles == nullptr) {
        log_error("void mesh_set_skip_projection( trap_t ): Resources not "
                  "initialized.\n");
        return;
    }
    if (BAD_TRAP(sMesh)) {
        log_error("void mesh_set_skip_projection( trap_t ): Mesh is null.\n");
        return;
    }

    mesh_t *pMesh = resource_get(_handles, sMesh);
    if (pMesh == nullptr) {
        log_error("void mesh_set_skip_projection( trap_t ): Mesh is null.\n");
        return;
    }
    pMesh->aFlags |= MESHFLAGS_SKIP_PROJECTION;
}

/*
 *    Sets a mesh to not be clipped.
 *
 *    @param trap_t    The mesh.
 */
void mesh_set_skip_clipping(trap_t sMesh) {
    if (_handles == nullptr) {
        log_error("void mesh_set_skip_clipping( trap_t ): Resources not "
                  "initialized.\n");
        return;
    }
    if (BAD_TRAP(sMesh)) {
        log_error("void mesh_set_skip_clipping( trap_t ): Mesh is null.\n");
        return;
    }

    mesh_t *pMesh = resource_get(_handles, sMesh);
    if (pMesh == nullptr) {
        log_error("void mesh_set_skip_clipping( trap_t ): Mesh is null.\n");
        return;
    }
    pMesh->aFlags |= MESHFLAGS_SKIP_CLIPPING;
}

/*
 *    Sets the vertex buffer of a mesh.
 *
 *    @param trap_t    The mesh.
 *    @param trap_t    The vertex buffer.
 */
void mesh_set_vertex_buffer(trap_t sMesh, trap_t sVBuffer) {
    if (_handles == nullptr) {
        log_error("void mesh_set_vertex_buffer( trap_t, trap_t ): Resources "
                  "not initialized.\n");
        return;
    }
    if (BAD_TRAP(sMesh)) {
        log_error(
            "void mesh_set_vertex_buffer( trap_t, trap_t ): Mesh is null.\n");
        return;
    }
    if (BAD_TRAP(sVBuffer)) {
        log_error("void mesh_set_vertex_buffer( trap_t, trap_t ): Vertex "
                  "buffer is null.\n");
        return;
    }

    mesh_t *pMesh = resource_get(_handles, sMesh);
    if (pMesh == nullptr) {
        log_error(
            "void mesh_set_vertex_buffer( trap_t, trap_t ): Mesh is null.\n");
        return;
    }
    vbuffer_t *pVBuffer = resource_get(_handles, sVBuffer);
    if (pVBuffer == nullptr) {
        log_error("void mesh_set_vertex_buffer( trap_t, trap_t ): Vertex "
                  "buffer is null.\n");
        return;
    }

    pMesh->aVBuf = sVBuffer;
}

/*
 *    Sets the texture of a mesh.
 *
 *    @param trap_t    The mesh.
 *    @param trap_t    The texture.
 */
void mesh_set_texture(trap_t sMesh, trap_t sTex) {
    if (_handles == nullptr) {
        log_error("void mesh_set_texture( trap_t, trap_t ): Resources not "
                  "initialized.\n");
        return;
    }
    if (BAD_TRAP(sMesh)) {
        log_error("void mesh_set_texture( trap_t, trap_t ): Mesh is null.\n");
        return;
    }
    if (BAD_TRAP(sTex)) {
        log_error(
            "void mesh_set_texture( trap_t, trap_t ): Texture is null.\n");
        return;
    }

    mesh_t *pMesh = resource_get(_handles, sMesh);
    if (pMesh == nullptr) {
        log_error("void mesh_set_texture( trap_t, trap_t ): Mesh is null.\n");
        return;
    }

    pMesh->aTex = sTex;
}

vec3_t gMeshTranslate = {0.f, 0.f, 0.f};
vec3_t gMeshRotate = {0.f, 0.f, 0.f};
vec3_t gMeshScale = {1.f, 1.f, 1.f};

/*
 *    Translates a mesh.
 *
 *    @param vec3_t      The translation vector.
 */
void mesh_translate(vec3_t sTranslation) { gMeshTranslate = sTranslation; }

/*
 *    Rotates a mesh.
 *
 *    @param vec3_t      The rotation vector.
 */
void mesh_rotate(vec3_t sRotation) { gMeshRotate = sRotation; }

/*
 *    Scales a mesh.
 *
 *    @param vec3_t      The scale vector.
 */
void mesh_scale(vec3_t sScale) { gMeshScale = sScale; }

/*
 *    Draws a vertex buffer.
 *
 *    @param trap_t          The handle to the vertex buffer.
 *    @param meshflags_e       The flags to use for the mesh.
 */
void vbuffer_draw(trap_t sBuffer, meshflags_e sFlags) {
    if (_camera == nullptr) {
        log_error("void vbuffer_draw( trap_t, meshflags_e ): No camera.\n");
        return;
    }

    vbuffer_t *pBuf = resource_get(_handles, sBuffer);
    if (pBuf == nullptr) {
        log_error("void vbuffer_draw( trap_t, meshflags_e ): Failed to get "
                  "vertex resource.\n");
        return;
    }

    mat4_t view = camera_view(_camera);
    u32 numVerts = pBuf->size / pBuf->stride;

    vertexasm_set_layout(pBuf->layout);

    for (unsigned long i = 0; i < numVerts; i += 3) {
        u8 a0[VERTEX_ASM_MAX_VERTEX_SIZE];
        u8 b0[VERTEX_ASM_MAX_VERTEX_SIZE];
        u8 c0[VERTEX_ASM_MAX_VERTEX_SIZE];

        /*
         *    Copy the vertex data into a buffer.
         */
        memcpy(a0, pBuf->buf + (i + 0) * pBuf->stride, pBuf->stride);
        memcpy(b0, pBuf->buf + (i + 1) * pBuf->stride, pBuf->stride);
        memcpy(c0, pBuf->buf + (i + 2) * pBuf->stride, pBuf->stride);

        /*
         *    Get the positions of the vertices.
         */
        vec4_t pa = vertex_get_position(a0);
        pa.x *= gMeshScale.x;
        pa.y *= gMeshScale.y;
        pa.z *= gMeshScale.z;
        vec4_t pb = vertex_get_position(b0);
        pb.x *= gMeshScale.x;
        pb.y *= gMeshScale.y;
        pb.z *= gMeshScale.z;
        vec4_t pc = vertex_get_position(c0);
        pc.x *= gMeshScale.x;
        pc.y *= gMeshScale.y;
        pc.z *= gMeshScale.z;

        /*
         *    Transform the positions.
         *
         *    If we have an active translation/rotation, apply it.
         */

        mat4_t ta =
            m4_mul_v4(m4_rotate(gMeshRotate.x, (vec3_t){1.f, 0.f, 0.f}), pa);
        ta = m4_mul_m4(m4_rotate(gMeshRotate.y, (vec3_t){0.f, 1.f, 0.f}), ta);
        ta = m4_mul_m4(m4_rotate(gMeshRotate.z, (vec3_t){0.f, 0.f, 1.f}), ta);
        ta = m4_mul_m4(m4_translate(gMeshTranslate), ta);

        mat4_t tb =
            m4_mul_v4(m4_rotate(gMeshRotate.x, (vec3_t){1.f, 0.f, 0.f}), pb);
        tb = m4_mul_m4(m4_rotate(gMeshRotate.y, (vec3_t){0.f, 1.f, 0.f}), tb);
        tb = m4_mul_m4(m4_rotate(gMeshRotate.z, (vec3_t){0.f, 0.f, 1.f}), tb);
        tb = m4_mul_m4(m4_translate(gMeshTranslate), tb);

        mat4_t tc =
            m4_mul_v4(m4_rotate(gMeshRotate.x, (vec3_t){1.f, 0.f, 0.f}), pc);
        tc = m4_mul_m4(m4_rotate(gMeshRotate.y, (vec3_t){0.f, 1.f, 0.f}), tc);
        tc = m4_mul_m4(m4_rotate(gMeshRotate.z, (vec3_t){0.f, 0.f, 1.f}), tc);
        tc = m4_mul_m4(m4_translate(gMeshTranslate), tc);

        mat4_t ma = m4_identity();
        mat4_t mb = m4_identity();
        mat4_t mc = m4_identity();

        if (!(sFlags & MESHFLAGS_SKIP_PROJECTION)) {
            ma = m4_mul_m4(view, ta);
            mb = m4_mul_m4(view, tb);
            mc = m4_mul_m4(view, tc);
        } else {
            ma = ta;
            mb = tb;
            mc = tc;
        }

        /*
         *    Update the vertex data.
         */
        vertex_set_position(a0, (vec4_t){ma.v[0], ma.v[4], ma.v[8], ma.v[12]});
        vertex_set_position(b0, (vec4_t){mb.v[0], mb.v[4], mb.v[8], mb.v[12]});
        vertex_set_position(c0, (vec4_t){mc.v[0], mc.v[4], mc.v[8], mc.v[12]});

        /*
         *    If the vertex is outside of the view frustum, use
         *    linear interpolation to find the point on the triangle
         *    that is inside the view frustum.
         */
        s32 numVertices = 0;

        u8 *pVerts = cull_clip_triangle(a0, b0, c0, &numVertices,
                                        !(sFlags & MESHFLAGS_SKIP_CLIPPING));

        /*
         *    Draw the clipped vertices.
         */
        for (s64 i = 0; i < numVertices - 2; ++i) {
            memcpy(a0, pVerts + (0 + 0) * VERTEX_ASM_MAX_VERTEX_SIZE,
                   pBuf->stride);
            memcpy(b0, pVerts + (i + 1) * VERTEX_ASM_MAX_VERTEX_SIZE,
                   pBuf->stride);
            memcpy(c0, pVerts + (i + 2) * VERTEX_ASM_MAX_VERTEX_SIZE,
                   pBuf->stride);

            if (!(sFlags & MESHFLAGS_SKIP_PROJECTION)) {
                /*
                 *    Transform the vertex to screen space.
                 */
                pa = vertex_get_position(a0);
                pb = vertex_get_position(b0);
                pc = vertex_get_position(c0);

                pa.x /= pa.w;
                pa.y /= pa.w;

                pb.x /= pb.w;
                pb.y /= pb.w;

                pc.x /= pc.w;
                pc.y /= pc.w;

                vertex_set_position(a0, pa);
                vertex_set_position(b0, pb);
                vertex_set_position(c0, pc);
            }

            /*
             *    Draw the triangle.
             */
            triangle_t *pTri = (triangle_t *)malloc(sizeof(triangle_t));
            pTri->v0 = malloc(VERTEX_ASM_MAX_VERTEX_SIZE);
            pTri->v1 = malloc(VERTEX_ASM_MAX_VERTEX_SIZE);
            pTri->v2 = malloc(VERTEX_ASM_MAX_VERTEX_SIZE);

            memcpy(pTri->v0, a0, VERTEX_ASM_MAX_VERTEX_SIZE);
            memcpy(pTri->v1, b0, VERTEX_ASM_MAX_VERTEX_SIZE);
            memcpy(pTri->v2, c0, VERTEX_ASM_MAX_VERTEX_SIZE);
            threadpool_submit(raster_rasterize_triangle_thread, (void *)pTri);
        }
    }
    threadpool_wait();
}

/*
 *    Draws a mesh.
 *
 *    @param trap_t    The mesh.
 */
void mesh_draw(trap_t sMesh) {
    if (_handles == nullptr) {
        log_error("void mesh_draw( trap_t ): Resources not initialized.\n");
        return;
    }
    if (BAD_TRAP(sMesh)) {
        log_error("void mesh_draw( trap_t ): Mesh is null.\n");
        return;
    }

    mesh_t *pMesh = resource_get(_handles, sMesh);
    if (pMesh == nullptr) {
        log_error("void mesh_draw( trap_t ): Mesh is null.\n");
        return;
    }

    texture_t *pTex = resource_get(_handles, pMesh->aTex);
    if (pTex == nullptr) {
        log_error("void mesh_draw( trap_t ): Texture is null.\n");
        return;
    }

    image_t *pImage = pTex->image;
    if (pImage == nullptr) {
        log_error("void mesh_draw( trap_t ): Image is null.\n");
        return;
    }

    vertexasm_bind_uniform(pTex);
    vbuffer_draw(pMesh->aVBuf, pMesh->aFlags);
}

/*
 *    Frees a mesh.
 *
 *    @param trap_t      The mesh to free.
 */
void mesh_free(trap_t sMesh) {
    if (_handles == nullptr) {
        log_error("Resources not initialized.");
        return;
    }
    if (BAD_TRAP(sMesh)) {
        log_error("Mesh is null.");
        return;
    }
}