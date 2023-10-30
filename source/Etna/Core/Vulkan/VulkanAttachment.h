#ifndef VULKANATTACHMENT_H
#define VULKANATTACHMENT_H

#include "VulkanHeader.h"
#include <vector>

namespace vkc
{
    // TODO: add builder for attachments
    class Attachment
    {
    public:
        Attachment() = default;

        std::vector<VkAttachmentDescription> GetDescriptions();
        std::vector<VkAttachmentReference> GetReferences();
    private:
        // TODO: implement this class
    };
}

#endif //VULKANATTACHMENT_H
