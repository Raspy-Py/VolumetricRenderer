#include "Core/Vulkan/VulkanContext.h"
#include "Core/Vulkan/VulkanCore.h"
#include "Core/Vulkan/VulkanRenderer.h"
#include <memory>

int main(int argc, char** argv)
{
    // Create window with Vulkan context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto window = glfwCreateWindow(1280, 720, "Etna App", nullptr, nullptr);
    vkc::Renderer renderer;

    vkc::Context::Create(window);
    renderer.Init(window);
    {
        vkc::RenderPassCreateInfo createInfo = {
            .DepthEnabled = false,
            .Type = vkc::RenderPassType::Graphic,
            .TargetFormat = renderer.GetSwapchainImageFormat(),
            .VertexShaderPath = "bin/shaders/Vert.spv",
            .FragmentShaderPath = "bin/shaders/Frag.spv",
            .VertexLayoutInfo = vkc::CreateVertexLayout<glm::vec2, glm::vec3>(),
            .DescriptorSetLayout = VK_NULL_HANDLE
        };

        std::vector<VkFramebuffer> framebuffers;
        renderer.AddRenderPass("triangle_pass", createInfo);
        renderer.CreateSwapchainFramebuffers(framebuffers, "triangle_pass");

        while (!glfwWindowShouldClose(window))
        {
            uint32_t currentImage = renderer.GetSwapchainCurrentImage();
            VkRect2D rect = {{0,0}, renderer.GetSwapchainExtent()};
            renderer.EnqueueRenderPass(
                "triangle_pass",
                framebuffers[currentImage],
                rect,
                [rect](VkCommandBuffer cmdBuffer)
                {
                    VkViewport viewport = {
                        0.0f, 0.0f,
                        static_cast<float>(rect.extent.width), static_cast<float>(rect.extent.height),
                        0.0f, 1.0f,
                    };
                    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

                    VkRect2D scissor = rect;
                    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
                });

            renderer.BeginFrame();
            renderer.RenderFrame();
            renderer.EndFrame();
        }

    }
    renderer.Shutdown();
    vkc::Context::Destroy();

    return 0;
}
