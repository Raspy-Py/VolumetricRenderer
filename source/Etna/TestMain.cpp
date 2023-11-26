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

#include <FastNoise/FastNoise.h>

#include "Etna/Core/Clock.h"

struct Vertex
{
    glm::vec3 Position;
    glm::vec2 TexCoord;
};

struct ObjectShaderData
{
    glm::mat4 Model;
    glm::mat4 View;
    glm::mat4 Projection;
};

struct GlobalShaderData
{
    glm::mat4 WorldToLocal;
    glm::vec3 CameraPosition;
    glm::mat4 MediaScroll;
};

int main(int argc, char** argv)
{
    auto fnCellular = FastNoise::New<FastNoise::CellularDistance>();
    auto fnPerlin = FastNoise::New<FastNoise::Perlin>();
    auto fnSimplex = FastNoise::New<FastNoise::Simplex>();

    //fnPerlin->SetSource( fnSimplex );
    //fnPerlin->SetOctaveCount( 10 );


    int size = 128;
    int sizeCube = size * size * size;
    std::vector<float> noiseOutput1(sizeCube);
    std::vector<float> noiseOutput2(sizeCube);
    std::vector<float> noiseOutput3(sizeCube);
    std::vector<float> noiseOutput4(sizeCube);
    std::vector<unsigned char> pixelData(sizeCube*4);

    auto output1 = fnCellular->GenUniformGrid3D(noiseOutput1.data(), 0, 0, 0, size, size, size, 0.01f, 1);
    auto output2 = fnCellular->GenUniformGrid3D(noiseOutput1.data(), 0, 0, 0, size, size, size, 0.03f, 2);
    auto output3 = fnPerlin->GenUniformGrid3D(noiseOutput3.data(), 0, 0, 0, size, size, size, 0.19f, 3);
    auto output4 = fnSimplex->GenUniformGrid3D(noiseOutput4.data(), 0, 0, 0, size, size, size, 0.15f, 4);

    float inverseRange1 = 1 / (output1.max - output1.min);
    float inverseRange2 = 1 / (output2.max - output2.min);
    float inverseRange3 = 1 / (output3.max - output3.min);
    float inverseRange4 = 1 / (output4.max - output4.min);
    int index = 0;
    for (int z = 0; z < size; z++)
    {
        for (int y = 0; y < size; y++)
        {
            for (int x = 0; x < size; x++)
            {
                float sample1 = 1 - (noiseOutput1[index] - output1.min) * inverseRange1;
                float sample2 = 1 - (noiseOutput2[index] - output2.min) * inverseRange2;
                float sample3 = 1 - (noiseOutput3[index] - output3.min) * inverseRange3;
                float sample4 = 1 - (noiseOutput4[index] - output4.min) * inverseRange4;

                sample1 *= sample1 * sample1 * sample1;
                //sample2 *= sample2 * sample2;
                //sample3 *= sample3 * sample3;

                pixelData[index*4   ] = static_cast<unsigned char>((sample1) * 255.0f);
                pixelData[index*4+1 ] = static_cast<unsigned char>((sample2) * 255.0f);
                pixelData[index*4+2 ] = static_cast<unsigned char>((sample3) * 255.0f);
                pixelData[index*4+3 ] = static_cast<unsigned char>((sample4) * 255.0f);

                index++;
            }
        }
    }

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


    Clock clock;
    vkc::Renderer renderer;
    renderer.Init();
    // All vulkanish code should go inside the following scope
    {
        vkc::IndexBuffer indexBuffer(vkc::Context::GetTransferCommandPool(), indices.data(), indices.size());
        vkc::VertexBuffer<Vertex> vertexBuffer(vkc::Context::GetTransferCommandPool(), vertices.data(), vertices.size());

        vkc::Texture3D texture(pixelData.data(), VkExtent3D(size, size, size));
        vkc::UniformBuffer<ObjectShaderData> objectUniformBuffer(renderer.GetFramesCount());
        vkc::UniformBuffer<GlobalShaderData> globalUniformBuffer(renderer.GetFramesCount());

        auto perFrameLayout = vkc::DescriptorSetLayout::Builder{}
            .AddBinding(0, vkc::DescriptorType::UniformBuffer, vkc::ShaderStage::Vertex)
            .AddBinding(1, vkc::DescriptorType::UniformBuffer, vkc::ShaderStage::Fragment)
            .AddBinding(2, vkc::DescriptorType::CombinedImageSampler, vkc::ShaderStage::Fragment)
            .Build();
        auto perFramePool = std::make_unique<vkc::DescriptorSetPool>(*perFrameLayout, renderer.GetFramesCount());
        auto layouts = {
            perFrameLayout->Handle,
        };
        std::vector<VkDescriptorSet> perFrameSets;
        for (uint32_t i = 0; i < renderer.GetFramesCount(); i++)
        {
            auto objectBuffer = objectUniformBuffer.Buffers[i];
            auto globalBuffer = globalUniformBuffer.Buffers[i];

            perFrameSets.push_back(
                vkc::DescriptorSetWriter{*perFrameLayout, *perFramePool}
                    .WriteBuffer(0, objectBuffer, 0, sizeof(ObjectShaderData))
                    .WriteBuffer(1, globalBuffer, 0, sizeof(GlobalShaderData))
                    .WriteImage(2, texture.GetView(), texture.GetSampler())
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

        renderer.AddPresentPass("BasePass", createInfo);

        auto startTime = std::chrono::high_resolution_clock::now();
        float cubePhi = 0;
        float cubeTheta = 0;
        float rotationSpeed = 100.f;
        float controlledFrameTime = 0;
        while (!glfwWindowShouldClose(vkc::Context::GetWindow()))
        {
            glfwPollEvents();
            float deltaTime = 0.016;
            if (glfwGetKey(vkc::Context::GetWindow(), GLFW_KEY_A) == GLFW_PRESS)
                cubePhi -= rotationSpeed * deltaTime;
            if (glfwGetKey(vkc::Context::GetWindow(), GLFW_KEY_D) == GLFW_PRESS)
                cubePhi += rotationSpeed * deltaTime;
            if (glfwGetKey(vkc::Context::GetWindow(), GLFW_KEY_W) == GLFW_PRESS)
                cubeTheta -= rotationSpeed * deltaTime;
            if (glfwGetKey(vkc::Context::GetWindow(), GLFW_KEY_S) == GLFW_PRESS)
                cubeTheta += rotationSpeed * deltaTime;
            if (glfwGetKey(vkc::Context::GetWindow(), GLFW_KEY_Q) == GLFW_PRESS)
                controlledFrameTime += deltaTime * 0.2f;
            if (glfwGetKey(vkc::Context::GetWindow(), GLFW_KEY_E) == GLFW_PRESS)
                controlledFrameTime -= deltaTime * 0.2f;
            if (glfwGetKey(vkc::Context::GetWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
                glfwSetWindowShouldClose(vkc::Context::GetWindow(), true);

            VkRect2D rect = {{0,0}, renderer.GetSwapchainExtent()};

            renderer.EnqueuePresentPass("BasePass", rect,
                [rect, &perFrameSets, &indexBuffer, &vertexBuffer, indicesCount]
                (vkc::RenderPassContext&& rpc)
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
            auto rot = glm::rotate(glm::mat4(1.0f), glm::radians(cubePhi), glm::vec3(0.0f, 0.0f, 1.0f));
            ObjectShaderData osd = {
                .Model = glm::rotate(rot, glm::radians(cubeTheta), glm::vec3(0.0f, 1.0f, 0.0f)),
                .View = glm::lookAt(glm::vec3(3.0f, 3.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
                .Projection = glm::perspective(glm::radians(45.0f), (float)1280 / (float)720, 0.1f, 10.0f)
            };
            osd.Projection[1][1] *= -1;

            glm::mat4 worldToLocal = glm::inverse(osd.Model);

            float frameTime = clock.Elapsed();
            glm::mat4 mediaScroll = {
                {-frameTime, frameTime, -frameTime,0},
                {frameTime, -frameTime, frameTime, 0},
                {frameTime, frameTime, -frameTime, 0},
                {frameTime, frameTime, frameTime,0}
            };

            GlobalShaderData gsd = {
                .WorldToLocal = worldToLocal,
                .CameraPosition = glm::vec3(3.0f, 3.0f, 3.0f),
                //.FrameTime = (float)cos(clock.Elapsed() * 0.5f) * 0.49f + 0.5f,
                .MediaScroll = mediaScroll
            };
            //InfoLog("FrameTime: %f", gsd.FrameTime);

            objectUniformBuffer.Update(&osd, renderer.GetCurrentFrame());
            globalUniformBuffer.Update(&gsd, renderer.GetCurrentFrame());

            renderer.RenderFrame();
            renderer.EndFrame();
        }

        vkDeviceWaitIdle(vkc::Context::GetDevice());
    }
    renderer.Shutdown();

    return 0;
}
