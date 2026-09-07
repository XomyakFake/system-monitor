#include "network_monitor.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>


namespace{
    std::vector<std::string> split(const std::string& str){
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream iss(str);
        while(iss >> token){
            tokens.push_back(token);
        }
        return tokens;
    }

    bool isIgnoredInterface(const std::string& name){
        return name == "lo";
    }

    std::vector<NetworkMonitor::InterfaceTraffic> parseInterfaceStats(){
        std::ifstream file("/proc/net/dev");
        if(!file.is_open()){
            throw std::runtime_error("Cannot open /proc/net/dev");
        }
        std::vector<NetworkMonitor::InterfaceTraffic> interfaces;
        std::string line;
        while(std::getline(file, line)){
            const std::size_t colonPos = line.find(':');
                if (colonPos == std::string::npos) {
                    continue; 
                }

                std::string name = line.substr(0, colonPos);
                name.erase(0, name.find_first_not_of(" \t\r\n"));
                name.erase(name.find_last_not_of(" \t\r\n") + 1);

                if(isIgnoredInterface(name)){
                    continue;
                }

                std::string stat = line.substr(colonPos + 1);
                const auto tokens = split(stat);
                if(tokens.size() < 16){
                    continue;
                }
                interfaces.push_back({name, std::stoull(tokens[0]), std::stoull(tokens[8])});
        }
        return interfaces;
    }
}

std::vector<NetworkMonitor::InterfaceTraffic> NetworkMonitor::readInterfaces() const {
    return parseInterfaceStats();
}

NetworkMonitor::NetworkRate NetworkMonitor::measureRate(std::chrono::milliseconds interval) const{
    const auto before = readInterfaces();
    std::this_thread::sleep_for(interval);
    const auto after = readInterfaces();

    std::uint64_t recvDelta = 0;
    std::uint64_t sendDelta = 0;

    NetworkRate rate{};
    for(const auto& net : before){
        for(const auto& nett : after){
            if(net.name == nett.name){
                if (nett.bytesRe > net.bytesRe) {
                    recvDelta += (nett.bytesRe - net.bytesRe);
                }
                if (nett.bytesTr > net.bytesTr) {
                    sendDelta += (nett.bytesTr - net.bytesTr);
                }
            }
        }
    }
    const double seconds = static_cast<double>(interval.count()) / 1000.0;

    rate.speedBRe = static_cast<std::uint64_t>(recvDelta / seconds);
    rate.speedBTr = static_cast<std::uint64_t>(sendDelta / seconds);
    rate.speedMbRe = static_cast<double>(rate.speedBRe) / (1024.0 * 1024.0);
    rate.speedMbTr = static_cast<double>(rate.speedBTr) / (1024.0 * 1024.0);

    return rate;
}


