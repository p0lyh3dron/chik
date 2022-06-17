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
    if ( gpCamera == NULL ) {
        log_error( "No camera.\n" );
        return;
    }

    vbuffer_t *pBuf = resource_get( gpResources, sBuffer );
    if ( pBuf == NULL ) {
        log_error( "Failed to get vertex resource.\n" );
        return;
    }
    mat4_t   view   = camera_view( gpCamera );
    u32      size   = pBuf->aSize / sizeof( chik_vertex_t );
    for ( u32 i = 0; i < size; i += 3 ) {
        chik_vertex_t a0 = *( chik_vertex_t* )( pBuf->apData + i * sizeof( chik_vertex_t ) + 0 * pBuf->aVStride );
        a0.aPos.w = 1.0f;
        chik_vertex_t b0 = *( chik_vertex_t* )( pBuf->apData + i * sizeof( chik_vertex_t ) + 1 * pBuf->aVStride );
        b0.aPos.w = 1.0f;
        chik_vertex_t c0 = *( chik_vertex_t* )( pBuf->apData + i * sizeof( chik_vertex_t ) + 2 * pBuf->aVStride );
        c0.aPos.w = 1.0f;

        mat4_t        ma = m4_mul_v4( view, ( vec4_t ){ -a0.aPos.x, -a0.aPos.y, -a0.aPos.z, 1 } );
        mat4_t        mb = m4_mul_v4( view, ( vec4_t ){ -b0.aPos.x, -b0.aPos.y, -b0.aPos.z, 1 } );
        mat4_t        mc = m4_mul_v4( view, ( vec4_t ){ -c0.aPos.x, -c0.aPos.y, -c0.aPos.z, 1 } );

        int j = 0;

        chik_vertex_t a = a0;
        a.aPos = ( vec4_t ){ ma.v[ j ], ma.v[ j + 4 ], ma.v[ j + 8 ], ma.v[ j + 12 ] };
        
        chik_vertex_t b = b0;
        b.aPos = ( vec4_t ){ mb.v[ j ], mb.v[ j + 4 ], mb.v[ j + 8 ], mb.v[ j + 12 ] };

        chik_vertex_t c = c0;
        c.aPos = ( vec4_t ){ mc.v[ j ], mc.v[ j + 4 ], mc.v[ j + 8 ], mc.v[ j + 12 ] };

        /*
         *    Don't draw the triangle if it is behind the camera.
         *    if the dot product of the surface normal and the camera to the surface is greater than 0,
         *    then the surface is in discarded.
         */
        vec3_t v0;
        vec3_t v1;
        vec3_sub( &v0, &a.aPos, &b.aPos );
        vec3_sub( &v1, &a.aPos, &c.aPos );

        vec3_t n;
        vec3_cross( &n, &v0, &v1 );

        vec3_t v;
        vec3_sub( &v, &gpCamera->aPosition, &a.aPos );

        /*if ( vec3_dot( &n, &v ) >= 0.0f ) {
            continue;
        }*/

        /*
         *    If the vertex is outside of the view frustum, use
         *    linear interpolation to find the point on the triangle
         *    that is inside the view frustum.
         */
        s32 numVertices = 0;
        /*a.aPos.z = 1 / a.aPos.z;
        b.aPos.z = 1 / b.aPos.z;
        c.aPos.z = 1 / c.aPos.z;*/

        chik_vertex_t *pVerts = cull_clip_triangle( &a, &b, &c, &numVertices );
        /*u8 pVerts[ 3 * VERTEX_ASM_MAX_VERTEX_SIZE ];
        memcpy( pVerts, &a, sizeof( chik_vertex_t ) );
        memcpy( pVerts + 1 * VERTEX_ASM_MAX_VERTEX_SIZE, &b, sizeof( chik_vertex_t ) );
        memcpy( pVerts + VERTEX_ASM_MAX_VERTEX_SIZE * 2, &c, sizeof( chik_vertex_t ) );
        numVertices = 3;*/

        /*log_note( "%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n", view.v[ 0 ], view.v[ 1 ], view.v[ 2 ], view.v[ 3 ],
                  view.v[ 4 ], view.v[ 5 ], view.v[ 6 ], view.v[ 7 ],
                  view.v[ 8 ], view.v[ 9 ], view.v[ 10 ], view.v[ 11 ],
                  view.v[ 12 ], view.v[ 13 ], view.v[ 14 ], view.v[ 15 ] );*/

        

        /*a.aPos.z = 1 / a.aPos.z;
        b.aPos.z = 1 / b.aPos.z;
        c.aPos.z = 1 / c.aPos.z;*/
        //log_note( "a = %f, %f, %f\nb = %f, %f, %f\nc = %f, %f, %f\n", a.aPos.x, a.aPos.y, a.aPos.z, b.aPos.x, b.aPos.y, b.aPos.z, c.aPos.x, c.aPos.y, c.aPos.z );

        /*
         *    Draw the clipped vertices.
         */
        for ( s64 i = 0; i < numVertices - 2; ++i ) {
            chik_vertex_t a = *( chik_vertex_t* )( ( u8 * )pVerts + ( 0 ) * VERTEX_ASM_MAX_VERTEX_SIZE );
            chik_vertex_t b = *( chik_vertex_t* )( ( u8 * )pVerts + ( 1 + i ) * VERTEX_ASM_MAX_VERTEX_SIZE );
            chik_vertex_t c = *( chik_vertex_t* )( ( u8 * )pVerts + ( 2 + i ) * VERTEX_ASM_MAX_VERTEX_SIZE );

            if ( a.aPos.w == 0.0f || b.aPos.w == 0.0f || c.aPos.w == 0.0f ) {
                continue;
            }

            /*
             *    Transform the vertex to screen space.
             */
            a.aPos.x /= a.aPos.w;
            a.aPos.y /= a.aPos.w;
            //a.aPos.z /= a.aPos.w;

            b.aPos.x /= b.aPos.w;
            b.aPos.y /= b.aPos.w;
            //b.aPos.z /= b.aPos.w;

            c.aPos.x /= c.aPos.w;
            c.aPos.y /= c.aPos.w;
            //c.aPos.z /= c.aPos.w;
            //log_note( "a = %f, %f, %f\nb = %f, %f, %f\nc = %f, %f, %f\n", a.aPos.x, a.aPos.y, a.aPos.z, b.aPos.x, b.aPos.y, b.aPos.z, c.aPos.x, c.aPos.y, c.aPos.z );
            /*
             *    Draw the triangle.
             */
            raster_rasterize_triangle( &a, &b, &c );
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