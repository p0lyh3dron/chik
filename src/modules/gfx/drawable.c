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

#include "camera.h"
#include "vertexasm.h"
#include "raster.h"
#include "cull.h"

resource_t *gpResources = nullptr;
mempool_t  *gpMempool   = nullptr;

/*
 *    Initializes resources for below functions.
 *
 *    @return u32    Whether or not the initialization was successful.
 *                   1 = success, 0 = failure.
 */
u32 init_drawable_resources() {
    gpResources = resource_new( 64 * 1024 * 1000 );
    gpMempool   = mempool_new( 64 * 1024 * 1000 );
    if ( gpResources == nullptr ) {
        return 0;
    }
    if ( gpMempool == nullptr ) {
        return 0;
    }
    return 1;
}

/*
 *    Creates a vertex buffer.
 *
 *    @param void *         The vertex data.
 *    @param u32            The size of the vertex data.
 *    @param u32            The stride of the vertex data.
 *    @param v_layout_t     The layout of the vertex data.
 * 
 *    @return handle_t      The vertex buffer.
 *                          INVALID_HANDLE if the vertex buffer could not be created.
 *                          The mesh should be freed with vbuffer_free().
 */
handle_t vbuffer_create( void *spVerts, u32 sSize, u32 sVStride, v_layout_t sLayout ) {
    if ( gpResources == nullptr ) {
        log_error( "Resources not initialized." );
        return INVALID_HANDLE;
    }
    if ( gpMempool == nullptr ) {
        log_error( "Memory pool not initialized." );
        return INVALID_HANDLE;
    }
    if ( spVerts == nullptr ) {
        log_error( "Vertex data is null." );
        return INVALID_HANDLE;
    }
    if ( sSize == 0 ) {
        log_error( "Vertex data size is zero." );
        return INVALID_HANDLE;
    }
    if ( sVStride == 0 ) {
        log_error( "Vertex data stride is zero." );
        return INVALID_HANDLE;
    }

    vbuffer_t buf = {};
    buf.apData     = mempool_alloc( gpMempool, sSize );
    buf.aSize      = sSize;
    buf.aVStride   = sVStride;
    buf.aLayout    = sLayout;

    if ( buf.apData == nullptr ) {
        log_error( "Could not allocate vertex buffer." );
        return INVALID_HANDLE;
    }

    memcpy( buf.apData, spVerts, sSize );

    handle_t h = resource_add( gpResources, &buf, sizeof( vbuffer_t ) );

    if ( h == INVALID_HANDLE ) {
        log_error( "Could not add vertex buffer to resource list." );
        return INVALID_HANDLE;
    }
    
    return h;
}

/*
 *    Frees a vertex buffer.
 *
 *    @param handle_t    The vertex buffer to free.
 */
void vbuffer_free( handle_t sVBuffer ) {
    if ( gpResources == nullptr ) {
        log_error( "Resources not initialized." );
        return;
    }
    if ( gpMempool == nullptr ) {
        log_error( "Memory pool not initialized." );
        return;
    }
    if ( sVBuffer == INVALID_HANDLE ) {
        log_error( "Vertex buffer is null." );
        return;
    }

    vbuffer_t *pVBuffer = resource_get( gpResources, sVBuffer );
    if ( pVBuffer == nullptr ) {
        log_error( "Vertex buffer is null." );
        return;
    }

    mempool_free( gpMempool, pVBuffer->apData );
    resource_remove( gpResources, sVBuffer );
}

/*
 *    Creates a texture from a file.
 *
 *    @param s8 *          The path to the texture file.
 *    @param u32           The format of the texture.
 * 
 *    @return handle_t     The texture.
 */
handle_t texture_create_from_file( s8 *spPath, u32 sFormat ) {
    if ( gpResources == nullptr ) {
        log_error( "Resources not initialized." );
        return INVALID_HANDLE;
    }
    if ( gpMempool == nullptr ) {
        log_error( "Memory pool not initialized." );
        return INVALID_HANDLE;
    }
    if ( spPath == nullptr ) {
        log_error( "Texture path is null." );
        return INVALID_HANDLE;
    }

    texture_t tex = {};
    tex.apImage = image_create_from_file( spPath, sFormat );

    if ( tex.apImage == nullptr ) {
        log_error( "Could not create texture from file." );
        return INVALID_HANDLE;
    }

    handle_t h = resource_add( gpResources, &tex, sizeof( texture_t ) );

    if ( h == INVALID_HANDLE ) {
        log_error( "Could not add texture to resource list." );
        return INVALID_HANDLE;
    }

    return h;
}

