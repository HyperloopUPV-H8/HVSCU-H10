#include "Comms/Comms.hpp"

namespace HVSCU {
ServerSocket* Comms::control_station = nullptr;
DatagramSocket* Comms::packets_endpoint = nullptr;

std::vector<HeapPacket*> Comms::packets{};

void Comms::start() {
    control_station = new ServerSocket(IPV4(HVSCU_IP), CONTROL_STATION_PORT);
    packets_endpoint =
        new DatagramSocket(IPV4(HVSCU_IP), PACKETS_ENDPOINT_PORT,
                           IPV4(CONTROL_SATION_IP), PACKETS_ENDPOINT_PORT);
}

void Comms::add_packet(HeapPacket* packet) { packets.push_back(packet); }

void Comms::send_packets() {
    for (auto p : packets) {
        packets_endpoint->send_packet(*p);
    }
}
}  // namespace HVSCU