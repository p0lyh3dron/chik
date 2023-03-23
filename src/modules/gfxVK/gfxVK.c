/*
 *    gfxVK.c    --    source for vulkan abstraction
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on October 23, 2022.
 *
 *    This file is part of the Chik engine.
 *
 *    This file defines rendering functions that will be used by Chik
 *    applications.
 */
#include "libchik.h"

#include "gfxVK.h"

#include "instance.h"

unsigned int graphics_init(void);
unsigned int graphics_update(float);
unsigned int graphics_exit(void);

CHIK_MODULE(graphics_init, graphics_update, graphics_exit)

void *(*surface_get_window)(void);

/*
 *    Creates the graphics context.
 */
unsigned int graphics_init(void) {
    *(void **)(&surface_get_window) = engine_load_function("surface_get_window");

    if (surface_get_window == (void *)0x0) {
        LOGF_ERR("Failed to load surface_get_window.\n");
        return 0;
    }

    instance_init();
    instance_pick_gpu(0);

    return 1;
}

/*
 *    Updates the graphics context.
 *
 *    @param float dt    The time since the last update.
 *
 *    @return unsigned int   The return code.
 */
unsigned int graphics_update(float dt) {
}

/*
 *    Cleans up the graphics subsystem.
 */
unsigned int graphics_exit(void) {
    instance_destroy();

    return 1;
}