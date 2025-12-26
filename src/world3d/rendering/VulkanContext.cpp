#include "world3d/rendering/VulkanContext.hpp"
#include <set>
#include <stdexcept>

namespace World3D::Rendering {

// Debug callback function
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    (void)messageType; (void)pUserData;
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;
    }
    return VK_FALSE;
}

VulkanContext::VulkanContext(SDL_Window* window, const std::string& appName) {
    createInstance(window, appName);
    setupDebugMessenger();
    createSurface(window); 
    pickPhysicalDevice();
    createLogicalDevice();
}

VulkanContext::~VulkanContext() {
    if (device_) {
        device_.destroy();
    }
    if (enableValidationLayers) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance_, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance_, debugMessenger_, nullptr);
        }
    }
    if (surface_) {
        instance_.destroySurfaceKHR(surface_);
    }
    if (instance_) {
        instance_.destroy();
    }
}

void VulkanContext::createInstance(SDL_Window* window, const std::string& appName) {
    bool layersFound = checkValidationLayerSupport();
    if (enableValidationLayers && !layersFound) {
        std::cerr << "Warning: Validation layers requested but not available. Continuing without them." << std::endl;
    }
    
    bool useLayers = enableValidationLayers && layersFound;

    vk::ApplicationInfo appInfo(appName.c_str(), 1, "SisterPEC Engine", 1, VK_API_VERSION_1_2);

    auto extensions = getRequiredExtensions(window);

    vk::InstanceCreateInfo createInfo({}, &appInfo, 
        useLayers ? static_cast<uint32_t>(validationLayers.size()) : 0,
        useLayers ? validationLayers.data() : nullptr,
        static_cast<uint32_t>(extensions.size()),
        extensions.data()
    );

    instance_ = vk::createInstance(createInfo);
}

void VulkanContext::setupDebugMessenger() {
    if (!enableValidationLayers || !checkValidationLayerSupport()) return;

    vk::DebugUtilsMessengerCreateInfoEXT createInfo(
        {},
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        debugCallback
    );

    // Dynamic dispatch for extension function
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance_, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        VkDebugUtilsMessengerEXT messenger;
        if (func(instance_, (const VkDebugUtilsMessengerCreateInfoEXT*)&createInfo, nullptr, &messenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
        debugMessenger_ = messenger;
    } else {
         throw std::runtime_error("failed to set up debug messenger extension!");
    }
}

void VulkanContext::createSurface(SDL_Window* window) {
    VkSurfaceKHR c_surface;
    if (!SDL_Vulkan_CreateSurface(window, instance_, &c_surface)) {
        throw std::runtime_error("failed to create window surface!");
    }
    surface_ = c_surface;
}

void VulkanContext::pickPhysicalDevice() {
    auto devices = instance_.enumeratePhysicalDevices();
    if (devices.empty()) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    for (const auto& device : devices) {
        // Simple selection: pick first discrete GPU or just first one
        auto props = device.getProperties();
        if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            physicalDevice_ = device;
            std::cout << "Selected GPU: " << props.deviceName << std::endl;
            break;
        }
    }

    if (!physicalDevice_) {
        physicalDevice_ = devices[0];
        auto props = physicalDevice_.getProperties();
        std::cout << "Selected GPU (Fallback): " << props.deviceName << std::endl;
    }
}

void VulkanContext::createLogicalDevice() {
    // Find queue families
    auto queueFamilies = physicalDevice_.getQueueFamilyProperties();
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            graphicsQueueFamilyIndex_ = i;
            break;
        }
        i++;
    }

    if (graphicsQueueFamilyIndex_ == -1) {
        throw std::runtime_error("failed to find a graphics queue family!");
    }

    float queuePriority = 1.0f;
    vk::DeviceQueueCreateInfo queueCreateInfo({}, graphicsQueueFamilyIndex_, 1, &queuePriority);

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME 
    };

    vk::DeviceCreateInfo createInfo({}, 1, &queueCreateInfo,
        enableValidationLayers ? static_cast<uint32_t>(validationLayers.size()) : 0,
        enableValidationLayers ? validationLayers.data() : nullptr,
        static_cast<uint32_t>(deviceExtensions.size()),
        deviceExtensions.data()
    );

    device_ = physicalDevice_.createDevice(createInfo);
    graphicsQueue_ = device_.getQueue(graphicsQueueFamilyIndex_, 0);
}

bool VulkanContext::checkValidationLayerSupport() {
    auto availableLayers = vk::enumerateInstanceLayerProperties();
    for (const char* layerName : validationLayers) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) return false;
    }
    return true;
}

std::vector<const char*> VulkanContext::getRequiredExtensions(SDL_Window* window) {
    unsigned int count = 0;
    
    if (!SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr)) {
        throw std::runtime_error("Failed to get SDL Vulkan extensions count");
    }

    std::vector<const char*> extensions(count);
    if (!SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data())) {
       throw std::runtime_error("Failed to get SDL Vulkan extensions");
    }

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

} // namespace World3D::Rendering
