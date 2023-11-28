#include "VulkanRenderPass.h"

#include "Etna/Core/Utils.h"
#include "VulkanContext.h"

#include <vector>

namespace vkc
{
    RenderPass::RenderPass(const RenderPassCreateInfo& initInfo)
    {
        if (initInfo.Type != RenderPassType::Graphic)
        {
            Error("Only graphic passes supported.");
        }

        ClearValues = {
            {.color = {{0.0f, 0.0f, 0.0f, 1.0f}}}
        };

        std::vector<VkAttachmentDescription> attachments(1);
        attachments[0].format = initInfo.TargetFormat;
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // Unused, when initInfo.DepthEnabled == false
        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        if (initInfo.DepthEnabled)
        {
            ClearValues.push_back(
                {.depthStencil = {1.0f, 0}}
            );
            attachments.push_back({});
            attachments[1].format = FindDepthFormat();
            attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            subpass.pDepthStencilAttachment = &depthAttachmentRef;
        }

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(Context::GetDevice(), &renderPassInfo, Context::GetAllocator(), &Handle) != VK_SUCCESS)
        {
            Error("Failed to create render pass.");
        }

        auto pipelineBuilder = PipelineBuilder{}
            .SetRenderPass(Handle)
            .SetVertexLayout(initInfo.VertexLayoutInfo)
            .SetVertexShader(initInfo.VertexShaderPath)
            .SetFragmentShader(initInfo.FragmentShaderPath)
            .EnableDepthTesting(initInfo.DepthEnabled);
        for (auto& layout : initInfo.DescriptorSetLayouts)
            pipelineBuilder.AddDescriptorSetLayout(layout);

        RenderPipeline = pipelineBuilder.Build();
    }

    void RenderPass::Begin(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer, VkRect2D renderArea)
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = Handle;
        renderPassInfo.framebuffer = framebuffer;
        renderPassInfo.renderArea = renderArea;
        renderPassInfo.clearValueCount = static_cast<uint32_t>(ClearValues.size());
        renderPassInfo.pClearValues = ClearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        RenderPipeline.Bind(commandBuffer);
    }

    void RenderPass::End(VkCommandBuffer commandBuffer)
    {
        vkCmdEndRenderPass(commandBuffer);
    }

    VkPipelineLayout RenderPass::GetLayout() const
    {
        return RenderPipeline.Layout;
    }
}
