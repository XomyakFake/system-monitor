#include "system_monitor.hpp"

#include <utility>

SystemMonitor::SystemMonitor(std::chrono::milliseconds interval)
    : interval_(interval) {}

SystemMonitor::~SystemMonitor() {
    stop();
}

void SystemMonitor::start() {
    if (running_.exchange(true)) {
        return;
    }

    worker_ = std::thread(&SystemMonitor::run, this);
}

void SystemMonitor::stop() {
    running_ = false;
    stop_condition_.notify_all();

    if (worker_.joinable()) {
        worker_.join();
    }
}

SystemSnapshot SystemMonitor::snapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return last_snapshot_;
}

void SystemMonitor::run() {
    while (running_) {
        SystemSnapshot next = collect();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            last_snapshot_ = std::move(next);
        }

        std::unique_lock<std::mutex> lock(mutex_);
        stop_condition_.wait_for(lock, interval_, [this] {
            return !running_;
        });
    }
}

SystemSnapshot SystemMonitor::collect() {
    SystemSnapshot snapshot{};
    snapshot.cpu = cpu_monitor_.read();
    snapshot.cpu_usage_percent = cpu_monitor_.usagePercent(interval_);
    snapshot.memory = memory_monitor_.read();
    snapshot.network = network_monitor_.measureRate(interval_);
    snapshot.processes = process_monitor_.getTopProcess(10, interval_);
    snapshot.valid = true;
    return snapshot;
}