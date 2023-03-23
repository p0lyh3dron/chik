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
    dl_handle_t handle;
    const char *name;
    unsigned int (*update)(float sDT);
    unsigned int (*exit)(void);
} module_t;

/*
 *    Initializes the engine with the specified modules.
 *
 *    @param const char *modules    The name of the modules to initialize.
 *    @param ...                  The other modules to initialize.
 *
 *    @return unsigned int          Returns 0 on failure, 1 on success.
 */
unsigned int engine_init(const char *modules, ...);

/*
 *    Updates the shell.
 *
 *    @return unsigned int           Returns 0 on failure, 1 on success.
 */
unsigned int engine_update_shell(void);

/*
 *    Updates the engine.
 *
 *    @return unsigned int           Returns 0 on failure, 1 on success.
 */
unsigned int engine_update(void);

/*
 *    Frees the engine.
 */
void engine_free();