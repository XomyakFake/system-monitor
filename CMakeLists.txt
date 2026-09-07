cmake_minimum_required(VERSION 3.16)
project(system_monitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(system_monitor
    src/main.cpp
    src/cpu_monitor.cpp
    src/memory_monitor.cpp
    src/network_monitor.cpp
)
