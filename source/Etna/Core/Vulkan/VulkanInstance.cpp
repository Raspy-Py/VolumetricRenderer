#include "VulkanInstance.h"

#include "Etna/Core/Utils.h"
#include "VulkanCore.h"

#include <cstring>


static VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;


static bool IsExtensionAvailable(const std::vector<VkExtensionProperties>& properties, const char* extension)
{
    for (const VkExtensionProperties& p : properties)
        if (strcmp(p.extensionName, extension) == 0)
            return true;
    return false;
}


#ifdef _DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objectType,
    uint64_t object,
    size_t location,
    int32_t messageCode,
    const char* pLayerPrefix,
    const char* pMessage,
    void* pUserData)
{
    (void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
    fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
    return VK_FALSE;
}
#endif // _DENUG

namespace vkc
{
#ifdef _DEBUG
    static const bool EnableValidationLayers = true;
#else
    static const bool EnableValidationLayers = false;
#endif // _DEBUG

    static const std::vector<const char*> ValidationLayers	= {
        "VK_LAYER_KHRONOS_validation"
    };

    std::vector<const char*> InstanceBuilder::GetRequiredExtensions()
    {
        uint32_t glfwExtensionsCount = 0;
        const char** ppGlfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

        std::vector<const char*> instanceExtensions(ppGlfwExtensions, ppGlfwExtensions + glfwExtensionsCount);

        if (EnableValidationLayers)
        {
            //instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        uint32_t properties_count;
        std::vector<VkExtensionProperties> properties;
        vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
        properties.resize(properties_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, properties.data());
;

        if (IsExtensionAvailable(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
            instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

        //instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        instanceExtensions.push_back("VK_EXT_debug_report");

        return instanceExtensions;
    }

    bool InstanceBuilder::CheckInstanceExtensionsSupport(const std::vector<const char*>* requiredExtensions)
    {
        // Get the extensions count first
        uint32_t extensionsCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);

        // Get them
        std::vector<VkExtensionProperties>  extensions(extensionsCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensions.data());

        for (const auto& requiredExtension : *requiredExtensions)
        {
            bool foundExtension = false;

            for (const auto& extension : extensions)
            {
                if (strcmp(extension.extensionName, requiredExtension) == 0)
                {
                    foundExtension = true;
                    break;
                }
            }

            if (!foundExtension)
            {
                return false;
            }
        }

        return true;
    }

    bool InstanceBuilder::CheckValidationLayersSupport()
    {
        uint32_t layersCount = 0;
        vkEnumerateInstanceLayerProperties(&layersCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layersCount);
        vkEnumerateInstanceLayerProperties(&layersCount, availableLayers.data());

        for (const auto& layer : ValidationLayers)
        {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers)
            {
                if (strcmp(layer, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                return false;
            }
        }

        return true;
    }

    Instance InstanceBuilder::Build()
    {
        // Checking whether requested validation layers are available
        if (EnableValidationLayers && !CheckValidationLayersSupport())
        {
            Error("Some validation layers are requested, but not available!");
        }

        // Application information
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vulkan Course Application";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Vulkan Course Application Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        // List to hold instance extensions
        auto instanceExtensions = GetRequiredExtensions();
        if (!CheckInstanceExtensionsSupport(&instanceExtensions))
        {
            Error("VkInstance does not support required extension.");
        }

        // Creation info for the VkInstance
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
        createInfo.ppEnabledExtensionNames = instanceExtensions.data();
        if (EnableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
            createInfo.ppEnabledLayerNames = ValidationLayers.data();

            //VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
            //PopulateDebugMessengerCreateInfo(debugCreateInfo);
            //createInfo.pNext = &debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
        }

        Instance instance{};
        if (vkCreateInstance(&createInfo, nullptr, &instance.Handle) != VK_SUCCESS)
        {
            Error("Failed to create a Vulkan instance.");
        }

        auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance.Handle, "vkCreateDebugReportCallbackEXT");
        if(vkCreateDebugReportCallbackEXT == nullptr)
        {
            Error("Failed to load vkCreateDebugReportCallbackEXT. ");
        }
        VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
        debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        debug_report_ci.pfnCallback = debug_report;
        debug_report_ci.pUserData = nullptr;
        if (vkCreateDebugReportCallbackEXT(instance.Handle, &debug_report_ci, nullptr, &g_DebugReport) != VK_SUCCESS)
        {
            Error("Failed to create debug report object.");
        }

        return instance;
    }
}