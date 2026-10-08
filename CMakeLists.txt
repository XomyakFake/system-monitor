cmake_minimum_required(VERSION 3.16)
project(system_monitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(system_monitor
    src/main.cpp
    src/main_window.cpp
    src/system_monitor.cpp
    src/cpu_monitor.cpp
    src/memory_monitor.cpp
    src/network_monitor.cpp
    src/process_monitor.cpp
    include/main_window.hpp
    include/system_monitor.hpp
    include/system_snapshot.hpp
)

target_link_libraries(system_monitor PRIVATE Qt6::Widgets Qt6::Charts)
