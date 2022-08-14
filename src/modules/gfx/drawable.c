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
#include "vertexasm.h"
#include "raster.h"
#include "cull.h"

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
    mesh.aFlags   = MESHFLAGS_NONE;
    mesh.aVBuf    = sVBuffer;
    mesh.aTex     = sTex;

    handle_t h = resource_add( gpResources, &mesh, sizeof( mesh_t ) );

    if ( h == INVALID_HANDLE ) {
        log_error( "void mesh_create( handle_t, handle_t ): Could not add mesh to resource list.\n" );
        return INVALID_HANDLE;
    }

    return h;
}

/*
 *    Sets a mesh to not use the projection matrix.
 *
 *    @param handle_t    The mesh.
 */
void mesh_set_skip_projection( handle_t sMesh ) {
    if ( gpResources == nullptr ) {
        log_error( "void mesh_set_skip_projection( handle_t ): Resources not initialized.\n" );
        return;
    }
    if ( gpMempool == nullptr ) {
        log_error( "void mesh_set_skip_projection( handle_t ): Memory pool not initialized.\n" );
        return;
    }
    if ( sMesh == INVALID_HANDLE ) {
        log_error( "void mesh_set_skip_projection( handle_t ): Mesh is null.\n" );
        return;
    }

    mesh_t *pMesh = resource_get( gpResources, sMesh );
    if ( pMesh == nullptr ) {
        log_error( "void mesh_set_skip_projection( handle_t ): Mesh is null.\n" );
        return;
    }
    pMesh->aFlags |= MESHFLAGS_SKIP_PROJECTION;
}

/*
 *    Sets a mesh to not be clipped.
 *
 *    @param handle_t    The mesh.
 */
void mesh_set_skip_clipping( handle_t sMesh ) {
    if ( gpResources == nullptr ) {
        log_error( "void mesh_set_skip_clipping( handle_t ): Resources not initialized.\n" );
        return;
    }
    if ( gpMempool == nullptr ) {
        log_error( "void mesh_set_skip_clipping( handle_t ): Memory pool not initialized.\n" );
        return;
    }
    if ( sMesh == INVALID_HANDLE ) {
        log_error( "void mesh_set_skip_clipping( handle_t ): Mesh is null.\n" );
        return;
    }

    mesh_t *pMesh = resource_get( gpResources, sMesh );
    if ( pMesh == nullptr ) {
        log_error( "void mesh_set_skip_clipping( handle_t ): Mesh is null.\n" );
        return;
    }
    pMesh->aFlags |= MESHFLAGS_SKIP_CLIPPING;
}

/*
 *    Sets the vertex buffer of a mesh.
 *
 *    @param handle_t    The mesh.
 *    @param handle_t    The vertex buffer.
 */
void mesh_set_vertex_buffer( handle_t sMesh, handle_t sVBuffer ) {
    if ( gpResources == nullptr ) {
        log_error( "void mesh_set_vertex_buffer( handle_t, handle_t ): Resources not initialized.\n" );
        return;
    }
    if ( gpMempool == nullptr ) {
        log_error( "void mesh_set_vertex_buffer( handle_t, handle_t ): Memory pool not initialized.\n" );
        return;
    }
    if ( sMesh == INVALID_HANDLE ) {
        log_error( "void mesh_set_vertex_buffer( handle_t, handle_t ): Mesh is null.\n" );
        return;
    }
    if ( sVBuffer == INVALID_HANDLE ) {
        log_error( "void mesh_set_vertex_buffer( handle_t, handle_t ): Vertex buffer is null.\n" );
        return;
    }

    mesh_t *pMesh = resource_get( gpResources, sMesh );
    if ( pMesh == nullptr ) {
        log_error( "void mesh_set_vertex_buffer( handle_t, handle_t ): Mesh is null.\n" );
        return;
    }
    vbuffer_t *pVBuffer = resource_get( gpResources, sVBuffer );
    if ( pVBuffer == nullptr ) {
        log_error( "void mesh_set_vertex_buffer( handle_t, handle_t ): Vertex buffer is null.\n" );
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
        log_error( "void mesh_set_texture( handle_t, handle_t ): Resources not initialized.\n" );
        return;
    }
    if ( gpMempool == nullptr ) {
        log_error( "void mesh_set_texture( handle_t, handle_t ): Memory pool not initialized.\n" );
        return;
    }
    if ( sMesh == INVALID_HANDLE ) {
        log_error( "void mesh_set_texture( handle_t, handle_t ): Mesh is null.\n" );
        return;
    }
    if ( sTex == INVALID_HANDLE ) {
        log_error( "void mesh_set_texture( handle_t, handle_t ): Texture is null.\n" );
        return;
    }

    mesh_t *pMesh = resource_get( gpResources, sMesh );
    if ( pMesh == nullptr ) {
        log_error( "void mesh_set_texture( handle_t, handle_t ): Mesh is null.\n" );
        return;
    }

    pMesh->aTex = sTex;
}

vec3_t gMeshTranslate = { 0.f, 0.f, 0.f };
vec3_t gMeshRotate    = { 0.f, 0.f, 0.f };
vec3_t gMeshScale     = { 1.f, 1.f, 1.f };

/*
 *    Translates a mesh.
 *
 *    @param vec3_t      The translation vector.
 */
void mesh_translate( vec3_t sTranslation ) {
    gMeshTranslate = sTranslation;
}

/*
 *    Rotates a mesh.
 *
 *    @param vec3_t      The rotation vector.
 */
void mesh_rotate( vec3_t sRotation ) {
    gMeshRotate = sRotation;
}

