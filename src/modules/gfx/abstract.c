/*
 *    abstract.c    --    header file for declaring abstract functions
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on March 20, 2022
 *
 *    This file is part of the Chik engine.
 * 
 *    Included here are the definitions for the abstract functions used by the Chik engine.
 */
#include "abstract.h"

#include "libchik.h"
#include "surface.h"
#include "rendertarget.h"

chik_surface_t *gpSurface = NULL;
rendertarget_t *gpRenderTarget = NULL;

/*
 *    Initialize libraries used for drawing.
 */
__attribute__( ( constructor ) )
void platform_init( void ) {
    s32 width  = args_get_int( "-w" );
    s32 height = args_get_int( "-h" );
    /*if ( width == -1 || height == -1 ) {
        width  = 1280;
        height = 720;
    }*/
#if USE_SDL
    gpSurface = surface_create( DEFAULT_WIDTH, DEFAULT_HEIGHT, 32 );
    if( gpSurface == NULL ) {
        log_fatal( "Could not create surface." );
    }
    SDL_Window *pWindow = SDL_CreateWindow( WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, gpSurface->aWidth, gpSurface->aHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE );
    if( pWindow == NULL ) {
        log_fatal( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
    }
    SDL_Renderer *pRenderer = SDL_CreateRenderer( pWindow, -1, SDL_RENDERER_ACCELERATED );
    if( pRenderer == NULL ) {
        log_fatal( "Renderer could not be created! SDL_Error: %s\n", SDL_GetError() );
    }
    SDL_Texture *pTexture = SDL_CreateTexture( pRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, gpSurface->aWidth, gpSurface->aHeight );
    if( pTexture == NULL ) {
        log_fatal( "Texture could not be created! SDL_Error: %s\n", SDL_GetError() );
    }
    gpSurface->apWindow = pWindow;
    gpSurface->apRenderer = pRenderer;
    gpSurface->apTexture = pTexture;
#endif /* USE_SDL  */
}

#if USE_SDL
/*
 *    Returns the SDL window.
 *
 *    @return SDL_Window *        The SDL window.
 */
SDL_Window *get_window( void ) {
    return gpSurface->apWindow;
}
#endif /* USE_SDL  */

/*
 *    Renders buffer to the screen.
 */
void platform_draw_frame( void ) {
    gpRenderTarget = rendertarget_get_backbuffer();
#if USE_SDL
    SDL_RenderClear( gpSurface->apRenderer );
    SDL_UpdateTexture( gpSurface->apTexture, NULL, gpRenderTarget->apTarget->apData, gpSurface->aWidth * sizeof( u32 ) );
    SDL_RenderCopy( gpSurface->apRenderer, gpSurface->apTexture, NULL, NULL );
    SDL_RenderPresent( gpSurface->apRenderer );
#endif /* USE_SDL  */
    image_clear( gpRenderTarget->apTarget, 0xFF000000 );
}