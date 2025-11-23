#include <native/util/timer.h>

void FrameTimer::tick()
{
	auto curr_time = Clock::now();
	std::chrono::duration<double> elapsed = curr_time - this->m_last_time;
	this->m_delta_time = elapsed.count();
	this->m_last_time = curr_time;
}

void FrameTimer::cap_fps()
{
	auto curr_time = Clock::now();
	std::chrono::duration<double> elapsed = curr_time - this->m_last_time;

	if (elapsed.count() < this->m_target_duration)
	{
		std::chrono::duration<double> remaining(this->m_target_duration - elapsed.count());
		std::this_thread::sleep_for(remaining);
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