#include "Comms/Comms.hpp"

ServerSocket* Comms::control_station = nullptr;

void Comms::start() {
    control_station = new ServerSocket(IPV4(HVSCU_IP), CONTROL_STATION_PORT);
}