#ifndef FLAT_MEMORY_ALLOCATOR_H
#define FLAT_MEMORY_ALLOCATOR_H

#include <vector>
#include <iostream>
#include "IMemoryAllocator.hpp"
#include <mutex>
#include <map>

class FlatMemoryAllocator : public IMemoryAllocator
{
public:
    FlatMemoryAllocator(size_t maximum_size, size_t mem_per_frame);
    ~FlatMemoryAllocator();
    void* allocate(std::shared_ptr<Process> process) override;
    void deallocate(std::shared_ptr<Process> process) override;
    void visualizeMemory() override;
    int getNProcess() override;
    std::map<size_t, std::shared_ptr<Process>> getProcessList() override;
    size_t getMaxMemory() override;
    size_t getExternalFragmentation() override;
    void deallocateOldest(size_t mem_size) override;
    size_t getPageIn() override;
    size_t getPageOut() override;

private:
    size_t maximum_size;                        
    size_t mem_per_frame;                       
    size_t allocated_size;                      
    std::vector<char> memory;                   
    std::vector<bool> allocation_map;           
    int n_process;                             
    std::mutex memory_mutex;                    
    std::map<size_t, std::shared_ptr<Process>> process_list; 
    std::map<size_t, size_t> free_blocks;
    void initializeMemory();
    bool canAllocateAt(size_t index, size_t size) const;
    void allocateAt(size_t start, size_t size, std::shared_ptr<Process> process);
    void deallocateAt(size_t index, size_t size);
};

#endif