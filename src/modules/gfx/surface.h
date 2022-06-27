/*
 *    surface.h    --    header file to interface with the draw surface
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on March 20, 2022
 *
 *    This file is part of the Chik engine.
 * 
 *    Included here are the functions used to interface with the
 *    draw surface.
 */
#pragma once

#define DEFAULT_WIDTH  480
#define DEFAULT_HEIGHT 270

#define WINDOW_TITLE   "Chik ( Version 1.0 )"

#if USE_SDL
#include <SDL2/SDL.h>
#endif /* USE_SDL  */

#include "libchik.h"

typedef struct {
    u32 aWidth;
    u32 aHeight;
    u32 aBPP;
#if USE_SDL
    SDL_Window    *apWindow;
    SDL_Renderer  *apRenderer;
    SDL_Texture   *apTexture;    
#endif /* USE_SDL  */
} chik_surface_t;

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
chik_surface_t *surface_create( u32 sWidth, u32 sHeight, u32 sBpp );

/*
 *    Gets the surface.
 *
 *    @return chik_surface_t *    The surface.
 *                                NULL on failure.
 */
chik_surface_t *surface_get( void );

/*
 *    Destroy a surface.
 *
 *    @param chik_surface_t *    The surface to destroy.
 */
void surface_destroy( chik_surface_t *spSurface );