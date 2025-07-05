#include "Comms/Comms.hpp"

namespace HVSCU {
std::vector<HeapPacket*> Comms::backend_packets{};
std::vector<HeapPacket*> Comms::master_packets{};

ServerSocket* Comms::control_station = nullptr;
DatagramSocket* Comms::packets_endpoint = nullptr;
DatagramSocket* Comms::master_endpoint = nullptr;

void Comms::start() {
    control_station = new ServerSocket(IPV4(HVSCU_IP), CONTROL_STATION_PORT);
    packets_endpoint =
        new DatagramSocket(IPV4(HVSCU_IP), PACKETS_ENDPOINT_PORT,
                           IPV4(CONTROL_SATION_IP), PACKETS_ENDPOINT_PORT);
    master_endpoint =
        new DatagramSocket(IPV4(HVSCU_IP), PACKETS_ENDPOINT_PORT,
                           IPV4(MASTER_IP), PACKETS_ENDPOINT_PORT);
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
        packets_endpoint->send_packet(*p);
    }

    for (auto p : master_packets) {
        master_endpoint->send_packet(*p);
    }
}
}  // namespace HVSCU