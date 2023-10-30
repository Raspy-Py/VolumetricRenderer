#include <chrono>

class Clock {
public:
    Clock();
    ~Clock() = default;

    void Restart();

    float Elapsed() const;
    float Stamp();

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> StartTime;
};
