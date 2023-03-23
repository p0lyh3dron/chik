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

#include <vulkan/vulkan.h>
#include <SDL2/SDL_vulkan.h>

#include "gfxVK.h"

#define CHIK_GFXVK_INSTANCE_DEVICE_EXTENSIONS 1

#define CHIK_GFXVK_INSTANCE_LAYERS 1

const char *_instance_device_extensions[CHIK_GFXVK_INSTANCE_DEVICE_EXTENSIONS] = (const char*[]) {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

const char *_instance_layers[CHIK_GFXVK_INSTANCE_LAYERS] = (const char*[]) {
    "VK_LAYER_KHRONOS_validation",
};

VkInstance               _instance;
VkPhysicalDevice         _physical_device;
VkDebugUtilsMessengerEXT _debug_messenger;

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
    vkEnumerateDeviceExtensionProperties(device, (const char*)0x0, &extension_count, (VkExtensionProperties*)0x0);
    VkExtensionProperties* extensions = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extension_count);

    vkEnumerateDeviceExtensionProperties(device, (const char*)0x0, &extension_count, extensions);

    for (unsigned int i = 0; i < CHIK_GFXVK_INSTANCE_DEVICE_EXTENSIONS; i++) {
        unsigned int found = 0;

        for (unsigned int j = 0; j < extension_count; j++) {
            if (strcmp(_instance_device_extensions[i], extensions[j].extensionName) == 0) {
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
    vkEnumerateInstanceLayerProperties(&layer_count, (VkLayerProperties*)0x0);
    VkLayerProperties* layers = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * layer_count);

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
        .pNext           = (const void*)0x0,
        .flags           = 0,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = instance_validation_layer_callback,
        .pUserData       = (void*)0x0,
    };

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT");

    if (vkCreateDebugUtilsMessengerEXT == (PFN_vkCreateDebugUtilsMessengerEXT)0x0) {
        LOGF_ERR("Failed to create debug messenger.\n");
        return;
    }

    if (vkCreateDebugUtilsMessengerEXT(_instance, &debug_info, (const VkAllocationCallbacks*)0x0, &_debug_messenger) != VK_SUCCESS)
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

    vkDestroyDebugUtilsMessengerEXT(_instance, _debug_messenger, (const VkAllocationCallbacks*)0x0);
}

/*
 *    Initializes the vulkan instance.
 */
void instance_init(void) {
    unsigned int extensions_count = 0;
    _win = surface_get_window();

    if (SDL_Vulkan_GetInstanceExtensions(_win, &extensions_count, (const char**)0x0) == SDL_FALSE) {
        VLOGF_ERR("Failed to get instance extensions: %s.\n", SDL_GetError());
        return;
    }

    const char** extensions = (const char**)malloc(sizeof(const char*) * (extensions_count + 1));
    SDL_Vulkan_GetInstanceExtensions(_win, &extensions_count, extensions);


    if (args_has("--vklayers")) {
        extensions[extensions_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
        extensions_count++;
    }

    vec3s_t version        = app_get_version();
    vec3s_t engine_version = app_get_engine_version();

    VkApplicationInfo app_info = {
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext              = (const void*)0x0,
        .pApplicationName   = app_get_name(),
        .applicationVersion = VK_MAKE_VERSION(version.x, version.y, version.z),
        .pEngineName        = app_get_engine_name(),
        .engineVersion      = VK_MAKE_VERSION(engine_version.x, engine_version.y, engine_version.z),
        .apiVersion         = VK_API_VERSION_1_3,
    };
    VkInstanceCreateInfo instance_info = {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                   = (const void*)0x0,
        .flags                   = 0,
        .pApplicationInfo        = &app_info,
        .enabledLayerCount       = args_has("--vklayers") ? CHIK_GFXVK_INSTANCE_LAYERS : 0,
        .ppEnabledLayerNames     = args_has("--vklayers") ? _instance_layers : (const char**)0x0,
        .enabledExtensionCount   = extensions_count,
        .ppEnabledExtensionNames = extensions,
    };

    VkResult   result = vkCreateInstance(&instance_info, (const void*)0x0, &_instance);

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

    vkEnumeratePhysicalDevices(_instance, &gpu_count, (VkPhysicalDevice*)0x0);

    if (gpu > gpu_count) {
        LOGF_ERR("Invalid GPU index.\n");
        return;
    }

    VkPhysicalDevice* gpus = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gpu_count);

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
 *    Destroys the vulkan instance.
 */
void instance_destroy(void) {
    if (args_has("--vklayers"))
        instance_destroy_layers();

    vkDestroyInstance(_instance, (const void*)0x0);
}