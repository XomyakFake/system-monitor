#include "cpu_monitor.hpp"
#include "memory_monitor.hpp"
#include "network_monitor.hpp"
#include "process_monitor.hpp"

#include <iomanip>
#include <iostream>

int main() {
    CpuMonitor cpuMonitor;
    MemoryMonitor memoryMonitor;
    NetworkMonitor networkMonitor;
    ProcessMonitor processMonitor;

    const double cpuUsage = cpuMonitor.usagePercent();
    const MemoryUsage memoryUsage = memoryMonitor.read();
    const auto networkRate = networkMonitor.measureRate();
    const auto topProcesses = processMonitor.getTopProcess();

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "CPU usage: " << cpuUsage << "%\n";
    std::cout << "Memory total: " << memoryUsage.total / (1024.0 * 1024.0) << " MiB\n";
    std::cout << "Memory available: " << memoryUsage.available / (1024.0 * 1024.0) << " MiB\n";
    std::cout << "Memory used: " << memoryUsage.used / (1024.0 * 1024.0) << " MiB\n";
    std::cout << "Network received: " << networkRate.speedMbRe << " MB/s\n";
    std::cout << "Network transmitted: " << networkRate.speedMbTr << " MB/s\n";
    std::cout << "Top processes by CPU:\n";
    for (const auto& process : topProcesses) {
        std::cout << process.pid << ' ' << process.name << " CPU: " << process.cpu_percent << "%" << " RSS: " << process.vm_rss_kb << " kB\n";
    }

    return 0;
}