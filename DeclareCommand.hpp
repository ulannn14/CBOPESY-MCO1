#ifndef DECLARE_COMMAND_H
#define DECLARE_COMMAND_H

#include "ICommand.hpp"
#include "SymbolTable.hpp"

#include <ctime>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <vector>

class DeclareCommand : public ICommand
{

public:
    DeclareCommand(int pid, int core, std::string name, SymbolTable& table,
        const std::string& varName, uint16_t value,
        std::vector<std::string>* logList)
        : ICommand(pid, CommandType::DECLARE), core_(core), table_(table),
        varName(varName), name_(name), log_list_(logList)
    {
        st = ST(ST::DataType::UINT16, std::to_string(value));
    }

    void execute() override
    {
        table_.insert(varName, st);

        std::ostringstream oss;
        oss << getCurrentTimestamp() << " Core:" << core_;
        if (sub_level_ > 0) oss << " [SUBCOMMAND-" << sub_level_ << "]";
        oss << " \"Inserted to the Symbol table: " << varName << " " << st.value << "\"";
        std::string log_line = oss.str();

        std::ofstream outfile(name_ + ".txt", std::ios::app);
        outfile << log_line << std::endl;
        outfile.close();

        if (log_list_)
            log_list_->push_back(log_line);
    }


    void setCore(int core) override
    {
        core_ = core;
    }

private:
    int core_;
    std::string varName;
    ST st;
    std::string name_;
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