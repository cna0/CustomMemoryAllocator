#include "MemoryAllocator.hpp"

#include <iostream>
#include <cassert>

void test_basic_allocation() {
    MemoryAllocator allocator(1024, AllocationStrategy::FIRST_FIT);

    void* ptr = allocator.allocate(100);

    assert(ptr != nullptr);
}

int main() {
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

    std::cout << "All tests passed.\n";
}