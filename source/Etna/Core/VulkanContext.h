#ifndef VULKANCONTEXT_H
#define VULKANCONTEXT_H

#include "Environment.h"

#include <cstring>
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
        std::vector <VkSurfaceFormatKHR> formats;
        std::vector <VkPresentModeKHR> presentModes;
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
        PreUI,
        UI,
        PostUI
    };

    /// Data used to queue a single frame
    struct Frame
    {
        VkCommandPool       CommandPool;
        VkCommandBuffer     CommandBuffer;
        VkFence             Fence;
        VkImage             Backbuffer;
        VkImageView         BackbufferView;
        VkFramebuffer       Framebuffer;
    };

    struct FrameSemaphores
    {
        VkSemaphore         ImageAcquiredSemaphore;
        VkSemaphore         RenderCompleteSemaphore;
    };

    // Had to group this together as it used by ImGui in a similar struct
    struct SurfaceWindowData
    {
        VkExtent2D          Extent;
        bool                ClearEnable;
        uint32_t            ImageCount;
        VkClearValue        ClearValue;
        VkSwapchainKHR      Swapchain;
        VkSurfaceKHR        Surface;
        VkSurfaceFormatKHR  SurfaceFormat;
        VkPresentModeKHR    PresentMode;

        std::vector<Frame>              Frames;
        std::vector<FrameSemaphores>    Semaphores;

        SurfaceWindowData()
        {
            memset((void*)this, 0, sizeof(*this));
            PresentMode = (VkPresentModeKHR)~0;     // Ensure we get an error if user doesn't set this.
            ClearEnable = true;
        }
    };

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

    // TODO: to be extended
    void AddRenderPass(RenderPassBindingPoint bindingPoint);

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

#endif //VULKANCONTEXT_H
