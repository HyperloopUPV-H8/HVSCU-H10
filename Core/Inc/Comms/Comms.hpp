#pragma once

#include "ST-LIB.hpp"

class Comms {
   public:
    enum class IDOrder : uint16_t {
        CLOSE_CONTACTORS_ID = 900,
        OPEN_CONTACTORS_ID = 901,
        SDC_OBCCU_ID = 902,
        IMD_BYPASS_ID = 903,
        ZEROING = 904
    };

    enum class IDPacket : uint16_t {
        BATTERY_1 = 910,
        BATTERY_2 = 911,
        BATTERY_3 = 912,
        BATTERY_4 = 913,
        BATTERY_5 = 914,
        BATTERY_6 = 915,
        BATTERY_7 = 916,
        BATTERY_8 = 917,
        BATTERY_9 = 918,
        BATTERY_10 = 919,
        BATTERY_11 = 920,
        BATTERY_12 = 921,
        BATTERY_13 = 922,
        BATTERY_14 = 923,
        BATTERY_15 = 924,
        BATTERY_16 = 925,
        BATTERY_17 = 926,
        BATTERY_18 = 927,
        CURRENT = 930,
        PPU_VOLTAGE = 551
    };

    static const uint16_t CONTROL_STATION_PORT = 50500;
    static const uint16_t PACKETS_ENDPOINT_PORT = 50400;
    static const uint16_t PCU_ENDPOINT_PORT = 50417;
    static constexpr std::string CONTROL_SATION_IP = "192.168.0.9";
    static constexpr std::string PCU_IP = "192.168.1.5";
    static constexpr std::string HVSCU_IP = "192.168.1.7";

    static ServerSocket* control_station;
    static DatagramSocket* packets_endpoint;
    static DatagramSocket* PCU_endpoint;

    static void start();
};
