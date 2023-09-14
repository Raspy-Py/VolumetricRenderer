#ifndef RENDERUTILS_H
#define RENDERUTILS_H

#include <vulkan/vulkan.h>

#include <functional>

namespace Etna
{
    void AddRenderPass(std::function<void()>&& drawFunc);
    void DispatchRenderQueue();
}

#endif //RENDERUTILS_H
