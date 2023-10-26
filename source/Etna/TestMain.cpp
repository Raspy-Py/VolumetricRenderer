#include "Core/Vulkan/VulkanContext.h"
#include "Core/Vulkan/VulkanCore.h"
#include "Core/Vulkan/VulkanRenderer.h"
#include "Core/Vulkan/VulkanVertexBuffer.h"
#include "Core/Vulkan/VulkanIndexBuffer.h"

#include "Core/Utils.h"
#include <memory>


struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;
};


int main(int argc, char** argv)
{
    std::vector<Vertex> vertices = {
        {{-1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
        {{1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
        {{1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        {{-1.0f, 1.0f}, {1.0f, 0.5f, 1.0f}}
    };

    std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    uint32_t indicesCount = indices.size();

    LogsInit();
    glfwInit();

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
            .VertexShaderPath = "shaders/vert.spv",
            .FragmentShaderPath = "shaders/frag.spv",
            .VertexLayoutInfo = vkc::CreateVertexLayout<glm::vec2, glm::vec3>(),
            .DescriptorSetLayout = VK_NULL_HANDLE
        };

        vkc::IndexBuffer indexBuffer(vkc::Context::GetTransferCommandPool(), indices.data(), indices.size());
        vkc::VertexBuffer<Vertex> vertexBuffer(vkc::Context::GetTransferCommandPool(), vertices.data(), vertices.size());

        std::vector<VkFramebuffer> framebuffers;
        renderer.AddRenderPass("base_pass", createInfo);
        renderer.CreateSwapchainFramebuffers(framebuffers, "base_pass");

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            uint32_t currentImage = renderer.GetSwapchainCurrentImage();
            VkRect2D rect = {{0,0}, renderer.GetSwapchainExtent()};

            renderer.EnqueueRenderPass(
                "base_pass",
                framebuffers[currentImage],
                rect,
                [rect, &indexBuffer, &vertexBuffer, indicesCount](VkCommandBuffer cmdBuffer)
                {
                    VkViewport viewport = {
                        0.0f, 0.0f,
                        static_cast<float>(rect.extent.width), static_cast<float>(rect.extent.height),
                        0.0f, 1.0f,
                    };
                    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
                    VkRect2D scissor = rect;
                    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
                    indexBuffer.Bind(cmdBuffer, 0);
                    vertexBuffer.Bind(cmdBuffer, 0);
                    vkCmdDrawIndexed(cmdBuffer, indicesCount, 1, 0, 0, 0);
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
