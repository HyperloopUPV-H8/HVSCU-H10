#include "Actuators/Contactor.hpp"

Contactor::Contactor(DigitalOutput output, bool normally_opened)
    : output(output), normally_opened(normally_opened) {}

void Contactor::open() {
    if (normally_opened)
        output.turn_off();
    else {
        output.turn_on();
    }
}

void Contactor::close() {
    if (normally_opened)
        output.turn_on();
    else {
        output.turn_off();
    }
}