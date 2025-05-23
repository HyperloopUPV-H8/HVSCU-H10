#include "Actuators/Contactor.hpp"

Contactor::Contactor(Pin& pin, bool normally_opened)
    : output{pin},
      normally_opened(normally_opened),
      state(normally_opened ? State::OPEN : State::CLOSED) {}

void Contactor::open() {
    if (normally_opened)
        output.turn_off();
    else {
        output.turn_on();
    }
    state = State::OPEN;
}

bool Contactor::is_open() { return state == State::OPEN; }

void Contactor::close() {
    if (normally_opened)
        output.turn_on();
    else {
        output.turn_off();
    }
    state = State::CLOSED;
}

bool Contactor::is_closed() { return state == State::CLOSED; }