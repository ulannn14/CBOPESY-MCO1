#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "Clock.hpp"
#include "Globals.hpp"
#include "FlatMemoryAllocator.hpp"

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <memory>
#include <fstream>
#include <string>
#include <map>
#include <tuple>

class Process;

class Scheduler
{
public:
    Scheduler(std::string scheduler_algo, int delays_per_exec, int n_cpu, int quantum_cycle, Clock* cpu_clock, IMemoryAllocator* memory_allocator);
    void addProcess(std::shared_ptr<Process> process);
    void setAlgorithm(const std::string& algorithm);
    void setNumCPUs(int num);
    void setDelays(int delay);
    void setQuantumCycle(int quantum_cycle);
    void start();
    void stop();
    void clearQueue();
    void setCPUClock(Clock* cpu_clock);

private:
    void run(int core_id);
    void scheduleFCFS(int core_id);
    void scheduleRR(int core_id);
    void startMemoryLog();
    void logMemoryState(int cycle);

    bool memory_log_ = false;
    bool is_running;
    int active_threads_;
    int cpu_count;
    int delay_per_execution;
    int quantum_cycle;
    int ready_threads;
    std::string scheduler_algorithm;
    std::queue<std::shared_ptr<Process>> process_queue_;
    std::vector<std::thread> worker_threads_;
    std::mutex queue_mutex_;
    std::mutex active_threads_mutex_;
    std::condition_variable queue_condition_;
    std::mutex start_mutex_;
    std::mutex log_mutex_;
    std::condition_variable start_condition_;
    Clock* cpu_clock;
    IMemoryAllocator* memory_allocator_;
    std::thread memory_logging_thread_;
};

#endif