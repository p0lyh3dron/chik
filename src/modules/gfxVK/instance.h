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

#include <vulkan/vulkan.h>

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
 *    Performs the rest of the instance initialization.
 */
void instance_finish_init(void);

/*
 *    Gets the vulkan physical device.
 *
 *    @return VkPhysicalDevice    The vulkan physical device.
 */
VkPhysicalDevice instance_get_gpu(void);

/*
 *    Gets the vulkan surface.
 *
 *    @return VkSurfaceKHR    The vulkan surface.
 */
VkSurfaceKHR instance_get_surface(void);

/*
 *    Gets the vulkan device.
 *
 *    @return VkDevice    The vulkan device.
 */
VkDevice instance_get_device(void);

/*
 *    Destroys the vulkan instance.
 */
void instance_destroy(void);

#endif /* CHIK_GFXVK_INSTANCE_H  */