/*
 *    Frees a texture.
 *
 *    @param handle_t    The texture to free.
 */
void texture_free( handle_t sTex ) {
    if ( gpResources == nullptr ) {
        log_error( "Resources not initialized." );
        return;
    }
    if ( gpMempool == nullptr ) {
        log_error( "Memory pool not initialized." );
        return;
    }
    if ( sTex == INVALID_HANDLE ) {
        log_error( "Texture is null." );
        return;
    }

    texture_t *pTex = resource_get( gpResources, sTex );
    if ( pTex == nullptr ) {
        log_error( "Texture is null." );
        return;
    }

    image_free( pTex->apImage );
    resource_remove( gpResources, sTex );
}

/*
 *    Creates a mesh.
 *
 *    @param handle_t    The vertex buffer.
 *    @param handle_t    The texture.
 *
 *    @return handle_t      The mesh.
 */
handle_t mesh_create( handle_t sVBuffer, handle_t sTex ) {
    mesh_t mesh = {};
    mesh.aVBuf    = sVBuffer;
    mesh.aTex     = sTex;

    handle_t h = resource_add( gpResources, &mesh, sizeof( mesh_t ) );

    if ( h == INVALID_HANDLE ) {
        log_error( "Could not add mesh to resource list." );
        return INVALID_HANDLE;
    }

    return h;
}

/*
 *    Sets the vertex buffer of a mesh.
 *
 *    @param handle_t    The mesh.
 *    @param handle_t    The vertex buffer.
 */
void mesh_set_vertex_buffer( handle_t sMesh, handle_t sVBuffer ) {
    if ( gpResources == nullptr ) {
        log_error( "Resources not initialized." );
        return;
    }
    if ( gpMempool == nullptr ) {
        log_error( "Memory pool not initialized." );
        return;
    }
    if ( sMesh == INVALID_HANDLE ) {
        log_error( "Mesh is null." );
        return;
    }
    if ( sVBuffer == INVALID_HANDLE ) {
        log_error( "Vertex buffer is null." );
        return;
    }

    mesh_t *pMesh = resource_get( gpResources, sMesh );
    if ( pMesh == nullptr ) {
        log_error( "Mesh is null." );
        return;
    }
    vbuffer_t *pVBuffer = resource_get( gpResources, sVBuffer );
    if ( pVBuffer == nullptr ) {
        log_error( "Vertex buffer is null." );
        return;
    }

    pMesh->aVBuf = sVBuffer;
}

/*
 *    Sets the texture of a mesh.
 *
 *    @param handle_t    The mesh.
 *    @param handle_t    The texture.
 */
void mesh_set_texture( handle_t sMesh, handle_t sTex ) {
    if ( gpResources == nullptr ) {
        log_error( "Resources not initialized." );
        return;
    }
    if ( gpMempool == nullptr ) {
        log_error( "Memory pool not initialized." );
        return;
    }
    if ( sMesh == INVALID_HANDLE ) {
        log_error( "Mesh is null." );
        return;
    }
    if ( sTex == INVALID_HANDLE ) {
        log_error( "Texture is null." );
        return;
    }

    mesh_t *pMesh = resource_get( gpResources, sMesh );
    if ( pMesh == nullptr ) {
        log_error( "Mesh is null." );
        return;
    }

    pMesh->aTex = sTex;
}

/*
 *    Draws a vertex buffer.
 *
 *    @param handle_t          The handle to the vertex buffer.
 */
