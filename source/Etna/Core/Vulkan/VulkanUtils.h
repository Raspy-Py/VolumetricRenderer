#ifndef VULKANSTRUCTS_H
#define VULKANSTRUCTS_H

#include "VulkanHeader.h"

#include <vector>
#include <optional>

namespace vkc
{
    /// Struct holding indices of the different queue families used by application
    struct QueueFamilyIndices
    {
        bool IsValid()
        {
            return graphicsFamily.has_value() && presentationFamily.has_value() && transferFamily.has_value();
        }

        std::optional<uint32_t> transferFamily;
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentationFamily;
    };

    /// Holds swapchain support details obtained from device properties
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    /// Info baked into every render pass
    struct RenderPassObject
    {
        VkRenderPass RenderPass;
        VkPipeline Pipeline;
    };

    /// Defines the order of render pass submits
    enum class RenderPassBindingPoint : uint8_t
    {
        PreUI, UI, PostUI
    };

    /// Data used to queue a single frame
    struct Frame
    {
        VkCommandPool CommandPool;
        VkCommandBuffer CommandBuffer;
        VkFence Fence;
        VkImage Backbuffer;
        VkImageView BackbufferView;
        VkFramebuffer Framebuffer;
    };

    struct FrameSemaphores
    {
        VkSemaphore ImageAcquiredSemaphore;
        VkSemaphore RenderCompleteSemaphore;
    };

    // Had to group this together as it used by ImGui in a similar struct
    struct SurfaceWindowData
    {
        VkExtent2D Extent;
        bool ClearEnable;
        uint32_t ImageCount;
        VkClearValue ClearValue;
        VkSwapchainKHR Swapchain;
        VkSurfaceKHR Surface;
        VkSurfaceFormatKHR SurfaceFormat;
        VkPresentModeKHR PresentMode;

        std::vector<Frame> Frames;
        std::vector<FrameSemaphores> Semaphores;

        SurfaceWindowData()
        {
            //memset((void*)this, 0, sizeof(*this));
            PresentMode = (VkPresentModeKHR) ~0;     // Ensure we get an error if user doesn't set this.
            ClearEnable = true;
        }
    };

    /*
     * Helper functions with multiple uses
     */

    void GetSwapChainSupportDetails(
        SwapChainSupportDetails &swapChainSupportDetails,
        VkPhysicalDevice device,
        VkSurfaceKHR surface);

    QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    VkCommandPool CreateCommandPool(uint32_t queueFamilyIndex);

    VkCommandBuffer CreateCommandBuffer(VkCommandPool commandPool);

    void CreateCommandBuffers(VkCommandPool commandPool, std::vector<VkCommandBuffer>& buffers, uint32_t count);

}

#endif //VULKANSTRUCTS_H
