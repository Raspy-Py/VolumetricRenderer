#ifndef VULKANVERTEXLAYOUT_H
#define VULKANVERTEXLAYOUT_H

#include "VulkanHeader.h"
#include <vector>


#define DEFINE_VERTEX_ATTRIBUTE_TYPE(data_type, vk_format)                                          \
    uint32_t AddAttribute(VertexLayout& layout, Tag<data_type>, uint32_t binding, uint32_t offset)  \
    {                                                                                               \
        layout.AttributeDescriptions.push_back({                                                    \
            .location = static_cast<uint32_t>(layout.AttributeDescriptions.size()),                 \
            .binding = binding,                                                                     \
            .format = vk_format,                                                                    \
            .offset = offset                                                                        \
        });                                                                                         \
                                                                                                    \
        return offset + sizeof(data_type);                                                          \
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

    DEFINE_VERTEX_ATTRIBUTE_TYPE(glm::ivec4, VK_FORMAT_R32G32B32A32_SINT)
    DEFINE_VERTEX_ATTRIBUTE_TYPE(glm::ivec3, VK_FORMAT_R32G32B32_SINT)
    DEFINE_VERTEX_ATTRIBUTE_TYPE(glm::ivec2, VK_FORMAT_R32G32_SINT)
    DEFINE_VERTEX_ATTRIBUTE_TYPE(int32_t, VK_FORMAT_R32_SINT)

    DEFINE_VERTEX_ATTRIBUTE_TYPE(glm::uvec4, VK_FORMAT_R32G32B32A32_UINT)
    DEFINE_VERTEX_ATTRIBUTE_TYPE(glm::uvec3, VK_FORMAT_R32G32B32_UINT)
    DEFINE_VERTEX_ATTRIBUTE_TYPE(glm::uvec2, VK_FORMAT_R32G32_UINT)
    DEFINE_VERTEX_ATTRIBUTE_TYPE(uint32_t, VK_FORMAT_R32_UINT)

    DEFINE_VERTEX_ATTRIBUTE_TYPE(glm::vec4, VK_FORMAT_R32G32B32A32_SFLOAT)
    DEFINE_VERTEX_ATTRIBUTE_TYPE(glm::vec3, VK_FORMAT_R32G32B32_SFLOAT)
    DEFINE_VERTEX_ATTRIBUTE_TYPE(glm::vec2, VK_FORMAT_R32G32_SFLOAT)
    DEFINE_VERTEX_ATTRIBUTE_TYPE(float, VK_FORMAT_R32_SFLOAT)

    DEFINE_VERTEX_ATTRIBUTE_TYPE(glm::dvec4, VK_FORMAT_R64G64B64A64_SFLOAT)
    DEFINE_VERTEX_ATTRIBUTE_TYPE(glm::dvec3, VK_FORMAT_R64G64B64_SFLOAT)
    DEFINE_VERTEX_ATTRIBUTE_TYPE(glm::dvec2, VK_FORMAT_R64G64_SFLOAT)
    DEFINE_VERTEX_ATTRIBUTE_TYPE(double, VK_FORMAT_R64_SFLOAT)

    // I bet there is a way to make it runtime dynamic...
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
