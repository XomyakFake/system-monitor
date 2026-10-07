#pragma once

#include "cpu_monitor.hpp"
#include "memory_monitor.hpp"
#include "network_monitor.hpp"
#include "process_monitor.hpp"

#include <vector>

struct SystemSnapshot{
    CpuSample cpu{};
    double cpu_usage_percent = 0.0;
    MemoryUsage memory{};
    NetworkMonitor::NetworkRate network{};
    std::vector<ProcessInfo> processes;
    bool valid = false;
};
