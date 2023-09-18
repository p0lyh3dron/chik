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
#include "presentation.h"

vulkan_image_t *_temp_texture;

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
 *    Transitions an image layout.
 *
 *    @param VkImage               image           The image.
 *    @param VkFormat              format          The format of the image.
 *    @param VkImageLayout         old_layout      The old layout of the image.
 *    @param VkImageLayout         new_layout      The new layout of the image.
 *    @param uint32_t              mip_levels      The number of mip levels.
 */
void imageops_transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels) {
    VkCommandBuffer command = presentation_create_command();

    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .image = image,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = mip_levels,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, (const VkMemoryBarrier*)0x0, 0, (const VkBufferMemoryBarrier*)0x0, 1, &barrier);

    presentation_destroy_command(command);
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

/*
 *    Creates the temporary texture.
 */
void imageops_create_temp_texture() {
    VkDeviceSize image_size = 2 * 2 * 4;

    char pixels[] = {
        110, 200, 250, 255,        220, 250, 255, 255,
        220, 250, 255, 255,        110, 200, 250, 255,
    };

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    instance_create_buffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_buffer_memory);

    void *data;
    vkMapMemory(instance_get_device(), staging_buffer_memory, 0, image_size, 0, &data);
    memcpy(data, pixels, (size_t)image_size);
    vkUnmapMemory(instance_get_device(), staging_buffer_memory);

    _temp_texture = imageops_create_image(VK_FORMAT_R8G8B8A8_SRGB, 2, 2, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);

    imageops_transition_image_layout(_temp_texture->image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);

    VkCommandBuffer command = presentation_create_command();

    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .imageOffset = {0, 0, 0},
        .imageExtent = {2, 2, 1},
    };

    vkCmdCopyBufferToImage(command, staging_buffer, _temp_texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    presentation_destroy_command(command);

    vkDestroyBuffer(instance_get_device(), staging_buffer, (const VkAllocationCallbacks*)0x0);
    vkFreeMemory(instance_get_device(), staging_buffer_memory, (const VkAllocationCallbacks*)0x0);
}

/*
 *    Gets the temporary texture.
 *
 *    @return VkImageView    The temporary texture.
 */
VkImageView imageops_get_temp_texture() {
    return _temp_texture->view;
}

/*
 *    Destroys the temporary texture.
 */
void imageops_destroy_temp_texture() {
    imageops_destroy_image(_temp_texture);
}