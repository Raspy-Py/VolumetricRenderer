#pragma once
#ifndef ETNA_H
#define ETNA_H

#include "Utils.h"
#include "Clock.h"

#include <queue>
#include <vector>
#include <memory>
#include <functional>

namespace Etna
{
    void Init(int argc, char** argv);
    void Run();
    void Shutdown();

    class Layer
    {
    public:
        virtual void Init(){}
        virtual void Shutdown(){}

        /// Returns true if layer should be detached
        virtual bool Update(float deltaTime) { return false; }
        virtual void Render(){}
    };

    struct ApplicationInfo
    {
        std::string Name = "Etna Application";
    };

    class Application
    {
    public:
        explicit Application(ApplicationInfo appInfo = ApplicationInfo());
        ~Application();

        void RenderFrame();

        template <class LayerType, typename ...Args>
        void PushLayer(Args... args)
        {
            auto layer = std::make_shared<LayerType>(args...);
            PushQueue.emplace(layer);
        }

        [[nodiscard]] inline const std::string& GetName() const { return AppInfo.Name; }

    private:
        Clock DeltaClock;
        ApplicationInfo AppInfo;

        // Staging queue to control Layers' initialization time
        std::queue<std::shared_ptr<Layer>> PushQueue;
        std::vector<std::shared_ptr<Layer>> Layers;
    };

    Application* CreateApplication();
}

#endif // ETNA_H
