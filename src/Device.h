#pragma once

#include <string>
#include <chrono>

class Device
{
public:
    std::string mac;
    std::chrono::system_clock::time_point last_heard;

    Device(std::string m, std::chrono::system_clock::time_point l) : mac(m), last_heard(l) {}
    Device(const Device &) = delete;
    Device& operator=(const Device&) = delete;
    Device(Device &&) = delete;
    Device& operator=(Device&&) = delete;
};