#ifndef PIPELINE_H
#define PIPELINE_H

#include "VulkanCore.h"
#include "VulkanVertexLayout.h"

#include <string>
#include <vector>

namespace vkc
{
    struct PipelineInfo
    {
        VkRenderPass RenderPass;
        VertexLayout VertexLayoutInfo;
        std::string VertexShaderPath;
        std::string FragmentShaderPath;
        ::std::vector<VkPushConstantRange> PushConstantRanges;
        ::std::vector<VkDescriptorSetLayout> DescriptorSetLayouts;
    };

    class Pipeline
    {
    public:
        Pipeline() = default;
        ~Pipeline() = default;

    public:
        VkPipeline Handle;
        VkPipelineLayout Layout;
    };

    class PipelineBuilder
    {
    public:
        PipelineBuilder() = default;
        ~PipelineBuilder() = default;

        PipelineBuilder& SetVertexLayout(const vkc::VertexLayout& vertexLayout);
        PipelineBuilder& SetVertexShader(std::string& path);
        PipelineBuilder& SetFragmentShader(std::string& path);
        PipelineBuilder& SetRenderPass(VkRenderPass renderPass);
        PipelineBuilder& AddDescriptorSetLayout(VkDescriptorSetLayout layout);
        PipelineBuilder& AddPushConstantRange(VkPushConstantRange range);
        
        Pipeline Build();

    private:
        VkRenderPass RenderPass;
        VertexLayout VertexLayoutInfo;
        std::string VertexShaderPath;
        std::string FragmentShaderPath;
        ::std::vector<VkPushConstantRange> PushConstantRanges;
        ::std::vector<VkDescriptorSetLayout> DescriptorSetLayouts;
    };
}

#endif //PIPELINE_H
