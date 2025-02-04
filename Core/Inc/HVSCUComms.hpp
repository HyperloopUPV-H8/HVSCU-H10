#pragma once

#include "ST-LIB.hpp"

namespace HVSCUComms {
    const uint16_t CS_PORT = 50500;
    const std::string HVSCU_IP = "192.168.1.9";

    ServerSocket* control_station;

    void start();
};