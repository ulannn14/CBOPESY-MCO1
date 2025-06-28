#ifndef CLOCK_H
#define CLOCK_H

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <thread>
#include <mutex>

class Clock
{
public:
    Clock();
    int getCpuClock();
    void startCpuClock();
    void stopCpuClock();
    std::atomic<int> getActiveCpuNum();
    void incrementActiveCpuNum();
    std::condition_variable& getCondition()
    {
        return cycle_condition;
    }
    std::mutex& getMutex()
    {
        return clock_mutex;
    }

private:
    std::atomic<int> cpu_clock;
    bool is_running = false;
    std::thread cpu_clock_thread;
    std::condition_variable cycle_condition;
    std::mutex clock_mutex;
    std::atomic<int> active_num;
};

#endif