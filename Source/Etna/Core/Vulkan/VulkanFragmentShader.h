#ifndef VULKANFRAGMENTSHADER_H
#define VULKANFRAGMENTSHADER_H

#include "VulkanShader.h"

namespace vkc
{
    class FragmentShader : public Shader
    {
    public:
        FragmentShader(const std::string& shaderFileName);
        FragmentShader(const FragmentShader&) = delete;
        FragmentShader& operator=(const FragmentShader&) = delete;
        ~FragmentShader() = default;
    };
}

#endif //VULKANFRAGMENTSHADER_H
