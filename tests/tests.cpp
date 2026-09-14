#include "MemoryAllocator.hpp"

#include <iostream>
#include <cassert>

// Test basic single allocation functionality
void test_basic_allocation() {
    // Initialize a 1024-byte pool with First Fit strategy
    MemoryAllocator allocator(1024, AllocationStrategy::FIRST_FIT);

    // Request a 100-byte block
    void* ptr = allocator.allocate(100);

    // Verify that the allocation succeeded and returned a valid pointer
    assert(ptr != nullptr);
}

int main() {
    // Run test cases covering primary allocation and edge-case behavior
    test_basic_allocation();
    test_multiple_allocations();
    test_oversized_allocation();
    test_zero_byte_allocation();
    test_deallocation();
    test_memory_reuse();
    test_coalescing();
    test_fragmentation();
    test_double_free();
    test_invalid_pointer();
    test_best_fit();

    // Confirm that all assertions passed successfully
    std::cout << "All tests passed.\n";
}