#ifndef FLAT_MEMORY_ALLOCATOR_H
#define FLAT_MEMORY_ALLOCATOR_H

#include <vector>
#include <iostream>
#include "IMemoryAllocator.hpp"
#include <mutex>
#include <map>

/**
 * @class FlatMemoryAllocator
 * @brief Implements a memory allocator that allocates and deallocates memory for processes using a flat memory model.
 */
class FlatMemoryAllocator : public IMemoryAllocator
{
public:
    /**
     * @brief Constructor for FlatMemoryAllocator.
     * @param maximum_size The total size of the memory pool.
     * @param mem_per_frame The size of each memory frame.
     */
    FlatMemoryAllocator(size_t maximum_size, size_t mem_per_frame);

    /**
     * @brief Destructor for FlatMemoryAllocator.
     */
    ~FlatMemoryAllocator();

    /**
     * @brief Allocate memory for a process.
     * @param process Shared pointer to the process requesting memory.
     * @return Pointer to the allocated memory block.
     */
    void* allocate(std::shared_ptr<Process> process) override;

    /**
     * @brief Deallocate memory for a process.
     * @param process Shared pointer to the process whose memory is to be deallocated.
     */
    void deallocate(std::shared_ptr<Process> process) override;

    /**
     * @brief Visualize the current memory allocation state.
     */
    void visualizeMemory() override;

    /**
     * @brief Get the number of processes in memory.
     * @return The number of processes in memory.
     */
    int getNProcess() override;

    /**
     * @brief Get a list of all processes in memory.
     * @return A map of starting memory indices to process pointers.
     */
    std::map<size_t, std::shared_ptr<Process>> getProcessList() override;

    /**
     * @brief Get the maximum memory size of the allocator.
     * @return The maximum memory size.
     */
    size_t getMaxMemory() override;

    /**
     * @brief Get the current amount of external fragmentation.
     * @return The size of external fragmentation.
     */
    size_t getExternalFragmentation() override;

    /**
     * @brief Deallocate the oldest process in memory to make space.
     * @param mem_size The size of memory required to be deallocated.
     */
    void deallocateOldest(size_t mem_size) override;

    /**
     * @brief Get the number of page-ins that occurred.
     * @return The number of page-ins.
     */
    size_t getPageIn() override;

    /**
     * @brief Get the number of page-outs that occurred.
     * @return The number of page-outs.
     */
    size_t getPageOut() override;

private:
    size_t maximum_size;                        ///< Total size of the memory pool.
    size_t mem_per_frame;                       ///< Size of each memory frame.
    size_t allocated_size;                      ///< Currently allocated size.
    std::vector<char> memory;                   ///< Memory pool representation.
    std::vector<bool> allocation_map;           ///< Allocation tracking map.
    int n_process;                              ///< Number of processes in memory.

    std::mutex memory_mutex;                    ///< Mutex for thread-safe memory access.
    std::map<size_t, std::shared_ptr<Process>> process_list; ///< Map of starting memory indices to processes.
    std::map<size_t, size_t> free_blocks;       ///< Map of free memory blocks.

    /**
     * @brief Initializes memory and allocation map.
     */
    void initializeMemory();

    /**
     * @brief Checks if memory can be allocated at an index.
     * @param index The index to check.
     * @param size The size of memory to allocate.
     * @return True if memory can be allocated at the index, false otherwise.
     */
    bool canAllocateAt(size_t index, size_t size) const;

    /**
     * @brief Marks a block of memory as allocated.
     * @param index The starting index of the block to allocate.
     * @param size The size of the block to allocate.
     */
    void allocateAt(size_t index, size_t size);

    /**
     * @brief Frees an allocated block of memory starting at index.
     * @param index The starting index of the block to deallocate.
     * @param size The size of the block to deallocate.
     */
    void deallocateAt(size_t index, size_t size);
};

#endif