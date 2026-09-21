#include "process_monitor.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace {

struct PrevCpuTimes {
    std::uint64_t utime{0};
    std::uint64_t stime{0};
};

struct ProcessSample {
    ProcessInfo info;
    PrevCpuTimes cpu;
};

bool isPidDirectory(const std::filesystem::directory_entry& entry) {
    if (!entry.is_directory()) {
        return false;
    }

    const std::string name = entry.path().filename().string();
    if (name.empty()) {
        return false;
    }

    for (char c : name) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }

    return true;
}


bool readProcessStatus(const std::filesystem::path& processPath, ProcessInfo& info) {
    std::ifstream file(processPath / "status");
    if (!file.is_open()) {
        return false;
    }

    bool hasName = false;
    bool hasRss = false;
    std::string line;
    while (std::getline(file, line)) {
        if (line.rfind("Name:", 0) == 0) {
            std::istringstream parser(line.substr(5));
            parser >> info.name;
            hasName = !info.name.empty();
        } else if (line.rfind("VmRSS:", 0) == 0) {
            std::istringstream parser(line.substr(6));
            parser >> info.vm_rss_kb;
            hasRss = static_cast<bool>(parser);
        }
    }

    return hasName && hasRss;
}

bool readProcessCpu(const std::filesystem::path& processPath, PrevCpuTimes& cpu) {
    std::ifstream file(processPath / "stat");
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    if (!std::getline(file, line)) {
        return false;
    }

    const std::size_t closingParenthesis = line.rfind(')');
    if (closingParenthesis == std::string::npos) {
        return false;
    }

    std::istringstream parser(line.substr(closingParenthesis + 2));
    char state = 0;
    parser >> state;
    if (!parser) {
        return false;
    }

    std::vector<std::uint64_t> fields;
    std::uint64_t value = 0;
    while (parser >> value) {
        fields.push_back(value);
    }

    if (fields.size() < 2) {
        return false;
    }

    cpu.utime = fields[0];
    cpu.stime = fields[1];
    return true;
}

std::uint64_t readTotalCpuTicks() {
    std::ifstream file("/proc/stat");
    if (!file.is_open()) {
        return 0;
    }

    std::string line;
    if (!std::getline(file, line) || line.rfind("cpu ", 0) != 0) {
        return 0;
    }

    std::istringstream parser(line);
    std::string label;
    parser >> label;

    std::uint64_t total = 0;
    std::uint64_t value = 0;
    while (parser >> value) {
        total += value;
    }
    return total;
}

std::vector<ProcessSample> readProcessSamples() {
    std::vector<ProcessSample> samples;
    for (const auto& entry : std::filesystem::directory_iterator("/proc")) {
        if (!isPidDirectory(entry)) {
            continue;
        }

        ProcessSample sample;
        sample.info.pid = std::stoi(entry.path().filename().string());
        if (readProcessStatus(entry.path(), sample.info) && readProcessCpu(entry.path(), sample.cpu)) {
            samples.push_back(sample);
        }
    }
    return samples;
}

} 

std::vector<ProcessInfo> ProcessMonitor::getTopProcess(
    std::size_t top_count, std::chrono::milliseconds interval) {
    if (top_count == 0 || interval.count() <= 0) {
        return {};
    }

    const std::vector<ProcessSample> first = readProcessSamples();
    const std::uint64_t totalTicks1 = readTotalCpuTicks();

    std::this_thread::sleep_for(interval);

    const std::vector<ProcessSample> second = readProcessSamples();
    const std::uint64_t totalTicks2 = readTotalCpuTicks();
    const std::uint64_t totalDelta = totalTicks2 > totalTicks1 ? totalTicks2 - totalTicks1 : 0;

    std::unordered_map<int, PrevCpuTimes> previous;
    for (const auto& sample : first) {
        previous[sample.info.pid] = sample.cpu;
    }

    std::vector<ProcessInfo> processes;
    for (const auto& sample : second) {
        ProcessInfo info = sample.info;
        const auto previousIt = previous.find(info.pid);
        if (previousIt != previous.end() && totalDelta > 0) {
            const std::uint64_t processNow = sample.cpu.utime + sample.cpu.stime;
            const std::uint64_t processBefore = previousIt->second.utime + previousIt->second.stime;
            const std::uint64_t processDelta = processNow > processBefore ? processNow - processBefore : 0;
            info.cpu_percent = 100.0 * static_cast<double>(processDelta) / static_cast<double>(totalDelta);
        }
        processes.push_back(info);
    }

    std::sort(processes.begin(), processes.end(), [](const ProcessInfo& firstInfo, const ProcessInfo& secondInfo) {
        return firstInfo.cpu_percent > secondInfo.cpu_percent;
    });

    if (processes.size() > top_count) {
        processes.resize(top_count);
    }
    return processes;
}