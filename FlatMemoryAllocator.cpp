#include "FlatMemoryAllocator.hpp"
#include "Process.hpp"
#include <iostream>
#include <fstream>
#include <ctime>
#include <limits>
#include <cstring>
#include <chrono>
#include <iomanip>
#include <memory>

FlatMemoryAllocator::FlatMemoryAllocator(size_t maximum_size, size_t mem_per_frame)
    : maximum_size(maximum_size), allocated_size(0), memory(maximum_size, '.'),
    allocation_map(maximum_size, false), mem_per_frame(mem_per_frame), n_process(0)
{
    initializeMemory();
}

FlatMemoryAllocator::~FlatMemoryAllocator()
{
    std::lock_guard<std::mutex> lock(memory_mutex);
    memory.clear();
    allocation_map.clear();
}

void* FlatMemoryAllocator::allocate(std::shared_ptr<Process> process)
{
    size_t size = process->getMemoryRequired();

    std::lock_guard<std::mutex> lock(memory_mutex);

    for (auto it = free_blocks.begin(); it != free_blocks.end(); ++it)
    {
        size_t block_start = it->first;
        size_t block_size = it->second;

        if (block_size >= size)
        {
            allocateAt(block_start, size);
            n_process++;
            process_list[block_start] = process;
            return reinterpret_cast<void*>(&memory[block_start]);
        }
    }
    return nullptr;
}

void FlatMemoryAllocator::deallocate(std::shared_ptr<Process> process)
{
    std::lock_guard<std::mutex> lock(memory_mutex);

    size_t index = static_cast<char*>(process->getMemory()) - &memory[0];
    if (index < maximum_size && process_list.count(index))
    {
        size_t size = process->getMemoryRequired();
        deallocateAt(index, size);
        process_list.erase(index);
        n_process--;
    }
}

void FlatMemoryAllocator::visualizeMemory()
{
    std::lock_guard<std::mutex> lock(memory_mutex);

    for (char cell : memory)
    {
        std::cout << cell;
    }
    std::cout << std::endl;
}

void FlatMemoryAllocator::initializeMemory()
{
    std::lock_guard<std::mutex> lock(memory_mutex);
    std::fill(memory.begin(), memory.end(), '.');
    std::fill(allocation_map.begin(), allocation_map.end(), false);
    free_blocks.clear();
    free_blocks[0] = maximum_size;
}

bool FlatMemoryAllocator::canAllocateAt(size_t index, size_t size) const
{
    auto it = free_blocks.lower_bound(index);
    if (it == free_blocks.end() || it->first > index || index + size > it->first + it->second)
    {
        return false;
    }
    return true;
}

void FlatMemoryAllocator::allocateAt(size_t index, size_t size)
{
    auto it = free_blocks.lower_bound(index);
    if (it != free_blocks.end() && it->first <= index && it->first + it->second >= index + size)
    {
        size_t block_start = it->first;
        size_t block_size = it->second;
        free_blocks.erase(it);

        if (block_start < index)
        {
            free_blocks[block_start] = index - block_start;
        }
        if (index + size < block_start + block_size)
        {
            free_blocks[index + size] = (block_start + block_size) - (index + size);
        }

        allocated_size += size;
    }
}

void FlatMemoryAllocator::deallocateAt(size_t index, size_t size)
{
    auto next = free_blocks.lower_bound(index);
    auto prev = (next == free_blocks.begin()) ? free_blocks.end() : std::prev(next);

    size_t new_start = index;
    size_t new_size = size;

    if (prev != free_blocks.end() && prev->first + prev->second == index)
    {
        new_start = prev->first;
        new_size += prev->second;
        free_blocks.erase(prev);
    }

    if (next != free_blocks.end() && index + size == next->first)
    {
        new_size += next->second;
        free_blocks.erase(next);
    }

    free_blocks[new_start] = new_size;
    allocated_size -= size;
}

int FlatMemoryAllocator::getNProcess()
{
    std::lock_guard<std::mutex> lock(memory_mutex);
    return n_process;
}

std::map<size_t, std::shared_ptr<Process>> FlatMemoryAllocator::getProcessList()
{
    std::lock_guard<std::mutex> lock(memory_mutex);
    return process_list;
}

size_t FlatMemoryAllocator::getMaxMemory()
{
    std::lock_guard<std::mutex> lock(memory_mutex);
    return maximum_size;
}

size_t FlatMemoryAllocator::getExternalFragmentation()
{
    std::lock_guard<std::mutex> lock(memory_mutex);
    size_t total_free_space = 0;
    size_t current_free_block_size = 0;

    for (size_t i = 0; i < maximum_size; ++i)
    {
        if (!allocation_map[i])
        {
            current_free_block_size++;
        }
        else
        {
            if (current_free_block_size > 0)
            {
                total_free_space += current_free_block_size;
                current_free_block_size = 0;
            }
        }
    }

    if (current_free_block_size > 0)
    {
        total_free_space += current_free_block_size;
    }

    return total_free_space;
}

void FlatMemoryAllocator::deallocateOldest(size_t mem_size)
{
    std::chrono::time_point<std::chrono::system_clock> oldest_time = std::chrono::time_point<std::chrono::system_clock>::max();
    size_t oldest_index = 0;
    std::shared_ptr<Process> oldest_process = nullptr;

    for (const auto& pair : process_list)
    {
        size_t index = pair.first;
        std::shared_ptr<Process> process = pair.second;

        std::chrono::time_point<std::chrono::system_clock> alloc_time = process->getAllocTime();

        if (alloc_time < oldest_time)
        {
            oldest_time = alloc_time;
            oldest_index = index;
            oldest_process = process;
        }
    }

    if (oldest_process)
    {
        while (oldest_process->getState() == Process::ProcessState::RUNNING)
        {
            
        }

        std::ofstream backing_store("backingstore.txt", std::ios::app);

        if (backing_store.is_open())
        {
            std::time_t alloc_time_t = std::chrono::system_clock::to_time_t(oldest_process->getAllocTime());
            std::tm alloc_time_tm;

            if (localtime_s(&alloc_time_tm, &alloc_time_t) == 0)
            {
                backing_store << "Process ID: " << oldest_process->getPID();
                backing_store << "  Name: " << oldest_process->getName();
                backing_store << "  Command Counter: " << oldest_process->getCommandCounter()
                    << "/" << oldest_process->getLinesOfCode() << "\n";
                backing_store << "Memory Size: " << oldest_process->getMemoryRequired() << " KB\n";
                backing_store << "Num Pages: " << oldest_process->getNumPages() << "\n";
                backing_store << "============================================================================\n";

                backing_store.close();
            }
            else
            {
                std::cerr << "Failed to convert time to local time format." << std::endl;
            }
        }

        if (oldest_process->getState() != Process::ProcessState::FINISHED)
        {
            deallocate(oldest_process);
            oldest_process->setMemory(nullptr);
        }
    }
    else
    {
        std::cerr << "No process found to deallocate.\n";
    }
}

size_t FlatMemoryAllocator::getPageIn()
{
    return 0;
}

size_t FlatMemoryAllocator::getPageOut()
{
    return 0;
}