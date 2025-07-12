#include "Process.hpp"
#include "DeclareCommand.hpp"
#include "AddCommand.hpp"
#include "SubtractCommand.hpp"
#include "SleepCommand.hpp"
#include "ForCommand.hpp"
#include "SymbolTable.hpp"
#include <random>
#include <string>

Process::Process(int pid, const std::string& name, const std::string& time, std::chrono::time_point<std::chrono::system_clock> creation_time, int core, int min_ins, int max_ins, size_t mem_per_proc, size_t mem_per_frame)
    : pid_(pid),
    name_(name),
    time_(time),
	creation_time_(creation_time),
    cpu_core_id_(core),
    process_state_(READY),
    gen_(std::random_device{}()), 
    mem_per_proc_(mem_per_proc),
    mem_per_frame_(mem_per_frame),
    memory_(nullptr)
{
    calculateFrame();
}


void Process::executeCurrentCommand()
{   
    if (command_counter_ < command_list_.size())
    {
        command_list_[command_counter_]->setCore(cpu_core_id_);
        command_list_[command_counter_]->execute();
        command_counter_++;
    }
}

void Process::calculateFrame()
{
    num_pages_ = static_cast<size_t>(std::ceil(static_cast<double>(mem_per_proc_) / mem_per_frame_));
}

int Process::getCommandCounter() const
{
    return command_counter_;
}

void Process::setMemory(void* memory)
{
    memory_ = memory;
}

void* Process::getMemory() const
{
    return memory_;
}

int Process::getLinesOfCode() const
{
    return static_cast<int>(command_list_.size());
}

size_t Process::getMemoryRequired() const
{
    return mem_per_proc_;
}

int Process::getCPUCoreID() const
{
    return cpu_core_id_;
}

void Process::setCPUCoreID(int core)
{
    cpu_core_id_ = core;
}


Process::ProcessState Process::getState() const
{
    return process_state_;
}

void Process::setState(ProcessState state)
{
    process_state_ = state;
}

size_t Process::getPID() const
{
    return pid_;
}

std::string Process::getName() const
{
    return name_;
}

std::string Process::getTime() const
{
    return time_;
}

std::chrono::time_point<std::chrono::system_clock> Process::getCreationTime() const
{
	return creation_time_;
}

void Process::generateCommands(int min_ins, int max_ins)
{ 
    std::uniform_int_distribution<> distrib(min_ins, max_ins);
    std::uniform_int_distribution<uint16_t> value_uint16(0, 65535);
    std::uniform_int_distribution<int> value_uint8(0, 255); // Use int here


    uint16_t int16 = 0;
    uint8_t X = 0;
    std::string varName = "";
    std::string varName2 = "";
    std::string varName3 = "";
	std::string msg = "";

    int num_commands = distrib(gen_);

    for (int i = 1; i <= num_commands; ++i)
    {
        std::uniform_int_distribution<> cmd_type(0, 5);  // 6 possible commands: 0-5
        int choice = cmd_type(gen_);

        std::shared_ptr<ICommand> cmd;

        switch (choice)
        {
        case 0:
            msg = "Hello World From " + name_ + " started.";
            cmd = std::make_shared<PrintCommand>(pid_, cpu_core_id_, msg, name_, &log_list_);
        	break;

        case 1:
            varName = "var" + std::to_string(var_counter_++);
            int16 = value_uint16(gen_);
            cmd = std::make_shared<DeclareCommand>(pid_, cpu_core_id_, name_, symbol_table_, varName, int16, &log_list_);
            break;

        case 2:
            varName = "var" + std::to_string(var_counter_++);
            
            int16 = value_uint16(gen_);
            varName2 = "var" + std::to_string(var_counter_++);
            cmd = std::make_shared<DeclareCommand>(pid_, cpu_core_id_, name_, symbol_table_, varName2, int16, &log_list_);
            command_list_.push_back(cmd);

            int16 = value_uint16(gen_);
            varName3 = "var" + std::to_string(var_counter_++);

            cmd = std::make_shared<DeclareCommand>(pid_, cpu_core_id_, name_, symbol_table_, varName3, int16, &log_list_);
            command_list_.push_back(cmd);

            cmd = std::make_shared<AddCommand>(pid_, cpu_core_id_, name_, symbol_table_, varName, varName2, varName3, &log_list_);
            break;

        case 3:
            varName = "var" + std::to_string(var_counter_++);

            int16 = value_uint16(gen_);
            varName2 = "var" + std::to_string(var_counter_++);
            cmd = std::make_shared<DeclareCommand>(pid_, cpu_core_id_, name_, symbol_table_, varName2, int16, &log_list_);
            command_list_.push_back(cmd);

            int16 = value_uint16(gen_);
            varName3 = "var" + std::to_string(var_counter_++);

            cmd = std::make_shared<DeclareCommand>(pid_, cpu_core_id_, name_, symbol_table_, varName3, int16, &log_list_);
            command_list_.push_back(cmd);

            cmd = std::make_shared<SubtractCommand>(pid_, cpu_core_id_, name_, symbol_table_, varName, varName2, varName3, &log_list_);
            break;

        case 4:
            X = static_cast<uint8_t>(value_uint8(gen_));
            cmd = std::make_shared<SleepCommand>(pid_, cpu_core_id_, X, &log_list_);
            break;

        case 5:
            int repeat_count = value_uint8(gen_) % 5 + 1;
            int inner_cmd_count = value_uint8(gen_) % 3 + 1;
            auto loop_cmds = generateRandomCommands(inner_cmd_count, 1);
            cmd = std::make_shared<ForCommand>(pid_, cpu_core_id_, name_, loop_cmds, repeat_count, &log_list_);
            break;
        }
        command_list_.push_back(cmd);
        
    } 
    /* FOR NUMBER 4
    std::uniform_int_distribution<> distrib(min_ins, max_ins);
    std::uniform_int_distribution<uint16_t> value_uint10(1, 10);

    int num_commands = distrib(gen_);
    if (num_commands % 2 != 0) num_commands--;  // ensure even count for ADD + PRINT pairing

    // Declare x = 0
    symbol_table_.insert("x", ST(ST::DataType::UINT16, "0"));

    for (int i = 0; i < num_commands; i += 2)
    {
        std::shared_ptr<ICommand> cmd;

        // Step 1: Update c with a new random value
        uint16_t new_c_val = value_uint10(gen_);
        symbol_table_.insert("c", ST(ST::DataType::UINT16, std::to_string(new_c_val)));

        // Step 2: Add x = x + c
        cmd = std::make_shared<AddCommand>(
            pid_, cpu_core_id_, name_, symbol_table_,
            "x", "x", "c", &log_list_
        );
        command_list_.push_back(cmd);


        cmd = std::make_shared<PrintCommand>(pid_, cpu_core_id_, "x", name_, symbol_table_, &log_list_);
        command_list_.push_back(cmd);
    }
    */
}

