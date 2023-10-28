#include "VulkanTexture.h"

#include "Etna/Core/Utils.h"
#include <cstring>

namespace vkc
{

    Texture::Texture(const std::string &imagePath)
    {
        stbi_uc* image = stbi_load(imagePath.c_str(), &Width, &Height, &Channels, STBI_rgb_alpha);
        VkDeviceSize imageSize = Width * Height * 4;

        if (!image)
        {
            Error("Failed to load texture image from file: %s.", imagePath.c_str());
        }

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingMemory;
        CreateBuffer(
            imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingMemory
        );

        void* data;
        vkMapMemory(Context::GetDevice(), stagingMemory, 0, imageSize, 0, &data);
            memcpy(data, image,imageSize);
        vkUnmapMemory(Context::GetDevice(), stagingMemory);

        stbi_image_free(image);

        VkImageCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.imageType = VK_IMAGE_TYPE_2D;
        createInfo.extent.width = static_cast<uint32_t>(Width);
        createInfo.extent.height = static_cast<uint32_t>(Height);
        createInfo.extent.depth = 1;
        createInfo.mipLevels = 1;
        createInfo.arrayLayers = 1;
        createInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        createInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        if (vkCreateImage(Context::GetDevice(), &createInfo, Context::GetAllocator(), &Image) != VK_SUCCESS)
        {
            Error("Failed to create image [%s].", imagePath.c_str());
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(Context::GetDevice(), Image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = ChooseMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(Context::GetDevice(), &allocInfo, Context::GetAllocator(), &Memory) != VK_SUCCESS)
        {
            Error("Failed to allocate image memory!");
        }

        vkBindImageMemory(Context::GetDevice(), Image, Memory, 0);
    }

    Texture::~Texture()
    {

    }
}
