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
 *    @param unsigned int size             The size of the vertex data.
 *    @param unsigned int stride           The stride of the vertex data.
 *    @param v_layout_t layout    The layout of the vertex data.
 *
 *    @return void *              The vertex buffer.
 */
void *vbuffer_create(void *v, unsigned int size, unsigned int stride, v_layout_t layout) {
    vbuffer_t *buf;

    if (v == nullptr) {
        LOGF_ERR("Vertex data is null.\n");
        return (void *)0x0;
    }
    if (size == 0) {
        LOGF_ERR("Vertex data size is zero.\n");
        return (void *)0x0;
    }
    if (stride == 0) {
        LOGF_ERR("Vertex data stride is zero.\n");
        return (void *)0x0;
    }

    buf = (vbuffer_t *)malloc(sizeof(vbuffer_t));

    if (buf == (vbuffer_t *)0x0) {
        LOGF_ERR("Could not allocate vertex buffer.\n");
        return (void *)0x0;
    }

    buf->buf    = malloc(size);
    buf->size   = size;
    buf->stride = stride;
    buf->layout = layout;

    if (buf->buf == (void *)0x0) {
        log_error("Could not allocate vertex buffer.\n");
        return (void *)0x0;
    }

    memcpy(buf->buf, v, size);

    return (void *)buf;
}

/*
 *    Frees a vertex buffer.
 *
 *    @param void *buf   The vertex buffer to free.
 */
void vbuffer_free(void *buf) {
    if (buf == (void *)0x0) {
        LOGF_ERR("Vertex buffer pointer is null.\n");
        return;
    }

    vbuffer_t *vbuf = (vbuffer_t *)buf;

    if (vbuf->buf == nullptr) {
        LOGF_ERR("Vertex buffer is null.\n");
        return;
    }

    free(vbuf->buf);
    free(buf);
}

/*
 *    Creates a mesh.
 *
 *    @param void *v    The vertex buffer.
 *
 *    @return void *    The mesh.
 */
void *mesh_create(void *v) {
    size_t i;
    mesh_t       *mesh = (mesh_t *)malloc(sizeof(mesh_t));

    if (mesh == (mesh_t *)0x0) {
        LOGF_ERR("Could not allocate mesh.\n");
        return (void *)0x0;
    }

    memset(mesh, 0, sizeof(mesh_t));

    mesh->vbuf         = (vbuffer_t *)v;
    mesh->assets       = (void *)0x0;
    mesh->assets_size  = CHIK_GFX_DRAWABLE_MESH_MAX_ASSETS * 8;
    mesh->assets_count = 0;

    return (void *)mesh;
}

/*
 *    Sets the vertex buffer of a mesh.
 *
 *    @param void *m    The mesh.
 *    @param void *v    The vertex buffer.
 */
void mesh_set_vbuffer(void *m, void *v) {
    if (m == (void *)0x0) {
        LOGF_ERR("Mesh is null.\n");
        return;
    }
    if (v == (void *)0x0) {
        LOGF_ERR("Vertex buffer is null.\n");
        return;
    }

    mesh_t *mesh = (mesh_t *)m;
    mesh->vbuf   = (vbuffer_t *)v;
}

/*
 *    Appends an asset to a mesh.
 *
 *    @param void *m              The mesh.
 *    @param void *a              The asset.
 *    @param size_t size   The size of the asset.
 */
void mesh_append_asset(void *m, void *a, size_t size) {
    size_t j;
    size_t index;
    size_t offset = 8 * CHIK_GFX_DRAWABLE_MESH_MAX_ASSETS;

    if (m == (void *)0x0) {
        LOGF_ERR("Mesh is null.\n");
        return;
    }
    if (a == (void *)0x0) {
        LOGF_ERR("Asset is null.\n");
        return;
    }

    mesh_t *mesh = (mesh_t *)m;
    mesh->assets = realloc(mesh->assets, mesh->assets_size + size + offset);

    if (mesh->assets == (void *)0x0) {
        LOGF_ERR("Could not allocate mesh asset.\n");
        return;
    }

    memset(mesh->assets, 0, mesh->assets_size + size + offset);

    auto value = _heapchk();

    memcpy((void *)((size_t)mesh->assets + mesh->assets_count * 8), &mesh->assets_size, sizeof(mesh->assets_size));
    memcpy((void *)((size_t)mesh->assets + mesh->assets_size), a, size);
    mesh->assets_size += size;
    mesh->assets_count++;
}

/*
 *    Sets the assets of a mesh.
 *
 *    @param void *m              The mesh.
 *    @param void *a              The asset.
 *    @param size_t size   The size of the asset.
 *    @param size_t i      The index of the asset.
 */
