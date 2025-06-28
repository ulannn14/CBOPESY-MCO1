#include "Scheduler.hpp"
#include "Process.hpp"
#include "CoreStateManager.hpp"
#include "Clock.hpp"
#include "Globals.hpp"
#include "ProcessManager.hpp"

#include <iostream>
#include <chrono>
#include <iomanip>

Scheduler::Scheduler(std::string scheduler_algo, int delays_per_exec, int n_cpu, int quantum_cycle, Clock* cpu_clock)
    : is_running(false), active_threads_(0), ready_threads(0), scheduler_algorithm(scheduler_algo), delay_per_execution(delays_per_exec),
    cpu_count(n_cpu), quantum_cycle(quantum_cycle), cpu_clock(cpu_clock)
{
}

void Scheduler::addProcess(std::shared_ptr<Process> process)
{
    std::unique_lock<std::mutex> lock(queue_mutex_);
    process_queue_.push(process);
    queue_condition_.notify_one();
}

void Scheduler::setAlgorithm(const std::string& algorithm)
{
    scheduler_algorithm = algorithm;
}

void Scheduler::setNumCPUs(int num)
{
    cpu_count = num;
    CoreStateManager::getInstance().initialize(cpu_count);
}

void Scheduler::setDelays(int delay)
{
    delay_per_execution = delay;
}

void Scheduler::setCPUClock(Clock* clock)
{
    cpu_clock = clock;
}

void Scheduler::setQuantumCycle(int quantum_cycle)
{
    quantum_cycle = quantum_cycle;
}

void Scheduler::start()
{
    is_running = true;
    for (int i = 1; i <= cpu_count; ++i)
    {
        worker_threads_.emplace_back(&Scheduler::run, this, i);
    }

    {
        std::unique_lock<std::mutex> lock(start_mutex_);
        start_condition_.wait(lock, [this]
            {
                return ready_threads == cpu_count;
            });
    }
}

void Scheduler::stop()
{
    is_running = false;

    cpu_clock->getCondition().notify_all(); 
    queue_condition_.notify_all();         

    for (auto& thread : worker_threads_)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    std::cout << "Scheduler fully stopped.\n";
}

void Scheduler::clearQueue()
{
    std::lock_guard<std::mutex> lock(queue_mutex_);
    std::queue<std::shared_ptr<Process>> empty;
    std::swap(process_queue_, empty);
}


void Scheduler::run(int core_id)
{
    {
        std::lock_guard<std::mutex> lock(start_mutex_);
        ready_threads++;
        if (ready_threads == cpu_count)
        {
            start_condition_.notify_one();
        }
    }

    if (scheduler_algorithm == "rr")
    {
        scheduleRR(core_id);
    }
    else if (scheduler_algorithm == "fcfs")
    {
        scheduleFCFS(core_id);
    }
}

void Scheduler::scheduleFCFS(int core_id)
{
    while (is_running && !GLOBAL_SHUTTING_DOWN)
    {
        std::shared_ptr<Process> process;

        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_condition_.wait(lock, [this]
                {
                    return !process_queue_.empty() || !is_running;
                });

            if (!is_running)
            {
                break;
            }

            process = process_queue_.front();
            process_queue_.pop();
        }

        

        if (process)
        {
            {
                std::lock_guard<std::mutex> lock(active_threads_mutex_);
                active_threads_++;
                if (active_threads_ > cpu_count)
                {
                    std::cerr << "Error: Exceeded CPU limit!" << std::endl;
                    active_threads_--;
                    continue;
                }
            }

            process->setState(Process::ProcessState::RUNNING);
            process->setCPUCoreID(core_id);
            CoreStateManager::getInstance().setCoreState(core_id, true, process->getName());

            int last_clock = cpu_clock->getCpuClock();
            bool first_command_executed = false;
            int cycle_counter = 0;

            while (process->getCommandCounter() < process->getLinesOfCode())
            {
                if (GLOBAL_SHUTTING_DOWN) {
                    break;
                }

                {
                    std::unique_lock<std::mutex> lock(cpu_clock->getMutex());
                    cpu_clock->getCondition().wait(lock, [&]
                        {
                            return cpu_clock->getCpuClock() > last_clock;
                        });
                    last_clock = cpu_clock->getCpuClock();

                    auto procs = GLOBAL_PM->getAllProcess();
                    for (auto& [name, p] : procs)
                    {
                        if (p->getState() == Process::WAITING && p->isSleeping())
                        {
                            p->decrementSleepTick();
                        }
                    }
                }

                if (!first_command_executed || (++cycle_counter >= delay_per_execution))
                {
                    process->executeCurrentCommand();
                    first_command_executed = true;
                    cycle_counter = 0;
                }
            }

            process->setState(Process::ProcessState::FINISHED);

            {
                std::lock_guard<std::mutex> lock(active_threads_mutex_);
                active_threads_--;
            }

            queue_condition_.notify_one();
        }

        CoreStateManager::getInstance().setCoreState(core_id, false, "");
    }
}

void Scheduler::scheduleRR(int core_id)
{
    while (is_running)
    {
        std::shared_ptr<Process> process;

        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_condition_.wait(lock, [this]
                {
                    return !process_queue_.empty() || !is_running;
                });

            if (!is_running)
            {
                break;
            }

            process = process_queue_.front();
            process_queue_.pop();
        }

        if (process)
        {
            {
                std::lock_guard<std::mutex> lock(active_threads_mutex_);
                active_threads_++;
                if (active_threads_ > cpu_count)
                {
                    std::cerr << "Error: Exceeded CPU limit!" << std::endl;
                    active_threads_--;
                    continue;
                }
            }

            process->setState(Process::ProcessState::RUNNING);
            process->setCPUCoreID(core_id);
            CoreStateManager::getInstance().setCoreState(core_id, true, process->getName());

            int quantum = 0;
            int last_clock = cpu_clock->getCpuClock();
            bool first_command_executed = true;
            int cycle_counter = 0;

            while (process->getCommandCounter() < process->getLinesOfCode() && quantum < quantum_cycle)
            {
                if (delay_per_execution != 0)
                {
                    std::unique_lock<std::mutex> lock(cpu_clock->getMutex());
                    cpu_clock->getCondition().wait(lock, [&]
                        {
                            return cpu_clock->getCpuClock() > last_clock;
                        });
                    last_clock = cpu_clock->getCpuClock();

                    auto procs = GLOBAL_PM->getAllProcess();
                    for (auto& [name, p] : procs)
                    {
                        if (p->getState() == Process::WAITING && p->isSleeping())
                        {
                            p->decrementSleepTick();
                        }
                    }
                }

                if (!first_command_executed || (++cycle_counter >= delay_per_execution))
                {
                    process->executeCurrentCommand();
                    first_command_executed = false;
                    cycle_counter = 0;
                    quantum++;
                }
            }

            std::this_thread::sleep_for(std::chrono::microseconds(2000));

            if (process->getCommandCounter() < process->getLinesOfCode())
            {
                process->setState(Process::ProcessState::READY);
                std::lock_guard<std::mutex> lock(queue_mutex_);
                process_queue_.push(process);
            }
            else
            {
                process->setState(Process::ProcessState::FINISHED);
                process->setMemory(nullptr);
            }

            {
                std::lock_guard<std::mutex> lock(active_threads_mutex_);
                active_threads_--;
            }

            queue_condition_.notify_one();
        }

        CoreStateManager::getInstance().setCoreState(core_id, false, "");
    }
}