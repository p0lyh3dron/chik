/*
 *    renderpasses.h    --    header for vulkan renderpasses
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on April 7, 2023
 *
 *    This file is part of the Chik engine.
 *
 *    This file declares the interface to create and manage vulkan renderpasses.
 */
#ifndef CHIK_RENDERPASSES_H
#define CHIK_RENDERPASSES_H

#include <vulkan/vulkan.h>

#define CHIK_GFXVK_RENDERPASSES_FORMAT VK_FORMAT_B8G8R8A8_UNORM

/*
 *    Creates the renderpasses used by the engine.
 */
void renderpasses_init();

/*
 *    Returns a renderpass for the given parameters.
 * 
 *    @return VkRenderPass    The renderpass.
 */
VkRenderPass renderpasses_get();

/*
 *    Destroys the renderpasses used by the engine.
 */
void renderpasses_destroy();

#endif /* CHIK_RENDERPASSES_H  */