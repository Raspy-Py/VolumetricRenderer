#ifndef VULKANVERTEXLAYOUT_H
#define VULKANVERTEXLAYOUT_H

#include "Etna/Core/Utils.h"
#include "VulkanHeader.h"
#include <vector>

#define DECLARE_VERTEX_ATTRIBUTE_TYPE(data_type, vk_format)                                              \
    uint32_t AddAttribute(VertexLayout& layout, Tag<data_type> tag, uint32_t binding, uint32_t offset)
#define DEFINE_VERTEX_ATTRIBUTE_TYPE(data_type, vk_format)                                              \
    uint32_t AddAttribute(VertexLayout& layout, Tag<data_type> tag, uint32_t binding, uint32_t offset)  \
    {                                                                                                   \
        UNUSED(tag);                                                                                    \
        layout.AttributeDescriptions.push_back({                                                        \
            .location = static_cast<uint32_t>(layout.AttributeDescriptions.size()),                     \
            .binding = binding,                                                                         \
            .format = vk_format,                                                                        \
            .offset = offset                                                                            \
        });                                                                                             \
                                                                                                        \
        return offset + sizeof(data_type);                                                              \
    }


namespace vkc
{
    template <typename Attribute>
    struct Tag {};

    struct VertexLayout
    {
        VkVertexInputBindingDescription BindingDescription;
        std::vector<VkVertexInputAttributeDescription> AttributeDescriptions;
    };

    DECLARE_VERTEX_ATTRIBUTE_TYPE(glm::ivec4, VK_FORMAT_R32G32B32A32_SINT);
    DECLARE_VERTEX_ATTRIBUTE_TYPE(glm::ivec3, VK_FORMAT_R32G32B32_SINT);
    DECLARE_VERTEX_ATTRIBUTE_TYPE(glm::ivec2, VK_FORMAT_R32G32_SINT);
    DECLARE_VERTEX_ATTRIBUTE_TYPE(int32_t, VK_FORMAT_R32_SINT);

    DECLARE_VERTEX_ATTRIBUTE_TYPE(glm::uvec4, VK_FORMAT_R32G32B32A32_UINT);
    DECLARE_VERTEX_ATTRIBUTE_TYPE(glm::uvec3, VK_FORMAT_R32G32B32_UINT);
    DECLARE_VERTEX_ATTRIBUTE_TYPE(glm::uvec2, VK_FORMAT_R32G32_UINT);
    DECLARE_VERTEX_ATTRIBUTE_TYPE(uint32_t, VK_FORMAT_R32_UINT);

    DECLARE_VERTEX_ATTRIBUTE_TYPE(glm::vec4, VK_FORMAT_R32G32B32A32_SFLOAT);
    DECLARE_VERTEX_ATTRIBUTE_TYPE(glm::vec3, VK_FORMAT_R32G32B32_SFLOAT);
    DECLARE_VERTEX_ATTRIBUTE_TYPE(glm::vec2, VK_FORMAT_R32G32_SFLOAT);
    DECLARE_VERTEX_ATTRIBUTE_TYPE(float, VK_FORMAT_R32_SFLOAT);

    DECLARE_VERTEX_ATTRIBUTE_TYPE(glm::dvec4, VK_FORMAT_R64G64B64A64_SFLOAT);
    DECLARE_VERTEX_ATTRIBUTE_TYPE(glm::dvec3, VK_FORMAT_R64G64B64_SFLOAT);
    DECLARE_VERTEX_ATTRIBUTE_TYPE(glm::dvec2, VK_FORMAT_R64G64_SFLOAT);
    DECLARE_VERTEX_ATTRIBUTE_TYPE(double, VK_FORMAT_R64_SFLOAT);

    /*
     * Usage:
     *      auto layout = CreateVertexLayout<VertexAttributeType1,
     *                                      VertexAttributeType2,
     *                                      ...
     *                                      VertexAttributeTypeN>();
     */
    template<typename ...Attributes>
    VertexLayout CreateVertexLayout(uint32_t binding = 0)
    {
        VertexLayout vertexLayout;
        uint32_t offset = 0;
        ((offset = AddAttribute(vertexLayout, Tag<Attributes>(), binding, offset)), ...);

        vertexLayout.BindingDescription.binding = binding;
        vertexLayout.BindingDescription.stride = offset;
        vertexLayout.BindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return vertexLayout;
    }
}

#endif //VULKANVERTEXLAYOUT_H
