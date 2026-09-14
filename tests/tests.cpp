#include "MemoryAllocator.hpp"

#include <cassert>
#include <chrono>
#include <iostream>
#include <vector>

// ============================================================
// TEST SUITE
// ============================================================

// Verify that a simple allocation within capacity returns a non-null pointer.
void test_basic_allocation() {
    MemoryAllocator allocator(1000, AllocationStrategy::FIRST_FIT);

    void* ptr = allocator.allocate(100);

    assert(ptr != nullptr);

    std::cout << "[PASS] Basic allocation\n";
}

// Verify that sequential allocations return distinct, non-null addresses.
void test_multiple_allocations() {
    MemoryAllocator allocator(1000, AllocationStrategy::FIRST_FIT);

    void* ptr1 = allocator.allocate(100);
    void* ptr2 = allocator.allocate(200);

    assert(ptr1 != nullptr);
    assert(ptr2 != nullptr);
    assert(ptr1 != ptr2);

    std::cout << "[PASS] Multiple allocations\n";
}

// Verify that requesting more memory than total pool capacity fails safely by returning nullptr.
void test_oversized_allocation() {
    MemoryAllocator allocator(1000, AllocationStrategy::FIRST_FIT);

    // Requesting 1001 bytes from a 1000-byte allocator
    void* ptr = allocator.allocate(1001);

    assert(ptr == nullptr);

    std::cout << "[PASS] Oversized allocation\n";
}

// Verify that requesting 0 bytes returns nullptr.
void test_zero_byte_allocation() {
    MemoryAllocator allocator(1000, AllocationStrategy::FIRST_FIT);

    void* ptr = allocator.allocate(0);

    assert(ptr == nullptr);

    std::cout << "[PASS] Zero-byte allocation\n";
}

// Verify that freeing a block allows new allocations to succeed.
void test_deallocation() {
    MemoryAllocator allocator(1000, AllocationStrategy::FIRST_FIT);

    void* ptr = allocator.allocate(100);
    allocator.deallocate(ptr);

    // Allocating again should succeed after deallocation
    void* new_ptr = allocator.allocate(100);

    assert(new_ptr != nullptr);

    std::cout << "[PASS] Deallocation\n";
}

// Verify that the allocator recycles previously freed memory addresses.
void test_memory_reuse() {
    MemoryAllocator allocator(1000, AllocationStrategy::FIRST_FIT);

    void* ptr = allocator.allocate(100);
    allocator.deallocate(ptr);

    // The allocator should reuse the exact block that was just freed
    void* new_ptr = allocator.allocate(100);

    assert(new_ptr == ptr);

    std::cout << "[PASS] Memory reuse\n";
}

// Verify adjacent freed blocks are merged back into a single contiguous block.
void test_coalescing() {
    MemoryAllocator allocator(1000, AllocationStrategy::FIRST_FIT);

    void* ptr1 = allocator.allocate(300);
    void* ptr2 = allocator.allocate(300);
    void* ptr3 = allocator.allocate(400);

    // Free all three blocks in arbitrary order
    allocator.deallocate(ptr1);
    allocator.deallocate(ptr3);
    allocator.deallocate(ptr2);

    // Coalescing should merge them back into a single 1000-byte block
    void* ptr4 = allocator.allocate(1000);

    assert(ptr4 != nullptr);

    std::cout << "[PASS] Coalescing\n";
}

// Verify allocations fail when free memory is fragmented into non-contiguous chunks.
void test_fragmentation() {
    MemoryAllocator allocator(1000, AllocationStrategy::FIRST_FIT);

    void* ptr1 = allocator.allocate(200);
    void* ptr2 = allocator.allocate(200);
    void* ptr3 = allocator.allocate(200);
    void* ptr4 = allocator.allocate(200);

    // Free non-adjacent blocks 1 and 3 to create fragmented gaps
    allocator.deallocate(ptr1);
    allocator.deallocate(ptr3);

    // Total free space is 600 bytes, but max contiguous block is only 200 bytes.
    // A 500-byte allocation should fail.
    void* ptr5 = allocator.allocate(500);

    assert(ptr5 == nullptr);

    std::cout << "[PASS] Fragmentation handling\n";
}

// Verify that attempting to free the same block twice is handled safely.
void test_double_free() {
    MemoryAllocator allocator(1000, AllocationStrategy::FIRST_FIT);

    void* ptr = allocator.allocate(100);

    allocator.deallocate(ptr);
    allocator.deallocate(ptr); // Deliberate duplicate deallocation

    // Allocator state should remain robust
    void* new_ptr = allocator.allocate(100);

    assert(new_ptr != nullptr);

    std::cout << "[PASS] Double-free protection\n";
}

