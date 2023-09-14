#include "Clock.h"

Clock::Clock()
{
    Restart();
}

void Clock::Restart()
{
    StartTime = std::chrono::high_resolution_clock::now();
}

float Clock::Elapsed() const
{
    auto endTime = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(endTime - StartTime);
    return elapsedTime.count();
}

float Clock::Stamp()
{
    auto endTime = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(endTime - StartTime);
    StartTime = endTime;
    return elapsedTime.count();
}
