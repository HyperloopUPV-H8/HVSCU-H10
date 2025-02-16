#include "Control/Control.hpp"

#include "Comms/Comms.hpp"
// #include "Sensors/"
#include "Actuators/Actuators.hpp"

Control::Control() : state_machine(0), orders() {
    Actuators::start();

    add_states();
    add_transitions();

    Comms::start();
    add_orders();
}

void Control::add_states() {
    state_machine.add_state(State::CONNECTING);
    state_machine.add_state(State::OPERATIONAL);
    state_machine.add_state(State::FAULT);
}

void Control::add_transitions() {
    state_machine.add_transition(State::CONNECTING, State::OPERATIONAL, []() {
        return Comms::control_station->is_connected();
    });
    state_machine.add_transition(State::OPERATIONAL, State::FAULT, []() {
        return !Comms::control_station->is_connected();
    });

    state_machine.add_enter_action(
        [this]() {
            Actuators::open_contactors();
            Actuators::led_fault->turn_on();
        },
        State::FAULT);

    state_machine.add_low_precision_cyclic_action(
        [this]() { Actuators::led_operational->toggle(); },
        std::chrono::duration<int64_t, std::milli>(500), State::CONNECTING);

    state_machine.add_enter_action(
        [this]() { Actuators::led_operational->turn_on(); },
        State::OPERATIONAL);
}

void Control::add_orders() {
    HVSCUOrder open_contactor_order(Comms::OPEN_CONTACTORS_ID,
                                    []() { Actuators::open_contactors(); });
    orders[State::OPERATIONAL].push_back(open_contactor_order);

    HVSCUOrder close_contactor_order(Comms::CLOSE_CONTACTORS_ID,
                                     []() { Actuators::close_contactors(); });
    orders[State::OPERATIONAL].push_back(close_contactor_order);

    HVSCUOrder sdc_obccu_order(Comms::SDC_OBCCU_ID,
                               []() { Actuators::sdc_obccu->toggle(); });
    orders[State::OPERATIONAL].push_back(sdc_obccu_order);

    HVSCUOrder imd_bypass_order(Comms::IMD_BYPASS_ID,
                               []() { Actuators::imd_bypass->toggle(); });
    orders[State::OPERATIONAL].push_back(imd_bypass_order);
}

void Control::update() {
    state_machine.check_transitions();
    for (auto &order :
         orders[static_cast<State>(state_machine.current_state)]) {
        order.check_order();
    }
}