// Verify that passing an unmanaged stack pointer to deallocate does not crash the allocator.
void test_invalid_pointer() {
    MemoryAllocator allocator(1000, AllocationStrategy::FIRST_FIT);

    int stack_value = 42;

    // Pass an external pointer that was not returned by allocator.allocate()
    allocator.deallocate(&stack_value);

    // Allocator state should remain robust
    void* ptr = allocator.allocate(100);

    assert(ptr != nullptr);

    std::cout << "[PASS] Invalid pointer protection\n";
}

// Verify that BEST_FIT selects the smallest fitting free block rather than the first one.
void test_best_fit() {
    MemoryAllocator allocator(1000, AllocationStrategy::BEST_FIT);

    // Create layout: [100 USED][300 USED][200 USED][400 USED]
    void* ptr1 = allocator.allocate(100);
    void* ptr2 = allocator.allocate(300);
    void* ptr3 = allocator.allocate(200);
    void* ptr4 = allocator.allocate(400);

    // Create non-adjacent free slots: [100 USED][300 FREE][200 USED][400 FREE]
    allocator.deallocate(ptr2);
    allocator.deallocate(ptr4);

    // A 250-byte request fits both 300 and 400.
    // BEST_FIT must choose the 300-byte block (ptr2).
    void* ptr5 = allocator.allocate(250);

    assert(ptr5 == ptr2);

    std::cout << "[PASS] Best-fit allocation\n";
}

// Verify behavioral differences between FIRST_FIT and BEST_FIT strategies on identical layouts.
void test_strategy_comparison() {
    MemoryAllocator first_fit(1200, AllocationStrategy::FIRST_FIT);
    MemoryAllocator best_fit(1200, AllocationStrategy::BEST_FIT);

    // Create layout: [100][200][400][200][300]
    void* ff1 = first_fit.allocate(100);
    void* ff2 = first_fit.allocate(200);
    void* ff3 = first_fit.allocate(400);
    void* ff4 = first_fit.allocate(200);
    void* ff5 = first_fit.allocate(300);

    void* bf1 = best_fit.allocate(100);
    void* bf2 = best_fit.allocate(200);
    void* bf3 = best_fit.allocate(400);
    void* bf4 = best_fit.allocate(200);
    void* bf5 = best_fit.allocate(300);

    // Free the 400-byte and 300-byte blocks
    first_fit.deallocate(ff3);
    first_fit.deallocate(ff5);

    best_fit.deallocate(bf3);
    best_fit.deallocate(bf5);

    // Request 250 bytes:
    // First-Fit picks the 400-byte block (first suitable block encountered)
    // Best-Fit picks the 300-byte block (smallest suitable block)
    void* ff6 = first_fit.allocate(250);
    void* bf6 = best_fit.allocate(250);

    assert(ff6 == ff3);
    assert(bf6 == bf5);

    std::cout << "[PASS] Strategy comparison (First-Fit vs Best-Fit)\n";
}

// Measure performance execution time for thousands of allocations/deallocations.
void benchmark_strategy(AllocationStrategy strategy) {
    const int number_of_allocations = 10000;
    MemoryAllocator allocator(1000000, strategy);
    std::vector<void*> pointers;
    pointers.reserve(number_of_allocations);

    auto start = std::chrono::high_resolution_clock::now();

    // Perform mass allocations
    for (int i = 0; i < number_of_allocations; ++i) {
        void* ptr = allocator.allocate(50);
        if (ptr != nullptr) {
            pointers.push_back(ptr);
        }
    }

    // Free all allocated memory
    for (void* ptr : pointers) {
        allocator.deallocate(ptr);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    if (strategy == AllocationStrategy::FIRST_FIT) {
        std::cout << "First-fit benchmark: " << duration.count() << " microseconds\n";
    } else {
        std::cout << "Best-fit benchmark:  " << duration.count() << " microseconds\n";
    }
}

// ============================================================
// MAIN ENTRY POINT
// ============================================================

int main() {
    std::cout << "========== RUNNING ALLOCATOR TESTS ==========\n\n";

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
    test_strategy_comparison();

    std::cout << "\nAll unit tests passed successfully!\n";

    std::cout << "\n========== RUNNING BENCHMARKS ==========\n\n";
    benchmark_strategy(AllocationStrategy::FIRST_FIT);
    benchmark_strategy(AllocationStrategy::BEST_FIT);

    return 0;
}