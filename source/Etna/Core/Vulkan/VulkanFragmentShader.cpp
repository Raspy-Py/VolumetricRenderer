#include "VulkanFragmentShader.h"

namespace vkc
{
    FragmentShader::FragmentShader(const std::string &shaderFileName)
        : Shader(ShaderStage::Fragment, shaderFileName)
    {}
}
