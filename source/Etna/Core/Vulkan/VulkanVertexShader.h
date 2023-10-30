#ifndef VULKANVERTEXSHADER_H
#define VULKANVERTEXSHADER_H

#include "VulkanShader.h"

namespace vkc
{
    class VertexShader : public Shader
    {
    public:
        VertexShader(const std::string& shaderFileName);
        VertexShader(const VertexShader&) = delete;
        VertexShader& operator=(const VertexShader&) = delete;
        ~VertexShader() = default;
    };
}

#endif //VULKANVERTEXSHADER_H