/*
 *    Scales a mesh.
 *
 *    @param vec3_t      The scale vector.
 */
void mesh_scale( vec3_t sScale ) {
    gMeshScale = sScale;
}

/*
 *    Draws a vertex buffer.
 *
 *    @param handle_t          The handle to the vertex buffer.
 *    @param meshflags_e       The flags to use for the mesh.
 */
void vbuffer_draw( handle_t sBuffer, meshflags_e sFlags ) {
    if ( gpCamera == nullptr ) {
        log_error( "void vbuffer_draw( handle_t, meshflags_e ): No camera.\n" );
        return;
    }

    vbuffer_t *pBuf = resource_get( gpResources, sBuffer );
    if ( pBuf == nullptr ) {
        log_error( "void vbuffer_draw( handle_t, meshflags_e ): Failed to get vertex resource.\n" );
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
        pa.x *= gMeshScale.x;
        pa.y *= gMeshScale.y;
        pa.z *= gMeshScale.z;
        vec4_t pb = vertex_get_position( b0 );
        pb.x *= gMeshScale.x;
        pb.y *= gMeshScale.y;
        pb.z *= gMeshScale.z;
        vec4_t pc = vertex_get_position( c0 );
        pc.x *= gMeshScale.x;
        pc.y *= gMeshScale.y;
        pc.z *= gMeshScale.z;

        /*
         *    Transform the positions.
         *
         *    If we have an active translation/rotation, apply it.
         */

        mat4_t        ta = m4_mul_v4( m4_rotate( gMeshRotate.x, ( vec3_t ){ 1.f, 0.f, 0.f } ), pa );
                      ta = m4_mul_m4( m4_rotate( gMeshRotate.y, ( vec3_t ){ 0.f, 1.f, 0.f } ), ta );
                      ta = m4_mul_m4( m4_rotate( gMeshRotate.z, ( vec3_t ){ 0.f, 0.f, 1.f } ), ta );
                      ta = m4_mul_m4( m4_translate( gMeshTranslate ), ta );
        
        mat4_t        tb = m4_mul_v4( m4_rotate( gMeshRotate.x, ( vec3_t ){ 1.f, 0.f, 0.f } ), pb );
                      tb = m4_mul_m4( m4_rotate( gMeshRotate.y, ( vec3_t ){ 0.f, 1.f, 0.f } ), tb );
                      tb = m4_mul_m4( m4_rotate( gMeshRotate.z, ( vec3_t ){ 0.f, 0.f, 1.f } ), tb );
                      tb = m4_mul_m4( m4_translate( gMeshTranslate ), tb );

        mat4_t        tc = m4_mul_v4( m4_rotate( gMeshRotate.x, ( vec3_t ){ 1.f, 0.f, 0.f } ), pc );
                      tc = m4_mul_m4( m4_rotate( gMeshRotate.y, ( vec3_t ){ 0.f, 1.f, 0.f } ), tc );
                      tc = m4_mul_m4( m4_rotate( gMeshRotate.z, ( vec3_t ){ 0.f, 0.f, 1.f } ), tc );
                      tc = m4_mul_m4( m4_translate( gMeshTranslate ), tc );

        mat4_t ma = m4_identity();
        mat4_t mb = m4_identity();
        mat4_t mc = m4_identity();
        
        if ( !( sFlags & MESHFLAGS_SKIP_PROJECTION ) ) {
            ma = m4_mul_m4( view, ta );
            mb = m4_mul_m4( view, tb );
            mc = m4_mul_m4( view, tc );
        }
        else {
            ma = ta;
            mb = tb;
            mc = tc;
        }

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

        u8 *pVerts = cull_clip_triangle( a0, b0, c0, &numVertices, !( sFlags & MESHFLAGS_SKIP_CLIPPING ) );

        /*
         *    Draw the clipped vertices.
         */
        for ( s64 i = 0; i < numVertices - 2; ++i ) {
            memcpy( a0, pVerts + ( 0 + 0 ) * VERTEX_ASM_MAX_VERTEX_SIZE, pBuf->aVStride );
            memcpy( b0, pVerts + ( i + 1 ) * VERTEX_ASM_MAX_VERTEX_SIZE, pBuf->aVStride );
            memcpy( c0, pVerts + ( i + 2 ) * VERTEX_ASM_MAX_VERTEX_SIZE, pBuf->aVStride );

            if ( !( sFlags & MESHFLAGS_SKIP_PROJECTION ) ) {
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
            }

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
        log_error( "void mesh_draw( handle_t ): Resources not initialized.\n" );
        return;
    }
    if ( gpMempool == nullptr ) {
        log_error( "void mesh_draw( handle_t ): Memory pool not initialized.\n" );
        return;
    }
    if ( sMesh == INVALID_HANDLE ) {
        log_error( "void mesh_draw( handle_t ): Mesh is null.\n" );
        return;
    }

    mesh_t *pMesh = resource_get( gpResources, sMesh );
    if ( pMesh == nullptr ) {
        log_error( "void mesh_draw( handle_t ): Mesh is null.\n" );
        return;
    }

    texture_t *pTex = resource_get( gpResources, pMesh->aTex );
    if ( pTex == nullptr ) {
        log_error( "void mesh_draw( handle_t ): Texture is null.\n" );
        return;
    }

    image_t *pImage = pTex->apImage;
    if ( pImage == nullptr ) {
        log_error( "void mesh_draw( handle_t ): Image is null.\n" );
        return;
    }

    vertexasm_bind_uniform( pTex );
    vbuffer_draw( pMesh->aVBuf, pMesh->aFlags );
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
}