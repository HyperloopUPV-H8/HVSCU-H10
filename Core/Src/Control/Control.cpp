#include "Control/Control.hpp"

#include "Actuators/Actuators.hpp"
#include "Comms/Comms.hpp"
#include "Sensors/Sensors.hpp"
namespace HVSCU {

Control::Control() : state_machine(), orders(), send_packets_flag(false) {
    Actuators::start();
    Sensors::start();

    add_states();
    add_transitions();

    add_protections();

    STLIB::start(Comms::HVSCU_IP);

    Comms::start();
    add_orders();
    // Sensors::bmsh->initialize();

    Time::register_low_precision_alarm(17, [&]() { send_packets_flag = true; });
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
            Actuators::open_HV();
            Actuators::led_fault->turn_on();
        },
        State::FAULT);

    state_machine.add_low_precision_cyclic_action(
        [this]() { Actuators::led_operational->toggle(); },
        std::chrono::duration<int64_t, std::milli>(500), State::CONNECTING);

    state_machine.add_enter_action(
        [this]() { Actuators::led_operational->turn_on(); },
        State::OPERATIONAL);

#ifdef NUCLEO
    state_machine.add_low_precision_cyclic_action(
        [this]() { Actuators::led_nucleo->toggle(); },
        std::chrono::duration<int64_t, std::milli>(500), State::CONNECTING);
    state_machine.add_enter_action(
        [this]() { Actuators::led_nucleo->turn_on(); }, State::OPERATIONAL);
    state_machine.add_enter_action(
        [this]() { Actuators::led_nucleo->turn_off(); }, State::FAULT);
#endif
}

void Control::add_protections() {
    ProtectionManager::link_state_machine(state_machine, State::FAULT);

    add_protection(&Sensors::voltage_sensor->reading,
                   Boundary<float, ABOVE>{320});
    add_protection(&Sensors::current_sensor->reading,
                   Boundary<float, OUT_OF_RANGE>{-15, 70});

    ProtectionManager::initialize();
}

void Control::add_orders() {
    auto open_contactor_order =
        new Order<Comms::IDOrder::OPEN_CONTACTORS_ID>([this]() {
            cancel_timeouts();
            Actuators::open_HV();
        });
    orders[State::OPERATIONAL].push_back(open_contactor_order);
    orders[State::FAULT].push_back(open_contactor_order);

    auto close_contactor_order =
        new Order<Comms::IDOrder::CLOSE_CONTACTORS_ID>([this]() {
            Actuators::start_precharge();
#if SMART_PRECHARGE
            precharge_timer_id = 0;
            precharge_timeout_id = Time::set_timeout(4000, [this]() {
                Time::unregister_mid_precision_alarm(precharge_timer_id);
                Actuators::open_HV();
                ProtectionManager::fault_and_propagate();
            });
            precharge_timer_id =
                Time::register_mid_precision_alarm(100, [this]() {
                    if (Sensors::voltage_sensor->reading /
                            Sensors::total_voltage >
                        PERCENTAGE_TO_FINISH_PRECHARGE) {
                        cancel_timeouts();
                        Actuators::close_HV();
                    }
                });
#else
            contactors_timeout_id =
                Time::set_timeout(3000, []() { Actuators::close_HV(); });
#endif
        });
    orders[State::OPERATIONAL].push_back(close_contactor_order);

    auto sdc_obccu_order = new Order<Comms::IDOrder::SDC_OBCCU_ID>(
        []() { Actuators::sdc_obccu->toggle(); });
    orders[State::OPERATIONAL].push_back(sdc_obccu_order);

    auto imd_bypass_order = new Order<Comms::IDOrder::IMD_BYPASS_ID>(
        []() { Actuators::imd_bypass->toggle(); });
    orders[State::OPERATIONAL].push_back(imd_bypass_order);
}

void Control::update() {
    STLIB::update();
    Sensors::update();
    state_machine.check_transitions();
    ProtectionManager::check_protections();

    for (auto &order :
         orders[static_cast<State>(state_machine.current_state)]) {
        order->check_order();
    }

    if (send_packets_flag) {
        Comms::send_packets();
        send_packets_flag = false;
    }
}
}  // namespace HVSCU