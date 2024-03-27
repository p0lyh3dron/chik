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
 *    Gets the amount of surfaces a mesh has
 *
 *    @param void *m              The mesh.
 *
 *    @return u32                 The amount of surfaces the mesh has
 */
u32 mesh_get_surface_count(void* m) {
    if (m == (void*)0x0) {
        LOGF_ERR("Mesh is null.\n");
        return 0;
    }

    mesh_t* mesh = (mesh_t*)m;
    return mesh->surface_count;
}

/*
 *    Sets the amount of surfaces a mesh has
 *
 *    @param void *m              The mesh.
 *    @param u32                  Set the amount of surfaces for the mesh
 */
bool mesh_set_surface_count(void* m, u32 count) {
    if (m == (void*)0x0) {
        LOGF_ERR("Mesh is null.\n");
        return false;
    }

    mesh_t* mesh = (mesh_t*)m;

    u32 old_count = mesh->surface_count;

    void* surfaces = realloc( mesh->surfaces, sizeof( mesh_surface_t ) * count );

    if (surfaces == CH_NULL) {
        free(mesh->surfaces);
        return false;
    }

    mesh->surfaces = surfaces;
    mesh->surface_count = count;

    // zero out memory of new surfaces
    for (u32 i = old_count; i < mesh->surface_count; i++) {
        memset(&mesh->surfaces[i], 0, sizeof(mesh_surface_t));
    }

    return true;
}

/*
 *    Sets the buffer data for a surfaces a mesh has
 *
 *    @param void *m              The mesh.
 *    @param u32                  The surface to set buffer data for
 *    @param u32                  How many vertices offset from the start this surface uses
 *    @param u32                  How many vertices this surfaces uses
 */
void mesh_set_surface_buffer_data(void* m, u32 surface, u32 offset, u32 size) {
    if (m == (void*)0x0) {
        LOGF_ERR("Mesh is null.\n");
        return;
    }

    mesh_t* mesh = (mesh_t*)m;

    if (mesh->surface_count == 0 || mesh->surfaces == 0x0) {
        LOGF_ERR("Mesh has no surfaces.\n");
        return;
    }

    if (surface > mesh->surface_count) {
        VLOGF_ERR("Mesh does not have %d surfaces, only %d\n", surface, mesh->surface_count);
        return;
    }

    mesh->surfaces[surface].offset = offset;
    mesh->surfaces[surface].size   = size;
}

/*
 *    Gets a material on a mesh surface
 *
 *    @param void *m              The mesh.
 *    @param u32                  The surface to set buffer data for
 *
 *    @return material_t*         The material for this surface
 */
material_t* mesh_get_material(void* m, u32 surface) {
    if (m == (void*)0x0) {
        LOGF_ERR("Mesh is null.\n");
        return CH_NULL;
    }

    mesh_t* mesh = (mesh_t*)m;

    if (mesh->surface_count == 0 || mesh->surfaces == 0x0) {
        LOGF_ERR("Mesh has no surfaces.\n");
        return CH_NULL;
    }

    if (surface > mesh->surface_count) {
        VLOGF_ERR("Mesh does not have %d surfaces, only %d\n", surface, mesh->surface_count);
        return CH_NULL;
    }

    return &mesh->surfaces[surface].material;
}

/*
 *    Rasterizes a mesh multithreaded
 *
 *    @param void *m    The mesh.
 *    @param     The mesh.
 */

void mesh_surface_raster_threaded(unsigned char* a0, unsigned char* b0, unsigned char* c0, char* assets, material_t* material) {
    triangle_t* pTri = (triangle_t*)malloc(sizeof(triangle_t));
    pTri->v0 = malloc(VERTEX_ASM_MAX_VERTEX_SIZE);
    pTri->v1 = malloc(VERTEX_ASM_MAX_VERTEX_SIZE);
    pTri->v2 = malloc(VERTEX_ASM_MAX_VERTEX_SIZE);
    pTri->assets = assets;
    pTri->material = material;

    memcpy(pTri->v0, a0, VERTEX_ASM_MAX_VERTEX_SIZE);
    memcpy(pTri->v1, b0, VERTEX_ASM_MAX_VERTEX_SIZE);
    memcpy(pTri->v2, c0, VERTEX_ASM_MAX_VERTEX_SIZE);
    threadpool_submit(raster_rasterize_triangle_thread, (void*)pTri);
}

char* (*mesh_surface_raster_func)(unsigned char*, unsigned char*, unsigned char*, char* assets, material_t* material) = 0;

/*
 *    Draws a mesh surface.
 *
 *    @param void *m    The mesh.
 *    @param     The mesh.
 */
void mesh_surface_draw(mesh_t* mesh, mesh_surface_t* surface) {
    vbuffer_t* buf = mesh->vbuf;

    char* buffer = buf->buf + (surface->offset * buf->stride);

    unsigned int num_verts = surface->size;

    vertexasm_set_layout(buf->layout);

    for (unsigned int i = 0; i < num_verts; i += 3) {
        unsigned char a0[VERTEX_ASM_MAX_VERTEX_SIZE];
        unsigned char b0[VERTEX_ASM_MAX_VERTEX_SIZE];
        unsigned char c0[VERTEX_ASM_MAX_VERTEX_SIZE];

        void* a = buffer + (i + 0) * buf->stride;
        void* b = buffer + (i + 1) * buf->stride;
        void* c = buffer + (i + 2) * buf->stride;

        /*
         *    Copy the vertex data into a buffer.
         */
        memcpy(a0, a, buf->stride);
        memcpy(b0, b, buf->stride);
        memcpy(c0, c, buf->stride);

        /*
         *    Apply the vertex shader.
         *    TODO: Do this check earlier
         */
        if (buf->layout.v_fun != (void*)0x0) {
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

        unsigned char* new_verts = cull_clip_triangle(a0, b0, c0, &clipped_vertices, 1);

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
            mesh_surface_raster_func(a0, b0, c0, mesh->assets, &surface->material);
        }
    }
    //threadpool_wait();
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

    if (mesh->surface_count == 0 || mesh->surfaces == 0x0) {
        LOGF_ERR("Mesh has no surfaces.\n");
        return;
    }

    if (mesh->vbuf == (vbuffer_t*)0x0) {
        LOGF_ERR("Vertex buffer is null.\n");
        return;
    }

    for ( u32 i = 0; i < mesh->surface_count; i++ ) {
        mesh_surface_draw(mesh, &mesh->surfaces[i]);
    }
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

    if (mesh->surfaces != (void*)0x0) {
        for (u32 i = 0; i < mesh->surface_count; i++) {
            if (mesh->surfaces[i].material.albedo) {

            }
        }

        free(mesh->surfaces);
    }

    free(mesh);
}

void mesh_init() {
    if (args_has("--multithreaded-render")) {
        mesh_surface_raster_func = mesh_surface_raster_threaded;
    }
    else {
        mesh_surface_raster_func = raster_rasterize_triangle;
    }
}