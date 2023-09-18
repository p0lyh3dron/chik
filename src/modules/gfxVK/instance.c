/*
 *    instance.c    --    source for vulkan instance
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on February 21, 2023
 *
 *    This file is part of the Chik engine.
 *
 *    This file defines functions for interfacing with the vulkan instance.
 */
#include "instance.h"

#include "libchik.h"

#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "gfxVK.h"

#define CHIK_GFXVK_INSTANCE_DEVICE_EXTENSIONS 1

#define CHIK_GFXVK_INSTANCE_LAYERS 1

const char *_device_extensions[CHIK_GFXVK_INSTANCE_DEVICE_EXTENSIONS] = (const char *[]){
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

const char *_instance_layers[CHIK_GFXVK_INSTANCE_LAYERS] = (const char *[]){
    "VK_LAYER_KHRONOS_validation",
};

VkInstance               _instance;
VkPhysicalDevice         _physical_device;
VkDebugUtilsMessengerEXT _debug_messenger;
VkDevice                 _device;
VkSurfaceKHR             _surface;
long                     _graphics_queue_idx;
long                     _present_queue_idx;
VkQueue                  _graphics_queue;
VkQueue                  _present_queue;
VkFramebuffer            _framebuffer;
VkSampler                _texture_sampler;

SDL_Window *_win;

/*
 *    The validation layer callback.
 *
 *    @param VkDebugUtilsMessageSeverityFlagBitsEXT      severity     The severity of the
 *                                                                    message.
 *    @param VkDebugUtilsMessageTypeFlagsEXT             type         The type of the
 *                                                                    message.
 *    @param const VkDebugUtilsMessengerCallbackDataEXT *data         The data of the
 *                                                                    message.
 *    @param void                                       *user_data    The user data.
 *
 *    @return VkBool32                                  Whether or not the message
 *                                                      should be suppressed.
 */
VKAPI_ATTR VkBool32 VKAPI_CALL instance_validation_layer_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      severity,
                                                                  VkDebugUtilsMessageTypeFlagsEXT             type,
                                                                  const VkDebugUtilsMessengerCallbackDataEXT *data,
                                                                  void                                       *user_data) {
    switch (severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        VLOGF_NOTE("%s\n", data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        VLOGF_NOTE("%s\n", data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        VLOGF_WARN("%s\n", data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        VLOGF_ERR("%s\n", data->pMessage);
        break;
    default:
        LOGF_ERR("Unknown severity level.\n");
        break;
    }

    return VK_FALSE;
}

/*
 *    Determines if a device supports the required extensions.
 *
 *    @param VkPhysicalDevice device    The device to check.
 *
 *    @return unsigned int             Whether or not the device supports the
 *                                     required extensions.
 */
unsigned int instance_device_supports_extensions(VkPhysicalDevice device) {
    unsigned int supported = 1;

    unsigned long extension_count = 0;
    vkEnumerateDeviceExtensionProperties(device, (const char *)0x0, &extension_count, (VkExtensionProperties *)0x0);
    VkExtensionProperties *extensions = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * extension_count);

    vkEnumerateDeviceExtensionProperties(device, (const char *)0x0, &extension_count, extensions);

    for (unsigned int i = 0; i < CHIK_GFXVK_INSTANCE_DEVICE_EXTENSIONS; i++) {
        unsigned int found = 0;

        for (unsigned int j = 0; j < extension_count; j++) {
            if (strcmp(_device_extensions[i], extensions[j].extensionName) == 0) {
                found = 1;
                break;
            }
        }

        if (!found) {
            supported = 0;
            break;
        }
    }

    free(extensions);

    return supported;
}

/*
 *    Determines if the instance supports the required layers.
 *
 *    @return unsigned int    Whether or not the instance supports the required
 *                            layers.
 */
unsigned int instance_supports_layers(void) {
    unsigned int supported = 1;

    unsigned long layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, (VkLayerProperties *)0x0);
    VkLayerProperties *layers = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * layer_count);

    vkEnumerateInstanceLayerProperties(&layer_count, layers);

    for (unsigned int i = 0; i < CHIK_GFXVK_INSTANCE_LAYERS; i++) {
        unsigned int found = 0;

        for (unsigned int j = 0; j < layer_count; j++) {
            if (strcmp(_instance_layers[i], layers[j].layerName) == 0) {
                found = 1;
                break;
            }
        }

        if (!found) {
            supported = 0;
            break;
        }
    }

    free(layers);

    return supported;
}

