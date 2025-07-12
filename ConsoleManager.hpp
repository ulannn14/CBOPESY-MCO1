#ifndef CONSOLE_MANAGER_H
#define CONSOLE_MANAGER_H

#include "ProcessManager.hpp"
#include "ConsoleScreen.hpp"
#include "Clock.hpp"

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <mutex>

class ConsoleManager
{
private:
    int num_cpu = 0;
    std::string scheduler = "";
    int quantum_cycles = 0;
    int batch_process_freq = 0;
    int min_ins = 0;
    int max_ins = 0;
    int delays_per_exec = 0;
    bool initialized = false;
    bool scheduler_running = false;
    Clock* cpu_clock;
    size_t mem_per_proc;
    size_t max_overall_mem;
    size_t mem_per_frame;

    struct Screen
    {
        std::string process_name;
        int current_line = 0;
        int total_lines = 0;
        std::string timestamp;
    };

    std::map<std::string, Screen> screens;
    ConsoleScreen screen_manager;
    ProcessManager* process_manager = nullptr;

public:
    ~ConsoleManager();
    void createSession(const std::string& name);
    void generateSession(const std::string& name);
    void displayAllScreens();
    void getInput(const std::string& command);
};

#endif