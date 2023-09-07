#pragma once
#ifndef ETNA_H
#define ETNA_H

namespace Etna
{
    void Init(int argc, char** argv);
    void Run();
    void Shutdown();

    struct ApplicationInfo
    {

    };

    class Application
    {
    public:
        explicit Application(ApplicationInfo appInfo = ApplicationInfo()) {}
        ~Application() {}

        void RenderFrame()
        {
            // Go through all the layers - update and draw them
        }

    private:

    };

    Application* CreateApplication();
}

#endif // ETNA_H