void vbuffer_draw( handle_t sBuffer ) {
    if ( gpCamera == nullptr ) {
        log_error( "No camera.\n" );
        return;
    }

    vbuffer_t *pBuf = resource_get( gpResources, sBuffer );
    if ( pBuf == nullptr ) {
        log_error( "Failed to get vertex resource.\n" );
        return;
    }

    mat4_t   view       = camera_view( gpCamera );
    u32      numVerts   = pBuf->aSize / pBuf->aVStride;

    vertexasm_set_layout( pBuf->aLayout );
    
    for ( u32 i = 0; i < numVerts; i += 3 ) {
        u8 a0[ VERTEX_ASM_MAX_VERTEX_SIZE ];
        u8 b0[ VERTEX_ASM_MAX_VERTEX_SIZE ];
        u8 c0[ VERTEX_ASM_MAX_VERTEX_SIZE ];

        /*
         *    Copy the vertex data into a buffer.
         */
        memcpy( a0, pBuf->apData + ( i + 0 ) * pBuf->aVStride, pBuf->aVStride );
        memcpy( b0, pBuf->apData + ( i + 1 ) * pBuf->aVStride, pBuf->aVStride );
        memcpy( c0, pBuf->apData + ( i + 2 ) * pBuf->aVStride, pBuf->aVStride );

        /*
         *    Get the positions of the vertices.
         */
        vec4_t pa = vertex_get_position( a0 );
        vec4_t pb = vertex_get_position( b0 );
        vec4_t pc = vertex_get_position( c0 );

        /*
         *    Transform the positions.
         */
        mat4_t        ma = m4_mul_v4( view, pa );
        mat4_t        mb = m4_mul_v4( view, pb );
        mat4_t        mc = m4_mul_v4( view, pc );

        /*
         *    Update the vertex data.
         */
        vertex_set_position( a0, ( vec4_t ){ ma.v[ 0 ], ma.v[ 4 ], ma.v[ 8 ], ma.v[ 12 ] } );
        vertex_set_position( b0, ( vec4_t ){ mb.v[ 0 ], mb.v[ 4 ], mb.v[ 8 ], mb.v[ 12 ] } );
        vertex_set_position( c0, ( vec4_t ){ mc.v[ 0 ], mc.v[ 4 ], mc.v[ 8 ], mc.v[ 12 ] } );

        /*
         *    If the vertex is outside of the view frustum, use
         *    linear interpolation to find the point on the triangle
         *    that is inside the view frustum.
         */
        s32 numVertices = 0;

        u8 *pVerts = cull_clip_triangle( a0, b0, c0, &numVertices );

        /*
         *    Draw the clipped vertices.
         */
        for ( s64 i = 0; i < numVertices - 2; ++i ) {
            memcpy( a0, pVerts + ( 0 + 0 ) * VERTEX_ASM_MAX_VERTEX_SIZE, pBuf->aVStride );
            memcpy( b0, pVerts + ( i + 1 ) * VERTEX_ASM_MAX_VERTEX_SIZE, pBuf->aVStride );
            memcpy( c0, pVerts + ( i + 2 ) * VERTEX_ASM_MAX_VERTEX_SIZE, pBuf->aVStride );

            /*
             *    Transform the vertex to screen space.
             */
            pa = vertex_get_position( a0 );
            pb = vertex_get_position( b0 );
            pc = vertex_get_position( c0 );

            pa.x /= pa.w;
            pa.y /= pa.w;

            pb.x /= pb.w;
            pb.y /= pb.w;

            pc.x /= pc.w;
            pc.y /= pc.w;

            vertex_set_position( a0, pa );
            vertex_set_position( b0, pb );
            vertex_set_position( c0, pc );
            
            /*
             *    Draw the triangle.
             */
            raster_rasterize_triangle( a0, b0, c0 );
        }
    }
}

/*
 *    Draws a mesh.
 *
 *    @param handle_t    The mesh.
 */
void mesh_draw( handle_t sMesh ) {
    if ( gpResources == nullptr ) {
        log_error( "Resources not initialized." );
        return;
    }
    if ( gpMempool == nullptr ) {
        log_error( "Memory pool not initialized." );
        return;
    }
    if ( sMesh == INVALID_HANDLE ) {
        log_error( "Mesh is null." );
        return;
    }

    mesh_t *pMesh = resource_get( gpResources, sMesh );
    if ( pMesh == nullptr ) {
        log_error( "Mesh is null." );
        return;
    }

    texture_t *pTex = resource_get( gpResources, pMesh->aTex );
    if ( pTex == nullptr ) {
        log_error( "Texture is null." );
        return;
    }

    image_t *pImage = pTex->apImage;
    if ( pImage == nullptr ) {
        log_error( "Image is null." );
        return;
    }

    vertexasm_bind_uniform( pTex );
    vbuffer_draw( pMesh->aVBuf );
}

/*
 *    Frees a mesh.
 *
 *    @param handle_t      The mesh to free.
 */
void mesh_free( handle_t sMesh ) {
    if ( gpResources == nullptr ) {
        log_error( "Resources not initialized." );
        return;
    }
    if ( gpMempool == nullptr ) {
        log_error( "Memory pool not initialized." );
        return;
    }
    if ( sMesh == INVALID_HANDLE ) {
        log_error( "Mesh is null." );
        return;
    }

    resource_remove( gpResources, sMesh );
}