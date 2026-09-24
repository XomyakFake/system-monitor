#include "system_snapshot.hpp"

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

class SystemMonitor{
    public:
        explicit SystemMonitor(std::chrono::milliseconds interval = std::chrono::milliseconds(500)) : Interval(interval), Running(false) {}

        void start(){
            if(Running){
                return;
            }
            Running = true;
            worker = std::thread(&SystemMonitor::run, this);
        }
        void stop(){
            Running = false;
            if(worker.joinable()){
                worker.join();
            }
        }

        SystemSnapshot snapshot(){
            std::lock_guard<std::mutex> lock(mutex);
        return last_snapshot_;
    }
    private:
        std::chrono::milliseconds Interval;
        std::atomic<bool> Running;
        std::thread worker;
        SystemSnapshot last_snapshot_{};
        mutable std::mutex mutex;
        std::chrono::milliseconds interval;

        void run(){
            while(Running){
                SystemSnapshot next = collect();
                {
                std::lock_guard<std::mutex> lock(mutex);
                last_snapshot_ = next;
                }

                std::this_thread::sleep_for(interval);
            }
        }
        SystemSnapshot collect() const {
            SystemSnapshot snap{};
            CpuMonitor cpu;
            MemoryMonitor memory;
            NetworkMonitor network;
            ProcessMonitor processes;
        
            snap.cpu = cpu.read();
            snap.memory = memory.read();
            snap.network = network.measureRate();
            snap.processes = processes.getTopProcess();

            return snap;
        }
};