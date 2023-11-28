#ifndef VULKANTEXTURE_H
#define VULKANTEXTURE_H

#include "VulkanCore.h"
#include "VulkanMemory.h"

#include <stb_image.h>
#include <string>
#include <memory>

namespace vkc
{
    class Texture
    {
    public:
        Texture() = default;
        ~Texture();

        [[nodiscard]] VkImageView GetView() const { return ImageView; }
        [[nodiscard]] VkImage GetImage() const { return Image; }
        [[nodiscard]] VkSampler GetSampler() const { return Sampler; }
        [[nodiscard]] VkFormat GetFormat() const { return Format; }
        [[nodiscard]] uint32_t GetWidth() const { return Width; }
        [[nodiscard]] uint32_t GetHeight() const { return Height; }
        [[nodiscard]] VkExtent2D GetExtent() const { return {static_cast<uint32_t>(Width), static_cast<uint32_t>(Height)}; }

    protected:
        int Width;
        int Height;
        int Depth;
        int Channels;

        VkImage Image;
        VkFormat Format;
        VkSampler Sampler;
        VkImageView ImageView;
        VkDeviceMemory Memory;
    };


    class Texture2D : public Texture
    {
    public:

    public:
        explicit Texture2D(const std::string& imagePath);
        Texture2D() = default;
        ~Texture2D() = default;

    public:
        static Ref<Texture2D> CreateDepthBuffer(uint32_t width, uint32_t height);
        static Ref<Texture2D> CreateRenderTarget(uint32_t width, uint32_t height, VkFormat format);
    };

    class Texture3D : public Texture
    {
    public:
        Texture3D(unsigned char* data, VkExtent3D extent);
        ~Texture3D() = default;
    };

}

#endif //VULKANTEXTURE_H
