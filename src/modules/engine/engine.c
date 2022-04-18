/*
 *    engine.c    --    source for engine module
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on April 16, 2022.
 * 
 *    This file is part of the Chik engine.
 * 
 *    The brain of Chik Engine resides in this module. This
 *    file will define how the engine will do module handling.
 */
#include "engine.h"

#include <time.h>
#include <stdarg.h>

#include "stat.h"

mempool_t *gpEnginePool                    = nullptr;

module_t   gpModules[ ENGINE_MAX_MODULES ] = { 0 };

/*
 *    Initializes the engine with the specified modules.
 *
 *    @param const s8 *    The name of the modules to initialize.
 *    @param ...           The other modules to initialize.
 *
 *    @return u32          Returns 0 on failure, 1 on success.
 */
u32 engine_init( const s8 *modules, ... ) {
    /*
     *    Set the start time.
     */
    stat_t *pStat     = stat_get();
    pStat->aStartTime = time( nullptr );

    /*
     *    Initialize the engine modules.
     */
    va_list args;
    u32     result      = 1;
    u32     moduleIndex = 0;

    va_start( args, modules );

    while ( modules ) {
        dl_handle_t h = dl_open( modules );

        if ( h == nullptr ) {
            log_fatal( "u32 engine_init( const s8 *, ... ): Unable to open module: %s\n", modules );
            result = 0;
            break;
        }

        u32 ( *entry )( void * ) = dl_sym( h, "chik_module_entry" );
        if ( entry == nullptr || !entry( &engine_load_function ) ) {
            log_warn( "u32 engine_init( const s8 *, ... ): Unable to load module entry: %s\n", modules );
        }

        gpModules[ moduleIndex++ ] = ( module_t ){ h, modules };
        modules = va_arg( args, const s8 * );
    }
    
    va_end( args );
    return result;
}

/*
 *    Loads a function from the engine for external use.
 *
 *    @param const s8 *    The name of the function to load.
 * 
 *    @return void *       Returns a pointer to the function.
 */
void *engine_load_function( const s8 *spName ) {
    void *pFunc = nullptr;

    for ( u64 i = 0; i < ENGINE_MAX_MODULES; i++ ) {
        /*
         *    If the module isn't null, we'll try to load the function.
         */
        if ( gpModules[ i ].apName && gpModules[ i ].aHandle ) {
            pFunc = dl_sym( gpModules[ i ].aHandle, spName );

            if ( pFunc ) {
                break;
            }
        }
    }

    return pFunc;
}

/*
 *    Frees the engine.
 */
__attribute__( ( destructor ) )
void engine_free() {
    for ( u64 i = 0; i < ENGINE_MAX_MODULES; ++i ) {
        if ( gpModules[ i ].aHandle ) {
            dl_close( gpModules[ i ].aHandle );
        }
    }
}