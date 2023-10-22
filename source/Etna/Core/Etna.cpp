#include "Etna.h"

#include "ImGuiTheme.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"

#include "Vulkan/VulkanCore.h"
#include "Vulkan/VulkanContext.h"
#include "Vulkan/VulkanSwapchain.h"
#include "Vulkan/VulkanRenderPass.h"

#include <vector>

// Our state // TODO: don't forget to remove
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

static void UIRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data)
{
    VkResult err;

    ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
    {
        err = vkWaitForFences(g_Device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking

        err = vkResetFences(g_Device, 1, &fd->Fence);
    }
    {
        err = vkResetCommandPool(g_Device, fd->CommandPool, 0);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
    }
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = wd->RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = wd->Width;
        info.renderArea.extent.height = wd->Height;
        info.clearValueCount = 1;
        info.pClearValues = &wd->ClearValue;
        vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

    // Submit command buffer
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_acquired_semaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->CommandBuffer;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &render_complete_semaphore;

        err = vkEndCommandBuffer(fd->CommandBuffer);
        check_vk_result(err);
        err = vkQueueSubmit(g_Queue, 1, &info, fd->Fence);
        check_vk_result(err);
    }
}

static void PostUIRender(ImGui_ImplVulkanH_Window* wd)
{

}

static void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data)
{
    UIRender(wd, draw_data);
}

static void FramePresent(ImGui_ImplVulkanH_Window* wd)
{
    if (g_SwapChainRebuild)
        return;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;
    VkResult err = vkQueuePresentKHR(g_Queue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        g_SwapChainRebuild = true;
        return;
    }
    wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->ImageCount; // Now we can use the next set of semaphores
}

namespace Etna
{
    struct ImGuiBackendData
    {
        vkc::RenderPass RenderPass;
        std::vector<VkFramebuffer> Framebuffers;
        std::unique_ptr<vkc::DescriptorPool> DescriptorPool;

        std::vector<VkCommandPool> CommandPools;
        std::vector<VkCommandBuffer> CommandBuffers;

        //std::vector<VkSemaphore>
    };

    ImGuiBackendData ImGuiData;

    static vkc::Swapchain           g_Swapchain;
    static ImGui_ImplVulkanH_Window g_MainWindowData;
    static uint32_t                 g_MinImageCount = 2;
    static bool                     g_SwapChainRebuild = false;
    static GLFWwindow*              g_Window = nullptr;
    static Application*             g_Application = nullptr;

    void Init(int argc, char **argv)
    {
        // Create window with Vulkan context
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        g_Window = glfwCreateWindow(1280, 720, "Etna App", nullptr, nullptr);

        // Init Vulkan
        vkc::Context::Create(g_Window);
        g_Swapchain = vkc::SwapchainBuilder().Build();

        // Init ImGui backend
        {
            ImGuiData.RenderPass = RenderPass()

            // Setup descriptor pool for ImGui font texture
            ImGuiData.DescriptorPool = std::make_unique<vkc::DescriptorPool>(vkc::DescriptorType::CombinedImageSampler, 1, true);

            // Create Framebuffers
            vkc::CreateFramebuffers(
                ImGuiData.Framebuffers.data(),
                ImGuiData.RenderPass.Handle,
                g_Swapchain.GetExtent(),
                g_Swapchain.GetImageViews().data(),
                nullptr, 1);

            // Create Command Buffers
            auto indices = vkc::GetQueueFamilies(vkc::Context::GetPhysicalDevice(), vkc::Context::GetSurface());
            for (uint32_t i = 0; i < g_MinImageCount; i++)
            {
                auto pool = vkc::CreateCommandPool(indices.GraphicsFamily.value());
                ImGuiData.CommandPools.push_back(pool);
                ImGuiData.CommandBuffers.push_back(vkc::CreateCommandBuffer(pool));

                {
                    VkFenceCreateInfo info = {};
                    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                    info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
                    err = vkCreateFence(device, &info, allocator, &fd->Fence);
                    check_vk_result(err);
                }
                {
                    VkSemaphoreCreateInfo info = {};
                    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                    err = vkCreateSemaphore(device, &info, allocator, &fsd->ImageAcquiredSemaphore);
                    check_vk_result(err);
                    err = vkCreateSemaphore(device, &info, allocator, &fsd->RenderCompleteSemaphore);
                    check_vk_result(err);
                }
            }
        }

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void) io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

