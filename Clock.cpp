#include "Clock.hpp"

Clock::Clock() : cpu_clock(0), active_num(0)
{
}

int Clock::getCpuClock()
{
    return cpu_clock.load();
}

void Clock::startCpuClock()
{
    if (!is_running)
    {
        is_running = true;
        std::cout << "CPU Clock started\n";
        cpu_clock_thread = std::thread([this]()
        {
            while (is_running)
            {
                {
                    std::lock_guard<std::mutex> lock(clock_mutex);
                    cpu_clock++;
                }

                cycle_condition.notify_all();

                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }
}

void Clock::stopCpuClock()
{
    is_running = false;
    if (cpu_clock_thread.joinable())
    {
        cpu_clock_thread.join();
        std::cout << "CPU Clock stopped\n";
    }
}

std::atomic<int> Clock::getActiveCpuNum()
{
    return active_num.load();
}

void Clock::incrementActiveCpuNum()
{
    active_num++;
}