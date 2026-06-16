cmake_minimum_required(VERSION 3.10)

project(CpuScheduler C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

add_executable(main
    src/color_codes.c
    src/cpu_scheduling_simulator.c
    src/data_structures.c
    src/input_handler.c
    src/job.c
    src/print_handler.c
    src/scheduling_methods.c
)

target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(main PRIVATE m)