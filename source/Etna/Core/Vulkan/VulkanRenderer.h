/*
 * Main Vulkan render subsystem file.
 * Whole system should function as a "black box"
 * executing clients requests.
 */

#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include "VulkanContext.h"

namespace vkc
{
    /*
     * Vulkan context data
     */

    VkInstance              Instance = VK_NULL_HANDLE;
    VkPhysicalDevice        PhysicalDevice = VK_NULL_HANDLE;
    VkDevice                Device = VK_NULL_HANDLE;

    VkQueue                 GraphicsQueue = VK_NULL_HANDLE;
    VkQueue                 TransferQueue = VK_NULL_HANDLE;
    VkQueue                 PresentationQueue = VK_NULL_HANDLE;

    VkDescriptorPool        ImGuiFontDescriptorPool = VK_NULL_HANDLE;

    uint32_t                FrameCount;
    uint32_t                FrameIndex;
    QueueFamilyIndices      QueueIndices;

    int                     MinImageCount = 2;
    bool                    SwapChainRebuild = false;

    GLFWwindow*             Window = nullptr;

    // Dynamic data
    SurfaceWindowData       WindowData;


    void Init(GLFWwindow* window);
    void Shutdown();

    /// Acquire new image from swapchain, rebuild it if necessary
    void BeginFrame();
    /// Wait for rendering to finish and present result to the screen
    void EndFrame();

    VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT		messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT				messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
}


#endif //VULKANRENDERER_H
