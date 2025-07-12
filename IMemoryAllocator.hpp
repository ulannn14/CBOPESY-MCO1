#ifndef IMEMORY_ALLOCATOR_H
#define IMEMORY_ALLOCATOR_H

#include <vector>
#include <unordered_map>
#include <iostream>
#include <map>
#include <tuple>
#include "Process.hpp"

/**
 * @class IMemoryAllocator
 * @brief Abstract base class for memory allocator interfaces.
 *
 * This class defines the interface for memory allocation, deallocation, visualization,
 * and management functions that any derived memory allocator class must implement.
 */
class IMemoryAllocator
{
public:
    /**
     * @brief Allocate memory for a process.
     * @param process Shared pointer to the process requesting memory allocation.
     * @return A pointer to the allocated memory block.
     */
    virtual void* allocate(std::shared_ptr<Process> process) = 0;

    /**
     * @brief Deallocate memory for a process.
     * @param process Shared pointer to the process whose memory is to be deallocated.
     */
    virtual void deallocate(std::shared_ptr<Process> process) = 0;

    /**
     * @brief Visualize the current state of the memory allocation.
     *
     * This method outputs the current memory map to give a visual representation of memory allocation.
     */
    virtual void visualizeMemory() = 0;

    /**
     * @brief Get the number of processes currently in memory.
     * @return The number of processes currently allocated in memory.
     */
    virtual int getNProcess() = 0;

    /**
     * @brief Get a list of all processes currently allocated in memory.
     * @return A map of starting memory indices to process pointers.
     */
    virtual std::map<size_t, std::shared_ptr<Process>> getProcessList() = 0;

    /**
     * @brief Get the maximum memory capacity managed by the allocator.
     * @return The maximum memory size.
     */
    virtual size_t getMaxMemory() = 0;

    /**
     * @brief Get the amount of external fragmentation in memory.
     * @return The size of external fragmentation in the memory pool.
     */
    virtual size_t getExternalFragmentation() = 0;

    /**
     * @brief Deallocate the oldest process in memory.
     * @param mem_size The size of memory to free.
     *
     * This method deallocates the oldest process to make space for new memory requests.
     */
    virtual void deallocateOldest(size_t mem_size) = 0;

    /**
     * @brief Get the number of page-ins that have occurred.
     * @return The number of page-ins.
     */
    virtual size_t getPageIn() = 0;

    /**
     * @brief Get the number of page-outs that have occurred.
     * @return The number of page-outs.
     */
    virtual size_t getPageOut() = 0;
};

#endif