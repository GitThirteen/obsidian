#include <native/core/timer.h>

void FrameTimer::tick()
{
	auto curr_time = Clock::now();
	std::chrono::duration<double> elapsed = curr_time - this->m_last_time;
	this->m_delta_time = elapsed.count();
	this->m_last_time = curr_time;
}

void FrameTimer::cap_fps() const
{
    auto target_wake_time = this->m_last_time + std::chrono::duration<double>(this->m_target_duration);
    constexpr auto sleep_margin = std::chrono::duration<double>(0.002); // We need this to wake up slightly earlier than the target time to be precise.

    if (Clock::now() < (target_wake_time - sleep_margin))
    {
        std::this_thread::sleep_until(target_wake_time - sleep_margin);
    }

    while (Clock::now() < target_wake_time)
    {
        std::this_thread::yield();
    }
}

double FrameTimer::fps() const
{
	return 1.0 / this->m_delta_time;
}

double FrameTimer::dt() const
{
	return this->m_delta_time;
}