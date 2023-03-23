/*
 *    instance.h    --    header for vulkan instance
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on February 21, 2023
 *
 *    This file is part of the Chik engine.
 *
 *    This file declares functions for interfacing with the vulkan instance.
 */
#ifndef CHIK_GFXVK_INSTANCE_H
#define CHIK_GFXVK_INSTANCE_H

#include "libchik.h"

/*
 *    Initializes the vulkan instance.
 */
void instance_init(void);

/*
 *    Picks a graphics card from the system.
 *
 *    @param unsigned long gpu    The index of the graphics card to pick.
 */
void instance_pick_gpu(unsigned long gpu);

/*
 *    Destroys the vulkan instance.
 */
void instance_destroy(void);

#endif /* CHIK_GFXVK_INSTANCE_H  */