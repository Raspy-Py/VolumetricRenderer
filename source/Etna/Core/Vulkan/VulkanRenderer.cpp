#include "VulkanRenderer.h"

#include "Etna/Core/Utils.h"
#include "VulkanContext.h"
#include "VulkanSwapchain.h"
#include "VulkanCore.h"

static const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

namespace vkc
{
    void Renderer::Init(GLFWwindow* window)
    {
        Context::Create(window);

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

                pass.Pass->Begin(cmdBuffer, pass.FrameBuffer, pass.Area);
                pass.Delegate(cmdBuffer);
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
                                     VkFramebuffer framebuffer,
                                     VkRect2D area,
                                     RenderPassDelegate delegate)
    {
        auto ptr = ClientRenderPassesMap.find(name);
        if (ptr != ClientRenderPassesMap.end())
        {
            auto& pass = ptr->second;

            pass.FrameBuffer = framebuffer;
            pass.Area = area;
            pass.Delegate = std::move(delegate);

            ClientRenderQueue.push(name);
        }
    }

    void Renderer::AddRenderPass(const std::string& name, const RenderPassCreateInfo& initInfo)
    {
        ClientRenderPassesMap.emplace(name, std::move(RenderPassDispatchInfo(initInfo)));
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
        vkc::CreateFramebuffers(framebuffers.data(),
                                pass.Pass->Handle,
                                GSwapchain.GetExtent(),
                                GSwapchain.GetImageViews().data(),
                                nullptr,
                                framebuffers.size());
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
}
