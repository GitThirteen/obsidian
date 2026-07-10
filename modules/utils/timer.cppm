export module Obsidian.Utils:Timer;
import std;

export namespace obsidian 
{
    class FrameTimer 
    {
    public:
        FrameTimer(int target_fps = 60) : m_target_duration(1.0 / target_fps)
        {
            m_last_time = Clock::now();
        }

        auto tick() -> void
        {
            auto curr_time = Clock::now();
            std::chrono::duration<double> elapsed = curr_time - m_last_time;
            m_delta_time = elapsed.count();
            m_last_time = curr_time;
        }

        auto cap_fps() const -> void
        {
            auto target_wake_time = m_last_time + std::chrono::duration<double>(m_target_duration);
            constexpr auto sleep_margin = std::chrono::duration<double>(0.002);

            if (Clock::now() < (target_wake_time - sleep_margin))
            {
                std::this_thread::sleep_until(target_wake_time - sleep_margin);
            }

            while (Clock::now() < target_wake_time)
            {
                std::this_thread::yield();
            }
        }

        auto fps() const -> double 
        { 
            return 1.0 / m_delta_time;
        }

        auto dt() const -> double
        { 
            return m_delta_time;
        }

    private:
        using Clock = std::chrono::steady_clock;
        using TimePoint = std::chrono::time_point<Clock>;

        TimePoint m_last_time;
        double m_delta_time = 0.0;
        double m_target_duration;
    };
}