void mesh_set_asset(void *m, void *a, size_t size, size_t i) {
    size_t j;
    size_t offset;

    if (m == (void *)0x0) {
        LOGF_ERR("Mesh is null.\n");
        return;
    }
    if (a == (void *)0x0) {
        LOGF_ERR("Asset is null.\n");
        return;
    }

    mesh_t *mesh = (mesh_t *)m;

    offset = *(size_t *)((size_t)mesh->assets + i * 8);

    memcpy((void *)((size_t)mesh->assets + offset), a, size);
}

/*
 *    Returns the data of an asset.
 *
 *    @param void *a            The assets.
 *    @param size_t i    The index of the asset.
 *
 *    @return void *            The asset data.
 */
void *mesh_get_asset(void *a, size_t i) {
    if (a == (void *)0x0) {
        LOGF_ERR("Assets are null.\n");
        return (void *)0x0;
    }

    size_t offset = *(size_t *)((size_t)a + i * 8);

    return (void *)((size_t)a + offset);
}

/*
 *    Draws a mesh.
 *
 *    @param void *m    The mesh.
 */
void mesh_draw(void *m) {
    mesh_t *mesh = (mesh_t *)m;

    if (mesh == (mesh_t *)0x0) {
        LOGF_ERR("Mesh is null.\n");
        return;
    }

    vbuffer_t *buf = mesh->vbuf;

    if (buf == (vbuffer_t *)0x0) {
        LOGF_ERR("Vertex buffer is null.\n");
        return;
    }

    unsigned int num_verts = buf->size / buf->stride;

    vertexasm_set_layout(buf->layout);

    for (unsigned int i = 0; i < num_verts; i += 3) {
        unsigned char a0[VERTEX_ASM_MAX_VERTEX_SIZE];
        unsigned char b0[VERTEX_ASM_MAX_VERTEX_SIZE];
        unsigned char c0[VERTEX_ASM_MAX_VERTEX_SIZE];

        void *a = buf->buf + (i + 0) * buf->stride;
        void *b = buf->buf + (i + 1) * buf->stride;
        void *c = buf->buf + (i + 2) * buf->stride;

        /*
         *    Copy the vertex data into a buffer.
         */
        memcpy(a0, a, buf->stride);
        memcpy(b0, b, buf->stride);
        memcpy(c0, c, buf->stride);

        /*
         *    Apply the vertex shader.
         */
        if (buf->layout.v_fun != (void *)0x0) {
            buf->layout.v_fun(a0, a, mesh->assets);
            buf->layout.v_fun(b0, b, mesh->assets);
            buf->layout.v_fun(c0, c, mesh->assets);
        }

        /*
         *    If the vertex is outside of the view frustum, use
         *    linear interpolation to find the point on the triangle
         *    that is inside the view frustum.
         */
        int clipped_vertices = 0;

        unsigned char *new_verts = cull_clip_triangle(a0, b0, c0, &clipped_vertices, 1);

        /*
         *    Draw the clipped vertices.
         */
        for (long j = 0; j < clipped_vertices - 2; ++j) {
            memcpy(a0, new_verts + (0 + 0) * VERTEX_ASM_MAX_VERTEX_SIZE,
                   buf->stride);
            memcpy(b0, new_verts + (j + 1) * VERTEX_ASM_MAX_VERTEX_SIZE,
                   buf->stride);
            memcpy(c0, new_verts + (j + 2) * VERTEX_ASM_MAX_VERTEX_SIZE,
                   buf->stride);

            vertex_perspective_divide(a0);
            vertex_perspective_divide(b0);
            vertex_perspective_divide(c0);

            /*
             *    Draw the triangle.
             */
            if (args_has("--multithreaded-render")) {
                triangle_t *pTri = (triangle_t *)malloc(sizeof(triangle_t));
                pTri->v0         = malloc(VERTEX_ASM_MAX_VERTEX_SIZE);
                pTri->v1         = malloc(VERTEX_ASM_MAX_VERTEX_SIZE);
                pTri->v2         = malloc(VERTEX_ASM_MAX_VERTEX_SIZE);
                pTri->assets     = mesh->assets;

                memcpy(pTri->v0, a0, VERTEX_ASM_MAX_VERTEX_SIZE);
                memcpy(pTri->v1, b0, VERTEX_ASM_MAX_VERTEX_SIZE);
                memcpy(pTri->v2, c0, VERTEX_ASM_MAX_VERTEX_SIZE);
                threadpool_submit(raster_rasterize_triangle_thread, (void *)pTri);
            }
            else {
                raster_rasterize_triangle(a0, b0, c0, mesh->assets);
            }
        }
    }
    //threadpool_wait();
}

/*
 *    Frees a mesh.
 *
 *    @param void *m    The mesh.
 */
void mesh_free(void *m) {
    if (m == (void *)0x0) {
        LOGF_ERR("Mesh is null.\n");
        return;
    }

    mesh_t *mesh = (mesh_t *)m;

    if (mesh->assets != (void *)0x0)
        free(mesh->assets);

    free(mesh);
}