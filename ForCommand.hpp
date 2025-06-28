#ifndef FOR_COMMAND_H
#define FOR_COMMAND_H

#include "ICommand.hpp"
#include <vector>
#include <memory>
#include <string>
#include <stdexcept>
#include <thread>

class ForCommand : public ICommand
{
public:
    ForCommand(int pid, int core, const std::string& name, const std::vector<std::shared_ptr<ICommand>>& instructions, int repeats, std::vector<std::string>* log_list)
        : ICommand(pid, CommandType::FOR), core_(core),  name_(name), instructions_(instructions), repeats_(repeats), log_list_(log_list)
    {
    }

    void execute() override
    {
        if (GLOBAL_SHUTTING_DOWN)
            return;

        static thread_local int current_depth = 0;

        if (current_depth >= 3)
        {
            std::cerr << "Max FOR nesting depth (3) reached for PID: " << pid_ << std::endl;
            return;
        }

        ++current_depth;

        for (int i = 0; i < repeats_; ++i)
        {
            if (GLOBAL_SHUTTING_DOWN) break;

            std::ostringstream oss;
            oss << getCurrentTimestamp()
            << " Core:" << core_
            << " \"FOR loop iteration " << (i + 1) << " of " << repeats_ << "\"";

            std::string log_line = oss.str();

            std::ofstream outfile(name_ + ".txt", std::ios::app);
            outfile << log_line << std::endl;
            outfile.close();

            if (log_list_) log_list_->push_back(log_line);

            for (auto& cmd : instructions_)
            {
                cmd->setCore(core_);
                cmd->setSubcommandLevel(current_depth);
                cmd->execute();
            }
        }

        --current_depth;
    }

    void setCore(int core) override
    {
        core_ = core;
    }

private:
    int core_;
    int repeats_;
    std::string name_;
    std::vector<std::shared_ptr<ICommand>> instructions_;
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