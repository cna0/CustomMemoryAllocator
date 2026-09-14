#include "MemoryAllocator.hpp"
#include <iostream>

// Constructor: Initializes the memory pool, initial metadata block, and allocation strategy
MemoryAllocator::MemoryAllocator(
    std::size_t size,
    AllocationStrategy strategy
) {
    // Allocate the contiguous memory buffer
    memory = new char[size];

    // Create the initial free metadata block representing the whole pool
    first_block = new Block;
    first_block->size = size;
    first_block->is_free = true;
    first_block->next = nullptr;

    // Set the chosen allocation strategy (e.g., FIRST_FIT, BEST_FIT)
    this->strategy = strategy;
}

// Destructor: Cleans up dynamically allocated metadata blocks and main memory
MemoryAllocator::~MemoryAllocator() {
    Block* current = first_block;

    // Traverse the linked list of blocks and free each metadata structure
    while (current != nullptr) {
        Block* next = current->next;
        delete current;
        current = next;
    }

    // Deallocate the primary backing memory array
    delete[] memory;
}

// Finds the first available free block that has enough space (First Fit)
Block* MemoryAllocator::find_first_fit(std::size_t size) {
    Block* current = first_block;

    // Traverse from the start and return immediately upon finding a suitable block
    while (current != nullptr) {
        if (current->is_free && current->size >= size) {
            return current; // Return as soon as the first match is found
        }

        current = current->next;
    }

    return nullptr; // No block was large enough
}

// Finds the free block that minimizes wasted space for the requested size (Best Fit)
Block* MemoryAllocator::find_best_fit(std::size_t size) {
    Block* current = first_block;
    Block* best_block = nullptr;

    // Traverse the entire list to find the tightest fitting block
    while (current != nullptr) {
        if (current->is_free && current->size >= size) {
            // Keep track of the smallest suitable block found so far
            if (best_block == nullptr || current->size < best_block->size) {
                best_block = current;
            }
        }

        current = current->next;
    }

    return best_block; // Returns the best block, or nullptr if none fit
}