#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include "Process.hpp"
#include "Scheduler.hpp"
#include "Clock.hpp"

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
    size_t min_mem_per_proc_;
    size_t max_mem_per_proc_;
    size_t max_mem_;
    size_t mem_per_frame_;
    int num_cpu_;
    std::mutex process_list_mutex_;
    std::mutex core_states_mutex_;

public:
    ProcessManager(int min_ins, int max_ins, int n_cpu, std::string scheduler_algo, int delays_per_exec,
        int quantum_cycle, Clock* cpu_clock);

    void addProcess(std::string name, std::string time, std::chrono::time_point<std::chrono::system_clock> creation_time);
    std::shared_ptr<Process> getProcess(std::string name);
    std::map<std::string, std::shared_ptr<Process>> getAllProcess();

    ~ProcessManager();

    void processSmi();
};

#endif