#ifndef VULKANRENDERPASS_H
#define VULKANRENDERPASS_H

#include "VulkanCore.h"
#include "VulkanPipeline.h"
#include "VulkanAttachment.h"
#include "VulkanVertexBuffer.h"

#include <vector>
#include <memory>
#include <string>

namespace vkc
{
    struct RenderPassCreateInfo
    {
        bool DepthEnabled;
        RenderPassType Type;
        VkFormat TargetFormat;
        std::string VertexShaderPath;
        std::string FragmentShaderPath;
        VertexLayout VertexLayoutInfo;
        VkDescriptorSetLayout DescriptorSetLayout;
    };

    class RenderPass
    {
    public:
        RenderPass(const RenderPassCreateInfo& initInfo);

    public:
        void Begin(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer, VkRect2D renderArea);
        void End(VkCommandBuffer commandBuffer);

    public:
        [[nodiscard]] VkPipelineLayout GetLayout() const;

    public:
        VkRenderPass Handle;

    private:
        vkc::Pipeline RenderPipeline;
        std::vector<VkClearValue> ClearValues;
    };
}

#endif //VULKANRENDERPASS_H
