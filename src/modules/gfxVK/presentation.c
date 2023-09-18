/*
 *    presentation.c    --    source for vulkan screen presentation
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on September 14, 2023.
 *
 *    This file is part of the Chik engine.
 * 
 *    This file defines the command pool, command buffers, and
 *    synchronization elements used for recrding GPU commands and
 *    presenting them to the screen.
 */
#include "presentation.h"

#include "libchik.h"

#include "instance.h"
#include "shader.h"
#include "swapchain.h"
#include "renderpasses.h"

VkCommandPool   _command_pool;
VkCommandBuffer _command_buffers[CHIK_GFXVK_FRAMES_IN_FLIGHT];

VkSemaphore _image_available_semaphores[CHIK_GFXVK_FRAMES_IN_FLIGHT];
VkSemaphore _render_finished_semaphores[CHIK_GFXVK_FRAMES_IN_FLIGHT];
VkFence     _in_flight_fences[CHIK_GFXVK_FRAMES_IN_FLIGHT];

unsigned long _current_frame = 0;

/*
 *    Initialize the command pool.
 */
void presentation_init(void) {
    VkCommandPoolCreateInfo pool_info = {0};
    pool_info.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex        = instance_get_graphics_queue_idx();
    pool_info.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(instance_get_device(), &pool_info, (const VkAllocationCallbacks *)0x0, &_command_pool) != VK_SUCCESS) {
        LOGF_ERR("Failed to create command pool.\n");
    }

    VkCommandBufferAllocateInfo alloc_info = {0};
    alloc_info.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool                 = _command_pool;
    alloc_info.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount          = CHIK_GFXVK_FRAMES_IN_FLIGHT;

    if (vkAllocateCommandBuffers(instance_get_device(), &alloc_info, _command_buffers) != VK_SUCCESS) {
        LOGF_ERR("Failed to allocate command buffers.\n");
    }

    VkSemaphoreCreateInfo semaphore_info = {0};
    semaphore_info.sType                  = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {0};
    fence_info.sType              = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags              = VK_FENCE_CREATE_SIGNALED_BIT;

    for (unsigned int i = 0; i < CHIK_GFXVK_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(instance_get_device(), &semaphore_info, (const VkAllocationCallbacks *)0x0, &_image_available_semaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(instance_get_device(), &semaphore_info, (const VkAllocationCallbacks *)0x0, &_render_finished_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(instance_get_device(), &fence_info, (const VkAllocationCallbacks *)0x0, &_in_flight_fences[i]) != VK_SUCCESS) {
            LOGF_ERR("Failed to create synchronization objects for a frame.\n");
        }
    }
}

/*
 *    Destroy the command pool.
 */
void presentation_destroy(void) {
    for (unsigned int i = 0; i < CHIK_GFXVK_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(instance_get_device(), _render_finished_semaphores[i], (const VkAllocationCallbacks *)0x0);
        vkDestroySemaphore(instance_get_device(), _image_available_semaphores[i], (const VkAllocationCallbacks *)0x0);
        vkDestroyFence(instance_get_device(), _in_flight_fences[i], (const VkAllocationCallbacks *)0x0);
    }

    vkDestroyCommandPool(instance_get_device(), _command_pool, (const VkAllocationCallbacks *)0x0);
}

/*
 *    Records GPU commands for the current frame.
 */
