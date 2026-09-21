#include "memory_monitor.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace {
std::unordered_map<std::string, std::uint64_t> parseMemInfo() {
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open /proc/meminfo");
    }

    std::unordered_map<std::string, std::uint64_t> values;
    std::string line;
    while (std::getline(file, line)) {
        const std::size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, colonPos);
        std::string valuePart = line.substr(colonPos + 1);
        std::istringstream parser(valuePart);

        std::uint64_t value = 0;
        std::string unit;
        parser >> value >> unit;
        if (!parser) {
            continue;
        }

        values[key] = value;
    }

    return values;
}
} 

MemoryUsage MemoryMonitor::read() const {
    try {
        const auto values = parseMemInfo();

        const auto totalIt = values.find("MemTotal");
        const auto availableIt = values.find("MemAvailable");

        if (totalIt == values.end() || availableIt == values.end()) {
            throw std::runtime_error("Cannot find MemTotal or MemAvailable");
        }

        const std::uint64_t totalKb = totalIt->second;
        const std::uint64_t availableKb = availableIt->second;
        const std::uint64_t usedKb = totalKb - availableKb;

        MemoryUsage usage{};
        usage.total = totalKb * 1024ULL;
        usage.available = availableKb * 1024ULL;
        usage.used = usedKb * 1024ULL;
        return usage;
    } catch (const std::exception& ex) {
        std::cerr << "Error reading /proc/meminfo: " << ex.what() << '\n';
        return {};
    }
}
