#include "Actuators/Actuators.hpp"

Contactor* Actuators::contactor_low = nullptr;
Contactor* Actuators::contactor_high = nullptr;
Contactor* Actuators::contactor_precharge = nullptr;
Contactor* Actuators::contactor_discharge = nullptr;

uint8_t Actuators::contactors_timeout_id;

DigitalOutput* Actuators::led_operational = nullptr;
DigitalOutput* Actuators::led_fault = nullptr;
DigitalOutput* Actuators::sdc_obccu = nullptr;

void Actuators::start() {
    contactor_low = new Contactor(DigitalOutput(PG14), true);
    contactor_high = new Contactor(DigitalOutput(PG12), true);
    contactor_precharge = new Contactor(DigitalOutput(PD4), true);
    contactor_discharge = new Contactor(DigitalOutput(PF4), false);
    led_operational = new DigitalOutput(PG8);
    led_fault = new DigitalOutput(PG7);
    sdc_obccu = new DigitalOutput(PA11);
    imd_bypass = new DigitalOutput(PF5);
}

void Actuators::open_contactors() {
    if (contactors_timeout_id) Time::cancel_timeout(contactors_timeout_id);
    contactor_discharge->close();
    contactor_low->open();
    contactor_high->open();
    contactor_precharge->open();
}

void Actuators::close_contactors() {
    if (contactors_timeout_id) Time::cancel_timeout(contactors_timeout_id);
    contactor_discharge->open();
    contactor_low->close();
    contactor_precharge->close();
    contactor_high->open();
    contactors_timeout_id = Time::set_timeout(3000, []() {
        contactor_precharge->open();
        contactor_high->close();
    });
}
