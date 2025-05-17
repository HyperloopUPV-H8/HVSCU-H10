#include "Actuators/Actuators.hpp"

namespace HVSCU {
uint8_t Actuators::contactors_timeout_id;

void Actuators::open_HV() {
    contactor_discharge.close();
    contactor_low.open();
    contactor_high.open();
    contactor_precharge.open();
}

bool Actuators::is_HV_open() {
    return contactor_discharge.is_closed() && contactor_low.is_open() &&
           contactor_high.is_open() && contactor_precharge.is_open();
}

void Actuators::close_HV() {
    contactor_discharge.open();
    contactor_low.close();
    contactor_precharge.open();
    contactor_high.close();
}

bool Actuators::is_HV_closed() {
    return contactor_discharge.is_open() && contactor_low.is_closed() &&
           contactor_high.is_closed() && contactor_precharge.is_open();
}

void Actuators::start_precharge() {
    contactor_discharge.open();
    contactor_low.close();
    contactor_precharge.close();
    contactor_high.open();
}

bool Actuators::is_precharging() {
    return contactor_discharge.is_open() && contactor_low.is_closed() &&
           contactor_high.is_open() && contactor_precharge.is_closed();
}
}  // namespace HVSCU