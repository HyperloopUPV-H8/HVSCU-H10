#include "HVSCU/HVSCUStateMachine.hpp"

#include "HVSCU/Comms.hpp"

HVSCUStateMachine::HVSCUStateMachine() : sm(0), hvscu() {
    add_states();
    add_transitions();
}

void HVSCUStateMachine::add_states() {
    sm.add_state(State::CONNECTING);
    sm.add_state(State::OPERATIONAL);
    sm.add_state(State::FAULT);
}

void HVSCUStateMachine::add_transitions() {
    sm.add_transition(State::CONNECTING, State::OPERATIONAL,
                      []() { return Comms::control_station->is_connected(); });
    sm.add_transition(State::OPERATIONAL, State::FAULT,
                      []() { return !Comms::control_station->is_connected(); });

    sm.add_enter_action(
        [this]() {
            hvscu.open_contactors();
            hvscu.led_fault.turn_on();
        },
        State::FAULT);

    sm.add_low_precision_cyclic_action(
        [this]() { hvscu.led_operational.toggle(); },
        std::chrono::duration<int64_t, std::milli>(500), State::CONNECTING);

    sm.add_enter_action([this]() { hvscu.led_operational.turn_on(); },
                        State::OPERATIONAL);
}

void HVSCUStateMachine::check_open_contactors_order() {
    if (Comms::open_contactors_order_received) {
        hvscu.open_contactors();
        Comms::open_contactors_order_received = false;
    }
}

void HVSCUStateMachine::check_close_contactors_order() {
    if (Comms::close_contactors_order_received) {
        hvscu.close_contactors();
        Comms::close_contactors_order_received = false;
    }
}

void HVSCUStateMachine::update() {
    sm.check_transitions();
    switch (sm.current_state) {
        case State::CONNECTING:
            break;
        case State::OPERATIONAL:
            check_open_contactors_order();
            check_close_contactors_order();
            break;
        case State::FAULT:
            check_open_contactors_order();
            break;
    }
}
