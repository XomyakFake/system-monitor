#pragma once

#include <cstdint>

struct MemoryUsage {
    std::uint64_t total = 0;
    std::uint64_t available = 0;
    std::uint64_t used = 0;
};

class MemoryMonitor {
public:
    MemoryUsage read() const;
};
