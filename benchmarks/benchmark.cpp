#include "MemoryAllocator.hpp"
#include <chrono>
#include <iostream>

int main() {
    // Define the pool size (1 MB) and the number of allocation iterations
    constexpr std::size_t pool_size = 1024 * 1024;
    constexpr int iterations = 10000;

    // Instantiate the custom allocator using First-Fit search
    MemoryAllocator allocator(
        pool_size,
        AllocationStrategy::FIRST_FIT
    );

    // Record the start time before running the benchmark loop
    auto start = std::chrono::high_resolution_clock::now();

    // Repeatedly allocate and deallocate memory to measure throughput/overhead
    for (int i = 0; i < iterations; ++i) {
        void* ptr = allocator.allocate(50);

        // Deallocate immediately to test high-frequency alloc/free cycles
        if (ptr != nullptr) {
            allocator.deallocate(ptr);
        }
    }

    // Record the end time after completing all iterations
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate elapsed time in microseconds
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(
            end - start
        );

    // Output total Execution Time
    std::cout << "Time: "
              << duration.count()
              << " microseconds\n";
}