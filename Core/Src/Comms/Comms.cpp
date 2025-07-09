#include "Comms/Comms.hpp"

namespace HVSCU {
ServerSocket* Comms::order_endpoint{nullptr};
DatagramSocket* Comms::control_station_endpoint{nullptr};
DatagramSocket* Comms::master_endpoint{nullptr};
DatagramSocket* Comms::lcu_endpoint{nullptr};
std::unordered_map<Comms::Target, std::vector<HeapPacket*>> Comms::packets{};

void Comms::start() {
    order_endpoint = new ServerSocket(IPV4(HVSCU_IP), ORDER_PORT);
    control_station_endpoint =
        new DatagramSocket(IPV4(HVSCU_IP), CONTROL_STATION_PORT,
                           IPV4(CONTROL_SATION_IP), CONTROL_STATION_PORT);
    master_endpoint = new DatagramSocket(IPV4(HVSCU_IP), MASTER_PORT,
                                         IPV4(MASTER_IP), MASTER_PORT);
    lcu_endpoint =
        new DatagramSocket(IPV4(HVSCU_IP), LCU_PORT, IPV4(LCU_IP), LCU_PORT);
}

bool Comms::is_order_socket_connected() {
    return order_endpoint->is_connected();
}

void Comms::add_packet(Target target, HeapPacket* packet) {
    packets[target].push_back(packet);
}

void Comms::send_packets() {
    for (auto& [target, packet] : packets) {
        for (auto& p : packet) {
            switch (target) {
                case Target::LCU:
                    lcu_endpoint->send_packet(*p);
                    break;
                case Target::MASTER:
                    master_endpoint->send_packet(*p);
                    break;
                case Target::CONTROL_STATION:
                    control_station_endpoint->send_packet(*p);
                    break;
            }
        }
    }
}
}  // namespace HVSCU