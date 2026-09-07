#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <cstdint>

class NetworkMonitor {
public:
    struct InterfaceTraffic {
    std::string name;
    std::uint64_t bytesRe = 0;
    std::uint64_t bytesTr = 0;

    };

    struct NetworkRate {
        std::uint64_t speedBRe = 0;
        std::uint64_t speedBTr = 0;
        double speedMbRe = 0.0;
        double speedMbTr = 0.0;

    };

    std::vector<InterfaceTraffic> readInterfaces() const;
    NetworkRate measureRate(std::chrono::milliseconds interval = std::chrono::milliseconds(500)) const;
};
