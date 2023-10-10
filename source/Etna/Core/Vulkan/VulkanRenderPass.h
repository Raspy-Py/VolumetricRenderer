#ifndef VULKANRENDERPASS_H
#define VULKANRENDERPASS_H

#include "VulkanCore.h"
#include "VulkanAttachment.h"

#include <vector>

namespace vkc
{
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

        RenderPassBuilder& SetRTFormat(VkFormat format) { RenderTargetFormat = format; return *this; }

        /*
         * Optional parameters
         */

        RenderPassBuilder& SetType(RenderPassType type) { Type = type; return *this; }
        RenderPassBuilder& AddDepthBuffer()             { DepthEnabled = true; return *this; }

        RenderPass Build(VkDevice device);

    private:
        bool DepthEnabled = false;
        VkFormat RenderTargetFormat = VK_FORMAT_MAX_ENUM;
        RenderPassType Type = RenderPassType::None;
    };


}

#endif //VULKANRENDERPASS_H
