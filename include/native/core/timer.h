#pragma once

#include <native/core/include.h>
#include <native/core/config.h>

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;

class FrameTimer
{
public:
	FrameTimer(int target_fps = 60) : m_target_duration(1.0 / target_fps)
	{
		this->m_last_time = Clock::now();
	}

	void tick();
	void cap_fps();
	double fps() const;
	double dt() const;

private:
	TimePoint m_last_time;
	double m_delta_time = 0.0;
	double m_target_duration = 1.0 / 60.0;
};

class StopWatch
{
	// Todo add timer for measuring performance
};