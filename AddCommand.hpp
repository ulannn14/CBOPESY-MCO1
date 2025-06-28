#ifndef ADD_COMMAND_H
#define ADD_COMMAND_H

#include "ICommand.hpp"
#include "ST.hpp"
#include "SymbolTable.hpp"

#include <ctime>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <cstdint>
#include <algorithm>
#include <vector>

class AddCommand : public ICommand
{
public:
    AddCommand(int pid, int core, const std::string& name, SymbolTable& table,
        const std::string& varName, const std::string& var2Name, const std::string& var3Name,
        std::vector<std::string>* log_list)
        : ICommand(pid, CommandType::ADD), core_(core), name_(name),
        table_(table), varName_(varName), var2Name_(var2Name), var3Name_(var3Name),
        log_list_(log_list)
    {
    }

    void execute() override
    {
        ST st2 = table_.fetch(var2Name_);
        ST st3 = table_.fetch(var3Name_);

        int v2 = std::stoi(st2.value);
        int v3 = std::stoi(st3.value);
        int result = v2 + v3;

        uint16_t clampedResult = static_cast<uint16_t>(std::min(result, 65535));
        ST resultST(ST::DataType::UINT16, std::to_string(clampedResult));
        table_.insert(varName_, resultST);

        std::ostringstream oss;

        oss << getCurrentTimestamp() << " Core:" << core_;
        if (sub_level_ > 0) oss << " [SUBCOMMAND-" << sub_level_ << "]";
        oss << " \"ADD result: " << resultST.value
            << " (" << varName_ << ") <- "
            << v2 << " (" << var2Name_ << ") + "
            << v3 << " (" << var3Name_ << ")\"";

        std::string log_line = oss.str();

        std::ofstream outfile(name_ + ".txt", std::ios::app);
        outfile << log_line << std::endl;
        outfile.close();

        if (log_list_) log_list_->push_back(log_line);
    }

    void setCore(int core) override
    {
        core_ = core;
    }

private:
    int core_;
    std::string name_;
    std::string varName_, var2Name_, var3Name_;
    SymbolTable& table_;
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
