#ifndef VULKANTEXTURE_H
#define VULKANTEXTURE_H

#include "VulkanCore.h"

#include <stb_image.h>
#include <string>

namespace vkc
{
    class Texture
    {
    public:
        Texture() = default;
        ~Texture();

        [[nodiscard]] const VkImageView& GetView() const { return ImageView; }
        [[nodiscard]] const VkImage & GetImage() const { return Image; }
        [[nodiscard]] const VkSampler& GetSampler() const { return Sampler; }
        [[nodiscard]] const VkFormat& GetFormat() const { return Format; }

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
        explicit Texture2D(const std::string& imagePath);
        Texture2D() = default;
        ~Texture2D() = default;

    public:
        static void CreateDepthBuffer(Texture2D& texture, int width, int height);
    };

    class Texture3D : public Texture
    {
    public:
        Texture3D(std::byte* data, VkExtent3D extent);
        ~Texture3D() = default;
    };
}

#endif //VULKANTEXTURE_H
