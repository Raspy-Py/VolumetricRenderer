#include "VulkanRenderer.h"

#include "Etna/Core/Utils.h"
#include "VulkanContext.h"

/*
 static const int c_MaxFramesInFlight = 2;

static const std::vector<const char*> c_ValidationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
static const std::vector<const char*> c_DeviceExtensions = {
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
    VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
#endif
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
static const std::vector<const char*> c_DeviceTypes	= {
    "Other",
    "Integrated GPU",
    "Discrete GPU",
    "Virtual GPU",
    "CPU"
};
static const std::vector<VkDynamicState> c_DynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
};

#ifndef NDEBUG
static const bool c_EnableValidationLayers = true;
#else
static const bool c_EnableValidationLayers = false;
#endif // !NDEBUG


//Hidden helper functions


static bool IsExtensionAvailable(const std::vector<VkExtensionProperties>& properties, const char* extension)
{
    for (const VkExtensionProperties& p : properties)
    {
        if (strcmp(p.extensionName, extension) == 0)
        {
            return true;
        }
    }
    return false;
}

static vkc::QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device)
{
    vkc::QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyList.data());

    for (uint32_t i = 0; i < queueFamilyCount; i++)
    {
        const auto& queueFamily = queueFamilyList[i];

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            indices.transferFamily = i;
        }

        VkBool32 presentationFamilySupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vkc::WindowData.Surface, &presentationFamilySupport);

        if (presentationFamilySupport)
        {
            indices.presentationFamily = i;
        }

        if (indices.IsValid())
        {
            break;
        }
    }

    return indices;
}

static std::vector<const char*> GetRequiredExtensions()
{
    uint32_t glfwExtensionsCount = 0;
    const char** ppGlfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);

    std::vector<const char*> instanceExtensions(ppGlfwExtensions, ppGlfwExtensions + glfwExtensionsCount);

    if (c_EnableValidationLayers)
    {
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return instanceExtensions;
}


static bool CheckValidationLayersSupport()
{
    uint32_t layersCount = 0;
    vkEnumerateInstanceLayerProperties(&layersCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layersCount);
    vkEnumerateInstanceLayerProperties(&layersCount, availableLayers.data());

    for (const auto& layer : c_ValidationLayers)
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

static bool CheckInstanceExtensionsSupport(std::vector<const char*>* requiredExtensions)
{
    uint32_t extensionsCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);

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

static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = vkc::DebugCallback;
    createInfo.pUserData = nullptr;
}


static void CreateInstance()
{
    // Checking whether requested validation layers are available
    if (c_EnableValidationLayers && !CheckValidationLayersSupport())
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
    if (c_EnableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(c_ValidationLayers.size());
        createInfo.ppEnabledLayerNames = c_ValidationLayers.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &vkc::Instance) != VK_SUCCESS)
    {
        Error("Failed to create a Vulkan instance.");
    }
}


static VkSurfaceFormatKHR ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
    for (const auto& format : formats)
    {
        if (format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }

    return formats[0];
}

static VkPresentModeKHR ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& presentModes)
{
    for (const auto& presentMode : presentModes)
    {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return presentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width{}, height{};
        glfwGetFramebufferSize(vkc::Window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }

    return VkExtent2D();
}

static void CleanupSwapChain()
{
    for (size_t i = 0; i < vkc::WindowData.Frames.size(); i++)
    {
        vkDestroyFramebuffer(vkc::Device, vkc::WindowData.Frames[i].Framebuffer, nullptr);
    }

    for (size_t i = 0; i < vkc::WindowData.Frames.size(); i++)
    {
        vkDestroyImageView(vkc::Device, vkc::WindowData.Frames[i].BackbufferView, nullptr);
    }

    vkDestroySwapchainKHR(vkc::Device, vkc::WindowData.Swapchain, nullptr);
}

static void GetSwapChainSupportDetails(vkc::SwapChainSupportDetails& swapChainSupportDetails, VkPhysicalDevice device)
{
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vkc::WindowData.Surface, &swapChainSupportDetails.capabilities);

    uint32_t formatsCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkc::WindowData.Surface, &formatsCount, nullptr);

    if (formatsCount > 0)
    {
        swapChainSupportDetails.formats.resize(formatsCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkc::WindowData.Surface, &formatsCount, swapChainSupportDetails.formats.data());
    }

    uint32_t presentModesCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkc::WindowData.Surface, &presentModesCount, nullptr);

    if (presentModesCount > 0)
    {
        swapChainSupportDetails.presentModes.resize(presentModesCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkc::WindowData.Surface, &presentModesCount, swapChainSupportDetails.presentModes.data());
    }
}

static void CreateSwapChain()
{
    vkc::SwapChainSupportDetails swapChainSupport{};
    GetSwapChainSupportDetails(swapChainSupport, vkc::PhysicalDevice);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapChainSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapChainPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapChainExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.minImageCount > 0 &&
        imageCount < swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = vkc::WindowData.Surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto indices = GetQueueFamilies(vkc::PhysicalDevice);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(),  indices.presentationFamily.value() };
    if (indices.graphicsFamily != indices.presentationFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(vkc::Device, &createInfo, nullptr, &vkc::WindowData.Swapchain) != VK_SUCCESS)
    {
        Error("Failed to create swap chain.");
    }

    vkGetSwapchainImagesKHR(vkc::Device, vkc::WindowData.Swapchain, &vkc::WindowData.ImageCount, nullptr);
    vkc::WindowData.Frames.resize(vkc::WindowData.ImageCount);

    VkImage backBuffers[16]{};
    vkGetSwapchainImagesKHR(vkc::Device, vkc::WindowData.Swapchain, &imageCount, backBuffers);

    vkc::WindowData.SurfaceFormat = surfaceFormat;
    vkc::WindowData.Extent = extent;
}

static void CreateSwapChainImageViews()
{
    for (size_t i = 0; i < vkc::WindowData.ImageCount; i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = vkc::WindowData.Frames[i].Backbuffer;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = vkc::WindowData.SurfaceFormat.format;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(vkc::Device, &createInfo, nullptr, &vkc::WindowData.Frames[i].BackbufferView) != VK_SUCCESS)
        {
            Error("Failed to create swap chain image view");
        }
    }
}

static void CreateFramebuffers()
{
    for (size_t i = 0; i < vkc::WindowData.ImageCount; i++)
    {
        VkImageView attachments[] = {
            vkc::WindowData.Frames[i].BackbufferView
        };

        VkFramebufferCreateInfo frameBufferInfo{};
        frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferInfo.renderPass = m_RenderPass; // FUUUUUUUUUCK
        frameBufferInfo.attachmentCount = 1;
        frameBufferInfo.pAttachments = attachments;
        frameBufferInfo.width = vkc::WindowData.Extent.width;
        frameBufferInfo.height = vkc::WindowData.Extent.height;
        frameBufferInfo.layers = 1;

        if (vkCreateFramebuffer(vkc::Device, &frameBufferInfo, nullptr, &vkc::WindowData.Frames[i].Framebuffer) != VK_SUCCESS)
        {
            Error("Failed to create framebuffer.");
        }
    }
}

static void RecreateSwapChain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(vkc::Window, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(vkc::Window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(vkc::Device);

    CleanupSwapChain();

    CreateSwapChain();
    CreateSwapChainImageViews();
    CreateFramebuffers();
}

static VkPhysicalDevice SetupVulkan_SelectPhysicalDevice()
{
    uint32_t GPUCount;
    if (vkEnumeratePhysicalDevices(vkc::Instance, &GPUCount, nullptr))
    {
        Error("Failed to enumerate physical devices.");
    }

    IM_ASSERT(GPUCount > 0);

    ImVector<VkPhysicalDevice> GPUs;
    GPUs.resize(GPUCount);
    if (vkEnumeratePhysicalDevices(vkc::Instance, &GPUCount, GPUs.Data))
    {
        Error("Failed to enumerate physical devices.");
    }

    // If a number >1 of GPUs got reported, find discrete GPU if present, or use first one available. This covers
    // most common cases (multi-gpu/integrated+dedicated graphics). Handling more complicated setups (multiple
    // dedicated GPUs) is out of scope of this sample.
    for (VkPhysicalDevice& device : GPUs)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            return device;
        }
    }

    // Use first GPU (Integrated) is a Discrete one is not available.
    if (GPUCount > 0)
    {
        return GPUs[0];
    }

    return VK_NULL_HANDLE;
}


static void CreateLogicalDevice()
{

    std::set<uint32_t> queueFamilyIndices = {
        vkc::QueueIndices.graphicsFamily.value(),
        vkc::QueueIndices.presentationFamily.value(),
        vkc::QueueIndices.transferFamily.value()
    };
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    float priority = 1.0f;
    for (uint32_t queueFamilyIndex : queueFamilyIndices)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &priority;

        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(c_DeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = c_DeviceExtensions.data();
    createInfo.enabledLayerCount = 0;

    VkPhysicalDeviceFeatures deviceFeatures{};

    createInfo.pEnabledFeatures = &deviceFeatures;

    if (vkCreateDevice(vkc::PhysicalDevice, &createInfo, nullptr, &vkc::Device) != VK_SUCCESS)
    {
        Error("Failed to create a logical device.");
    }

    vkGetDeviceQueue(vkc::Device, vkc::QueueIndices.transferFamily.value(), 0, &vkc::TransferQueue);
    vkGetDeviceQueue(vkc::Device, vkc::QueueIndices.graphicsFamily.value(), 0, &vkc::GraphicsQueue);
    vkGetDeviceQueue(vkc::Device, vkc::QueueIndices.presentationFamily.value(), 0, &vkc::PresentationQueue);
}

static void SetupVulkan()
{
    VkResult err;

    // Create Vulkan Instance
    CreateInstance();

    // Select Physical Device (GPU)
    vkc::PhysicalDevice = SetupVulkan_SelectPhysicalDevice();

    // Create Window Surface
    if (glfwCreateWindowSurface(vkc::Instance, vkc::Window, nullptr, &vkc::WindowData.Surface))
    {
        Error("Failed to create window surface.");
    }


    // Select graphics queue family
    vkc::QueueIndices = GetQueueFamilies(vkc::PhysicalDevice);

    // Create Logical Device (with 1 queue)
    CreateLogicalDevice();

    // IMGUI SPECIFIC!!!
    // Create Descriptor Pool
    // The example only requires a single combined image sampler descriptor for the font image and only uses one descriptor set (for that)
    // If you wish to load e.g. additional textures you may need to alter pools sizes.
    {
        VkDescriptorPoolSize pool_sizes[] =
            {
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
            };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1;
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        if (vkCreateDescriptorPool(vkc::Device, &pool_info, nullptr, &vkc::ImGuiFontDescriptorPool))
        {
            Error("Failed to create descriptor pool for ImGui font texture.");
        }
    }
}


*/

namespace vkc
{
    void Init(GLFWwindow* window)
    {
        Window = window;

        Context::Create(window);
    }

    void Shutdown()
    {
        Context::Destroy();
    }

    void BeginFrame()
    {

    }

    void EndFrame()
    {
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapchains[] = { vkc::WindowData.Swapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        VkResult result = vkQueuePresentKHR(PresentationQueue, &presentInfo);

        // Check for swapchain result
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || SwapChainRebuild)
        {
            SwapChainRebuild = false;
            RecreateSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            Error("Failed to present swap chain image.");
        }

        m_CurrentFrame = (m_CurrentFrame + 1) % c_MaxFramesInFlight;
    }


    VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT		messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT				messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        switch (messageSeverity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                Warning("Validation layer: %s", pCallbackData->pMessage); break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                Error("Validation layer: %s", pCallbackData->pMessage);  break;
            default:
                // Ignore high-verbosity logs
                //InfoLog("Validation layer: %s", pCallbackData->pMessage);
                break;
        }

        return VK_FALSE;
    }
}
