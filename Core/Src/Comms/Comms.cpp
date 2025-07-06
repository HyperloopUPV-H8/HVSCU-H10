#include "Comms/Comms.hpp"

namespace HVSCU {
std::vector<HeapPacket*> Comms::backend_packets{};
std::vector<HeapPacket*> Comms::master_packets{};

ServerSocket* Comms::order_endpoint = nullptr;
DatagramSocket* Comms::control_station_packet_endpoint = nullptr;
#if ROBUST_SM
DatagramSocket* Comms::master_packet_endpoint = nullptr;
#endif
void Comms::start() {
    order_endpoint = new ServerSocket(IPV4(HVSCU_IP), ORDER_PORT);
    control_station_packet_endpoint = new DatagramSocket(
        IPV4(HVSCU_IP), CONTROL_STATION_PACKET_PORT, IPV4(CONTROL_SATION_IP),
        CONTROL_STATION_PACKET_PORT);
#if ROBUST_SM
    master_packet_endpoint =
        new DatagramSocket(IPV4(HVSCU_IP), MASTER_PACKET_PORT, IPV4(MASTER_IP),
                           MASTER_PACKET_PORT);
#endif
}

void Comms::add_packet(HeapPacket* packet, bool master) {
    if (master) {
        master_packets.push_back(packet);
    } else {
        backend_packets.push_back(packet);
    }
}

void Comms::send_packets() {
    for (auto p : backend_packets) {
        control_station_packet_endpoint->send_packet(*p);
    }

    for (auto p : master_packets) {
        master_packet_endpoint->send_packet(*p);
    }
}
}  // namespace HVSCU