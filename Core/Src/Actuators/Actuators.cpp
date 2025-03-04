#include "Actuators/Actuators.hpp"

#include "HVSCUPinout.hpp"

Contactor* Actuators::contactor_low = nullptr;
Contactor* Actuators::contactor_high = nullptr;
Contactor* Actuators::contactor_precharge = nullptr;
Contactor* Actuators::contactor_discharge = nullptr;

uint8_t Actuators::contactors_timeout_id;

DigitalOutput* Actuators::led_operational = nullptr;
DigitalOutput* Actuators::led_fault = nullptr;
DigitalOutput* Actuators::sdc_obccu = nullptr;
DigitalOutput* Actuators::imd_bypass = nullptr;
#ifdef NUCLEO
DigitalOutput* Actuators::led_nucleo = nullptr;
#endif

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
#ifdef NUCLEO
    led_nucleo->turn_on();
#endif
    contactors_timeout_id = Time::set_timeout(3000, []() {
        contactor_precharge->open();
        contactor_high->close();
#ifdef NUCLEO
        led_nucleo->turn_off();
#endif
    });
}
