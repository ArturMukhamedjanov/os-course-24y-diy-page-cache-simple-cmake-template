cmake_minimum_required(VERSION 3.10)
project(BlockCache LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)
add_subdirectory(src)

# Либа lab2
add_library(lab2 SHARED)
target_sources(lab2
    PRIVATE
        src/cache.cpp
        src/file_api.cpp
)
target_include_directories(lab2 PUBLIC include)
target_link_libraries(lab2 pthread)

# Основной main
add_executable(main_app src/main.cpp)
target_link_libraries(main_app lab2)

# Бенчмарк
add_executable(benchmark_app src/benchmark.cpp)
target_link_libraries(benchmark_app lab2)
