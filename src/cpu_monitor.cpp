#include "cpu_monitor.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

CpuSample CpuMonitor::read() const {
    std::ifstream file("/proc/stat");
    if (!file.is_open()) {
        std::cerr << "Cannot open /proc/stat\n";
        return {-1, -1};
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.rfind("cpu ", 0) == 0) {
            std::istringstream reader(line);
            std::string label;
            long long user = 0;
            long long nice = 0;
            long long system = 0;
            long long idle = 0;
            long long iowait = 0;
            long long irq = 0;
            long long softirq = 0;
            long long steal = 0;
            long long guest = 0;
            long long guest_nice = 0;

            reader >> label >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;

            CpuSample sample;
            sample.total = user + nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice;
            sample.idle = idle + iowait;
            return sample;
        }
    }

    std::cerr << "Cannot find 'cpu' in /proc/stat\n";
    return {-1, -1};
}

double CpuMonitor::usagePercent(std::chrono::milliseconds interval) const {
    CpuSample first = read();
    if (first.total < 0 || first.idle < 0) {
        return 0.0;
    }

    std::this_thread::sleep_for(interval);

    CpuSample second = read();
    long long deltaTotal = second.total - first.total;
    long long deltaIdle = second.idle - first.idle;

    if (deltaTotal <= 0) {
        return 0.0;
    }

    return 100.0 * (1.0 - static_cast<double>(deltaIdle) / static_cast<double>(deltaTotal));
}