std::vector<std::shared_ptr<ICommand>> Process::generateRandomCommands(int count, int depth)
{
    std::vector<std::shared_ptr<ICommand>> commands;
    std::uniform_int_distribution<> cmd_type(0, 5); // same 6 types
    std::uniform_int_distribution<uint16_t> value_uint16(0, 65535);
    std::uniform_int_distribution<int> value_uint8(0, 255);

    for (int i = 0; i < count; ++i)
    {
        std::shared_ptr<ICommand> cmd;
        int choice = cmd_type(gen_);

        switch (choice)
        {
        case 0:
        {
            std::string msg = "Hello World From " + name_ + " started.";
            cmd = std::make_shared<PrintCommand>(pid_, cpu_core_id_, msg, name_, &log_list_);
            break;
        }
        case 1:
        {
            std::string varName = "var" + std::to_string(var_counter_++);
            uint16_t value = value_uint16(gen_);
            cmd = std::make_shared<DeclareCommand>(pid_, cpu_core_id_, name_, symbol_table_, varName, value, &log_list_);
            break;
        }
        case 2:
        case 3:
        {
            std::string varName = "var" + std::to_string(var_counter_++);
            std::string var2Name = "var" + std::to_string(var_counter_++);
            std::string var3Name = "var" + std::to_string(var_counter_++);

            auto d1 = std::make_shared<DeclareCommand>(pid_, cpu_core_id_, name_, symbol_table_, var2Name, value_uint16(gen_), &log_list_);
            auto d2 = std::make_shared<DeclareCommand>(pid_, cpu_core_id_, name_, symbol_table_, var3Name, value_uint16(gen_), &log_list_);
            commands.push_back(d1);
            commands.push_back(d2);

            if (choice == 2)
                cmd = std::make_shared<AddCommand>(pid_, cpu_core_id_, name_, symbol_table_, varName, var2Name, var3Name, &log_list_);
            else
                cmd = std::make_shared<SubtractCommand>(pid_, cpu_core_id_, name_, symbol_table_, varName, var2Name, var3Name, &log_list_);
            break;
        }
        case 4:
        {
            uint8_t ticks = static_cast<uint8_t>(value_uint8(gen_));
            cmd = std::make_shared<SleepCommand>(pid_, cpu_core_id_, ticks, &log_list_);
            break;
        }
        case 5:
        {
            if (depth < 3) 
            {
                int repeats = value_uint8(gen_) % 5 + 1;
                int nested_count = value_uint8(gen_) % 3 + 1;
                auto nested_cmds = generateRandomCommands(nested_count, depth + 1);
                cmd = std::make_shared<ForCommand>(pid_, cpu_core_id_, name_, nested_cmds, repeats, &log_list_);
                break;
            }
            continue;
        }
        }

        if (cmd)
            commands.push_back(cmd);
    }

    return commands;
}

void Process::setAllocTime()
{
    allocation_time_ = std::chrono::system_clock::now();
}

std::chrono::time_point<std::chrono::system_clock> Process::getAllocTime() const
{
    return allocation_time_;
}

size_t Process::getNumPages() const
{
    return num_pages_;
}

void Process::setSleepTicks(uint8_t ticks)
{
    sleep_ticks_remaining_ = ticks;
}

void Process::decrementSleepTick()
{
    if (sleep_ticks_remaining_ > 0)
        sleep_ticks_remaining_--;
    if (sleep_ticks_remaining_ == 0 && process_state_ == Process::WAITING)
        process_state_ = Process::READY;
}

bool Process::isSleeping()
{
    return sleep_ticks_remaining_ > 0;
}

void Process::pushToLog(const std::string& message)
{
	log_list_.push_back(message);
}

void Process::displayLogs() const
{
    for (const auto& log : log_list_)
    {
        std::cout << log << std::endl;
    }
}

size_t Process::getLogCount() const
{
	return log_list_.size();
}

