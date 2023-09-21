/*
 *    imageops.h    --    header for vulkan image manipulation functions
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on April 10, 2023
 *
 *    This file is part of the Chik engine.
 *
 *    This file declares the functions for operating on vulkan images.
 */
#ifndef CHIK_GFXVK_IMAGEOPS_H
#define CHIK_GFXVK_IMAGEOPS_H

#include "libchik.h"

#include <vulkan/vulkan.h>

typedef struct {
    VkImage        image;
    VkDeviceMemory memory;
    VkImageView    view;
} vulkan_image_t;

typedef struct {
    vulkan_image_t *image;

    image_t        *image_data;
} uploaded_image_t;

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
                                      VkMemoryPropertyFlags properties, VkImageAspectFlags aspect);

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
VkImageView imageops_create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect, uint32_t mip_levels);

/*
 *    Destroys a vulkan image.
 *
 *    @param vulkan_image_t *image The vulkan image.
 */
void imageops_destroy_image(vulkan_image_t *image);

/*
 *    Uploads an image to the GPU.
 *
 *    @param image_t *image        The image.
 */
void imageops_upload_image(image_t *image);

/*
 *    Retreives a cached image.
 *
 *    @param image_t *image        The image.
 *
 *    @return vulkan_image_t *     The vulkan image.
 */
vulkan_image_t *imageops_get_cached_image(image_t *image);

/*
 *    Creates the temporary texture.
 */
void imageops_create_temp_texture();

/*
 *    Gets the temporary texture.
 *
 *    @return VkImageView    The temporary texture.
 */
VkImageView imageops_get_temp_texture();

/*
 *    Destroys the temporary texture.
 */
void imageops_destroy_temp_texture();

#endif /* CHIK_GFXVK_IMAGEOPS_H  */