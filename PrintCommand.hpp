#ifndef PRINT_COMMAND_H
#define PRINT_COMMAND_H

#include "ICommand.hpp"
#include "SymbolTable.hpp"
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <vector>

class PrintCommand : public ICommand
{
public:
	PrintCommand(int pid, int core, const std::string& to_print, const std::string& name,
		std::vector<std::string>* log_list)
		: ICommand(pid, CommandType::PRINT),
		core_(core),
		to_print_(to_print),
		name_(name),
		log_list_(log_list),
		dynamic_(false)
	{
	}

	PrintCommand(int pid, int core, const std::string& var_name, const std::string& name,
		SymbolTable& table, std::vector<std::string>* log_list)
		: ICommand(pid, CommandType::PRINT),
		core_(core),
		var_name_(var_name),
		name_(name),
		table_(&table),
		log_list_(log_list),
		dynamic_(true)
	{
	}

	void execute() override
	{
		std::string msg;

		if (dynamic_)
		{
			ST st = table_->fetch(var_name_);
			msg = "Current " + var_name_ + ": " + st.value;
		}
		else
		{
			msg = to_print_;
		}

		std::ostringstream oss;
		oss << getCurrentTimestamp() << " Core:" << core_ << " \"" << msg << "\"";
		std::string log_line = oss.str();

		std::ofstream outfile(name_ + ".txt", std::ios::app);
		outfile << log_line << std::endl;
		outfile.close();

		if (log_list_)
		{
			log_list_->push_back(log_line);
		}
	}

	void setCore(int core) override
	{
		core_ = core;
	}

private:
	int core_;
	std::string name_;

	std::string to_print_;

	std::string var_name_;
	SymbolTable* table_ = nullptr;

	std::vector<std::string>* log_list_ = nullptr;
	bool dynamic_ = false;

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
