/*
 *    shader.c    --    source for shader operations
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on May 16, 2023.
 *
 *    This file is part of the Chik engine.
 *
 *    This files defines the shader operations.
 */
#include "shader.h"

#define CHIK_GFXVK_SHADER_CACHE_SIZE 256

#include "libchik.h"

#include "spvlib/spvlib.h"

#include "instance.h"
#include "renderpasses.h"

cached_shader_t _shader_cache[CHIK_GFXVK_SHADER_CACHE_SIZE] = {0};

/*
 *    Creates the descriptor set layout for the given shader.
 *
 *    @param spv_t *vert    The vertex shader to create the descriptor set layout for.
 *    @param spv_t *frag    The fragment shader to create the descriptor set layout for.
 *
 *    @return VkDescriptorSetLayout    The descriptor set layout.
 */
VkDescriptorSetLayout create_descriptor_set_layout(spv_t *vert, spv_t *frag) {
    unsigned long i;
    unsigned long j;

    unsigned long num_uniforms_vert = spv_get_uniform_count(vert);
    unsigned long num_uniforms_frag = spv_get_uniform_count(frag);

    VkDescriptorSetLayoutBinding *bindings = (VkDescriptorSetLayoutBinding *)malloc(sizeof(VkDescriptorSetLayoutBinding) * (num_uniforms_vert + num_uniforms_frag));

    if (bindings == (VkDescriptorSetLayoutBinding *)0x0) {
        LOGF_ERR("Failed to allocate memory for descriptor set layout bindings.\n");
        return (VkDescriptorSetLayout)0x0;
    }

    for (i = 0; i < num_uniforms_vert; ++i) {
        bindings[i].binding            = i;

        _api_type_e type = spv_get_uniform_type(vert, i);

        switch (type) {
            case _API_TYPE_STORAGE_BUFFER:
                bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                break;
            case _API_TYPE_UNIFORM_BUFFER:
                bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                break;
            case _API_TYPE_SAMPLER:
                bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                break;
        }

        bindings[i].descriptorCount    = 1;
        bindings[i].stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
        bindings[i].pImmutableSamplers = (VkSampler *)0x0;
    }

    for (; i < num_uniforms_vert + num_uniforms_frag; ++i) {
        bindings[i].binding            = i;

        _api_type_e type = spv_get_uniform_type(frag, i);

        switch (type) {
            case _API_TYPE_STORAGE_BUFFER:
                bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                break;
            case _API_TYPE_UNIFORM_BUFFER:
                bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                break;
            case _API_TYPE_SAMPLER:
                bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                break;
        }

        bindings[i].descriptorCount    = 1;
        bindings[i].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[i].pImmutableSamplers = (VkSampler *)0x0;
    }

    VkDescriptorSetLayoutCreateInfo layout_info;
    layout_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.pNext        = (void *)0x0;
    layout_info.flags        = 0;
    layout_info.bindingCount = num_uniforms_vert + num_uniforms_frag;
    layout_info.pBindings    = bindings;

    VkDescriptorSetLayout layout;

    if (vkCreateDescriptorSetLayout(instance_get_device(), &layout_info, (VkAllocationCallbacks *)0x0, &layout) != VK_SUCCESS) {
        LOGF_ERR("Failed to create descriptor set layout.\n");
        return (VkDescriptorSetLayout)0x0;
    }

    return layout;
}

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
void *load_shader(const char *vert_file, const char *frag_file) {
    unsigned int vert_len;
    unsigned int frag_len;

    char *vert_src = file_read(vert_file, &vert_len);
    char *frag_src = file_read(frag_file, &frag_len);

    if (vert_src == (char *)0x0) {
        LOGF_ERR("Failed to load vertex shader.\n");
        return (void *)0x0;
    }

    if (frag_src == (char *)0x0) {
        LOGF_ERR("Failed to load fragment shader.\n");
        return (void *)0x0;
    }

    unsigned int vert_crc = calc_crc32(vert_src, vert_len);
    unsigned int frag_crc = calc_crc32(frag_src, frag_len);

    unsigned long i;
    for (i = 0; i < CHIK_GFXVK_SHADER_CACHE_SIZE; i++) {
        if (_shader_cache[i].frag_crc == frag_crc && _shader_cache[i].vert_crc == vert_crc) {
            return _shader_cache[i].shader;
        }
    }

    shader_t *shader = (shader_t *)malloc(sizeof(shader_t));

    if (shader == (shader_t *)0x0) {
        LOGF_ERR("Failed to allocate memory for shader.\n");
        return (void *)0x0;
    }

    VkShaderModuleCreateInfo vert_info;
    vert_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vert_info.pNext    = (void *)0x0;
    vert_info.flags    = 0;
    vert_info.codeSize = vert_len;
    vert_info.pCode    = (uint32_t *)vert_src;

    VkShaderModule vert_module;

    if (vkCreateShaderModule(instance_get_device(), &vert_info, (VkAllocationCallbacks *)0x0, &vert_module) != VK_SUCCESS) {
        LOGF_ERR("Failed to create vertex shader module.\n");
        return (void *)0x0;
    }

    VkShaderModuleCreateInfo frag_info;
    frag_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    frag_info.pNext    = (void *)0x0;
    frag_info.flags    = 0;
    frag_info.codeSize = frag_len;
    frag_info.pCode    = (uint32_t *)frag_src;

    VkShaderModule frag_module;

    if (vkCreateShaderModule(instance_get_device(), &frag_info, (VkAllocationCallbacks *)0x0, &frag_module) != VK_SUCCESS) {
        LOGF_ERR("Failed to create fragment shader module.\n");
        return (void *)0x0;
    }

    VkPipelineShaderStageCreateInfo vert_stage_info;
    vert_stage_info.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_stage_info.pNext               = (void *)0x0;
    vert_stage_info.flags               = 0;
    vert_stage_info.stage               = VK_SHADER_STAGE_VERTEX_BIT;
    vert_stage_info.module              = vert_module;
    vert_stage_info.pName               = "main";

    VkPipelineShaderStageCreateInfo frag_stage_info;
    frag_stage_info.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_stage_info.pNext               = (void *)0x0;
    frag_stage_info.flags               = 0;
    frag_stage_info.stage               = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_stage_info.module              = frag_module;
    frag_stage_info.pName               = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_stage_info, frag_stage_info};

    spv_t *vert_spv = spv_parse(vert_src);
    spv_t *frag_spv = spv_parse(frag_src);

    unsigned long input_count = spv_get_input_count(vert_spv);
    unsigned long offset      = 0;

    VkVertexInputAttributeDescription *attr = (VkVertexInputAttributeDescription *)malloc(sizeof(VkVertexInputAttributeDescription) * input_count);

    if (attr == (VkVertexInputAttributeDescription *)0x0) {
        LOGF_ERR("Failed to allocate memory for vertex input attribute descriptions.\n");
        return (void *)0x0;
    }

    for (i = 0; i < input_count; ++i) {
        attr[i].binding  = 0;
        attr[i].location = i;

        _api_type_e type = spv_get_input_type(vert_spv, i);

        switch (type) {
            case _API_TYPE_FLOAT:
                attr[i].format = VK_FORMAT_R32_SFLOAT;
                attr[i].offset = offset + 4;
                break;
            case _API_TYPE_VEC2:
                attr[i].format = VK_FORMAT_R32G32_SFLOAT;
                attr[i].offset = offset + 8;
                break;
            case _API_TYPE_VEC3:
                attr[i].format = VK_FORMAT_R32G32B32_SFLOAT;
                attr[i].offset = offset + 12;
                break;
            case _API_TYPE_VEC4:
                attr[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
                attr[i].offset = offset + 16;
                break;
            default:
                LOGF_ERR("Invalid number of floats for vertex input.\n");
                return (void *)0x0;
        }

        offset = attr[i].offset;
    }

    VkVertexInputBindingDescription binding_desc;
    binding_desc.binding   = 0;
    binding_desc.stride    = offset;
    binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkPipelineVertexInputStateCreateInfo vertex_input_info;
    vertex_input_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.pNext                           = (void *)0x0;
    vertex_input_info.flags                           = 0;
    vertex_input_info.vertexBindingDescriptionCount   = 1;
    vertex_input_info.pVertexBindingDescriptions      = &binding_desc;
    vertex_input_info.vertexAttributeDescriptionCount = input_count;
    vertex_input_info.pVertexAttributeDescriptions    = attr;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
    input_assembly_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_info.pNext                  = (void *)0x0;
    input_assembly_info.flags                  = 0;
    input_assembly_info.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_info.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport;
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = (float)shell_get_variable("gfx_width").i;
    viewport.height   = (float)shell_get_variable("gfx_height").i;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset.x      = 0;
    scissor.offset.y      = 0;
    scissor.extent.width  = shell_get_variable("gfx_width").i;
    scissor.extent.height = shell_get_variable("gfx_height").i;

    VkPipelineViewportStateCreateInfo viewport_info;
    viewport_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_info.pNext         = (void *)0x0;
    viewport_info.flags         = 0;
    viewport_info.viewportCount = 1;
    viewport_info.pViewports    = &viewport;
    viewport_info.scissorCount  = 1;
    viewport_info.pScissors     = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer_info;
    rasterizer_info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_info.pNext                   = (void *)0x0;
    rasterizer_info.flags                   = 0;
    rasterizer_info.depthClampEnable        = VK_FALSE;
    rasterizer_info.rasterizerDiscardEnable = VK_FALSE;
    rasterizer_info.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer_info.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizer_info.frontFace               = VK_FRONT_FACE_CLOCKWISE;
    rasterizer_info.depthBiasEnable         = VK_FALSE;
    rasterizer_info.depthBiasConstantFactor = 0.0f;
    rasterizer_info.depthBiasClamp          = 0.0f;
    rasterizer_info.depthBiasSlopeFactor    = 0.0f;
    rasterizer_info.lineWidth               = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisample_info;
    multisample_info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_info.pNext                 = (void *)0x0;
    multisample_info.flags                 = 0;
    multisample_info.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisample_info.sampleShadingEnable   = VK_FALSE;
    multisample_info.minSampleShading      = 1.0f;
    multisample_info.pSampleMask           = (VkSampleMask *)0x0;
    multisample_info.alphaToCoverageEnable = VK_FALSE;
    multisample_info.alphaToOneEnable      = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment;
    color_blend_attachment.blendEnable         = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp        = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp        = VK_BLEND_OP_ADD;
    color_blend_attachment.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    
    VkPipelineColorBlendStateCreateInfo color_blend_info;
    color_blend_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_info.pNext             = (void *)0x0;
    color_blend_info.flags             = 0;
    color_blend_info.logicOpEnable     = VK_FALSE;
    color_blend_info.logicOp           = VK_LOGIC_OP_COPY;
    color_blend_info.attachmentCount   = 1;
    color_blend_info.pAttachments      = &color_blend_attachment;
    color_blend_info.blendConstants[0] = 0.0f;
    color_blend_info.blendConstants[1] = 0.0f;

    VkDescriptorSetLayout descriptor_set_layout = create_descriptor_set_layout(vert_spv, frag_spv);
    
    VkPushConstantRange push_constant_range;
    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_range.offset     = 0;
    push_constant_range.size       = 64;

    VkPipelineLayoutCreateInfo layout_info;
    layout_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.pNext                  = (void *)0x0;
    layout_info.flags                  = 0;
    layout_info.setLayoutCount         = 1;
    layout_info.pSetLayouts            = &descriptor_set_layout;
    layout_info.pushConstantRangeCount = 1;
    layout_info.pPushConstantRanges    = &push_constant_range;

    if (vkCreatePipelineLayout(instance_get_device(), &layout_info, (VkAllocationCallbacks *)0x0, &shader->layout) != VK_SUCCESS) {
        LOGF_ERR("Failed to create pipeline layout.\n");
        return (void *)0x0;
    }

    VkStencilOpState stencil_op;
    stencil_op.failOp      = VK_STENCIL_OP_KEEP;
    stencil_op.passOp      = VK_STENCIL_OP_KEEP;
    stencil_op.depthFailOp = VK_STENCIL_OP_KEEP;
    stencil_op.compareOp   = VK_COMPARE_OP_ALWAYS;
    stencil_op.compareMask = 0;
    stencil_op.writeMask   = 0;
    stencil_op.reference   = 0;

    VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
    depth_stencil_info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_info.pNext                 = (void *)0x0;
    depth_stencil_info.flags                 = 0;
    depth_stencil_info.depthTestEnable       = VK_TRUE;
    depth_stencil_info.depthWriteEnable      = VK_TRUE;
    depth_stencil_info.depthCompareOp        = VK_COMPARE_OP_LESS;
    depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_info.stencilTestEnable     = VK_FALSE;
    depth_stencil_info.front                 = stencil_op;
    depth_stencil_info.back                  = stencil_op;
    depth_stencil_info.minDepthBounds        = 0.0f;
    depth_stencil_info.maxDepthBounds        = 1.0f;

    VkGraphicsPipelineCreateInfo pipeline_info;
    pipeline_info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.pNext               = (void *)0x0;
    pipeline_info.flags               = 0;
    pipeline_info.stageCount          = 2;
    pipeline_info.pStages             = shader_stages;
    pipeline_info.pVertexInputState   = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly_info;
    pipeline_info.pTessellationState  = (VkPipelineTessellationStateCreateInfo *)0x0;
    pipeline_info.pViewportState      = &viewport_info;
    pipeline_info.pRasterizationState = &rasterizer_info;
    pipeline_info.pMultisampleState   = &multisample_info;
    pipeline_info.pDepthStencilState  = &depth_stencil_info;
    pipeline_info.pColorBlendState    = &color_blend_info;
    pipeline_info.pDynamicState       = (VkPipelineDynamicStateCreateInfo *)0x0;
    pipeline_info.layout              = shader->layout;
    pipeline_info.renderPass          = renderpasses_get();

    if (vkCreateGraphicsPipelines(instance_get_device(), VK_NULL_HANDLE, 1, &pipeline_info, (VkAllocationCallbacks *)0x0, &shader->pipeline) != VK_SUCCESS) {
        LOGF_ERR("Failed to create graphics pipeline.\n");
        return (void *)0x0;
    }
}

/*
 *    Free the given shader.
 *
 *    @param void *shader    The shader to free.
 */
void free_shader(void *shader) {

}