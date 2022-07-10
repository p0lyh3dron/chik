/*
 *    engine.h    --    header for engine module
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on April 16, 2022.
 * 
 *    This file is part of the Chik engine.
 * 
 *    The brain of Chik Engine resides in this module. At
 *    her current state, this module will handle the loading
 *    of all the other modules, and the act as an interface
 *    between the modules that the game code will use.
 */
#pragma once

#define ENGINE_MAX_MODULES 16

#include "libchik.h"

typedef struct {
    dl_handle_t  aHandle;
    const s8    *apName;
    u32       ( *apUpdate )( f32 sDT );
    u32       ( *apExit )( void );
} module_t;

/*
 *    Initializes the engine with the specified modules.
 *
 *    @param const s8 *    The name of the modules to initialize.
 *    @param ...           The other modules to initialize.
 *
 *    @return u32          Returns 0 on failure, 1 on success.
 */
u32 engine_init( const s8 *spModules, ... );

/*
 *    Updates the engine.
 *
 *    @return u32           Returns 0 on failure, 1 on success.
 */
u32 engine_update( void );

/*
 *    Loads a function from the engine for external use.
 *
 *    @param const s8 *    The name of the function to load.
 * 
 *    @return void *       Returns a pointer to the function.
 */
void *engine_load_function( const s8 *spName );

/*
 *    Frees the engine.
 */
void engine_free();