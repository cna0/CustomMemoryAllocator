#pragma once

#include <cstddef>

struct Block {
    std::size_t size;
    bool is_free;
    Block* next;
};

enum class AllocationStrategy {
    FIRST_FIT,
    BEST_FIT
};

class MemoryAllocator {
private:
    char* memory;
    Block* first_block;
    AllocationStrategy strategy;

    Block* find_first_fit(std::size_t size);
    Block* find_best_fit(std::size_t size);

public:
    MemoryAllocator(std::size_t size, AllocationStrategy strategy);
    ~MemoryAllocator();

    void* allocate(std::size_t size);
    void deallocate(void* ptr);
    void print_state();
};