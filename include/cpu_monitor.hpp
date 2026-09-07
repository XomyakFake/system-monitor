#pragma once

#include <chrono>

struct CpuSample {
    long long total = 0;
    long long idle = 0;
};

class CpuMonitor {
public:
    CpuSample read() const;
    double usagePercent(std::chrono::milliseconds interval = std::chrono::milliseconds(500)) const;
};
