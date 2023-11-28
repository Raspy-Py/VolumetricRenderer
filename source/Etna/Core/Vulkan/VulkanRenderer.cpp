#include "VulkanRenderer.h"

#include "Etna/Core/Utils.h"
#include "Etna/Core/ImGuiTheme.h"

#include "VulkanContext.h"
#include "VulkanSwapchain.h"
#include "VulkanCore.h"
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"

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

        GraphicsCommandPool = CreateCommandPool(indices.GraphicsFamily.value());
        GraphicsCommandBuffers.resize(GetFramesCount());
        CreateCommandBuffers(GraphicsCommandPool, GraphicsCommandBuffers.data(), GetFramesCount());

        // Initialize ImGui
        {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();

            ImGuiIO &io = ImGui::GetIO();
            (void) io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        }

        // GUI
        {
            // Create Descriptor Pool
            GUI.DescriptorPool = CreateGUIDescriptorPool();

            // Create Render Pass
            GUI.RenderPass = CreateGUIRenderPass();

            // Create Command Pool
            GUI.CommandPool = CreateCommandPool(indices.GraphicsFamily.value());

            // Create command buffers
            GUI.CommandBuffers.resize(GetFramesCount());
            CreateCommandBuffers(GUI.CommandPool, GUI.CommandBuffers.data(), GetFramesCount());

            // Create Frame Buffers
            GUI.Framebuffers.resize(GSwapchain.GetImageCount());
            CreateFramebuffers(
                GUI.Framebuffers.data(),
                GUI.RenderPass,
                GSwapchain.GetExtent(),
                GSwapchain.GetImageViews().data(),
                VK_NULL_HANDLE, // Do not need depth testing here
                GUI.Framebuffers.size());

            // TODO: get viewport size from window's area
            const int vpWidth = 1280, vpHeight = 720;
            GUI.ViewportDepthBuffer = Texture2D::CreateDepthBuffer(vpWidth, vpHeight);
            for (uint32_t i = 0; i < GetFramesCount(); ++i)
            {
                GUI.ViewportRenderTargets.emplace_back(Texture2D::CreateRenderTarget(vpWidth, vpHeight, GetSwapchainImageFormat()));
            }

            // Init implementations
            ImGui_ImplGlfw_InitForVulkan(Context::GetWindow(), true);
            ImGui_ImplVulkan_InitInfo init_info = {};
            init_info.Instance = Context::GetInstance();
            init_info.PhysicalDevice = Context::GetPhysicalDevice();
            init_info.Device = Context::GetDevice();
            init_info.QueueFamily = indices.GraphicsFamily.value();
            init_info.Queue = Context::GetGraphicsQueue();
            init_info.PipelineCache = VK_NULL_HANDLE;
            init_info.DescriptorPool = GUI.DescriptorPool;
            init_info.Subpass = 0;
            init_info.MinImageCount = GetSwapchainImageCount();
            init_info.ImageCount = GetSwapchainImageCount();
            init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
            init_info.Allocator = Context::GetAllocator();
            init_info.CheckVkResultFn = nullptr; // TODO: add callback here
            ImGui_ImplVulkan_Init(&init_info, GUI.RenderPass);

            // Load font texture
            VkCommandBuffer commandBuffer = BeginSingleTimeCommands(GUI.CommandPool);
                ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
            EndSingleTimeCommands(commandBuffer, GUI.CommandPool);
            ImGui_ImplVulkan_DestroyFontUploadObjects();

            GUI.ViewportRenderTargetDescriptors.resize(GUI.ViewportRenderTargets.size());
            for (uint32_t i = 0; i < GUI.ViewportRenderTargets.size(); i++)
            {
                GUI.ViewportRenderTargetDescriptors[i] = ImGui_ImplVulkan_AddTexture(
                    GUI.ViewportRenderTargets[i]->GetSampler(),
                    GUI.ViewportRenderTargets[i]->GetView(),
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                );
            }

            EmbraceTheDarkness();
        }
    }

    void Renderer::Shutdown()
    {
        // Shutdown ImGui
        {
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }

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
        vkWaitForFences(Context::GetDevice(), 1, &FrameFences[CurrentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(Context::GetDevice(), 1, &FrameFences[CurrentFrame]);

        if(GSwapchain.AcquireNextImage(ImageAvailableSemaphores[CurrentFrame]))
        {
            // Recreate swapchain and stuff
            return;
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        GUI.BeginDockingSpace();
        GUI.RenderViewport(GetCurrentFrame());
    }

    void Renderer::EndFrame()
    {
        GUI.EndDockingSpace();
        RecordCommandBuffers();
        if (GSwapchain.PresentImage(RenderFinishedSemaphores[CurrentFrame]))
        {
            // Recreate swapchain and stuff
            return;
        }

        CurrentFrame = (CurrentFrame + 1) % MaxFramesInFlight;
    }

    void Renderer::RecordCommandBuffers()
    {
        auto commandBuffer = GraphicsCommandBuffers[CurrentFrame];

        // Record drawing commands
        vkResetCommandBuffer(commandBuffer, 0);
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
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

                pass.Pass->Begin(commandBuffer, pass.Framebuffers[pass.CurrentFramebufferIndex], pass.Area);
                pass.Delegate({commandBuffer, pass.Pass->GetLayout(), GSwapchain.GetCurrentImage(), CurrentFrame});
                pass.Pass->End(commandBuffer);
            }
        }

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            Error("Failed to record command buffer.");
        }

        // Record GUI render pass
        RenderGUI();

        VkSemaphore waitSemaphores[] = { ImageAvailableSemaphores[CurrentFrame] };
        VkSemaphore signalSemaphores[] = { RenderFinishedSemaphores[CurrentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        // Submit Viewport and GUI command buffers
        VkCommandBuffer commandBuffers[] = { GraphicsCommandBuffers[CurrentFrame], GUI.CommandBuffers[CurrentFrame]};

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 2;
        submitInfo.pCommandBuffers = commandBuffers;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(Context::GetGraphicsQueue(), 1, &submitInfo, FrameFences[CurrentFrame]) != VK_SUCCESS)
        {
            Error("Failed to submit a command.");
        }
    }

    void Renderer::AddRenderPass(const std::string& name, const RenderPassCreateInfo& initInfo)
    {
        ClientRenderPassesMap.emplace(name, std::move(RenderPassContainer(initInfo)));

        auto passPtr = ClientRenderPassesMap.find(name);
        if (passPtr == ClientRenderPassesMap.end())
        {
            Error("Failed to add a render pass.");
        }

        auto& passContainer = passPtr->second;
        VkExtent2D extent = GUI.ViewportDepthBuffer->GetExtent();
        for (uint32_t i = 0; i < GetFramesCount(); ++i)
        {
            VkFramebuffer framebuffer;
            auto colorView = GUI.ViewportRenderTargets[i]->GetView();
            auto depthView = GUI.ViewportDepthBuffer->GetView();
            CreateFramebuffers(
                &framebuffer,
                passContainer.Pass->Handle, extent,
                &colorView, &depthView
            );
            passContainer.Framebuffers.push_back(framebuffer);
        }
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

    void Renderer::EnqueueRenderPass(const std::string &name,
                                     VkRect2D area,
                                     const std::vector<std::string>& dependencies,
                                     RenderPassDelegate&& delegate)
    {
        auto ptr = ClientRenderPassesMap.find(name);
        if (ptr != ClientRenderPassesMap.end())
        {
            auto& pass = ptr->second;

            if (pass.Framebuffers.size() < (CurrentFrame + 1))
            {
                Error("Not able to bind framebuffer[%i] to render pass [%s]. "
                      "Only %i framebuffers exist for this render pass.",
                      CurrentFrame, name.c_str(), (int)pass.Framebuffers.size());
            }
            pass.CurrentFramebufferIndex = CurrentFrame;
            pass.Area = area;
            pass.Delegate = std::move(delegate);

            ClientRenderQueue.push(name);
        }
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

    VkRenderPass Renderer::CreateGUIRenderPass() const
    {
        VkAttachmentDescription attachment = {};
        attachment.format = GetSwapchainImageFormat();
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // may cause troubles
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        VkAttachmentReference color_attachment = {};
        color_attachment.attachment = 0;
        color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment;
        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        VkRenderPassCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        info.attachmentCount = 1;
        info.pAttachments = &attachment;
        info.subpassCount = 1;
        info.pSubpasses = &subpass;
        info.dependencyCount = 1;
        info.pDependencies = &dependency;
        VkRenderPass renderPass;
        if (vkCreateRenderPass(Context::GetDevice(), &info, Context::GetAllocator(), &renderPass) != VK_SUCCESS)
        {
            Error("Failed to create ImGui renderPass.");
        }

        return renderPass;
    }

    VkDescriptorPool Renderer::CreateGUIDescriptorPool() const
    {
        // o ma gad...
        uint32_t numDescriptors = 1000;
        std::vector<VkDescriptorPoolSize> poolSizes = {
            {VK_DESCRIPTOR_TYPE_SAMPLER, numDescriptors},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, numDescriptors},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, numDescriptors},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, numDescriptors},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, numDescriptors},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, numDescriptors},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, numDescriptors},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, numDescriptors},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, numDescriptors},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, numDescriptors},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, numDescriptors}
        };

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = numDescriptors * poolSizes.size();
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();

        VkDescriptorPool descriptorPool;
        if (vkCreateDescriptorPool(vkc::Context::GetDevice(), &poolInfo, vkc::Context::GetAllocator(),
                                   &descriptorPool) != VK_SUCCESS)
        {
            Error("Failed to create DescriptorPool for GUI!");
        }

        return descriptorPool;
    }

    void Renderer::RenderGUI()
    {
        ImGui::Render();

        auto commandBuffer = GUI.CommandBuffers[GetCurrentFrame()];
        vkResetCommandBuffer(commandBuffer, 0);
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            Error("Failed to begin recording command buffer.");
        }
        // Begin GUI render pass
        auto extent = GetSwapchainExtent();
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = GUI.RenderPass;
        info.framebuffer = GUI.Framebuffers[GetSwapchainCurrentImage()];
        info.renderArea.extent.width = extent.width;
        info.renderArea.extent.height = extent.height;
        info.clearValueCount = 1;
        info.pClearValues = &GUI.ClearValues;
        vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);

        // Record dear imgui primitives into command buffer
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

        vkCmdEndRenderPass(commandBuffer);
        // Submit command buffer
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            Error("Failed to record GUI command buffer.");
        }
    }
}
