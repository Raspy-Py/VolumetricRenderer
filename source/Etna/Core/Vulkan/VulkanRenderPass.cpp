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
        if (initInfo.DepthEnabled)
        {
            ClearValues.push_back(
                {.depthStencil = {1.0f, 0}}
            );
        }

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = initInfo.TargetFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        if (vkCreateRenderPass(Context::GetDevice(), &renderPassInfo, Context::GetAllocator(), &Handle) != VK_SUCCESS)
        {
            Error("Failed to create render pass.");
        }

        RenderPipeline = PipelineBuilder{}
            .SetRenderPass(Handle)
            .SetVertexLayout(initInfo.VertexLayoutInfo)
            .SetVertexShader(initInfo.VertexShaderPath)
            .SetFragmentShader(initInfo.FragmentShaderPath)
            .AddDescriptorSetLayout(initInfo.DescriptorSetLayout)
            .Build();
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
}
