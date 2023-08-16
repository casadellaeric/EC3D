#include "pch.hpp"
#include "timer.hpp"

namespace klast
{

Timer::Timer() :
  m_lastTick{ std::chrono ::steady_clock::now() }
{
}

void Timer::reset()
{
    m_lastTick = std::chrono::steady_clock::now();
}

float Timer::get_elapsed_time() const
{
    auto now{ std::chrono::steady_clock::now() };
    return std::chrono::duration<float>(now - m_lastTick).count();
}

float Timer::get_delta_time()
{
    float deltaTime{ get_elapsed_time() };
    reset();
    return deltaTime;
}

}  // namespace klast
