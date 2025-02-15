#include "HVSCU/Comms.hpp"

#include "HVSCU/HVSCU.hpp"

ServerSocket* Comms::control_station = nullptr;
std::vector<HVSCUOrder> orders;

void Comms::start() {
    control_station = new ServerSocket(IPV4(HVSCU_IP), CONTROL_STATION_PORT);

    HVSCUOrder open_contactor_order(OPEN_CONTACTORS_ID, []() {
        HVSCU::open_contactors();
    });
    orders.push_back(open_contactor_order);

    HVSCUOrder close_contactor_order(CLOSE_CONTACTORS_ID, []() {
        HVSCU::close_contactors();
    });
    orders.push_back(close_contactor_order);

    HVSCUOrder sdc_obccu_order(SDC_OBCCU_ID, []() {
        HVSCU::sdc_obccu->toggle();
    });
    orders.push_back(sdc_obccu_order);
}