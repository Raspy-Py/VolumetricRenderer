#include "RenderUtils.h"

#include <queue>

// TODO: render queue is currently static and not really a queue.
//  Consider making it dynamic (e.g. rebuild it every frame).
static std::vector<std::function<void()>> g_RenderQueue;

namespace Etna
{
    void AddRenderPass(std::function<void()>&& drawFunc)
    {
        g_RenderQueue.emplace_back(std::move(drawFunc));
    }

    void DispatchRenderQueue()
    {
        for (auto& drawCall : g_RenderQueue)
        {
            drawCall();
        }
    }
}