#include "VulkanInstance.h"

#include "Etna/Core/Utils.h"
#include "VulkanCore.h"

#include <cstring>

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
            instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

        return instanceExtensions;
    }

    bool InstanceBuilder::CheckInstanceExtensionsSupport(std::vector<const char*>* requiredExtensions)
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

            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
            PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = &debugCreateInfo;
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

        return instance;
    }
}