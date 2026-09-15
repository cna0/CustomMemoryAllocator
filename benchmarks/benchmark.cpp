#include "MemoryAllocator.hpp"

#include <chrono>
#include <iostream>
#include <random>
#include <vector>

// Structure to encapsulate performance and efficiency metrics from a benchmark run
struct BenchmarkResult {
    long long duration_microseconds;   // Total execution time in microseconds
    std::size_t successful_allocations; // Count of allocations that returned a valid pointer
    std::size_t failed_allocations;     // Count of allocations that returned nullptr (pool exhausted or fragmented)
};

/**
 * Executes a simulated workload of random allocation and deallocation operations.
 * 
 * @param strategy  The search policy used by the allocator (FIRST_FIT or BEST_FIT).
 * @param pool_size Total size of the underlying memory buffer in bytes.
 * @param operations Total number of random actions (allocations or deallocations) to execute.
 * @return BenchmarkResult containing time elapsed and success/failure counts.
 */
BenchmarkResult run_benchmark(
    AllocationStrategy strategy,
    std::size_t pool_size,
    int operations
) {
    // Initialize the custom memory allocator instance
    MemoryAllocator allocator(pool_size, strategy);

    // Track active memory addresses so we can pick valid pointers to free later
    std::vector<void*> allocations;

    // Fixed seed (42) guarantees identical, deterministic random operations across strategy runs
    std::mt19937 rng(42);

    // Distribution defining variable block sizes for requests (16 to 512 bytes)
    std::uniform_int_distribution<std::size_t> size_distribution(16, 512);

    // Coin-flip distribution: 0 triggers an allocation, 1 triggers a deallocation
    std::uniform_int_distribution<int> operation_distribution(0, 1);

    std::size_t successful_allocations = 0;
    std::size_t failed_allocations = 0;

    // Start tracking execution time
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < operations; ++i) {

        // Force allocation if no memory is currently held; otherwise roll 50/50
        bool should_allocate =
            allocations.empty() ||
            operation_distribution(rng) == 0;

        if (should_allocate) {
            // Pick a random block size
            std::size_t size = size_distribution(rng);

            // Attempt allocation from custom memory pool
            void* ptr = allocator.allocate(size);

            if (ptr != nullptr) {
                // Record address on success for subsequent deallocation
                allocations.push_back(ptr);
                ++successful_allocations;
            }
            else {
                // Memory exhausted or fragmented into blocks too small for requested size
                ++failed_allocations;
            }
        }
        else {
            // Select a random index corresponding to an active block in our vector
            std::uniform_int_distribution<std::size_t>
                index_distribution(0, allocations.size() - 1);

            std::size_t index = index_distribution(rng);

            // Free the selected block inside the custom allocator
            allocator.deallocate(allocations[index]);

            // Swap-and-pop technique: fast O(1) removal of the pointer from tracking vector
            allocations[index] = allocations.back();
            allocations.pop_back();
        }
    }

    // Stop tracking execution time
    auto end = std::chrono::high_resolution_clock::now();

    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(
            end - start
        );

    return {
        duration.count(),
        successful_allocations,
        failed_allocations
    };
}

int main() {
    constexpr std::size_t pool_size = 1024 * 1024; // 1 MiB pool size
    constexpr int operations = 100000;              // 100,000 total allocation/deallocation actions

    std::cout << "========== ALLOCATOR BENCHMARK ==========\n\n";

    // Run identical workload under First-Fit strategy
    BenchmarkResult first_fit = run_benchmark(
        AllocationStrategy::FIRST_FIT,
        pool_size,
        operations
    );

    // Run identical workload under Best-Fit strategy
    BenchmarkResult best_fit = run_benchmark(
        AllocationStrategy::BEST_FIT,
        pool_size,
        operations
    );

    // Display general environment parameters
    std::cout << "Operations: " << operations << '\n';
    std::cout << "Pool size:  " << pool_size << " bytes\n\n";

    // Display First-Fit metrics
    std::cout << "First-Fit\n";
    std::cout << "---------\n";
    std::cout << "Time:                   " << first_fit.duration_microseconds << " microseconds\n";
    std::cout << "Successful allocations: " << first_fit.successful_allocations << '\n';
    std::cout << "Failed allocations:     " << first_fit.failed_allocations << "\n\n";

    // Display Best-Fit metrics
    std::cout << "Best-Fit\n";
    std::cout << "--------\n";
    std::cout << "Time:                   " << best_fit.duration_microseconds << " microseconds\n";
    std::cout << "Successful allocations: " << best_fit.successful_allocations << '\n';
    std::cout << "Failed allocations:     " << best_fit.failed_allocations << '\n';

    return 0;
}