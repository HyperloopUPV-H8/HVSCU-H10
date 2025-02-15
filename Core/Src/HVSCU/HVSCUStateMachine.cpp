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
            HVSCU::led_fault->turn_on();
        },
        State::FAULT);

    sm.add_low_precision_cyclic_action(
        [this]() { HVSCU::led_operational->toggle(); },
        std::chrono::duration<int64_t, std::milli>(500), State::CONNECTING);

    sm.add_enter_action([this]() { HVSCU::led_operational->turn_on(); },
                        State::OPERATIONAL);
}

void HVSCUStateMachine::update() {
    sm.check_transitions();
    switch (sm.current_state) {
        case State::CONNECTING:
            break;
        case State::OPERATIONAL:
            check_sdc_obbcu_order();
            check_open_contactors_order();
            check_close_contactors_order();
            break;
        case State::FAULT:
            check_open_contactors_order();
            break;
    }
}
