/*
 *    abstract.h    --    header file for declaring abstract functions
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on March 20, 2022
 *
 *    This file is part of the Chik engine.
 * 
 *    Included here are the abstract functions used by the Chik engine.
 *    These functions are used to abstract the underlying graphics
 *    library used by the engine.
 */
#pragma once

#include "libchik.h"

/*
 *    Initialize libraries used for drawing.
 */
void platform_init( void );

/*
 *    Renders buffer to the screen.
 */
void platform_draw_frame( void );