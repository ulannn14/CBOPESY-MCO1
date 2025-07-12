#pragma once

#ifndef PROCESS_H
#define PROCESS_H

#include "PrintCommand.hpp"
#include "ICommand.hpp"
#include "AddCommand.hpp"
#include "SymbolTable.hpp"

#include <memory>
#include <string>
#include <vector>
#include <ctime>
#include <cmath>
#include <chrono>
#include <random>

class Process
{
public:
    struct RequirementFlags
    {
        bool require_files;
        int num_files;
        bool require_memory;
        int memory_required;
    };

    enum ProcessState
    {
        READY,
        RUNNING,
        WAITING,
        FINISHED
    };

    Process(int pid, const std::string& name, const std::string& time, std::chrono::time_point<std::chrono::system_clock> creation_time, int core, int min_ins, int max_ins, size_t mem_per_proc, size_t mem_per_frame);
    void executeCurrentCommand();
    int getCommandCounter() const;
    int getLinesOfCode() const;
    int getCPUCoreID() const;
    size_t getMemoryRequired() const;
    void setCPUCoreID(int core);
    ProcessState getState() const;
    void setState(ProcessState state);
    size_t getPID() const;
    std::string getName() const;
    std::string getTime() const;
    std::chrono::time_point<std::chrono::system_clock> getCreationTime() const;
    void setMemory(void* memory);
    void* getMemory() const;
    void setAllocTime();
    std::chrono::time_point<std::chrono::system_clock> getAllocTime() const;
    size_t getNumPages() const;
    void calculateFrame();
    void generateCommands(int min_ins, int max_ins);
    std::vector<std::shared_ptr<ICommand>> generateRandomCommands(int count, int depth);
    void setSleepTicks(uint8_t ticks);
    void decrementSleepTick();
    bool isSleeping();
	void pushToLog(const std::string& message);
    void displayLogs() const;
    size_t getLogCount() const;


private:
    size_t pid_;
    std::string name_;
    std::string time_;
    std::vector<std::shared_ptr<ICommand>> command_list_;
    std::vector<std::string> log_list_;
    std::chrono::time_point<std::chrono::system_clock> allocation_time_;
    size_t mem_per_proc_;
    size_t mem_per_frame_;
    size_t num_pages_;
    int command_counter_ = 0;
    int cpu_core_id_;
    RequirementFlags requirement_flags_;
    void* memory_;
    SymbolTable symbol_table_;
    std::mt19937 gen_;
    uint8_t sleep_ticks_remaining_ = 0;
    ProcessState process_state_ = ProcessState::READY;
    int var_counter_ = 0;
    std::chrono::time_point<std::chrono::system_clock> creation_time_;

};

#endif