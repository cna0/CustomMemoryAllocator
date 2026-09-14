cmake_minimum_required(VERSION 3.20)

project(CustomMemoryAllocator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(memory_allocator
    src/MemoryAllocator.cpp
)

target_include_directories(memory_allocator
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_executable(tests
    tests/tests.cpp
)

target_link_libraries(tests
    PRIVATE memory_allocator
)

add_executable(benchmark
    benchmarks/benchmark.cpp
)

target_link_libraries(benchmark
    PRIVATE memory_allocator
)