void presentation_record_commands(void) {
    unsigned int   count;
    unsigned int   i;
    mesh_t       **meshes = get_draw_commands(&count);

    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(_command_buffers[_current_frame], &begin_info) != VK_SUCCESS) {
        LOGF_ERR("Failed to begin recording command buffer.\n");
    }

    VkRenderPassBeginInfo render_pass_info = {0};
    render_pass_info.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass             = renderpasses_get();
    render_pass_info.framebuffer            = swapchain_get_framebuffers()[_current_frame];
    render_pass_info.renderArea.offset      = (VkOffset2D){0, 0};
    render_pass_info.renderArea.extent      = (VkExtent2D){shell_get_variable("gfx_width").i, shell_get_variable("gfx_height").i};

    VkClearValue clear_values[2] = {
        {.color = {0.0f, 0.0f, 0.0f, 1.0f}},
        {.depthStencil = {1.0f, 0}}
    };

    render_pass_info.clearValueCount = 2;
    render_pass_info.pClearValues    = clear_values;

    vkCmdBeginRenderPass(_command_buffers[_current_frame], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {0};
    viewport.x          = 0.0f;
    viewport.y          = 0.0f;
    viewport.width      = (float)shell_get_variable("gfx_width").i;
    viewport.height     = (float)shell_get_variable("gfx_height").i;
    viewport.minDepth   = 0.0f;
    viewport.maxDepth   = 1.0f;

    VkRect2D scissor = {0};
    scissor.offset   = (VkOffset2D){0, 0};
    scissor.extent   = (VkExtent2D){shell_get_variable("gfx_width").i, shell_get_variable("gfx_height").i};

    vkCmdSetViewport(_command_buffers[_current_frame], 0, 1, &viewport);
    vkCmdSetScissor(_command_buffers[_current_frame], 0, 1, &scissor);

    for (i = 0; i < count; ++i) {
        VkBuffer vertex_buffers[] = {meshes[i]->vbuffer->buffer};
        VkDeviceSize offsets[]    = {0};

        vkCmdBindPipeline(_command_buffers[_current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, shader_get_pipeline(meshes[i]->shader));
        vkCmdBindVertexBuffers(_command_buffers[_current_frame], 0, 1, vertex_buffers, offsets);
        vkCmdBindDescriptorSets(_command_buffers[_current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, meshes[i]->shader->p_layout, 0, 1, &meshes[i]->d_set[_current_frame], 0, nullptr);
        vkCmdDraw(_command_buffers[_current_frame], 3, 1, 0, 0);
    }
    
    vkCmdEndRenderPass(_command_buffers[_current_frame]);

    if (vkEndCommandBuffer(_command_buffers[_current_frame]) != VK_SUCCESS) {
        LOGF_ERR("Failed to record command buffer.\n");
    }
}

/*
 *    Draws a frame.
 */
void presentation_draw_frame(void) {
    vkWaitForFences(instance_get_device(), 1, &_in_flight_fences[_current_frame], VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(instance_get_device(), swapchain_get(), UINT64_MAX, _image_available_semaphores[_current_frame], VK_NULL_HANDLE, &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        LOGF_ERR("Swapchain out of date.\n");
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        LOGF_ERR("Failed to acquire swapchain image.\n");
    }

    vkResetFences(instance_get_device(), 1, &_in_flight_fences[_current_frame]);
    vkResetCommandBuffer(_command_buffers[_current_frame], 0);

    presentation_record_commands();

    VkSubmitInfo submit_info = {0};
    submit_info.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {_image_available_semaphores[_current_frame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores    = wait_semaphores;
    submit_info.pWaitDstStageMask  = wait_stages;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers    = &_command_buffers[_current_frame];

    VkSemaphore signal_semaphores[] = {_render_finished_semaphores[_current_frame]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores    = signal_semaphores;

    if (vkQueueSubmit(instance_get_graphics_queue(), 1, &submit_info, _in_flight_fences[_current_frame]) != VK_SUCCESS) {
        LOGF_ERR("Failed to submit draw command buffer.\n");
    }

    VkPresentInfoKHR present_info = {0};
    present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = signal_semaphores;

    VkSwapchainKHR swapchains[] = {swapchain_get()};
    present_info.swapchainCount = 1;
    present_info.pSwapchains    = swapchains;
    present_info.pImageIndices  = &image_index;

    result = vkQueuePresentKHR(instance_get_present_queue(), &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        LOGF_ERR("Swapchain out of date.\n");
    } else if (result != VK_SUCCESS) {
        LOGF_ERR("Failed to present swapchain image.\n");
    }

    _current_frame = (_current_frame + 1) % CHIK_GFXVK_FRAMES_IN_FLIGHT;
}

/*
 *    Create a command buffer for temporary use.
 */
VkCommandBuffer presentation_create_command(void) {
    VkCommandBufferAllocateInfo alloc_info = {0};
    alloc_info.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool                 = _command_pool;
    alloc_info.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount          = 1;

    VkCommandBuffer command;
    vkAllocateCommandBuffers(instance_get_device(), &alloc_info, &command);

    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command, &begin_info);

    return command;
}

/*
 *    Destroy a command buffer.
 */
void presentation_destroy_command(VkCommandBuffer command) {
    vkEndCommandBuffer(command);

    VkSubmitInfo submit_info       = {0};
    submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers    = &command;

    vkQueueSubmit(instance_get_graphics_queue(), 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(instance_get_graphics_queue());

    vkFreeCommandBuffers(instance_get_device(), _command_pool, 1, &command);
}