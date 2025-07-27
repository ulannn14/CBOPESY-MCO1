#include "ConsoleManager.hpp"
#include "Globals.hpp"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>


void ConsoleManager::createSession(const std::string& name)
{
    if (screens.find(name) != screens.end())
    {
        std::cout << "Screen '" << name << "' already exists. Reattaching...\n";
        return;
    }

    Screen new_screen = {"Process-" + name, 0, 100, screen_manager.getCurrentTimestamp()};
    screens[name] = new_screen;

    process_manager->addProcess(name, screen_manager.getCurrentTimestamp(), screen_manager.getCreationTime());

    std::cout << "Created screen: " << name << std::endl;
    system("cls");
    process_manager->getProcess(name);
    screen_manager.displayScreen(process_manager->getProcess(name));
}

void ConsoleManager::generateSession(const std::string& name)
{
    if (screens.find(name) != screens.end())
    {
        std::cout << "Screen '" << name << "' already exists. Reattaching...\n";
        return;
    }

    Screen new_screen = {"process" + name, 0, 100, screen_manager.getCurrentTimestamp()};
    screens[name] = new_screen;

    process_manager->addProcess(name, screen_manager.getCurrentTimestamp(), screen_manager.getCreationTime());
    process_manager->getProcess(name);
}

void ConsoleManager::displayAllScreens()
{
    screen_manager.displayAllProcess(process_manager->getAllProcess(), num_cpu);
}

void ConsoleManager::getInput(const std::string& command)
{
    static std::thread scheduler_thread;

    if (!initialized && !(command == "exit" || command == "initialize"))
    {
        std::cout << "[ERROR] Initialize first using the \"initialize\" command !!!\n";
        return;
    }

    if (command == "initialize")
    {
        system("cls");
        screen_manager.displayHeader();
        std::ifstream config_file("config.txt");

        std::string temp;

        if (config_file.is_open())
        {
            config_file >> temp >> num_cpu;  
            config_file >> temp >> std::quoted(scheduler);  
            config_file >> temp >> quantum_cycles;  
            config_file >> temp >> batch_process_freq;  
            config_file >> temp >> min_ins;  
            config_file >> temp >> max_ins;  
            config_file >> temp >> delays_per_exec;  
            config_file >> temp >> max_overall_mem;
            config_file >> temp >> mem_per_frame;
            config_file >> temp >> mem_per_proc;

            config_file.close();

            cpu_clock = new Clock();
            cpu_clock->startCpuClock();

            process_manager = new ProcessManager(min_ins, max_ins, num_cpu, scheduler, delays_per_exec, quantum_cycles, cpu_clock, max_overall_mem, mem_per_frame, mem_per_proc);
            GLOBAL_PM = process_manager;

            initialized = true;

        }
        else
        {
            std::cerr << "[ERROR] Unable to open config file." << std::endl;
        }
    }
    else if (command.rfind("screen -s ", 0) == 0)
    {
        std::string name = command.substr(10);
        createSession(name);
    }
    else if (command.rfind("screen -r ", 0) == 0)
    {
        std::string name = command.substr(10);
        std::shared_ptr<Process> process = process_manager->getProcess(name);

        if (process != nullptr)
        {
            if (process->getState() != Process::FINISHED)
            {
                system("cls");
                screen_manager.displayScreen(process);
            }
            else
            {
                std::cout << "Process " << name << " not found." << std::endl;
            }
        }
        else
        {
            std::cout << "Process " << name << " not found." << std::endl;
        }
    }
    else if (command.rfind("screen -ls", 0) == 0)
    {
        displayAllScreens();
    }
    else if (command == "report-util")
    {
        std::stringstream output;
        screen_manager.displayAllProcessToStream(process_manager->getAllProcess(), num_cpu, output);

        std::string filename = "csopesy-log.txt";
        std::filesystem::path filepath = std::filesystem::current_path() / filename;

        std::ofstream out_file(filepath);
        if (out_file.is_open())
        {
            out_file << output.str();
            out_file.close();
            std::cout << "Report generated at: " << filepath << std::endl;
        }
        else
        {
            std::cerr << "Failed to write to: " << filepath << std::endl;
        }
    }
    else if (command == "scheduler-start")
    {
        if (!scheduler_running)
        {
            scheduler_running = true;
            std::cout << "Scheduler-start started.\n";

            scheduler_thread = std::thread([this]()
            {
                int tick_counter = 0;
                std::unique_lock<std::mutex> lock(cpu_clock->getMutex(), std::defer_lock);

                while (scheduler_running)
                {
                    lock.lock();
                    cpu_clock->getCondition().wait(lock);
                    lock.unlock();

                    tick_counter++;
                    if (tick_counter >= batch_process_freq)
                    {
                        tick_counter = 0;
                    	std::string name = "process" + std::to_string(screens.size());
                        generateSession(name);
                    }

                    if (screens.size() > 4)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }
                }
            });
        }
        else
        {
            std::cout << "[ERROR] \"scheduler-start\" command is already running.\n";
        }
    }
    else if (command == "scheduler-stop")
    {
        GLOBAL_SHUTTING_DOWN = true;

        if (scheduler_running)
        {
            scheduler_running = false;
            if (scheduler_thread.joinable())
            {
                scheduler_thread.join();
                std::cout << "Scheduler-start stopped\n";
            }
        }
        else
        {
            std::cout << "[ERROR] \"scheduler-start\" is not running.\n";
        }
    }
    else if (command == "process-smi")
    {
        process_manager->processSmi();
    }
    else if (command == "vmstat")
    {
        process_manager->vmStat();
    }
    else if (command == "clear")
    {
        system("cls");
    }
    else if (command == "exit")
    {
        std::cout << "Exiting..." << std::endl;

        // Set global shutdown signal
        GLOBAL_SHUTTING_DOWN = true;

        // Stop scheduler
        if (scheduler_running)
        {
            scheduler_running = false;
            if (scheduler_thread.joinable())
                scheduler_thread.join();
            std::cout << "Scheduler stopped by exit.\n";
        }

        if (cpu_clock)
            cpu_clock->stopCpuClock();

    }
    else
    {
        std::cout << "Unknown command. Please try again." << std::endl;
    }
}

ConsoleManager::~ConsoleManager() {
    if (cpu_clock) {
        cpu_clock->stopCpuClock();
        delete cpu_clock;
        cpu_clock = nullptr;
    }

    if (process_manager) {
        delete process_manager;
        process_manager = nullptr;
    }

    std::cout << "ConsoleManager shutting down...\n";
}
