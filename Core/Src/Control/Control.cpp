#include "Control/Control.hpp"

#include "Actuators/Actuators.hpp"
#include "Comms/Comms.hpp"
#include "Sensors/Sensors.hpp"
namespace HVSCU {

Control::Control()
    : general_state_machine{},
      operational_state_machine{},
      orders{},
      send_packets_flag{false} {
    Sensors::init();
    Actuators::init();

    set_state_machines();

    add_protections();

    STLIB::start("00:80:e1:00:01:07", Comms::HVSCU_IP, "255.255.255.0");

    Sensors::start();
    Comms::start();
    add_orders();
    add_packets();

    Time::register_low_precision_alarm(17, [&]() { send_packets_flag = true; });
}

void Control::set_state_machines() {
    general_state_machine.add_state(GeneralSMState::CONNECTING);
    general_state_machine.add_state(GeneralSMState::OPERATIONAL);
    general_state_machine.add_state(GeneralSMState::FAULT);

    operational_state_machine.add_state(OperationalSMState::HV_OPEN);
    operational_state_machine.add_state(OperationalSMState::PRECHARGE);
    operational_state_machine.add_state(OperationalSMState::HV_CLOSED);
    operational_state_machine.add_state(OperationalSMState::CHARGING);

    general_state_machine.add_transition(
        GeneralSMState::CONNECTING, GeneralSMState::OPERATIONAL,
        []() { return Comms::control_station->is_connected(); });
    general_state_machine.add_transition(
        GeneralSMState::OPERATIONAL, GeneralSMState::FAULT,
        []() { return !Comms::control_station->is_connected(); });

    operational_state_machine.add_transition(
        OperationalSMState::HV_OPEN, OperationalSMState::PRECHARGE,
        []() { return Actuators::is_precharging(); });
    operational_state_machine.add_transition(
        OperationalSMState::HV_CLOSED, OperationalSMState::HV_OPEN,
        []() { return Actuators::is_HV_open(); });
    operational_state_machine.add_transition(
        OperationalSMState::PRECHARGE, OperationalSMState::HV_OPEN,
        []() { return Actuators::is_HV_open(); });
    operational_state_machine.add_transition(
        OperationalSMState::PRECHARGE, OperationalSMState::HV_CLOSED,
        []() { return Actuators::is_HV_closed(); });

    general_state_machine.add_low_precision_cyclic_action(
        [this]() { Actuators::led_operational().toggle(); },
        std::chrono::duration<int64_t, std::milli>(500),
        GeneralSMState::CONNECTING);

    general_state_machine.add_enter_action(
        [this]() { Actuators::led_operational().turn_on(); },
        GeneralSMState::OPERATIONAL);

    general_state_machine.add_enter_action(
        [this]() {
            Actuators::open_HV();
            Actuators::led_operational().turn_off();
            Actuators::led_fault().turn_on();
        },
        GeneralSMState::FAULT);

    general_state_machine.add_state_machine(operational_state_machine,
                                            GeneralSMState::OPERATIONAL);
}

void Control::add_protections() {
    ProtectionManager::link_state_machine(general_state_machine,
                                          GeneralSMState::FAULT);

    // DC-bus voltage
    add_protection(&Sensors::voltage_sensor().reading,
                   Boundary<float, ABOVE>{430});

    // Current
    add_protection(&Sensors::current_sensor().reading,
                   Boundary<float, OUT_OF_RANGE>{-15, 70});

    // SoCs
    for (auto& [_, soc] : Sensors::batteries().SoCs) {
        add_protection(&soc, Boundary<float, BELOW>(24));
    }

    // IMD
    Time::register_low_precision_alarm(
        2000, +[]() {
            add_protection(&Sensors::imd().is_ok,
                           Boundary<bool, EQUALS>(false));
        });

    ProtectionManager::initialize();
}

void Control::add_orders() {
    auto open_contactor_order =
        new Order<Comms::IDOrder::OPEN_CONTACTORS_ID>([this]() {
            cancel_timeouts();
            Actuators::open_HV();
        });
    orders[GeneralSMState::OPERATIONAL].push_back(open_contactor_order);
    orders[GeneralSMState::FAULT].push_back(open_contactor_order);

    auto close_contactor_order =
        new Order<Comms::IDOrder::CLOSE_CONTACTORS_ID>([this]() {
            Actuators::start_precharge();
#if SMART_PRECHARGE
            precharge_timer_id = 0;
            precharge_timeout_id = Time::set_timeout(4000, [this]() {
                Time::unregister_mid_precision_alarm(precharge_timer_id);
                Actuators::open_HV();
                ErrorHandler("PRECHARGE FAILED");
            });
            precharge_timer_id =
                Time::register_mid_precision_alarm(100, [this]() {
                    if (Sensors::voltage_sensor().reading /
                            Sensors::batteries().total_voltage >
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
    orders[GeneralSMState::OPERATIONAL].push_back(close_contactor_order);

    auto sdc_obccu_order = new Order<Comms::IDOrder::SDC_OBCCU_ID>(
        []() { Actuators::sdc_obccu().toggle(); });
    orders[GeneralSMState::OPERATIONAL].push_back(sdc_obccu_order);

    auto imd_bypass_order = new Order<Comms::IDOrder::IMD_BYPASS_ID>(
        []() { Actuators::imd_bypass().toggle(); });
    orders[GeneralSMState::OPERATIONAL].push_back(imd_bypass_order);
}

void Control::add_packets() {
    auto general_state_machine_packet = new HeapPacket(
        static_cast<uint16_t>(Comms::IDPacket::GENERAL_STATE_MACHINE_STATUS),
        &general_state_machine.current_state);
    Comms::add_packet(general_state_machine_packet);

    auto operational_state_machine_packet =
        new HeapPacket(static_cast<uint16_t>(
                           Comms::IDPacket::OPERATIONAL_STATE_MACHINE_STATUS),
                       &operational_state_machine.current_state);
    Comms::add_packet(operational_state_machine_packet);
}

void Control::update() {
    STLIB::update();
    Sensors::update();
    general_state_machine.check_transitions();
    operational_state_machine.check_transitions();
    ProtectionManager::check_protections();

    for (auto& order : orders[static_cast<GeneralSMState>(
             general_state_machine.current_state)]) {
        order->check_order();
    }

    if (send_packets_flag) {
        Comms::send_packets();
        send_packets_flag = false;
    }
}
}  // namespace HVSCU