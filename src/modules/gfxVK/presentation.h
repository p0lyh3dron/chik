/*
 *    presentation.h    --    header for vulkan screen presentation
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on September 14, 2023.
 *
 *    This file is part of the Chik engine.
 * 
 *    This file declares the functions and data structures used to present
 *    rendered images to the screen.
 */
#ifndef CHIK_GFXVK_PRESENTATION_H
#define CHIK_GFXVK_PRESENTATION_H

#include <vulkan/vulkan.h>

/*
 *    Initialize the command pool.
 */
void presentation_init(void);

/*
 *    Destroy the command pool.
 */
void presentation_destroy(void);

/*
 *    Records GPU commands for the current frame.
 */
void presentation_record_commands(void);

/*
 *    Draws a frame.
 */
void presentation_draw_frame(void);

/*
 *    Create a command buffer for temporary use.
 */
VkCommandBuffer presentation_create_command(void);

/*
 *    Destroy a command buffer.
 */
void presentation_destroy_command(VkCommandBuffer command);

#endif /* CHIK_GFXVK_PRESENTATION_H  */