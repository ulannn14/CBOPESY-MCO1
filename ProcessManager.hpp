#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include "Process.hpp"
#include "Scheduler.hpp"
#include "Clock.hpp"
#include "FlatMemoryAllocator.hpp"

#include <map>
#include <memory>
#include <vector>
#include <iostream>
#include <thread>
#include <mutex>

class ProcessManager
{
private:
    std::map<std::string, std::shared_ptr<Process>> process_list_;
    int pid_counter_ = 0;
    Scheduler* scheduler_;
    std::thread scheduler_thread_;
    int min_ins_;
    int max_ins_;
    Clock* cpu_clock;
    size_t mem_per_proc;
    size_t max_overall_mem;
    size_t mem_per_frame;
    IMemoryAllocator* memory_allocator_;
    int num_cpu_;
    std::mutex process_list_mutex_;
    std::mutex core_states_mutex_;

public:
    ProcessManager(int min_ins, int max_ins, int n_cpu, std::string scheduler_algo, int delays_per_exec,
        int quantum_cycle, Clock* cpu_clock, size_t max_overall_mem, size_t mem_per_frame, size_t mem_per_proc);

    void addProcess(std::string name, std::string time, std::chrono::time_point<std::chrono::system_clock> creation_time);
    std::shared_ptr<Process> getProcess(std::string name);
    std::map<std::string, std::shared_ptr<Process>> getAllProcess();

    ~ProcessManager();

    void processSmi();
};

#endif