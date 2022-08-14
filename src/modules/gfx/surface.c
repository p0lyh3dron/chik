/*
 *    surface.c    --    header file to interface with the draw surface
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on March 20, 2022
 *
 *    This file is part of the Chik engine.
 * 
 *    Included here are the functions used to interface with the
 *    draw surface.
 */
#include "surface.h"

#include "gfx.h"

static chik_surface_t *gpSurface = NULL;

/*
 *    Create a new surface.
 *
 *    @param u32                  The width of the surface.
 *    @param u32                  The height of the surface.
 *    @param u32                  The number of bits per pixel.
 * 
 *    @return chik_surface_t *    The new surface.
 *                                NULL on failure.
 */
chik_surface_t *surface_create( u32 sWidth, u32 sHeight, u32 sBpp ) {
    chik_surface_t *pSurface = NULL;
    
    pSurface = ( chik_surface_t * )mempool_alloc( gpMempool, sizeof( chik_surface_t ) );
    if( pSurface == NULL ) {
        log_fatal( "Could not allocate memory for surface." );
        return NULL;
    }
    
    pSurface->aWidth = sWidth;
    pSurface->aHeight = sHeight;
    pSurface->aBPP = sBpp;

    gpSurface = pSurface;
    
    return pSurface;
}

/*
 *    Gets the surface.
 *
 *    @return chik_surface_t *    The surface.
 *                                NULL on failure.
 */
chik_surface_t *surface_get( void ) {
    return gpSurface;
}

/*
 *    Destroy a surface.
 *
 *    @param chik_surface_t *    The surface to destroy.
 */
void surface_destroy( chik_surface_t *spSurface ) {
    mempool_free( gpMempool, spSurface );
}