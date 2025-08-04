#ifndef PAGING_ALLOCATOR_H
#define PAGING_ALLOCATOR_H

#include "IMemoryAllocator.hpp"
#include <vector>
#include <iostream>
#include <mutex>
#include <map>

class PagingAllocator : public IMemoryAllocator
{
public:
    PagingAllocator(size_t maximum_size, size_t mem_per_frame);

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
    size_t num_frames;            
    std::unordered_map<size_t, std::shared_ptr<Process>> frame_map;
    std::vector<size_t> free_frame_list;
    size_t n_paged_in;           
    size_t n_paged_out;           
    size_t mem_per_frame;        
    size_t allocated_size;        
    int n_process; 
    std::mutex memory_mutex; 
    std::map<size_t, std::shared_ptr<Process>> process_list;
    size_t allocateFrames(size_t num_frames, std::shared_ptr<Process> process);
    void deallocateFrames(size_t num_frames, size_t frame_index);
};

#endif
