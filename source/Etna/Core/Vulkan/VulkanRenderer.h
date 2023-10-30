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
    // Holds data, which is passed to the delegate
    // during pass dispatching
    struct RenderPassContext
    {
        VkCommandBuffer CommandBuffer;
        VkPipelineLayout PipelineLayout;
        uint32_t ImageIndex;
        uint32_t FrameIndex;
    };

    // Function, which is called between pass.Begin() an pass.End()
    // used for binding resources
    using RenderPassDelegate = std::function<void(RenderPassContext&&)>;

    // Contains all data for dispatching render pass
    struct RenderPassContainer
    {
        RenderPassContainer(const RenderPassCreateInfo& initInfo)
        {
            Area = {};
            Pass = std::make_unique<RenderPass>(initInfo);
            Delegate = [](RenderPassContext){};
            CurrentFramebufferIndex = 0;
        }

        VkRect2D Area;
        uint32_t CurrentFramebufferIndex;
        RenderPassDelegate Delegate;
        std::vector<VkFramebuffer> Framebuffers;
        std::unique_ptr<RenderPass> Pass;
    };


    class Renderer
    {
    public:
        void Init();
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

        void AddPresentPass(const std::string& name, const RenderPassCreateInfo& initInfo);

        /// Add render pass to queue
        void EnqueueRenderPass(const std::string& name,
                               uint32_t frameBufferIndex,
                               VkRect2D area,
                               RenderPassDelegate&& delegate = [](RenderPassContext){});

        void EnqueuePresentPass(const std::string& name,
                               VkRect2D area,
                               RenderPassDelegate&& delegate = [](RenderPassContext){});

        void EnqueueCommonPass(const std::string& name,
                               VkRect2D area,
                               RenderPassDelegate&& delegate = [](RenderPassContext){});

        [[nodiscard]] VkFormat GetSwapchainImageFormat() const;
        [[nodiscard]] uint32_t GetSwapchainImageCount() const;
        [[nodiscard]] uint32_t GetSwapchainCurrentImage() const;
        [[nodiscard]] VkExtent2D GetSwapchainExtent() const;


        void CreateSwapchainFramebuffers(std::vector<VkFramebuffer>& framebuffers, const std::string& renderPass);

    public:
        [[nodiscard]] uint32_t GetFramesCount() const;
        [[nodiscard]] uint32_t GetCurrentFrame() const;

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

        // Command buffers
        std::vector<VkCommandBuffer> GraphicsCommandBuffers;

        // Render passes
        std::queue<std::string> ClientRenderQueue;
        std::map<std::string, RenderPassContainer> ClientRenderPassesMap;
    };
}


#endif //VULKANRENDERER_H
