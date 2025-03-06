#include "Control/Control.hpp"

#include "Actuators/Actuators.hpp"
#include "Comms/Comms.hpp"
#include "Sensors/Sensors.hpp"

Control::Control() : state_machine(0), orders(), send_packets_flag(false) {
    Actuators::start();
    Sensors::start();

    add_states();
    add_transitions();

    STLIB::start(Comms::HVSCU_IP);

    Comms::start();
    add_orders();
    Sensors::bmsh->initialize();
    add_packets();

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

void Control::add_orders() {
    auto open_contactor_order =
        new HVSCUOrder<Comms::IDOrder::OPEN_CONTACTORS_ID>(
            []() { Actuators::open_HV(); });
    orders[State::OPERATIONAL].push_back(open_contactor_order);
    orders[State::FAULT].push_back(open_contactor_order);

    auto close_contactor_order =
        new HVSCUOrder<Comms::IDOrder::CLOSE_CONTACTORS_ID>([]() {
            Actuators::start_precharge();
#if SMART_PRECHARGE
            uint8_t precharge_timer_id = 0;
            precharge_timer_id =
                Time::register_mid_precision_alarm(100, [precharge_timer_id]() {
                    float average_PPUs_voltage =
                        (Sensors::PPU1_voltage + Sensors::PPU2_voltage) / 2;
                    if (average_PPUs_voltage / Sensors::total_voltage > 0.95) {
                        Actuators::close_HV();
                        Time::unregister_mid_precision_alarm(
                            precharge_timer_id);
                    }
                });
#else
            contactors_timeout_id = Time::set_timeout(3000, []() {
                contactor_precharge->open();
                contactor_high->close();
            });
#endif
        });
    orders[State::OPERATIONAL].push_back(close_contactor_order);

    auto sdc_obccu_order = new HVSCUOrder<Comms::IDOrder::SDC_OBCCU_ID>(
        []() { Actuators::sdc_obccu->toggle(); });
    orders[State::OPERATIONAL].push_back(sdc_obccu_order);

    auto imd_bypass_order = new HVSCUOrder<Comms::IDOrder::IMD_BYPASS_ID>(
        []() { Actuators::imd_bypass->toggle(); });
    orders[State::OPERATIONAL].push_back(imd_bypass_order);

    auto zeroing =
        new HVSCUOrder<Comms::IDOrder::ZEROING>([]() { Sensors::zeroing(); });
    orders[State::OPERATIONAL].push_back(zeroing);
}

void Control::add_packets() {
    for (int i = 0; i < 10; ++i) {
        auto battery_packet = new HeapPacket(
            static_cast<uint16_t>(Comms::IDPacket::BATTERY_1) + i,
            &Sensors::bmsh->external_adcs[i].battery.SOC,
            Sensors::bmsh->external_adcs[i].battery.cells[0],
            Sensors::bmsh->external_adcs[i].battery.cells[1],
            Sensors::bmsh->external_adcs[i].battery.cells[2],
            Sensors::bmsh->external_adcs[i].battery.cells[3],
            Sensors::bmsh->external_adcs[i].battery.cells[4],
            Sensors::bmsh->external_adcs[i].battery.cells[5],
            &Sensors::bmsh->external_adcs[i].battery.minimum_cell,
            &Sensors::bmsh->external_adcs[i].battery.maximum_cell,
            &Sensors::converted_temps[i],
            Sensors::bmsh->external_adcs[i].battery.temperature2,
            &Sensors::bmsh->external_adcs[i].battery.is_balancing,
            &Sensors::bmsh->external_adcs[i].battery.total_voltage);

        packets[State::OPERATIONAL].push_back(battery_packet);
    }

    auto current_packet =
        new HeapPacket(static_cast<uint16_t>(Comms::IDPacket::CURRENT),
                       &Sensors::voltage_reading, &Sensors::current_reading);
    packets[State::OPERATIONAL].push_back(current_packet);

    auto PPU_voltage_packet =
        new HeapPacket(static_cast<uint16_t>(Comms::IDPacket::CURRENT),
                       &Sensors::PPU1_voltage, &Sensors::PPU2_voltage);
    packets[State::OPERATIONAL].push_back(PPU_voltage_packet);
}

void Control::update() {
    STLIB::update();
    Sensors::update();
    state_machine.check_transitions();

    for (auto &order :
         orders[static_cast<State>(state_machine.current_state)]) {
        order->check_order();
    }

    if (send_packets_flag) {
        for (auto &packet :
             packets[static_cast<State>(state_machine.current_state)]) {
            Comms::packets_endpoint->send_packet(*packet);
        }
        send_packets_flag = false;
    }
}
