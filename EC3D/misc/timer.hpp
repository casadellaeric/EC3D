#pragma once

#include <chrono>

namespace ec
{

class Timer
{
public:

    Timer();

    void reset();
    float get_elapsed_time() const;
    float get_delta_time();

private:

    std::chrono::steady_clock::time_point m_lastTick;
};

}  // namespace ec
