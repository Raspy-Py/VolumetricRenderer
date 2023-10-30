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

        auto device = Context::GetDevice();
        void* data;
        vkMapMemory(device, stagingMemory, 0, imageSize, 0, &data);
            memcpy(data, image,imageSize);
        vkUnmapMemory(device, stagingMemory);
        stbi_image_free(image);

        CreateImage(
            Width, Height,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            Image, Memory
        );

        TransitionImageLayout(Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyBufferImage(stagingBuffer, Image, Width, Height);
        TransitionImageLayout(Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(device, stagingBuffer, Context::GetAllocator());
        vkFreeMemory(device, stagingMemory, Context::GetAllocator());

        ImageView = CreateImageView(Image, VK_FORMAT_R8G8B8A8_SRGB);
        Sampler = CreateSampler();
    }

    Texture::~Texture()
    {
        vkDestroyImageView(Context::GetDevice(), ImageView, Context::GetAllocator());
        vkDestroyImage(Context::GetDevice(), Image, Context::GetAllocator());
        vkFreeMemory(Context::GetDevice(), Memory, Context::GetAllocator());
    }
}
