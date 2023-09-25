#ifndef VULKANRENDERPASS_H
#define VULKANRENDERPASS_H

#include "VulkanHeader.h"
#include "VulkanAttachment.h"

#include <vector>

namespace vkc
{
    enum class RenderPassType
    {
        Graphic,
        Compute,
        None,
        Count
    };

    class RenderPass
    {
    public:
        VkRenderPass Handle;

    };

    class RenderPassBuilder
    {
    public:
        RenderPassBuilder() = default;
        ~RenderPassBuilder() = default;

        /*
         * Compulsory parameters
         */

        RenderPassBuilder& SetRTFormat(VkFormat format) { RenderTargetFormat = format; }

        /*
         * Optional parameters
         */

        RenderPassBuilder& SetType(RenderPassType type) { Type = type; }
        RenderPassBuilder& AddDepthBuffer()             { DepthEnabled = true; }

        RenderPass Build(VkDevice device);

    private:
        bool DepthEnabled = false;
        VkFormat RenderTargetFormat = VK_FORMAT_MAX_ENUM;
        RenderPassType Type = RenderPassType::None;
    };


}

#endif //VULKANRENDERPASS_H
