#include "VulkanVertexShader.h"

namespace vkc
{
    VertexShader::VertexShader(const std::string &shaderFileName)
        : Shader(ShaderStage::Vertex, shaderFileName)
    {}
}
