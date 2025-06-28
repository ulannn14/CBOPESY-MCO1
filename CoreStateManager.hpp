#ifndef CORE_STATE_MANAGER_H
#define CORE_STATE_MANAGER_H

#include <vector>
#include <mutex>
#include <string>

class CoreStateManager
{
public:
    static CoreStateManager& getInstance();
    void setCoreState(int core_id, int state, std::string process_name);
    int getCoreState(int core_id);
    const std::vector<int>& getCoreStates() const;
    void initialize(int num_core);
    const std::vector<std::string>& getProcess() const;

private:
    CoreStateManager() = default;
    CoreStateManager(const CoreStateManager&) = delete;
    CoreStateManager& operator=(const CoreStateManager&) = delete;

    std::vector<int> core_states;
    std::vector<std::string> process_names;
    mutable std::mutex core_states_mutex;
};

#endif