#ifndef CONSOLE_SCREEN_H
#define CONSOLE_SCREEN_H

#include "Process.hpp"

#include <map>
#include <memory>
#include <string>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <mutex>

class ConsoleScreen
{
public:
    void displayHeader();
    void displayAllProcess(std::map<std::string, std::shared_ptr<Process>> process_list, int num_cpu);
    void displayUpdatedProcess(std::shared_ptr<Process> process);
    void displayScreen(std::shared_ptr<Process> process);
    void displayAllProcessToStream(std::map<std::string, std::shared_ptr<Process>> process_list, int num_cpu, std::ostream& out);
    std::string getCurrentTimestamp();
    std::chrono::time_point<std::chrono::system_clock> getCreationTime();
    std::mutex process_list_mutex;
    std::mutex core_states_mutex;
};

#endif