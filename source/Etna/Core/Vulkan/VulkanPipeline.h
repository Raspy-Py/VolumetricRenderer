#ifndef PIPELINE_H
#define PIPELINE_H

#include "VulkanCore.h"
#include "VulkanVertexLayout.h"
#include "VulkanBindableInterface.h"

#include <string>
#include <vector>

// TODO: "Builder" pattern is absolutely redundant here.
//  Move it to Pipeline's constructor

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

    class Pipeline : public BindableInterface
    {
    public:
        Pipeline() = default;
        ~Pipeline() override = default;

        void Bind(VkCommandBuffer commandBuffer, uint32_t imageIndex = 0) override;

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
        PipelineBuilder& SetVertexShader(const std::string& path);
        PipelineBuilder& SetFragmentShader(const std::string& path);
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
