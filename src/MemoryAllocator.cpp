#include "MemoryAllocator.hpp"
#include <iostream>

// Constructor: Initializes the memory pool, the initial metadata block,
// and the selected allocation strategy.
MemoryAllocator::MemoryAllocator(
    std::size_t size,
    AllocationStrategy strategy
) {
    // Allocate the contiguous memory buffer used by the allocator.
    memory = new char[size];

    // Create the first metadata block.
    // Initially, the entire memory pool is one large free block.
    first_block = new Block;

    first_block->size = size;
    first_block->is_free = true;
    first_block->next = nullptr;

    // Store the allocation strategy selected by the user.
    this->strategy = strategy;
}

// Destructor: Releases all dynamically allocated memory.
MemoryAllocator::~MemoryAllocator() {
    Block* current = first_block;

    // Traverse the linked list and delete each metadata block.
    while (current != nullptr) {
        Block* next = current->next;

        delete current;

        current = next;
    }

    // Release the main memory pool.
    delete[] memory;
}

// Finds the first free block large enough to satisfy the request.
// This implements the First-Fit allocation strategy.
Block* MemoryAllocator::find_first_fit(std::size_t size) {
    Block* current = first_block;

    // Traverse the block list from the beginning.
    while (current != nullptr) {
        // Return the first free block that is large enough.
        if (current->is_free && current->size >= size) {
            return current;
        }

        current = current->next;
    }

    // No suitable block was found.
    return nullptr;
}

// Finds the smallest free block that can satisfy the request.
// This implements the Best-Fit allocation strategy.
Block* MemoryAllocator::find_best_fit(std::size_t size) {
    Block* current = first_block;
    Block* best_block = nullptr;

    // Traverse the entire block list.
    while (current != nullptr) {
        if (current->is_free && current->size >= size) {

            // Store the first suitable block found.
            if (best_block == nullptr) {
                best_block = current;
            }
            // Replace it if a smaller suitable block is found.
            else if (current->size < best_block->size) {
                best_block = current;
            }
        }

        current = current->next;
    }

    // Return the smallest suitable block, or nullptr if none exists.
    return best_block;
}

// Allocates a block of memory using the selected allocation strategy.
void* MemoryAllocator::allocate(std::size_t size) {

    // Reject zero-byte allocation requests.
    if (size == 0) {
        return nullptr;
    }

    Block* target = nullptr;

    // Select the appropriate allocation strategy.
    if (strategy == AllocationStrategy::FIRST_FIT) {
        target = find_first_fit(size);
    }
    else if (strategy == AllocationStrategy::BEST_FIT) {
        target = find_best_fit(size);
    }

    // Return nullptr if no suitable block was found.
    if (target == nullptr) {
        return nullptr;
    }

    // Calculate the target block's offset within the memory pool.
    Block* current = first_block;
    std::size_t offset = 0;

    while (current != target) {
        offset += current->size;
        current = current->next;
    }

    // If the block is larger than requested, split it into two blocks.
    if (target->size > size) {
        Block* new_block = new Block;

        // The new block contains the remaining unused space.
        new_block->size = target->size - size;
        new_block->is_free = true;
        new_block->next = target->next;

        // Insert the new block after the allocated block.
        target->next = new_block;
    }

    // Update the target block to represent the allocated memory.
    target->size = size;
    target->is_free = false;

    // Return the address of the allocated region.
    return memory + offset;
}

// Deallocates a previously allocated block.
// Also combines adjacent free blocks to reduce fragmentation.
void MemoryAllocator::deallocate(void* ptr) {

    // Ignore null pointers.
    if (ptr == nullptr) {
        return;
    }

    Block* current = first_block;
    std::size_t offset = 0;

    // Traverse the block list to find the block corresponding to ptr.
    while (current != nullptr) {
        char* block_address = memory + offset;

        // Check whether ptr points to the beginning of this block.
        if (ptr == block_address) {

            // Prevent a block from being freed twice.
            if (current->is_free) {
                std::cout << "Warning: Block is already free\n";
                return;
            }

            // Mark the block as free.
            current->is_free = true;

            // ---------------------------------------------------------
            // Coalesce with the next block
            // ---------------------------------------------------------

            // If the next block exists and is also free,
            // combine it with the current block.
            if (current->next != nullptr &&
                current->next->is_free) {

                Block* next = current->next;

                // Increase the current block's size.
                current->size += next->size;

                // Skip over the merged block.
                current->next = next->next;

                // Delete the old metadata block.
                delete next;
            }

            // ---------------------------------------------------------
            // Coalesce with the previous block
            // ---------------------------------------------------------

            // Find the block immediately before the current block.
            Block* previous = nullptr;
            Block* search = first_block;

            while (search != current) {
                previous = search;
                search = search->next;
            }

            // If the previous block is free, merge the current block
            // into it.
            if (previous != nullptr && previous->is_free) {

                // Increase the previous block's size.
                previous->size += current->size;

                // Remove the current block from the linked list.
                previous->next = current->next;

                // Delete the merged metadata block.
                delete current;
            }

            return;
        }

        // Move to the next block and update its offset.
        offset += current->size;
        current = current->next;
    }

    // The pointer did not correspond to a valid block in this allocator.
    std::cout << "Warning: pointer doesn't belong to allocator\n";
}

// Prints the current state of every block in the memory pool.
// Useful for debugging and visualising fragmentation/coalescing.
void MemoryAllocator::print_state() {
    Block* current = first_block;
    int block_number = 0;

    // Traverse and display every block.
    while (current != nullptr) {

        std::cout << "Block " << block_number << '\n';

        std::cout << "   Size: "
                  << current->size
                  << " bytes\n";

        std::cout << "   Free: "
                  << std::boolalpha
                  << current->is_free
                  << '\n';

        std::cout << '\n';

        current = current->next;
        ++block_number;
    }
}