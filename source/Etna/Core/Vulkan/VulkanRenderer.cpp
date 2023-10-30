#include "VulkanRenderer.h"

#include "Etna/Core/Utils.h"
#include "VulkanContext.h"
#include "VulkanSwapchain.h"
#include "VulkanCore.h"

static const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

namespace vkc
{
    void Renderer::Init()
    {
        Context::Create();
        CurrentFrame = 0;
        MaxFramesInFlight = MAX_FRAMES_IN_FLIGHT;
        GSwapchain = SwapchainBuilder{}.Build();

        FrameFences.resize(MaxFramesInFlight);
        ImageAvailableSemaphores.resize(MaxFramesInFlight);
        RenderFinishedSemaphores.resize(MaxFramesInFlight);

        CreateFences(FrameFences.data(), MaxFramesInFlight);
        CreateSemaphores(ImageAvailableSemaphores.data(), MaxFramesInFlight);
        CreateSemaphores(RenderFinishedSemaphores.data(), MaxFramesInFlight);

        auto indices = GetQueueFamilies(Context::GetPhysicalDevice(), Context::GetSurface());
        for (uint32_t i = 0; i < MaxFramesInFlight; ++i)
        {
            GraphicsCommandPools.push_back(CreateCommandPool(indices.GraphicsFamily.value()));
        }

        for (auto pool : GraphicsCommandPools)
        {
            GraphicsCommandBuffers.push_back(CreateCommandBuffer(pool)); 
        }
    }

    void Renderer::Shutdown()
    {
        GSwapchain.LogStatistics();
        vkDeviceWaitIdle(Context::GetDevice());

        for (size_t i = 0; i < MaxFramesInFlight; i++)
        {
            vkDestroySemaphore(Context::GetDevice(), ImageAvailableSemaphores[i], Context::GetAllocator());
            vkDestroySemaphore(Context::GetDevice(), RenderFinishedSemaphores[i], Context::GetAllocator());
            vkDestroyFence(Context::GetDevice(), FrameFences[i], Context::GetAllocator());
        }

        Context::Destroy();
    }