        // Setup Dear ImGui style
        EmbraceTheDarkness();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForVulkan(g_Window, true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = g_Instance;
        init_info.PhysicalDevice = g_PhysicalDevice;
        init_info.Device = g_Device;
        init_info.QueueFamily = g_QueueFamily;
        init_info.Queue = g_Queue;
        init_info.PipelineCache = g_PipelineCache;
        init_info.DescriptorPool = g_DescriptorPool;
        init_info.Subpass = 0;
        init_info.MinImageCount = g_MinImageCount;
        init_info.ImageCount = wd->ImageCount;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = g_Allocator;
        init_info.CheckVkResultFn = check_vk_result;
        ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);

        // Upload Fonts
        {
            // Use any command queue
            VkCommandPool command_pool = wd->Frames[wd->FrameIndex].CommandPool;
            VkCommandBuffer command_buffer = wd->Frames[wd->FrameIndex].CommandBuffer;

            err = vkResetCommandPool(g_Device, command_pool, 0);
            check_vk_result(err);
            VkCommandBufferBeginInfo begin_info = {};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            err = vkBeginCommandBuffer(command_buffer, &begin_info);
            check_vk_result(err);

            ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

            VkSubmitInfo end_info = {};
            end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            end_info.commandBufferCount = 1;
            end_info.pCommandBuffers = &command_buffer;
            err = vkEndCommandBuffer(command_buffer);
            check_vk_result(err);
            err = vkQueueSubmit(g_Queue, 1, &end_info, VK_NULL_HANDLE);
            check_vk_result(err);

            err = vkDeviceWaitIdle(g_Device);
            check_vk_result(err);
            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }

        g_Application = CreateApplication();
    }

    void Run()
    {
        ImGuiIO &io = ImGui::GetIO();
        (void) io;

        while (!glfwWindowShouldClose(g_Window)) {
            // Poll and handle events (inputs, window resize, etc.)
            // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
            // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
            // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
            // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
            glfwPollEvents();

            // Resize swap chain?
            if (g_SwapChainRebuild) {
                int width, height;
                glfwGetFramebufferSize(g_Window, &width, &height);
                if (width > 0 && height > 0) {
                    ImGui_ImplVulkan_SetMinImageCount(g_MinImageCount);
                    ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, &g_MainWindowData,
                                                           g_QueueFamily, g_Allocator, width, height, g_MinImageCount);
                    g_MainWindowData.FrameIndex = 0;
                    g_SwapChainRebuild = false;
                }
            }

            // Start the Dear ImGui frame
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Create a dockspaces
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

                /*if (m_MenubarCallback)
                {
                    if (ImGui::BeginMenuBar())
                    {
                        m_MenubarCallback();
                        ImGui::EndMenuBar();
                    }
                }*/

                s_Application->RenderFrame();

                ImGui::End();
            }

            // Rendering
            ImGui::Render();
            ImDrawData *draw_data = ImGui::GetDrawData();
            const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
            if (!is_minimized) {
                g_MainWindowData.ClearValue.color.float32[0] = clear_color.x * clear_color.w;
                g_MainWindowData.ClearValue.color.float32[1] = clear_color.y * clear_color.w;
                g_MainWindowData.ClearValue.color.float32[2] = clear_color.z * clear_color.w;
                g_MainWindowData.ClearValue.color.float32[3] = clear_color.w;
                FrameRender(&g_MainWindowData, draw_data);
                FramePresent(&g_MainWindowData);
            }
        }
    }

    void Shutdown()
    {
        vkDeviceWaitIdle(vkc::Context::GetDevice());
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        CleanupVulkanWindow();
        CleanupVulkan();

        glfwDestroyWindow(g_Window);
        glfwTerminate();
    }

    Application::Application(Etna::ApplicationInfo appInfo)
        : AppInfo(appInfo)
    {
        DeltaClock.Restart();
    }

    Application::~Application() {}

    void Application::RenderFrame()
    {
        float deltaTime = DeltaClock.Stamp();

        // Loop backward to be able to erase layers
        for (int i = (int)Layers.size() - 1; i >= 0; --i)
        {
            // Check on update if layer has requested detachment
            if (Layers[i]->Update(deltaTime))
            {
                Layers[i]->Shutdown();
                Layers.erase(Layers.begin()+i);
            }
        }

        while (!PushQueue.empty())
        {
            auto layer = PushQueue.front();
            PushQueue.pop();

            layer->Init();
            Layers.emplace_back(layer);
        }

        for (auto& layer : Layers)
        {
            layer->Render();
        }
    }
}
