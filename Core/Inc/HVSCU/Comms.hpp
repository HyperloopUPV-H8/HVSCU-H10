#pragma once

#include "ST-LIB.hpp"
#include "HVSCU/HVSCUOrder.hpp"

class Comms {
   private:
    static const uint16_t CLOSE_CONTACTORS_ID = 900;
    static const uint16_t OPEN_CONTACTORS_ID = 901;
    static const uint16_t SDC_OBCCU_ID = 902;
    static const uint16_t CONTROL_STATION_PORT = 50500;
    static std::vector<HVSCUOrder> orders;

   public:
    static constexpr std::string HVSCU_IP = "192.168.1.9";
    static ServerSocket* control_station;

    static void start();
};