    void Renderer::BeginFrame()
    {
        Context::StartFrame();
        vkWaitForFences(Context::GetDevice(), 1, &FrameFences[CurrentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(Context::GetDevice(), 1, &FrameFences[CurrentFrame]);

        if(GSwapchain.AcquireNextImage(ImageAvailableSemaphores[CurrentFrame]))
        {
            // Recreate swapchain and stuff
            return;
        }
    }

    void Renderer::EndFrame()
    {
        if (GSwapchain.PresentImage(RenderFinishedSemaphores[CurrentFrame]))
        {
            // Recreate swapchain and stuff
            return;
        }

        CurrentFrame = (CurrentFrame + 1) % MaxFramesInFlight;
        Context::EndFrame();
    }

    void Renderer::RenderFrame()
    {
        auto cmdBuffer = GraphicsCommandBuffers[CurrentFrame];

        // Record drawing commands
        vkResetCommandBuffer(cmdBuffer, 0);
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;
        if (vkBeginCommandBuffer(cmdBuffer, &beginInfo) != VK_SUCCESS)
        {
            Error("Failed to begin recording command buffer.");
        }

        while (!ClientRenderQueue.empty())
        {
            auto& passName = ClientRenderQueue.front();
            auto ptr = ClientRenderPassesMap.find(passName);
            if (ptr != ClientRenderPassesMap.end())
            {
                auto& pass = ptr->second;
                ClientRenderQueue.pop();

                pass.Pass->Begin(cmdBuffer, pass.Framebuffers[pass.CurrentFramebufferIndex], pass.Area);
                pass.Delegate({cmdBuffer, pass.Pass->GetLayout(), GSwapchain.GetCurrentImage(), CurrentFrame});
                pass.Pass->End(cmdBuffer);
            }
        }
        if (vkEndCommandBuffer(cmdBuffer) != VK_SUCCESS)
        {
            Error("Failed to record command buffer.");
        }

        VkSemaphore waitSemaphores[] = { ImageAvailableSemaphores[CurrentFrame] };
        VkSemaphore signalSemaphores[] = { RenderFinishedSemaphores[CurrentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(Context::GetGraphicsQueue(), 1, &submitInfo, FrameFences[CurrentFrame]) != VK_SUCCESS)
        {
            Error("Failed to submit command.");
        }
    }

    void Renderer::EnqueueRenderPass(const std::string &name,
                                     uint32_t frameBufferIndex,
                                     VkRect2D area,
                                     RenderPassDelegate&& delegate)
    {
        auto ptr = ClientRenderPassesMap.find(name);
        if (ptr != ClientRenderPassesMap.end())
        {
            auto& pass = ptr->second;

            if (pass.Framebuffers.size() < (frameBufferIndex + 1))
            {
                Error("Not able to bind framebuffer[%i] to render pass [%s]. "
                      "Only %i framebuffers exist for this render pass.",
                      frameBufferIndex, name.c_str(), (int)pass.Framebuffers.size());
            }
            pass.CurrentFramebufferIndex = frameBufferIndex;
            pass.Area = area;
            pass.Delegate = std::move(delegate);

            ClientRenderQueue.push(name);
        }
    }

    void Renderer::AddRenderPass(const std::string& name, const RenderPassCreateInfo& initInfo)
    {
        ClientRenderPassesMap.emplace(name, std::move(RenderPassContainer(initInfo)));
    }

    void Renderer::CreateSwapchainFramebuffers(std::vector<VkFramebuffer> &framebuffers, const std::string &renderPass)
    {
        auto ptr = ClientRenderPassesMap.find(renderPass);
        if (ptr == ClientRenderPassesMap.end())
        {
            Error("Render pass '%s' does not exist. ", renderPass.c_str());
        }
        auto& pass = ptr->second;

        framebuffers.resize(GSwapchain.GetImageCount());
        vkc::CreateFramebuffers(
            framebuffers.data(),
            pass.Pass->Handle,
            GSwapchain.GetExtent(),
            GSwapchain.GetImageViews().data(),
            nullptr, framebuffers.size()
        );
    }

    VkFormat Renderer::GetSwapchainImageFormat() const
    {
        return GSwapchain.GetFormat();
    }

    VkExtent2D Renderer::GetSwapchainExtent() const
    {
        return GSwapchain.GetExtent();
    }

    uint32_t Renderer::GetSwapchainCurrentImage() const
    {
        return GSwapchain.GetCurrentImage();
    }

    void Renderer::EnqueuePresentPass(const std::string &name, VkRect2D area, RenderPassDelegate&& delegate)
    {
        // For a framebuffer, which will be presented, we should specify
        // an index provided by swapchain
        EnqueueRenderPass(name, GSwapchain.GetCurrentImage(), area, std::move(delegate));
    }

    void Renderer::EnqueueCommonPass(const std::string &name, VkRect2D area, RenderPassDelegate&& delegate)
    {
        // Just use current "inflight frame" index
        EnqueueRenderPass(name, CurrentFrame, area, std::move(delegate));
    }

    void Renderer::AddPresentPass(const std::string &name, const RenderPassCreateInfo &initInfo)
    {
        AddRenderPass(name, initInfo);

        // Automatically create framebuffers for the present pass
        // using swapchain images
        auto ptr = ClientRenderPassesMap.find(name);
        if (ptr == ClientRenderPassesMap.end())
        {
            Error("Render pass '%s' does not exist. ", name.c_str());
        }

        auto &pass = ptr->second;
        const VkImageView* depthView = nullptr;

        if (initInfo.DepthEnabled)
        {
            pass.DepthBufferTexture = std::make_unique<Texture2D>();

            Texture2D::CreateDepthBuffer(
                *pass.DepthBufferTexture,
                static_cast<int>(GSwapchain.GetExtent().width),
                static_cast<int>(GSwapchain.GetExtent().height)
            );

            depthView = &pass.DepthBufferTexture->GetView();

            TransitionImageLayout(
                pass.DepthBufferTexture->GetImage(),
                pass.DepthBufferTexture->GetFormat(),
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            );
        }

        pass.Framebuffers.resize(GSwapchain.GetImageCount());
        vkc::CreateFramebuffers(
            pass.Framebuffers.data(),
            pass.Pass->Handle,
            GSwapchain.GetExtent(),
            GSwapchain.GetImageViews().data(),
            depthView,
            pass.Framebuffers.size());
    }

    uint32_t Renderer::GetFramesCount() const
    {
        return MaxFramesInFlight;
    }

    uint32_t Renderer::GetCurrentFrame() const
    {
        return CurrentFrame;
    }

    uint32_t Renderer::GetSwapchainImageCount() const
    {
        return GSwapchain.GetImageCount();
    }
}
