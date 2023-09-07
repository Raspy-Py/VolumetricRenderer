#include "Etna.h"

int main(int argc, char** argv)
{
    Etna::Init(argc, argv);
    Etna::Run();
    Etna::Shutdown();

    return 0;
}
