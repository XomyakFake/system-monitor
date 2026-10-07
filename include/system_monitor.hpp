#pragma once

#include "system_snapshot.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

class SystemMonitor {
public:
    explicit SystemMonitor(std::chrono::milliseconds interval = std::chrono::milliseconds(500));
    ~SystemMonitor();

    SystemMonitor(const SystemMonitor&) = delete;
    SystemMonitor& operator=(const SystemMonitor&) = delete;

    void start();
    void stop();
    SystemSnapshot snapshot() const;

private:
    void run();
    SystemSnapshot collect();

    const std::chrono::milliseconds interval_;
    std::atomic<bool> running_{false};
    std::thread worker_;
    mutable std::mutex mutex_;
    std::condition_variable stop_condition_;
    SystemSnapshot last_snapshot_{};
    CpuMonitor cpu_monitor_;
    MemoryMonitor memory_monitor_;
    NetworkMonitor network_monitor_;
    ProcessMonitor process_monitor_;
};