#include "ConsoleScreen.hpp"
#include "CoreStateManager.hpp"

void ConsoleScreen::displayHeader()
{
    std::cerr << 
    " _____ ____   ____  _____  ______ _____ __     __\n"
    "/ ____|  _ \\ / __ \\|  __ \\| ____|/ ____ \\ \\   / /\n"
    "| |   | |_) | |  | | |__) | |__  | (___  \\ \\_/ /\n"
    "| |   |  _ <| |  | |  ___/|  __|  \\___ \\  \\   /\n"
    "| |___| |_) | |__| | |    | |____ ____) |  | |\n"
    " \\____|____ /\\____/|_|    |______|_____/   |_|\n"
    << "Hello. Welcome to the CSOPESY commandline!\n"
    << "Type 'exit' to quit, 'clear' to clear the screen" 
    << std::endl;
}

void ConsoleScreen::displayAllProcess(std::map<std::string, std::shared_ptr<Process>> process_list, int num_cpu)
{
    displayAllProcessToStream(process_list, num_cpu, std::cout);
}

void ConsoleScreen::displayAllProcessToStream(std::map<std::string, std::shared_ptr<Process>> process_list, int num_cpu, std::ostream& out)
{
    static std::mutex process_list_mutex;
    std::lock_guard<std::mutex> lock(process_list_mutex);

    if (process_list.empty())
    {
        out << "No screens available." << std::endl;
        return;
    }

    std::stringstream ready;
    std::stringstream running;
    std::stringstream finished;
    int core_usage = 0;

    const std::vector<std::string>& running_processes = CoreStateManager::getInstance().getProcess();
    const std::vector<int>& core_states = CoreStateManager::getInstance().getCoreStates();

    for (int core_state : core_states)
    {
        if (core_state)
        {
            core_usage++;
        }
    }

    std::vector<std::shared_ptr<Process>> sorted_processes;
    for (const auto& pair : process_list)
        sorted_processes.push_back(pair.second);

    std::sort(sorted_processes.begin(), sorted_processes.end(),
        [](const std::shared_ptr<Process>& a, const std::shared_ptr<Process>& b) {
            return a->getCreationTime() < b->getCreationTime();
        });

    out << "\nExisting Screens:" << std::endl;
    for (const auto& process : sorted_processes)
    {
        std::stringstream temp;
        temp << std::left << std::setw(13) << process->getName()
            << " (" << process->getTime() << ") ";

        if (process->getState() == Process::RUNNING)
        {
            temp << "  Core: " << process->getCPUCoreID() << "   "
                << process->getCommandCounter() << " / "
                << process->getLinesOfCode() << std::endl;
            running << temp.str();
        }
        else if (process->getState() == Process::FINISHED)
        {
            temp << "  FINISHED " << "   "
                << process->getCommandCounter() << " / "
                << process->getLinesOfCode() << std::endl;
            finished << temp.str();
        }
    }

    out << "CPU utilization: " << (static_cast<double>(core_usage) / num_cpu) * 100 << "%\n";
    out << "Cores used: " << core_usage << "\n";
    out << "Cores available: " << num_cpu - core_usage << "\n";
    out << "------------------------------------------------";
    out << "\nRunning Processes: \n"
        << running.str();
    out << "\nFinished Processes: \n"
        << finished.str();
    out << "------------------------------------------------\n\n";
}


void ConsoleScreen::displayUpdatedProcess(std::shared_ptr<Process> process)
{
    if (process->getState() == Process::RUNNING || process->getState() == Process::READY)
    {
        std::cout << "Process name: " << process->getName() << std::endl;
        std::cout << "ID: " << process->getCPUCoreID() << std::endl;

        std::cout << "Logs: \n";
        process->displayLogs();

        std::cout << "Current instruction line: " << process->getCommandCounter() << std::endl;
        std::cout << "Lines of code: " << process->getLinesOfCode() << std::endl;
        std::cout << std::endl;
    }
    else
    {
        std::cout << "Process Name: " << process->getName() << std::endl;
        std::cout << "ID: " << process->getCPUCoreID() << std::endl;
        std::cout << "Logs: \n";
        process->displayLogs();
        std::cout << "Finished!" << std::endl;
        std::cout << std::endl;
    }
}

void ConsoleScreen::displayScreen(std::shared_ptr<Process> process)
{
    std::cout << "Screen: " << process->getName() << std::endl;
    std::cout << "Instruction: Line " << process->getCommandCounter() << " / "
              << process->getLinesOfCode() << std::endl;
    std::cout << "Created at: " << process->getTime() << std::endl;
    std::cout << "Type 'exit' to return to the main menu." << std::endl;

    std::string command;
    while (true)
    {
        std::cout << "Enter a command: ";
        std::getline(std::cin, command);
        if (command == "process-smi")
        {
            displayUpdatedProcess(process);
        }
        else if (command == "exit")
        {
            system("cls");
            displayHeader();
            break;
        }
        else
        {
            std::cout << "Unknown command. Please try again." << std::endl;
        }
    }
}

std::string ConsoleScreen::getCurrentTimestamp()
{
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm_struct;
    localtime_s(&tm_struct, &now);
    std::stringstream ss;
    ss << std::put_time(&tm_struct, "%m/%d/%Y, %I:%M:%S %p");
    return ss.str();
}

std::chrono::time_point<std::chrono::system_clock> ConsoleScreen::getCreationTime()
{
    return std::chrono::system_clock::now();
}