/*
 *    Creates validation layers.
 */
void instance_create_layers(void) {
    if (instance_supports_layers() == 0) {
        LOGF_ERR("Instance does not support required layers.\n");
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT debug_info = {
        .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext           = (const void *)0x0,
        .flags           = 0,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = instance_validation_layer_callback,
        .pUserData       = (void *)0x0,
    };

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT");

    if (vkCreateDebugUtilsMessengerEXT == (PFN_vkCreateDebugUtilsMessengerEXT)0x0) {
        LOGF_ERR("Failed to create debug messenger.\n");
        return;
    }

    if (vkCreateDebugUtilsMessengerEXT(_instance, &debug_info, (const VkAllocationCallbacks *)0x0, &_debug_messenger) != VK_SUCCESS)
        LOGF_ERR("Failed to create debug messenger.\n");
}

/*
 *    Destroys validation layers.
 */
void instance_destroy_layers(void) {
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT");

    if (vkDestroyDebugUtilsMessengerEXT == (PFN_vkDestroyDebugUtilsMessengerEXT)0x0) {
        LOGF_ERR("Failed to destroy debug messenger.\n");
        return;
    }

    vkDestroyDebugUtilsMessengerEXT(_instance, _debug_messenger, (const VkAllocationCallbacks *)0x0);
}

/*
 *    Initializes the vulkan instance.
 */
void instance_init(void) {
    unsigned int extensions_count = 0;
    _win                          = surface_get_window();

    if (SDL_Vulkan_GetInstanceExtensions(_win, &extensions_count, (const char **)0x0) == SDL_FALSE) {
        VLOGF_ERR("Failed to get instance extensions: %s.\n", SDL_GetError());
        return;
    }

    const char **extensions = (const char **)malloc(sizeof(const char *) * (extensions_count + 1));
    SDL_Vulkan_GetInstanceExtensions(_win, &extensions_count, extensions);

    if (args_has("--vklayers")) {
        extensions[extensions_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        extensions_count++;
    }

    vec3s_t version        = app_get_version();
    vec3s_t engine_version = app_get_engine_version();

    VkApplicationInfo app_info = {
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext              = (const void *)0x0,
        .pApplicationName   = app_get_name(),
        .applicationVersion = VK_MAKE_VERSION(version.x, version.y, version.z),
        .pEngineName        = app_get_engine_name(),
        .engineVersion      = VK_MAKE_VERSION(engine_version.x, engine_version.y, engine_version.z),
        .apiVersion         = VK_API_VERSION_1_0,
    };
    VkInstanceCreateInfo instance_info = {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                   = (const void *)0x0,
        .flags                   = 0,
        .pApplicationInfo        = &app_info,
        .enabledLayerCount       = args_has("--vklayers") ? CHIK_GFXVK_INSTANCE_LAYERS : 0,
        .ppEnabledLayerNames     = args_has("--vklayers") ? _instance_layers : (const char **)0x0,
        .enabledExtensionCount   = extensions_count,
        .ppEnabledExtensionNames = extensions,
    };

    VkResult result = vkCreateInstance(&instance_info, (const void *)0x0, &_instance);

    if (result != VK_SUCCESS)
        LOGF_ERR("Failed to create vulkan instance.\n");

    if (args_has("--vklayers"))
        instance_create_layers();
}

/*
 *    Picks a graphics card from the system.
 *
 *    @param unsigned long gpu    The index of the graphics card to pick.
 */
void instance_pick_gpu(unsigned long gpu) {
    unsigned long gpu_count = 0;

    vkEnumeratePhysicalDevices(_instance, &gpu_count, (VkPhysicalDevice *)0x0);

    if (gpu > gpu_count) {
        LOGF_ERR("Invalid GPU index.\n");
        return;
    }

    VkPhysicalDevice *gpus = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * gpu_count);

    vkEnumeratePhysicalDevices(_instance, &gpu_count, gpus);

    if (gpu >= gpu_count) {
        LOGF_ERR("Invalid graphics card index.\n");
        free(gpus);
        return;
    }

    if (!instance_device_supports_extensions(gpus[gpu])) {
        LOGF_ERR("Graphics card does not support required extensions.\n");
        free(gpus);
        return;
    }

    _physical_device = gpus[gpu];

    VkPhysicalDeviceProperties prop;
    vkGetPhysicalDeviceProperties(gpus[gpu], &prop);

    VLOGF_NOTE("Picked graphics card: %s.\n", prop.deviceName);

    free(gpus);
}

/*
 *    Performs the rest of the instance initialization.
 */
void instance_finish_init(void) {
    SDL_Vulkan_CreateSurface(_win, _instance, &_surface);
    if (_surface == VK_NULL_HANDLE) {
        LOGF_ERR("Failed to create vulkan surface.\n");
        return;
    }

    unsigned int queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(_physical_device, &queue_family_count, (VkQueueFamilyProperties *)0x0);

    VkQueueFamilyProperties *queue_families = (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) * queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(_physical_device, &queue_family_count, queue_families);

    _graphics_queue_idx = -1;
    _present_queue_idx  = -1;

    for (unsigned int i = 0; i < queue_family_count; i++) {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            _graphics_queue_idx = i;

        VkBool32 present_support = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(_physical_device, i, _surface, &present_support);

        if (present_support)
            _present_queue_idx = i;

        if (_graphics_queue_idx != -1 && _present_queue_idx != -1)
            break;
    }

    free(queue_families);

    if (_graphics_queue_idx == -1 || _present_queue_idx == -1) {
        LOGF_ERR("Failed to find queue families.\n");
        return;
    }

    const float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_infos[2] = {0};
    queue_infos[0] = (VkDeviceQueueCreateInfo){
        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext            = (const void *)0x0,
        .flags            = 0,
        .queueFamilyIndex = _graphics_queue_idx,
        .queueCount       = 1,
        .pQueuePriorities = &queue_priority,
    };

    queue_infos[1] = (VkDeviceQueueCreateInfo){
        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext            = (const void *)0x0,
        .flags            = 0,
        .queueFamilyIndex = _present_queue_idx,
        .queueCount       = 1,
        .pQueuePriorities = &queue_priority,
    };

    VkPhysicalDeviceFeatures device_features = {0};
    device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo device_info = {
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext                   = (const void *)0x0,
        .flags                   = 0,
        .queueCreateInfoCount    = _graphics_queue_idx == _present_queue_idx ? 1 : 2,
        .pQueueCreateInfos       = queue_infos,
        .enabledLayerCount       = 0,
        .ppEnabledLayerNames     = (const char **)0x0,
        .enabledExtensionCount   = CHIK_GFXVK_INSTANCE_DEVICE_EXTENSIONS,
        .ppEnabledExtensionNames = _device_extensions,
        .pEnabledFeatures        = &device_features,
    };

    VkResult result = vkCreateDevice(_physical_device, &device_info, (const void *)0x0, &_device);

    if (result != VK_SUCCESS) {
        LOGF_ERR("Failed to create vulkan device.\n");
        return;
    }

    vkGetDeviceQueue(_device, _graphics_queue_idx, 0, &_graphics_queue);
    vkGetDeviceQueue(_device, _present_queue_idx,  0, &_present_queue);

    VkPhysicalDeviceProperties prop;
    vkGetPhysicalDeviceProperties(_physical_device, &prop);

    VkSamplerCreateInfo sampler;
    sampler.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler.pNext                   = (const void *)0x0;
    sampler.flags                   = 0;
    sampler.magFilter               = VK_FILTER_LINEAR;
    sampler.minFilter               = VK_FILTER_LINEAR;
    sampler.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler.mipLodBias              = 0.0f;
    sampler.anisotropyEnable        = VK_TRUE;
    sampler.maxAnisotropy           = prop.limits.maxSamplerAnisotropy;
    sampler.compareEnable           = VK_FALSE;
    sampler.compareOp               = VK_COMPARE_OP_ALWAYS;
    sampler.minLod                  = 0.0f;
    sampler.maxLod                  = 0.0f;
    sampler.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler.unnormalizedCoordinates = VK_FALSE;

    if (vkCreateSampler(_device, &sampler, (const void *)0x0, &_texture_sampler) != VK_SUCCESS) {
        LOGF_ERR("Failed to create texture sampler.\n");
        return;
    }
}

/*
 *    Gets the vulkan physical device.
 *
 *    @return VkPhysicalDevice    The vulkan physical device.
 */
VkPhysicalDevice instance_get_gpu(void) {
    return _physical_device;
}

/*
 *    Gets the vulkan surface.
 *
 *    @return VkSurfaceKHR    The vulkan surface.
 */
VkSurfaceKHR instance_get_surface(void) {
    return _surface;
}

/*
 *    Gets the vulkan device.
 *
 *    @return VkDevice    The vulkan device.
 */
VkDevice instance_get_device(void) {
    return _device;
}

/*
 *    Gets the vulkan graphics queue index.
 *
 *    @return unsigned long    The vulkan graphics queue.
 */
unsigned long instance_get_graphics_queue_idx(void) {
    return _graphics_queue_idx;
}

/*
 *    Gets the vulkan graphics queue.
 *
 *    @return VkQueue    The vulkan graphics queue.
 */
VkQueue instance_get_graphics_queue(void) {
    return _graphics_queue;
}

/*
 *    Gets the vulkan present queue.
 *
 *    @return VkQueue    The vulkan present queue.
 */
VkQueue instance_get_present_queue(void) {
    return _present_queue;
}

/*
 *    Creates a GPU buffer.
 *
 *    @param VkDeviceSize          size          The size of the buffer.
 *    @param VkBufferUsageFlags    usage         The usage of the buffer.
 *    @param VkMemoryPropertyFlags properties    The properties of the buffer.
 *    @param VkBuffer             *buffer        The buffer to create.
 *    @param VkDeviceMemory       *buffer_memory The buffer memory to create.
 */
void instance_create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *buffer_memory) {
    VkBufferCreateInfo buffer_info = {
        .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext                 = (const void *)0x0,
        .flags                 = 0,
        .size                  = size,
        .usage                 = usage,
        .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices   = (unsigned int *)0x0,
    };

    if (vkCreateBuffer(_device, &buffer_info, (const void *)0x0, buffer) != VK_SUCCESS) {
        LOGF_ERR("Failed to create buffer.\n");
        return;
    }

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(_device, *buffer, &mem_requirements);

    VkMemoryAllocateInfo alloc_info = {
        .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext           = (const void *)0x0,
        .allocationSize  = mem_requirements.size,
        .memoryTypeIndex = 0,
    };

    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(_physical_device, &mem_properties);

    for (unsigned int i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((mem_requirements.memoryTypeBits & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            alloc_info.memoryTypeIndex = i;
            break;
        }
    }

    if (vkAllocateMemory(_device, &alloc_info, (const void *)0x0, buffer_memory) != VK_SUCCESS) {
        LOGF_ERR("Failed to allocate buffer memory.\n");
        return;
    }

    vkBindBufferMemory(_device, *buffer, *buffer_memory, 0);
}

/*
 *    Returns the texure sampler.
 *
 *    @return VkSampler    The texture sampler.
 */
VkSampler instance_get_texture_sampler(void) {
    return _texture_sampler;
}

/*
 *    Destroys the vulkan instance.
 */
void instance_destroy(void) {
    vkDestroySampler(_device, _texture_sampler, (const void *)0x0);
    vkDestroyDevice(_device, (const void *)0x0);
    vkDestroySurfaceKHR(_instance, _surface, (const void *)0x0);
    if (args_has("--vklayers"))
        instance_destroy_layers();

    vkDestroyInstance(_instance, (const void *)0x0);
}