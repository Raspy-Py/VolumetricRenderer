#ifndef PIPELINE_H
#define PIPELINE_H

#include <vulkan/vulkan.h>

#include <string>

namespace Etna
{

    struct PipelineInfo
    {
        std::string VertexShaderPath;
        std::string FragmentShaderPath;
    };

    class Pipeline
    {
    public:
        Pipeline(PipelineInfo* info);
        ~Pipeline() = default;

    private:
        VkPipeline PipelineHnd;
    };
}

#endif //PIPELINE_H
