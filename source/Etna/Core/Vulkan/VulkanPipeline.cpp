#include "VulkanPipeline.h"

#include "Etna/Core/Utils.h"
#include "Etna/Core/Exception.h"

#include "VulkanCore.h"
#include "VulkanContext.h"
#include "VulkanFragmentShader.h"
#include "VulkanVertexShader.h"
#include "VulkanVertexLayout.h"
#include "VulkanVertexBuffer.h"
#include "VulkanIndexBuffer.h"
#include "VulkanRenderPass.h"
#include "VulkanAttachment.h"

#include <vector>

namespace vkc
{
    using ::std::vector;

    static const vector<VkDynamicState> DynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

    PipelineBuilder& PipelineBuilder::SetVertexLayout(const vkc::VertexLayout& vertexLayout)
    {
        this->VertexLayoutInfo = vertexLayout;

        return *this;
    }

    PipelineBuilder& PipelineBuilder::SetVertexShader(std::string& path)
    {
        VertexShaderPath = path;
        
        return *this;
    }

    PipelineBuilder& PipelineBuilder::SetFragmentShader(std::string& path)
    {
        FragmentShaderPath = path;
        
        return *this;
    }

    PipelineBuilder& PipelineBuilder::SetRenderPass(VkRenderPass renderPass)
    {
        RenderPass = renderPass;
        
        return *this;
    }

    PipelineBuilder& PipelineBuilder::AddDescriptorSetLayout(VkDescriptorSetLayout layout)
    {
        DescriptorSetLayouts.push_back(layout);
        
        return *this;
    }

    PipelineBuilder& PipelineBuilder::AddPushConstantRange(VkPushConstantRange range)
    {
        PushConstantRanges.push_back(range);
        
        return *this;
    }

    Pipeline PipelineBuilder::Build()
    {
        auto vertexShaderModule = VertexShader(VertexShaderPath);
        auto fragmentShaderModule = FragmentShader(FragmentShaderPath);

        VkPipelineShaderStageCreateInfo shaderStages[] = {
            vertexShaderModule.GetShaderStageCreateInfo(),
            fragmentShaderModule.GetShaderStageCreateInfo()
        };

        
        VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
        dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(DynamicStates.size());
        dynamicStateInfo.pDynamicStates = DynamicStates.data();

        auto vertexLayout = this->VertexLayoutInfo;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &vertexLayout.BindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexLayout.AttributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = vertexLayout.AttributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
        inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportInfo{};
        viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportInfo.viewportCount = 1;
        viewportInfo.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
        rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizerInfo.depthClampEnable = VK_FALSE;
        rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizerInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizerInfo.lineWidth = 1.0f;
        rasterizerInfo.depthBiasEnable = VK_FALSE;
        rasterizerInfo.depthBiasConstantFactor = 0.0f;
        rasterizerInfo.depthBiasClamp = 0.0f;
        rasterizerInfo.depthBiasSlopeFactor = 0.0f;
        rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampleInfo{};
        multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleInfo.sampleShadingEnable = VK_FALSE;
        multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleInfo.minSampleShading = 1.0f;
        multisampleInfo.pSampleMask = nullptr;
        multisampleInfo.alphaToCoverageEnable = VK_FALSE;
        multisampleInfo.alphaToOneEnable = VK_FALSE;

        // TODO: configure depth stencil testing
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};

        // Blending is disabled
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
        colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendInfo.logicOpEnable = VK_FALSE;
        colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
        colorBlendInfo.attachmentCount = 1;
        colorBlendInfo.pAttachments = &colorBlendAttachment;
        colorBlendInfo.blendConstants[0] = 0.0f;
        colorBlendInfo.blendConstants[1] = 0.0f;
        colorBlendInfo.blendConstants[2] = 0.0f;
        colorBlendInfo.blendConstants[3] = 0.0f;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(DescriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = DescriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(PushConstantRanges.size());;
        pipelineLayoutInfo.pPushConstantRanges = PushConstantRanges.data();
        
        Pipeline pipeline;
        if (vkCreatePipelineLayout(Context::GetDevice(), &pipelineLayoutInfo, Context::GetAllocator(), &pipeline.Layout) != VK_SUCCESS)
        {
            Error("Failed to create pipeline layout.");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
        pipelineInfo.pViewportState = &viewportInfo;
        pipelineInfo.pRasterizationState = &rasterizerInfo;
        pipelineInfo.pMultisampleState = &multisampleInfo;
        pipelineInfo.pDepthStencilState = nullptr;
        pipelineInfo.pDynamicState = &dynamicStateInfo;
        pipelineInfo.pColorBlendState = &colorBlendInfo;
        pipelineInfo.layout = pipeline.Layout;
        pipelineInfo.renderPass = RenderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        if (vkCreateGraphicsPipelines(Context::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, Context::GetAllocator(), &pipeline.Handle) != VK_SUCCESS)
        {
            Error("Failed to create graphics pipeline.");
        }

        return pipeline;
    }
}
