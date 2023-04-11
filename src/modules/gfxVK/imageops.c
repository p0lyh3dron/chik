/*
 *    imageops.c    --    source for vulkan image manipulation functions
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on April 10, 2023
 *
 *    This file is part of the Chik engine.
 *
 *    This file defines the functions for manipulating vulkan images.
 */
#include "imageops.h"

#include "instance.h"

/*
 *    Finds the memory type for given properties.
 *
 *    @param uint32_t              type_filter     The type filter.
 *    @param VkMemoryPropertyFlags properties      The properties.
 * 
 *    @return uint32_t                The memory type.
 */
uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) {
    uint32_t                         i;
    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(instance_get_gpu(), &mem_props);

    for (i = 0; i < mem_props.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (mem_props.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
}

/*
 *    Creates a vulkan image.
 *
 *    @param VkFormat format                     The format of the image.
 *    @param uint32_t width                      The width of the image.
 *    @param uint32_t height                     The height of the image. 
 *    @param uint32_t mip_levels                 The number of mip levels.
 *    @param VkSampleCountFlagBits samples       The number of samples.
 *    @param VkImageTiling tiling                The tiling of the image.
 *    @param VkImageUsageFlags usage             The usage of the image.
 *    @param VkMemoryPropertyFlags properties    The properties of the image.
 *    @param VkImageAspectFlags aspect           The aspect of the image.
 * 
 *    @return vulkan_image_t *                   The vulkan image.
 */
vulkan_image_t *imageops_create_image(VkFormat format, uint32_t width, uint32_t height, 
                                      uint32_t mip_levels, VkSampleCountFlagBits samples, 
                                      VkImageTiling tiling, VkImageUsageFlags usage, 
                                      VkMemoryPropertyFlags properties, VkImageAspectFlags aspect) {
    vulkan_image_t *image = (vulkan_image_t*)malloc(sizeof(vulkan_image_t));

    if (image == (vulkan_image_t*)0x0) {
        LOGF_ERR("Failed to allocate memory for image.\n");

        return image;
    }

    VkImageCreateInfo image_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {
            .width = width,
            .height = height,
            .depth = 1
        },
        .mipLevels = mip_levels,
        .arrayLayers = 1,
        .samples = samples,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    if (vkCreateImage(instance_get_device(), &image_info, (const VkAllocationCallbacks*)0x0, &image->image) != VK_SUCCESS) {
        LOGF_ERR("Failed to create image.\n");

        return image;
    }

    VkMemoryRequirements mem_reqs;
    vkGetImageMemoryRequirements(instance_get_device(), image->image, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_reqs.size,
        .memoryTypeIndex = find_memory_type(mem_reqs.memoryTypeBits, properties)
    };

    if (vkAllocateMemory(instance_get_device(), &alloc_info, (const VkAllocationCallbacks*)0x0, &image->memory) != VK_SUCCESS) {
        LOGF_ERR("Failed to allocate image memory.\n");

        return image;
    }

    vkBindImageMemory(instance_get_device(), image->image, image->memory, 0);

    VkImageViewCreateInfo view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image->image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange = {
            .aspectMask = aspect,
            .baseMipLevel = 0,
            .levelCount = mip_levels,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    if (vkCreateImageView(instance_get_device(), &view_info, (const VkAllocationCallbacks*)0x0, &image->view) != VK_SUCCESS) {
        LOGF_ERR("Failed to create image view.\n");

        return image;
    }

    return image;
}

/*
 *    Creates an image view.
 *
 *    @param VkImage image        The image.
 *    @param VkFormat format      The format of the image.
 *    @param VkImageAspectFlags   The aspect of the image.
 *    @param uint32_t mip_levels  The number of mip levels.
 * 
 *    @return VkImageView         The image view.
 */
VkImageView imageops_create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect, uint32_t mip_levels) {
    VkImageViewCreateInfo view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange = {
            .aspectMask = aspect,
            .baseMipLevel = 0,
            .levelCount = mip_levels,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    VkImageView view;

    if (vkCreateImageView(instance_get_device(), &view_info, (const VkAllocationCallbacks*)0x0, &view) != VK_SUCCESS) {
        LOGF_ERR("Failed to create image view.\n");

        return view;
    }
}

/*
 *    Destroys a vulkan image.
 *
 *    @param vulkan_image_t *image The vulkan image.
 */
void imageops_destroy_image(vulkan_image_t *image) {
    vkDestroyImageView(instance_get_device(), image->view, (const VkAllocationCallbacks*)0x0);
    vkDestroyImage(instance_get_device(), image->image, (const VkAllocationCallbacks*)0x0);
    vkFreeMemory(instance_get_device(), image->memory, (const VkAllocationCallbacks*)0x0);

    free(image);
}