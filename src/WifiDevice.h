#pragma once

#include "device.h"

class WifiDevice : public Device
{
public:
    int rssi;

    WifiDevice(std::string m, std::chrono::system_clock::time_point l, int r) : Device(m, l), rssi(r) {}
    WifiDevice(const WifiDevice &) = delete;
    WifiDevice &operator=(const WifiDevice &) = delete;
    WifiDevice(WifiDevice &&) = delete;
    WifiDevice &operator=(WifiDevice &&) = delete;
};