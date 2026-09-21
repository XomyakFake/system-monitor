#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

struct ProcessInfo {
    int pid{0};
    std::string name;
    std::uint64_t vm_rss_kb{0};
    double cpu_percent{0.0};
};

class ProcessMonitor {
public:
    std::vector<ProcessInfo> getTopProcess(
        std::size_t top_count = 10,
        std::chrono::milliseconds interval = std::chrono::milliseconds(500));
};