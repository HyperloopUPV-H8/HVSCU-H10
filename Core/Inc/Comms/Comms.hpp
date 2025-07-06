#pragma once

#include "ST-LIB.hpp"

#define ROBUST_SM 1

namespace HVSCU {
class Comms {
    static std::vector<HeapPacket*> backend_packets;
    static std::vector<HeapPacket*> master_packets;

   public:
    enum class IDOrder : uint16_t {
        CLOSE_CONTACTORS_ID = 900,
        OPEN_CONTACTORS_ID = 901,
        SDC_OBCCU_ID = 902,
        IMD_BYPASS_ID = 903
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
        TOTAL_VOLTAGE = 928,
        VOLTAGE = 930,
        CURRENT = 931,
        GENERAL_STATE_MACHINE_STATUS = 940,
        OPERATIONAL_STATE_MACHINE_STATUS = 941,
        DRIVER_DIAG = 942,
        IMD = 943
    };

    static const uint16_t CONTROL_STATION_PACKET_PORT = 50400;
    static constexpr std::string CONTROL_SATION_IP = "192.168.0.9";

    static DatagramSocket* control_station_packet_endpoint;
#if ROBUST_SM
    static const uint16_t MASTER_PACKET_PORT = 50403;
    static constexpr std::string MASTER_IP = "192.168.1.3";

    static DatagramSocket* master_packet_endpoint;
#endif

    static constexpr std::string HVSCU_IP = "192.168.1.7";
    static const uint16_t ORDER_PORT = 50500;
    static ServerSocket* order_endpoint;

    static void start();
    static void add_packet(HeapPacket* packet, bool master = false);
    static void send_packets();
};
}  // namespace HVSCU