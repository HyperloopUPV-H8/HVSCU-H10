#include "HVSCU/Comms.hpp"

ServerSocket* Comms::control_station = nullptr;
bool Comms::open_contactors_order_received = false;
bool Comms::close_contactors_order_received = false;
bool Comms::sdc_obccu_order_received = false;
HeapOrder *Comms::open_contactors_order = nullptr;
HeapOrder *Comms::close_contactors_order = nullptr;
HeapOrder *Comms::sdc_obccu_order = nullptr;

void Comms::start() {
    control_station = new ServerSocket(IPV4(HVSCU_IP), CONTROL_STATION_PORT);
    open_contactors_order = new HeapOrder(OPEN_CONTACTORS_ID, []() {
        Comms::open_contactors_order_received = true;
    });

    close_contactors_order = new HeapOrder(CLOSE_CONTACTORS_ID, []() {
        Comms::close_contactors_order_received = true;
    });

    sdc_obccu_order = new HeapOrder(SDC_OBCCU_ID, []() {
        Comms::sdc_obccu_order_received = true;
    });
}