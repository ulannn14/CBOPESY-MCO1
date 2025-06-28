#ifndef SLEEP_COMMAND_H
#define SLEEP_COMMAND_H

#include "ICommand.hpp"
#include "ProcessManager.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <vector>

extern ProcessManager* GLOBAL_PM;

class SleepCommand : public ICommand
{
public:
    SleepCommand(int pid, int core, uint8_t ticks, std::vector<std::string>* log_list)
        : ICommand(pid, CommandType::SLEEP), core_(core), ticks_(ticks), log_list_(log_list)
    {
    }

    void execute() override
    {
        if (GLOBAL_SHUTTING_DOWN)
            return;
        
        auto processes = GLOBAL_PM->getAllProcess();
        for (auto& [name, proc] : processes)
        {
            if (proc->getPID() == pid_)
            {
                proc->setSleepTicks(ticks_);
                proc->setState(Process::WAITING);

                std::ostringstream oss;
                oss << getCurrentTimestamp() << " Core:" << core_;
                if (sub_level_ > 0) oss << " [SUBCOMMAND-" << sub_level_ << "]";
                oss << " \"SLEEP for " << std::to_string(ticks_) << " ticks.\"";

                std::string log_line = oss.str();

                std::ofstream outfile(name + ".txt", std::ios::app);
                outfile << log_line << std::endl;
                outfile.close();

                if (log_list_) log_list_->push_back(log_line);
                break;
            }
        }

    }

    void setCore(int core) override
    {
        core_ = core;
    }

private:
    int core_;
    uint8_t ticks_;
    std::vector<std::string>* log_list_;

    std::string getCurrentTimestamp()
    {
        auto now = std::chrono::system_clock::now();
        std::time_t time_now = std::chrono::system_clock::to_time_t(now);
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        std::tm local_time;
        localtime_s(&local_time, &time_now);

        std::ostringstream oss;
        oss << std::put_time(&local_time, "(%m/%d/%Y %I:%M:%S")
            << '.' << std::setfill('0') << std::setw(3) << milliseconds.count()
            << std::put_time(&local_time, "%p)");
        return oss.str();
    }
};

#endif
