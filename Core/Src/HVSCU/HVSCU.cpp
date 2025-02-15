#include "HVSCU/HVSCU.hpp"

Contactor* HVSCU::low = nullptr;
Contactor* HVSCU::high = nullptr;
Contactor* HVSCU::precharge = nullptr;
Contactor* HVSCU::discharge = nullptr;

uint8_t HVSCU::contactors_timeout_id;

DigitalOutput* HVSCU::led_operational = nullptr;
DigitalOutput* HVSCU::led_fault = nullptr;
DigitalOutput* HVSCU::sdc_obccu = nullptr;

void HVSCU::start() {
    low = new Contactor(DigitalOutput(PG14), true);
    high = new Contactor(DigitalOutput(PG12), true);
    precharge = new Contactor(DigitalOutput(PD4), true);
    discharge = new Contactor(DigitalOutput(PF4), false);
    led_operational = new DigitalOutput(PG8);
    led_fault = new DigitalOutput(PG7);
    sdc_obccu = new DigitalOutput(PA11);
}

void HVSCU::open_contactors() {
    if (HVSCU::contactors_timeout_id)
        Time::cancel_timeout(contactors_timeout_id);
    discharge->close();
    low->open();
    high->open();
    precharge->open();
}

void HVSCU::close_contactors() {
    if (contactors_timeout_id)
        Time::cancel_timeout(contactors_timeout_id);
    discharge->open();
    low->close();
    precharge->close();
    high->open();
    contactors_timeout_id = Time::set_timeout(3000, []() {
        precharge->open();
        high->close();
    });
}
