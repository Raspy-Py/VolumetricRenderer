#include "Core/Vulkan/VulkanCore.h"
#include "Core/Vulkan/VulkanContext.h"
#include "Core/Vulkan/VulkanRenderer.h"
#include "Core/Vulkan/VulkanVertexBuffer.h"
#include "Core/Vulkan/VulkanIndexBuffer.h"
#include "Core/Vulkan/VulkanUniformBuffer.h"

#include "Core/Utils.h"

#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;
};

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
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
    float iters = 0;

    vkc::Renderer renderer;
    renderer.Init();
    {
        vkc::IndexBuffer indexBuffer(vkc::Context::GetTransferCommandPool(), indices.data(), indices.size());
        vkc::VertexBuffer<Vertex> vertexBuffer(vkc::Context::GetTransferCommandPool(), vertices.data(), vertices.size());
        vkc::UniformBuffer<UniformBufferObject> uniformBuffer(renderer.GetFramesCount());

        auto descriptorPool = vkc::DescriptorPool(vkc::DescriptorType::UniformBuffer, renderer.GetFramesCount());
        auto descriptorSetLayouts = uniformBuffer.CreateDescriptorSetLayout(0, vkc::ShaderStage::Vertex);
        auto descriptorSets = uniformBuffer.CreateDescriptorSets(descriptorSetLayouts, descriptorPool);

        vkc::RenderPassCreateInfo createInfo = {
            .DepthEnabled = false,
            .Type = vkc::RenderPassType::Graphic,
            .TargetFormat = renderer.GetSwapchainImageFormat(),
            .VertexShaderPath = "shaders/vert.spv",
            .FragmentShaderPath = "shaders/frag.spv",
            .VertexLayoutInfo = vkc::CreateVertexLayout<glm::vec2, glm::vec3>(),
            .DescriptorSetLayout = descriptorSetLayouts
        };

        renderer.AddPresentPass("base_pass", createInfo);

        auto startTime = std::chrono::high_resolution_clock::now();

        while (!glfwWindowShouldClose(vkc::Context::GetWindow()))
        {
            glfwPollEvents();
            VkRect2D rect = {{0,0}, renderer.GetSwapchainExtent()};

            renderer.EnqueuePresentPass("base_pass", rect,
                [rect, &descriptorSets, &indexBuffer, &vertexBuffer, indicesCount]
                (vkc::RenderPassContext rpc)
                {
                    VkViewport viewport = {
                        0.0f, 0.0f,
                        static_cast<float>(rect.extent.width), static_cast<float>(rect.extent.height),
                        0.0f, 1.0f,
                    };
                    vkCmdSetViewport(rpc.CommandBuffer, 0, 1, &viewport);
                    VkRect2D scissor = rect;
                    vkCmdSetScissor(rpc.CommandBuffer, 0, 1, &scissor);
                    indexBuffer.Bind(rpc.CommandBuffer, 0);
                    vertexBuffer.Bind(rpc.CommandBuffer, 0);
                    vkCmdBindDescriptorSets(rpc.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, rpc.PipelineLayout,
                                            0, 1, &descriptorSets[rpc.FrameIndex], 0, nullptr);
                    vkCmdDrawIndexed(rpc.CommandBuffer, indicesCount, 1, 0, 0, 0);
                });

            renderer.BeginFrame();
            renderer.RenderFrame();
            renderer.EndFrame();

            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
            iters += 0.001;
            UniformBufferObject ubo = {
                .model = glm::rotate(glm::mat4(1.0f), iters * glm::radians(10.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
                .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
                .proj = glm::perspective(glm::radians(45.0f), (float)1280 / (float)720, 0.1f, 10.0f)
            };
            ubo.proj[1][1] *= -1;
            uniformBuffer.Update(&ubo, renderer.GetCurrentFrame());

        }
    }
    renderer.Shutdown();

    return 0;
}
