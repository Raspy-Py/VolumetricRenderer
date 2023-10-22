/*
 * Main Vulkan render subsystem file.
 * Whole system should function as a "black box"
 * executing client's requests.
 */

#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include "VulkanContext.h"
#include "VulkanSwapchain.h"
#include "VulkanRenderPass.h"

#include <map>
#include <queue>
#include <vector>
#include <functional>

namespace vkc
{
    using RenderPassDelegate = std::function<void(VkCommandBuffer)>;
    struct RenderPassDispatchInfo
    {
        RenderPassDispatchInfo(const RenderPassCreateInfo& initInfo)
        {
            Area = {};
            FrameBuffer = VK_NULL_HANDLE;
            Pass = std::make_unique<RenderPass>(initInfo);
            Delegate = [](VkCommandBuffer){};
        }

        VkRect2D Area;
        VkFramebuffer FrameBuffer;
        std::unique_ptr<RenderPass> Pass;
        std::function<void(VkCommandBuffer)> Delegate;
    };

    class Renderer
    {
    public:
        void Init(GLFWwindow* window);
        void Shutdown();

        /// Acquire new image from swapchain, rebuild it if necessary
        void BeginFrame();

        /// Wait for rendering to finish and present result to the screen
        void EndFrame();

        /// Dispatche all enqueued client passes
        void RenderFrame();

        /// Render ImGui
        void RenderGUI();

        /// Register new render pass
        void AddRenderPass(const std::string& name, const RenderPassCreateInfo& initInfo);

        /// Add render pass to queue
        void EnqueueRenderPass(const std::string& name,
                               VkFramebuffer framebuffer,
                               VkRect2D area,
                               RenderPassDelegate delegate = [](VkCommandBuffer){});

        [[nodiscard]] VkFormat GetSwapchainImageFormat() const;
        [[nodiscard]] uint32_t GetSwapchainCurrentImage() const;
        [[nodiscard]] VkExtent2D GetSwapchainExtent() const;


        void CreateSwapchainFramebuffers(std::vector<VkFramebuffer>& framebuffers, const std::string& renderPass);

    private:
        uint32_t MaxFramesInFlight;
        uint32_t CurrentFrame;
        vkc::Swapchain GSwapchain;

        // Synchronization primitives. One of a type per frame in flight
        std::vector<VkFence> FrameFences;
        std::vector<VkSemaphore> ImageAvailableSemaphores;
        std::vector<VkSemaphore> RenderFinishedSemaphores;

        // Command pools
        std::vector<VkCommandPool> GraphicsCommandPools;
        VkCommandPool TransferCommandPool;

        // Command buffers
        std::vector<VkCommandBuffer> GraphicsCommandBuffers;

        // Render passes
        std::queue<std::string> ClientRenderQueue;
        std::map<std::string, RenderPassDispatchInfo> ClientRenderPassesMap;
    };
}


#endif //VULKANRENDERER_H
