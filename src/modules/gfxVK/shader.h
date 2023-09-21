/*
 *    shader.h    --    header for shader operations
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on May 16, 2023.
 *
 *    This file is part of the Chik engine.
 *
 *    This file declares the shader type, and funcions for loading and
 *    setting shader variables.
 */
#ifndef CHIK_GFXVK_SHADER_H
#define CHIK_GFXVK_SHADER_H

#define CHIK_GFXVK_FRAMES_IN_FLIGHT 2

#include <vulkan/vulkan.h>

#include "spvlib/spvlib.h"

typedef struct {
    VkDescriptorSetLayout d_layout;
    VkPipelineLayout      p_layout;
    VkPipeline            pipeline;

    spv_t                *vert_spv;
    spv_t                *frag_spv;
} shader_t;

typedef struct {
    VkBuffer        buffer;
    VkDeviceMemory  memory;
    VkDeviceSize    size;
    unsigned long   count;
} vbuffer_t;

typedef struct {
    VkBuffer       *buffer;
    VkDeviceMemory *memory;
    void          **data;
} uniform_t;

typedef struct {
    vbuffer_t       *vbuffer;
    VkDescriptorSet *d_set;

    uniform_t        uniforms[CHIK_GFXVK_FRAMES_IN_FLIGHT];

    shader_t        *shader;
} mesh_t;

typedef struct {
    unsigned int frag_crc;
    unsigned int vert_crc;

    shader_t *shader;
} cached_shader_t;

/*
 *    Initializes the descriptor pool.
 */
void shader_init(void);

/*
 *    Frees the descriptor pool.
 */
void shader_exit(void);

/*
 *    Loads a shader from the given files, and returns a pointer to it.
 *
 *    If the shader has already been loaded, it will be returned from the
 *    cache instead of being reloaded.
 *
 *    If the shader has not been loaded, it will be loaded and cached.
 * 
 *    @param const char *vert_file    The vertex shader file to load.
 *    @param const char *frag_file    The fragment shader file to load.
 * 
 *    @return void *    A pointer to the loaded shader.
 */
void *load_shader(const char *vert_file, const char *frag_file);

/*
 *    Free the given shader.
 *
 *    @param void *shader    The shader to free.
 */
void free_shader(void *shader);

/*
 *    Returns the list of draw commands.
 *
 *    @param unsigned int *count    The number of draw commands.
 *
 *    @return mesh_t **    The list of draw commands.
 */
mesh_t **get_draw_commands(unsigned int *count);

#endif /* CHIK_GFX_VK_SHADER_H  */