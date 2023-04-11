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
#include "renderpasses.h"
#include "swapchain.h"

unsigned int graphics_init(void);
unsigned int graphics_update(float);
unsigned int graphics_exit(void);

CHIK_MODULE(graphics_init, graphics_update, graphics_exit)

void    *(*surface_get_window)(void);
vec2u_t  (*platform_get_screen_size)(void);

/*
 *    Creates the graphics context.
 */
unsigned int graphics_init(void) {
    *(void **)(&surface_get_window)       = engine_load_function("surface_get_window");
    *(void **)(&platform_get_screen_size) = engine_load_function("platform_get_screen_size");

    if (surface_get_window == (void *)0x0) {
        LOGF_ERR("Failed to load surface_get_window.\n");
        return 0;
    }

    if (platform_get_screen_size == (void *)0x0) {
        LOGF_ERR("Failed to load platform_get_screen_size.\n");
        return 0;
    }

    shell_variable_t vars[] = {
        {"gfx_width", "Framebuffer width", "1152", nullptr, SHELL_VAR_INT},
        {"gfx_height", "Framebuffer height", "864", nullptr, SHELL_VAR_INT},
        {"gfx_vsync", "Enable vsync", "1", nullptr, SHELL_VAR_INT},
        {"gfx_msaa_samples", "Number of MSAA samples to use", "1", nullptr, SHELL_VAR_INT},
        {"gfx_buffered_frames", "Number of buffered frames (e.g. double buffering, triple buffering...)", "1", nullptr, SHELL_VAR_INT},
        {(char*)0x0, (char*)0x0, (char*)0x0, nullptr, SHELL_VAR_INT},
    };

    shell_register_variables(vars);

    instance_init();
    instance_pick_gpu(0);
    instance_finish_init();

    renderpasses_init();

    swapchain_create(shell_get_variable("gfx_buffered_frames").i);

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
    swapchain_destroy();
    renderpasses_destroy();
    instance_destroy();

    return 1;
}