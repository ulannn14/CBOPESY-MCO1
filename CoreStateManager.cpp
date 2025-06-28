#include "CoreStateManager.hpp"

#include <iostream>

CoreStateManager& CoreStateManager::getInstance()
{
    static CoreStateManager instance;
    return instance;
}

void CoreStateManager::setCoreState(int core_id, int state, std::string process_name)
{
    std::lock_guard<std::mutex> lock(core_states_mutex);
    core_id--;
    if (core_id >= 0 && core_id < core_states.size())
    {
        core_states[core_id] = state;
        process_names[core_id] = process_name;
    }
    else
    {
        std::cerr << "Error: Core ID " << (core_id + 1) << " is out of range!" << std::endl;
    }
}

int CoreStateManager::getCoreState(int core_id)
{
    std::lock_guard<std::mutex> lock(core_states_mutex);
    core_id--;
    if (core_id >= 0 && core_id < core_states.size())
    {
        return core_states[core_id];
    }
    else
    {
        std::cerr << "Error: Core ID " << (core_id + 1) << " is out of range!" << std::endl;
        return false;
    }
}

const std::vector<std::string>& CoreStateManager::getProcess() const
{
    std::lock_guard<std::mutex> lock(core_states_mutex);
    return process_names;
}

const std::vector<int>& CoreStateManager::getCoreStates() const
{
    std::lock_guard<std::mutex> lock(core_states_mutex);
    if (core_states.empty())
    {
        static std::vector<int> empty_vec;
        return empty_vec;
    }
    return core_states;
}

void CoreStateManager::initialize(int num_core)
{
    std::lock_guard<std::mutex> lock(core_states_mutex);
    core_states.resize(num_core, false);
    process_names.resize(num_core, "");
}