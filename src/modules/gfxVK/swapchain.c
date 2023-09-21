/*
 *    swapchain.c    --    source for vulkan swapchain
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on April 7, 2023
 *
 *    This file is part of the Chik engine.
 *
 *    This file defines the swapchain functionality.
 */
#include "swapchain.h"

#include <vulkan/vulkan.h>

#include "libchik.h"

#include "imageops.h"
#include "instance.h"
#include "renderpasses.h"

vulkan_image_t *_color;
vulkan_image_t *_depth;

VkSwapchainKHR _swapchain;
uint32_t       _swapchain_image_count;
VkImage       *_swapchain_images;
VkImageView   *_swapchain_image_views;
VkFramebuffer *_swapchain_framebuffers;

/*
 *    Creates the swapchain.
 *
 *    @param unsigned long count    The number of images in the swapchain.
 */
void swapchain_create(unsigned long count) {
    _swapchain_image_count = 2;

    _color = imageops_create_image(
        CHIK_GFXVK_RENDERPASSES_FORMAT,
        shell_get_variable("gfx_width").i,
        shell_get_variable("gfx_height").i,
        1,
        shell_get_variable("gfx_msaa_samples").i,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT
    );

    _depth = imageops_create_image(
        VK_FORMAT_D32_SFLOAT,
        shell_get_variable("gfx_width").i,
        shell_get_variable("gfx_height").i,
        1,
        shell_get_variable("gfx_msaa_samples").i,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VK_IMAGE_ASPECT_DEPTH_BIT
    );

    VkSwapchainCreateInfoKHR swapchain_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = instance_get_surface(),
        .minImageCount = _swapchain_image_count,
        .imageFormat = CHIK_GFXVK_RENDERPASSES_FORMAT,
        .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = {
            .width = shell_get_variable("gfx_width").i,
            .height = shell_get_variable("gfx_height").i
        },
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };

    if (vkCreateSwapchainKHR(instance_get_device(), &swapchain_info, nullptr, &_swapchain) != VK_SUCCESS) {
        LOGF_ERR("Failed to create swapchain.\n");
    }
    
    _swapchain_images       = (VkImage *)malloc(sizeof(VkImage) * _swapchain_image_count);
    _swapchain_image_views  = (VkImageView *)malloc(sizeof(VkImageView) * _swapchain_image_count);
    _swapchain_framebuffers = (VkFramebuffer *)malloc(sizeof(VkFramebuffer) * _swapchain_image_count);

    vkGetSwapchainImagesKHR(instance_get_device(), _swapchain, &_swapchain_image_count, nullptr);
    vkGetSwapchainImagesKHR(instance_get_device(), _swapchain, &_swapchain_image_count, _swapchain_images);
    
    uint32_t i;
    for (i = 0; i < _swapchain_image_count; i++) {
        _swapchain_image_views[i] = imageops_create_image_view(_swapchain_images[i], CHIK_GFXVK_RENDERPASSES_FORMAT, VK_IMAGE_ASPECT_COLOR_BIT, 1);

        VkImageView attachments[3];
        
        if (shell_get_variable("gfx_msaa_samples").i > 1) {
            attachments[0] = _color->view;
            attachments[1] = _depth->view;
            attachments[2] = _swapchain_image_views[i];
        } else {
            attachments[0] = _swapchain_image_views[i];
            attachments[1] = _depth->view;
        }

        VkFramebufferCreateInfo framebuffer_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderpasses_get(),
            .attachmentCount = shell_get_variable("gfx_msaa_samples").i > 1 ? 3 : 2,
            .pAttachments = attachments,
            .width = shell_get_variable("gfx_width").i,
            .height = shell_get_variable("gfx_height").i,
            .layers = 1
        };

        if (vkCreateFramebuffer(instance_get_device(), &framebuffer_info, nullptr, &_swapchain_framebuffers[i]) != VK_SUCCESS) {
            LOGF_ERR("Failed to create framebuffer.\n");
        }
    }
}

/*
 *    Gets the swapchain.
 *
 *    @return VkSwapchainKHR    The swapchain.
 */
VkSwapchainKHR swapchain_get(void) {
    return _swapchain;
}

/*
 *    Gets the framebuffers.
 *
 *    @return VkFramebuffer *framebuffer    The framebuffers.
 */
VkFramebuffer *swapchain_get_framebuffers(void) {
    return _swapchain_framebuffers;
}

/*
 *    Destroys the swapchain.
 */
void swapchain_destroy(void) {
    uint32_t i;
    for (i = 0; i < _swapchain_image_count; i++) {
        vkDestroyImageView(instance_get_device(), _swapchain_image_views[i], nullptr);
        vkDestroyFramebuffer(instance_get_device(), _swapchain_framebuffers[i], nullptr);
    }

    vkDestroySwapchainKHR(instance_get_device(), _swapchain, nullptr);
    imageops_destroy_image(_color);
    imageops_destroy_image(_depth);

    free(_swapchain_images);
    free(_swapchain_image_views);
    free(_swapchain_framebuffers);
}