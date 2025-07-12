#include "Scheduler.hpp"
#include "Process.hpp"
#include "CoreStateManager.hpp"
#include "Clock.hpp"
#include "Globals.hpp"
#include "ProcessManager.hpp"

#include <iostream>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <string>
#include <ctime>
#include <atomic>

Scheduler::Scheduler(std::string scheduler_algo, int delays_per_exec, int n_cpu, int quantum_cycle, Clock* cpu_clock, IMemoryAllocator* memory_allocator)
    : is_running(false), active_threads_(0), ready_threads(0), scheduler_algorithm(scheduler_algo), delay_per_execution(delays_per_exec),
    cpu_count(n_cpu), quantum_cycle(quantum_cycle), cpu_clock(cpu_clock), memory_allocator_(memory_allocator)
{
}

void Scheduler::addProcess(std::shared_ptr<Process> process)
{
    if (!memory_log_)
    {
        startMemoryLog();
    }

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

void Scheduler::startMemoryLog()
{
    memory_log_ = true;
    memory_logging_thread_ = std::thread([this]()
        {
            std::unique_lock<std::mutex> lock(cpu_clock->getMutex());

            while (is_running)
            {
                // Wait for CPU clock tick increment
                cpu_clock->getCondition().wait(lock);

                bool any_core_active = false;

                // Check if any core is active
                for (int i = 1; i <= cpu_count; ++i)
                {
                    if (CoreStateManager::getInstance().getCoreState(i))
                    {
                        any_core_active = true;
                        break;
                    }
                }

                // Increment active CPU count if at least one core is active
                if (any_core_active)
                {
                    cpu_clock->incrementActiveCpuNum();
                }
            }
        });
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

            void* memory = process->getMemory();

            if (memory)
            {
                process->setAllocTime();
                process->setMemory(memory);
            }

            if (!memory)
            {
                memory = memory_allocator_->allocate(process);

                /* ========== REPLACEMENT START ========== */
                if (!memory) {
                    /* Couldn’t fit – push back to tail of queue, mark READY, and continue */
                    process->setState(Process::READY);
                    {
                        std::lock_guard<std::mutex> qlock(queue_mutex_);
                        process_queue_.push(process);
                    }
                    {
                        std::lock_guard<std::mutex> lock(active_threads_mutex_);
                        active_threads_--;
                    }
                    CoreStateManager::getInstance().setCoreState(core_id, false, "");
                    continue;                       // give the core a new job
                }
                process->setAllocTime();
                process->setMemory(memory);
                /* ==========  REPLACEMENT END  ========== */
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
                memory_allocator_->deallocate(process);
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
    static std::atomic<int> snapshot_ctr{ 0 };

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

            void* memory = process->getMemory();

            if (!memory)
            {
                memory = memory_allocator_->allocate(process);

                if (memory)
                {
                    process->setAllocTime();
                    process->setMemory(memory);
                }

                if (!memory) {
                    process->setState(Process::READY);
                    {
                        std::lock_guard<std::mutex> qlock(queue_mutex_);
                        process_queue_.push(process);   // tail of RR queue
                    }
                    {
                        std::lock_guard<std::mutex> lock(active_threads_mutex_);
                        active_threads_--;
                    }
                    CoreStateManager::getInstance().setCoreState(core_id, false, "");
                    queue_condition_.notify_one();      // wake another worker
                    continue;
                }
                process->setAllocTime();
                process->setMemory(memory);
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

                    if (quantum >= quantum_cycle)
                    {
                        logMemoryState(snapshot_ctr.fetch_add(1));  // memory_stamp_<qq>.txt
                    }
                }
            }

            quantum = 0;

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
                memory_allocator_->deallocate(process);
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

void Scheduler::logMemoryState(int cycle)
{
    std::string filename = "memory_stamp_" + std::to_string(cycle) + ".txt";


    //std::filesystem::path filepath = std::filesystem::current_path() / filename;

    std::filesystem::create_directories("outputs/memory-stamps"); // ensures folder exists

    std::filesystem::path filepath = std::filesystem::current_path()
        / "outputs" / "memory-stamps" / filename;



    std::ofstream out_file(filepath);

    if (out_file.is_open())
    {
        std::time_t current_time = std::time(nullptr);
        std::tm current_time_tm;

        if (localtime_s(&current_time_tm, &current_time) == 0)
        {
            char timestamp[100];
            std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &current_time_tm);
            out_file << "Timestamp: (" << timestamp << ")" << std::endl;
        }
        else
        {
            out_file << "Timestamp: (Error formatting time)" << std::endl;
        }

        out_file << "Number of processes in memory: " << memory_allocator_->getNProcess() << std::endl;
        out_file << "Total external fragmentation in KB: " << memory_allocator_->getExternalFragmentation() << std::endl;
        out_file << "\n----end---- = " << memory_allocator_->getMaxMemory() << std::endl << std::endl;

        auto process_list = memory_allocator_->getProcessList();
        for (auto it = process_list.rbegin(); it != process_list.rend(); ++it)
        {
            size_t index = it->first;
            std::shared_ptr<Process> process = it->second;

            size_t size = process->getMemoryRequired();
            const std::string& proc_name = process->getName();
            size_t lowerbound = index - size + 1;
            out_file << "Upper bound: " << index << std::endl;
            out_file << "Process Name: " << proc_name << std::endl;
            out_file << "Lower bound: " << lowerbound << " KB" << std::endl << std::endl;
        }

        out_file << "----start---- = 0" << std::endl;

        out_file.close();
    }
    else
    {
        std::cerr << "Error: Unable to open the file for writing: " << filename << std::endl;
    }
}