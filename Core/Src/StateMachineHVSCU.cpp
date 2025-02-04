#include "StateMachineHVSCU.hpp"

#include "HVSCU.hpp"
#include "HVSCUComms.hpp"

namespace HVSCU {
StateMachine* sm = nullptr;

SM::SM() {
    sm = new StateMachine(0);
    add_states();
    add_transitions();
}

void SM::add_states() {
    sm->add_state(State::CONNECTING);
    sm->add_state(State::OPERATIONAL);
    sm->add_state(State::FAULT);
}

void SM::add_transitions() {
    sm->add_transition(State::CONNECTING, State::OPERATIONAL,
                       []() { return Comms::control_station->is_connected(); });
    sm->add_transition(State::OPERATIONAL, State::FAULT, []() {
        return !Comms::control_station->is_connected();
    });
}

void SM::check_open_contactors_order() {
    if (Comms::open_contactors_order_received) {
        if (timeout_id) Time::cancel_timeout(timeout_id);
        action_contactor(ContactorAction::CLOSE, discharge);
        action_contactor(ContactorAction::OPEN, low);
        action_contactor(ContactorAction::OPEN, high);
        action_contactor(ContactorAction::OPEN, precharge);
    }
}

void SM::check_close_contactors_order() {
    if (Comms::close_contactors_order_received) {
        if (timeout_id) Time::cancel_timeout(timeout_id);
        action_contactor(ContactorAction::OPEN, discharge);
        action_contactor(ContactorAction::CLOSE, low);
        action_contactor(ContactorAction::CLOSE, precharge);
        action_contactor(ContactorAction::OPEN, high);
        timeout_id = Time::set_timeout(3000, []() {
            action_contactor(ContactorAction::OPEN, precharge);
            action_contactor(ContactorAction::CLOSE, high);
        });
    }
}

void SM::update() {
    sm->check_transitions();
    switch (sm->current_state) {
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
}  // namespace HVSCU