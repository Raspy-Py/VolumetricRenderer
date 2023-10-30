#include "VulkanTexture.h"

#include "Etna/Core/Utils.h"
#include <cstring>

namespace vkc
{
    Texture::~Texture()
    {
        vkDestroyImageView(Context::GetDevice(), ImageView, Context::GetAllocator());
        vkDestroyImage(Context::GetDevice(), Image, Context::GetAllocator());
        vkFreeMemory(Context::GetDevice(), Memory, Context::GetAllocator());
    }

    Texture2D::Texture2D(const std::string &imagePath)
    {
        stbi_uc* image = stbi_load(imagePath.c_str(), &Width, &Height, &Channels, STBI_rgb_alpha);
        VkDeviceSize imageSize = Width * Height * 4;
        Format = VK_FORMAT_R8G8B8A8_SRGB;

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

        FillBuffer(stagingMemory, image, imageSize);

        CreateImage2D(
            Width, Height,
            Format,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            Image, Memory
        );

        TransitionImageLayout(Image, Format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyBufferImage(stagingBuffer, Image, Width, Height);
        TransitionImageLayout(Image, Format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(Context::GetDevice(), stagingBuffer, Context::GetAllocator());
        vkFreeMemory(Context::GetDevice(), stagingMemory, Context::GetAllocator());

        ImageView = CreateImageView(Image, Format);
        Sampler = CreateSampler();
    }

    void Texture2D::CreateDepthBuffer(Texture2D &texture, int width, int height)
    {
        texture.Width = width;
        texture.Height = height;
        texture.Depth = 1;
        texture.Channels = 1;
        texture.Format = FindDepthFormat();

        CreateImage2D(
            width, height,
            texture.Format,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            texture.Image,
            texture.Memory
        );

        texture.ImageView = CreateImageView(
            texture.Image,
            texture.Format,
            VK_IMAGE_VIEW_TYPE_2D,
            VK_IMAGE_ASPECT_DEPTH_BIT
        );
    }

    Texture3D::Texture3D(std::byte *data, VkExtent3D extent)
    {
        Width = static_cast<int>(extent.width);
        Height = static_cast<int>(extent.height);
        Depth = static_cast<int>(extent.depth);
        Format = VK_FORMAT_R8G8B8A8_UNORM;
        Channels = 4;
        uint32_t imageSize = Width * Height * Depth * Channels;

        if (!data)
        {
            Error("Image data is not valid.");
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

        FillBuffer(stagingMemory, data, imageSize);

        CreateImage(
            Width, Height, Depth,
            VK_IMAGE_TYPE_3D,
            Format,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            Image, Memory
        );

        TransitionImageLayout(Image, Format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyBufferImage(stagingBuffer, Image, Width, Height);
        TransitionImageLayout(Image, Format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(Context::GetDevice(), stagingBuffer, Context::GetAllocator());
        vkFreeMemory(Context::GetDevice(), stagingMemory, Context::GetAllocator());

        ImageView = CreateImageView(Image, Format, VK_IMAGE_VIEW_TYPE_3D);
        Sampler = CreateSampler();
    }
}
