#include "Actuators/Actuators.hpp"

#include "HVSCUPinout.hpp"
namespace HVSCU {
Contactor* Actuators::contactor_low = nullptr;
Contactor* Actuators::contactor_high = nullptr;
Contactor* Actuators::contactor_precharge = nullptr;
Contactor* Actuators::contactor_discharge = nullptr;

uint8_t Actuators::contactors_timeout_id;

DigitalOutput* Actuators::led_operational = nullptr;
DigitalOutput* Actuators::led_fault = nullptr;
DigitalOutput* Actuators::sdc_obccu = nullptr;
DigitalOutput* Actuators::imd_bypass = nullptr;
DigitalOutput* Actuators::led_nucleo = nullptr;

void Actuators::start() {
    contactor_low = new Contactor(DigitalOutput(CONTACTOR_LOW), true);
    contactor_high = new Contactor(DigitalOutput(CONTACTOR_HIGH), true);
    contactor_precharge =
        new Contactor(DigitalOutput(CONTACTOR_PRECHARGE), true);
    contactor_discharge =
        new Contactor(DigitalOutput(CONTACTOR_DISCHARGE), false);
    led_operational = new DigitalOutput(LED_OPERATIONAL);
    led_fault = new DigitalOutput(LED_FAULT);
    sdc_obccu = new DigitalOutput(SDC_OBCCU);
    imd_bypass = new DigitalOutput(IMD_BYPASS);
#ifdef NUCLEO
    led_nucleo = new DigitalOutput(LED_NUCLEO);
#endif
}

void Actuators::open_HV() {
    contactor_discharge->close();
    contactor_low->open();
    contactor_high->open();
    contactor_precharge->open();
}

bool Actuators::is_HV_open() {
    return contactor_discharge->is_closed() && contactor_low->is_open() &&
           clontactor_high->is_open() && contactor_precharge->is_open();
}

void Actuators::close_HV() {
    contactor_discharge->open();
    contactor_low->close();
    contactor_precharge->open();
    contactor_high->close();
}

bool Actuators::is_HV_closed() {
    return contactor_discharge->is_open() && contactor_low->is_closed() &&
           clontactor_high->is_closed() && contactor_precharge->is_open();
}

void Actuators::start_precharge() {
    contactor_discharge->open();
    contactor_low->close();
    contactor_precharge->close();
    contactor_high->open();
}

bool Actuators::is_precharging() {
    return contactor_discharge->is_open() && contactor_low->is_closed() &&
           clontactor_high->is_open() && contactor_precharge->is_closed();
}
}  // namespace HVSCU