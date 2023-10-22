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

        // Color attachment
        VkAttachmentReference colorReference = {};
        colorReference.attachment = 0;
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.pColorAttachments = &colorReference;

        std::vector<VkAttachmentDescription> attachments = {{
            .flags = 0,
            .format = initInfo.TargetFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        }};

        // Depth attachment
        if (initInfo.DepthEnabled)
        {
            attachments.push_back({
                .flags = 0,
                .format = FindDepthFormat(),
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            });

            VkAttachmentReference depthReference = {
                .attachment = 1,
                .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            };

            subpassDescription.pDepthStencilAttachment = &depthReference;
        }

        // Subpass dependencies for layout transitions
        std::vector<VkSubpassDependency> dependencies = {{
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
        }};

        // Depth dependency
        if (initInfo.DepthEnabled)
        {
            dependencies.push_back({
                .srcSubpass = 0,
                .dstSubpass = VK_SUBPASS_EXTERNAL,
                .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
                .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            });
        }

        VkRenderPassCreateInfo renderPassInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = 1,
            .pSubpasses = &subpassDescription,
            .dependencyCount = static_cast<uint32_t>(dependencies.size()),
            .pDependencies = dependencies.data(),
        };

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
