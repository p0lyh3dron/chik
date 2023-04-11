/*
 *    swapchain.h    --    header for vulkan swapchain
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on April 7, 2023
 *
 *    This file is part of the Chik engine.
 *
 *    This file declares functions for interfacing with the vulkan swapchain.
 */
#ifndef CHIK_GFXVK_SWAPCHAIN_H
#define CHIK_GFXVK_SWAPCHAIN_H

/*
 *    Creates the swapchain.
 *
 *    @param unsigned long count    The number of images in the swapchain.
 */
void swapchain_create(unsigned long count);

/*
 *    Destroys the swapchain.
 */
void swapchain_destroy(void);

#endif /* CHIK_GFXVK_SWAPCHAIN_H  */