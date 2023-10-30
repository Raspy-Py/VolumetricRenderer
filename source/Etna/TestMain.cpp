#include "Core/Vulkan/VulkanCore.h"
#include "Core/Vulkan/VulkanContext.h"
#include "Core/Vulkan/VulkanRenderer.h"
#include "Core/Vulkan/VulkanVertexBuffer.h"
#include "Core/Vulkan/VulkanIndexBuffer.h"
#include "Core/Vulkan/VulkanUniformBuffer.h"
#include "Core/Vulkan/VulkanTexture.h"
#include "Core/Vulkan/VulkanDescriptors.h"

#include "Core/Utils.h"

#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

struct Vertex
{
    glm::vec3 pos;
    glm::vec2 texCoord;
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
        {{1.0, -1.0, -1.0}, {1.0, 0.0}},
        {{1.0, -1.0, 1.0}, {1.0, 1.0}},
        {{-1.0, -1.0, 1.0}, {0.0, 1.0}},
        {{-1.0, -1.0, -1.0}, {0.0, 0.0}},
        {{1.0, 1.0, -1.0}, {1.0, 0.0}},
        {{1.0, 1.0, 1.0}, {1.0, 1.0}},
        {{-1.0, 1.0, 1.0}, {0.0, 1.0}},
        {{-1.0, 1.0, -1.0}, {0.0, 0.0}}
    };

    std::vector<uint16_t> indices = {
        0, 1, 2, 0, 2, 3,
        3, 2, 6, 3, 6, 7,
        0, 3, 7, 0, 7, 4,
        4, 7, 6, 4, 6, 5,
        1, 5, 6, 1, 6, 2,
        4, 5, 1, 4, 1, 0
    };

    uint32_t indicesCount = indices.size();

    LogsInit();
    glfwInit();

    vkc::Renderer renderer;
    renderer.Init();
    // All vulkanish code should go inside the following scope
    {
        vkc::Texture2D texture("../assets/images/Johny.jpg");

        vkc::IndexBuffer indexBuffer(vkc::Context::GetTransferCommandPool(), indices.data(), indices.size());
        vkc::VertexBuffer<Vertex> vertexBuffer(vkc::Context::GetTransferCommandPool(), vertices.data(), vertices.size());
        vkc::UniformBuffer<UniformBufferObject> uniformBuffer(renderer.GetFramesCount());

        auto perFrameLayout = vkc::DescriptorSetLayout::Builder{}
            .AddBinding(0, vkc::DescriptorType::UniformBuffer, vkc::ShaderStage::Vertex)
            .AddBinding(1, vkc::DescriptorType::CombinedImageSampler, vkc::ShaderStage::Fragment)
            .Build();
        auto perFramePool = std::make_unique<vkc::DescriptorSetPool>(*perFrameLayout, renderer.GetFramesCount());
        auto layouts = {
            perFrameLayout->Handle,
        };
        std::vector<VkDescriptorSet> perFrameSets;
        for (auto buffer : uniformBuffer.Buffers)
        {
            perFrameSets.push_back(
                vkc::DescriptorSetWriter{*perFrameLayout, *perFramePool}
                    .WriteBuffer(0, buffer, 0, sizeof(UniformBufferObject))
                    .WriteImage(1, texture.GetView(), texture.GetSampler())
                    .Write()
            );
        }

        vkc::RenderPassCreateInfo createInfo = {
            .DepthEnabled = true,
            .Type = vkc::RenderPassType::Graphic,
            .TargetFormat = renderer.GetSwapchainImageFormat(),
            .VertexShaderPath = "shaders/vert.spv",
            .FragmentShaderPath = "shaders/frag.spv",
            .VertexLayoutInfo = vkc::CreateVertexLayout<glm::vec3, glm::vec2>(),
            .DescriptorSetLayouts = layouts
        };

        renderer.AddPresentPass("base_pass", createInfo);

        auto startTime = std::chrono::high_resolution_clock::now();

        while (!glfwWindowShouldClose(vkc::Context::GetWindow()))
        {
            glfwPollEvents();
            VkRect2D rect = {{0,0}, renderer.GetSwapchainExtent()};

            renderer.EnqueuePresentPass("base_pass", rect,
                [rect, &perFrameSets, &indexBuffer, &vertexBuffer, indicesCount]
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
                    // Bind per frame sets
                    vkCmdBindDescriptorSets(
                        rpc.CommandBuffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        rpc.PipelineLayout, 0, 1,
                        &perFrameSets[rpc.FrameIndex],
                        0, nullptr
                    );
                    vkCmdDrawIndexed(rpc.CommandBuffer, indicesCount, 1, 0, 0, 0);
                });

            renderer.BeginFrame();

            // Update MVP matrix
            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
            UniformBufferObject ubo = {
                .model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
                .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
                .proj = glm::perspective(glm::radians(45.0f), (float)1280 / (float)720, 0.1f, 10.0f)
            };
            ubo.proj[1][1] *= -1;
            uniformBuffer.Update(&ubo, renderer.GetCurrentFrame());



            renderer.RenderFrame();
            renderer.EndFrame();
        }

        vkDeviceWaitIdle(vkc::Context::GetDevice());
    }
    renderer.Shutdown();

    return 0;
}
