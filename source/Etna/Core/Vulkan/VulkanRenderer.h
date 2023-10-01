/*
 * Main Vulkan render subsystem file.
 * Whole system should function as a "black box"
 * executing client's requests.
 */

#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include "VulkanContext.h"

namespace vkc
{
    /*
     * Vulkan context data
     */


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
