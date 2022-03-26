/*
 *    rendertarget.c    --    source for using render targets
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on March 20, 2022
 * 
 *    This file is part of the Chik engine.
 *
 *    Here are the functions used for creating and using render targets.
 */
#include "rendertarget.h"

#include "surface.h"

rendertarget_t **gpRenderTargets = NULL;

rendertarget_t  *gpBackBuffer    = NULL;

/*
 *    Creates a render target.
 *
 *    @param u32           The width of the render target.
 *    @param u32           The height of the render target.
 *    @param u32           The format of the render target.
 *
 *    @return rendertarget_t *    The render target.
 *                                NULL if the render target could not be created.
 *                                The render target should be freed with rendertarget_free().
 */
rendertarget_t *rendertarget_create( u32 sWidth, u32 sHeight, u32 sFormat ) {
    rendertarget_t *pRenderTarget = malloc( sizeof( rendertarget_t ) );
    if( pRenderTarget == NULL ) {
        log_error( "Could not allocate memory for render target." );
    }

    image_t *pImage = image_create( sWidth, sHeight, sFormat );
    if( pImage == NULL ) {
        log_error( "Could not allocate memory for render target image." );
    }

    pRenderTarget->apTarget = pImage;

    /*
     *    Make a new list of render targets if there is none.
     */
    if ( gpRenderTargets == NULL ) {
        gpRenderTargets = malloc( sizeof( rendertarget_t * ) * 2 );
        if( gpRenderTargets == NULL ) {
            log_error( "Could not allocate memory for render target list." );
        }
        gpRenderTargets[ 0 ] = pRenderTarget;
        gpRenderTargets[ 1 ] = NULL;
    }
    else {
        u32 i = 0;
        while ( gpRenderTargets[ i ] != NULL ) {
            i++;
        }
        gpRenderTargets[ i ] = pRenderTarget;
        gpRenderTargets = realloc( gpRenderTargets, sizeof( rendertarget_t * ) * ( i + 1 ) );
        if( gpRenderTargets == NULL ) {
            log_error( "Could not reallocate memory for render target list." );
        }
    }

    return pRenderTarget;
}

/*
 *    Sets a camera to render to a render target.
 *
 *    @param rendertarget_t *    The render target.
 *    @param camera_t *          The camera.
 */
void rendertarget_set_camera( rendertarget_t *spRenderTarget, camera_t *spCamera ) {
    if ( spRenderTarget == NULL || spCamera == NULL ) {
        log_error( "Render target or Camera is NULL.\n" );
        return;
    }
    spRenderTarget->aCamera = *spCamera;
}

/*
 *    Frees a render target.
 *
 *    @param rendertarget_t *    The render target to free.
 */
void rendertarget_free( rendertarget_t *spRenderTarget ) {
    if( spRenderTarget == NULL ) {
        log_error( "Render target is NULL.\n" );
        return;
    }

    image_free( spRenderTarget->apTarget );
    free( spRenderTarget );
}

/*
 *    Gets a list of render targets.
 *
 *    @return rendertarget_t *    The list of render targets.
 *                                NULL if the list could not be created.
 */
rendertarget_t **rendertarget_get_list( void ) {
    return gpRenderTargets;
}

/*
 *    Create backbuffer render target.
 *
 *    @return rendertarget_t *    The render target.
 *                                NULL if the render target could not be created.
 *                                The render target should be freed with rendertarget_free().
 */
__attribute__( ( constructor ) )
rendertarget_t *rendertarget_create_backbuffer( void ) {
    chik_surface_t *pSurface      = surface_get();

    rendertarget_t *pRenderTarget = rendertarget_create( pSurface->aWidth, pSurface->aHeight, 69 );
    if( pRenderTarget == NULL ) {
        log_error( "Could not create backbuffer render target." );
        return NULL;
    }

    pRenderTarget->aCamera.aPosition.x = 0.f;
    pRenderTarget->aCamera.aPosition.y = -1.f;
    pRenderTarget->aCamera.aPosition.z = 0.f;

    pRenderTarget->aCamera.aDirection.x = 0;
    pRenderTarget->aCamera.aDirection.y = 0;

    pRenderTarget->aCamera.aNear = 0.1f;
    pRenderTarget->aCamera.aFar  = 100.f;

    pRenderTarget->aCamera.aFOV = 90.f;

    gpBackBuffer = pRenderTarget;

    return pRenderTarget;
}

/*
 *    Gets the backbuffer render target.
 *
 *    @return rendertarget_t *    The render target.
 *                                NULL if the render target could not be created.
 *                                The render target should be freed with rendertarget_free().
 */
rendertarget_t *rendertarget_get_backbuffer( void ) {
    return gpBackBuffer;
}

/*
 *    Frees all render targets.
 */
__attribute__( ( destructor ) )
void rendertarget_free_all( void ) {
    if ( gpRenderTargets == NULL ) {
        return;
    }

    u32 i = 0;
    while ( gpRenderTargets[ i ] != NULL ) {
        rendertarget_free( gpRenderTargets[ i ] );
        i++;
    }
    free( gpRenderTargets );
    gpRenderTargets = NULL;
}