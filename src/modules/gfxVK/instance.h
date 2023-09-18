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
 *    Gets the vulkan graphics queue index.
 *
 *    @return unsigned long    The vulkan graphics queue.
 */
unsigned long instance_get_graphics_queue_idx(void);

/*
 *    Gets the vulkan graphics queue.
 *
 *    @return VkQueue    The vulkan graphics queue.
 */
VkQueue instance_get_graphics_queue(void);

/*
 *    Gets the vulkan present queue.
 *
 *    @return VkQueue    The vulkan present queue.
 */
VkQueue instance_get_present_queue(void);


/*
 *    Creates a GPU buffer.
 *
 *    @param VkDeviceSize          size          The size of the buffer.
 *    @param VkBufferUsageFlags    usage         The usage of the buffer.
 *    @param VkMemoryPropertyFlags properties    The properties of the buffer.
 *    @param VkBuffer             *buffer        The buffer to create.
 *    @param VkDeviceMemory       *buffer_memory The buffer memory to create.
 */
void instance_create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *buffer_memory);

/*
 *    Returns the texure sampler.
 *
 *    @return VkSampler    The texture sampler.
 */
VkSampler instance_get_texture_sampler(void);

/*
 *    Destroys the vulkan instance.
 */
void instance_destroy(void);

#endif /* CHIK_GFXVK_INSTANCE_H  */