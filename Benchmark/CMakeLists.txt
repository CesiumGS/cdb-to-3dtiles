project(Benchmark)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(benchmark CONFIG REQUIRED)

set(sources
    "main.cpp")

add_executable(Benchmark ${sources})
target_link_libraries(Benchmark PRIVATE Core
                                        benchmark::benchmark
                                        benchmark::benchmark_main)
