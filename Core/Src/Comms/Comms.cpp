#include "Comms/Comms.hpp"

ServerSocket* Comms::control_station = nullptr;
DatagramSocket* Comms::packets_endpoint = nullptr;
DatagramSocket* Comms::PCU_endpoint = nullptr;

void Comms::start() {
    control_station = new ServerSocket(IPV4(HVSCU_IP), CONTROL_STATION_PORT);
    packets_endpoint =
        new DatagramSocket(IPV4(HVSCU_IP), PACKETS_ENDPOINT_PORT,
                           IPV4(CONTROL_SATION_IP), PACKETS_ENDPOINT_PORT);
    PCU_endpoint =
        new DatagramSocket(IPV4(HVSCU_IP), PCU_ENDPOINT_PORT,
                           IPV4(PCU_IP), PCU_ENDPOINT_PORT);
}