#include "VulkanUtils.h"

#include "Etna/Core/Utils.h"
#include "VulkanContext.h"

namespace vkc
{
    void GetSwapChainSupportDetails(
        SwapChainSupportDetails &swapChainSupportDetails,
        VkPhysicalDevice device,
        VkSurfaceKHR surface)
    {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapChainSupportDetails.capabilities);

        uint32_t formatsCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatsCount, nullptr);

        if (formatsCount > 0)
        {
            swapChainSupportDetails.formats.resize(formatsCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatsCount, swapChainSupportDetails.formats.data());
        }

        uint32_t presentModesCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, nullptr);

        if (presentModesCount > 0)
        {
            swapChainSupportDetails.presentModes.resize(presentModesCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, swapChainSupportDetails.presentModes.data());
        }
    }

    QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        QueueFamilyIndices indices;

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
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationFamilySupport);

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

    /*
     * Debug messenger utils
     */

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
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

    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;
        createInfo.pUserData = nullptr;
    }

    VkCommandPool CreateCommandPool(uint32_t queueFamilyIndex)
    {
        VkCommandPoolCreateInfo commandPoolInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queueFamilyIndex
        };

        VkCommandPool commandPool;
        if (vkCreateCommandPool(Context::GetDevice(), &commandPoolInfo, Context::GetAllocator(), &commandPool) != VK_SUCCESS)
        {
            Error("Failed to create graphics command pool.");
        }

        return commandPool;
    }

    VkCommandBuffer CreateCommandBuffer(VkCommandPool commandPool)
    {
        VkCommandBufferAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        VkCommandBuffer commandBuffer;
        if (vkAllocateCommandBuffers(Context::GetDevice(), &allocInfo, &commandBuffer))
        {
            Error("Failed to allocate command buffer.");
        }

        return commandBuffer;
    }

    void CreateCommandBuffers(VkCommandPool commandPool, std::vector<VkCommandBuffer>& buffers, uint32_t count)
    {
        buffers.resize(count);
        VkCommandBufferAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = count
        };

        VkCommandBuffer commandBuffer;
        if (vkAllocateCommandBuffers(Context::GetDevice(), &allocInfo, buffers.data()))
        {
            Error("Failed to allocate %d command buffers.", count);
        }
    }
}