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
#include "VulkanTexture.h"

#include "imgui.h"

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

        uint32_t ImageIndex; // Index of an image, acquired by swapchain
        uint32_t FrameIndex; // Index of a frame in flight
    };

    // Function, which is called between pass.Begin() an pass.End()
    // used for binding resources and making draw calls
    using RenderPassDelegate = std::function<void(RenderPassContext&&)>;

    struct RenderPassContainer
    {
        RenderPassContainer() = default;
        RenderPassContainer(const RenderPassCreateInfo& initInfo)
        {
            Area = {};
            Pass = Ref<RenderPass>(new RenderPass(initInfo));
            Delegate = [](RenderPassContext&&){};
            CurrentFramebufferIndex = 0;
        }

        VkRect2D Area;
        uint32_t CurrentFramebufferIndex;
        RenderPassDelegate Delegate;
        std::vector<VkFramebuffer> Framebuffers;
        Ref<RenderPass> Pass;
        Ref<Texture2D> DepthBufferTexture;
    };

    class Renderer
    {
    private:
        [[nodiscard]] VkRenderPass CreateGUIRenderPass() const;
        [[nodiscard]] VkDescriptorPool CreateGUIDescriptorPool() const;

        /// Dispatch all render passes
        void RecordCommandBuffers();

    public:
        void Init();
        void Shutdown();

        /// Acquire new image from swapchain, rebuild it if necessary
        void BeginFrame();

        /// Wait for rendering to finish and present result to the screen
        void EndFrame();

        /// Render ImGui
        void RenderGUI();

        /// Register new render pass
        void AddRenderPass(const std::string& name, const RenderPassCreateInfo& initInfo);

        /// Add render pass to queue
        void EnqueueRenderPass(const std::string& name,
                               VkRect2D area,
                               const std::vector<std::string>& dependencies = {},
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

        VkCommandPool GraphicsCommandPool;
        std::vector<VkCommandBuffer> GraphicsCommandBuffers;

        std::queue<std::string> ClientRenderQueue;
        std::map<std::string, RenderPassContainer> ClientRenderPassesMap;

        // GUI data
        struct
        {
            Ref<Texture2D> ViewportDepthBuffer;
            std::vector<Ref<Texture2D>> ViewportRenderTargets;
            std::vector<VkDescriptorSet> ViewportRenderTargetDescriptors;

            VkCommandPool CommandPool;
            VkDescriptorPool DescriptorPool;
            std::vector<VkCommandBuffer> CommandBuffers;
            std::vector<VkFramebuffer> Framebuffers;

            VkRenderPass RenderPass;
            VkClearValue ClearValues = {
                .color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}
            };

            void BeginDockingSpace()
            {
                static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

                // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
                // because it would be confusing to have two docking targets within each others.
                ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
                //if (m_MenubarCallback)
                //   window_flags |= ImGuiWindowFlags_MenuBar;

                const ImGuiViewport* viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos(viewport->WorkPos);
                ImGui::SetNextWindowSize(viewport->WorkSize);
                ImGui::SetNextWindowViewport(viewport->ID);
                //ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
                window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
                window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

                ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
                ImGui::Begin("Dockspace", nullptr, window_flags);
                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor();

                // Submit the DockSpace
                ImGuiIO& io = ImGui::GetIO();
                if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
                {
                    ImGuiID dockspaceId = ImGui::GetID("EtnaAppDockspace");
                    ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspace_flags);
                }
            }
            void EndDockingSpace() { ImGui::End(); }

            void RenderViewport(uint32_t frameIndex)
            {
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                ImGui::Begin("Viewport");

                ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
                ImGui::Image((uint64_t)ViewportRenderTargetDescriptors[frameIndex], ImVec2{viewportPanelSize.x, viewportPanelSize.y});

                ImGui::End();
                ImGui::PopStyleVar();
            }
        } GUI;
    };
}


#endif //VULKANRENDERER_H
