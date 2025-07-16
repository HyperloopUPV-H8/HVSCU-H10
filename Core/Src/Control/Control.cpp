#include "Control/Control.hpp"

#include "Actuators/Actuators.hpp"
#include "Comms/Comms.hpp"
#include "Sensors/Sensors.hpp"

#define set_protection_name(protection, name)                 \
    {                                                         \
        protection->set_name((char*)malloc(name.size() + 1)); \
        sprintf(protection->get_name(), "%s", name.c_str());  \
    }
namespace HVSCU {

Control::Control() {
    Sensors::init();
    Actuators::init();

    set_state_machines();

    add_protections();

    STLIB::start("00:80:e1:00:01:07", Comms::HVSCU_IP, "255.255.255.0");

    Actuators::sdc_obccu().turn_on();

    Sensors::start();
    Comms::start();
    add_orders();
    add_packets();

    Time::register_low_precision_alarm(17, [&]() { send_packets_flag = true; });
    Time::set_timeout(2000, [this]() {
        powered_on = true;
        Sensors::sdc().enable();
    });
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
        []() { return Comms::is_order_socket_connected(); });
    general_state_machine.add_transition(
        GeneralSMState::OPERATIONAL, GeneralSMState::FAULT,
        []() { return !Comms::is_order_socket_connected(); });

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
            Actuators::sdc_obccu().turn_off();
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

    // DC bus voltage
    Protection* protection = &ProtectionManager::_add_protection(
        &Sensors::voltage_sensor().reading, Boundary<float, ABOVE>{430});
    std::string name = "DC bus voltage";
    set_protection_name(protection, name);
    protections.push_back(protection);

    // Batteries current
    protection = &ProtectionManager::_add_protection(
        &Sensors::current_sensor().reading,
        Boundary<float, OUT_OF_RANGE>{-15, 85});
    name = "Battery pack current";
    set_protection_name(protection, name);
    protections.push_back(protection);

    // SoCs
    auto id{1};
    for (auto& [_, soc] : Sensors::batteries().SoCs) {
        protection = &ProtectionManager::_add_protection(
            &soc, Boundary<float, BELOW>(0.24));
        name = "SoC battery " + std::to_string(id);
        set_protection_name(protection, name);
        protections.push_back(protection);
        ++id;
    }

    // Batteries conversion rate
    id = 1;
    for (auto& battery : Sensors::batteries().batteries) {
        protection = &ProtectionManager::_add_protection(
            &battery.conv_rate, Boundary<float, BELOW>(0.5));
        name = "Conversion rate battery " + std::to_string(id);
        set_protection_name(protection, name);
        protections.push_back(protection);
        ++id;
    }

    // Batteries temperature
    id = 1;
    for (auto& temp : Sensors::batteries().batteries_temp) {
        protection = &ProtectionManager::_add_protection(
            &temp[0], Boundary<float, ABOVE>(50.0));
        name = "Temperature 1 battery " + std::to_string(id);
        set_protection_name(protection, name);
        protections.push_back(protection);

        protection = &ProtectionManager::_add_protection(
            &temp[1], Boundary<float, ABOVE>(50.0));
        name = "Temperature 2 battery " + std::to_string(id);
        set_protection_name(protection, name);
        protections.push_back(protection);

        ++id;
    }

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
    Comms::add_packet(Comms::Target::CONTROL_STATION,
                      general_state_machine_packet);
    Comms::add_packet(Comms::Target::MASTER, general_state_machine_packet);

    auto operational_state_machine_packet =
        new HeapPacket(static_cast<uint16_t>(
                           Comms::IDPacket::OPERATIONAL_STATE_MACHINE_STATUS),
                       &operational_state_machine.current_state);
    Comms::add_packet(Comms::Target::CONTROL_STATION,
                      operational_state_machine_packet);
    Comms::add_packet(Comms::Target::MASTER, operational_state_machine_packet);

    auto bms_status_packet = new HeapPacket(
        static_cast<uint16_t>(Comms::IDPacket::BMS_STATUS), &bms_status);
    Comms::add_packet(Comms::Target::CONTROL_STATION, bms_status_packet);
}

void Control::check_bms_status() {
    for (auto& p : protections) {
        if (p->check_state() == Protections::FaultType::FAULT) {
            bms_status = BMS_FAULT;
            return;
        }
    }
    bms_status = BmsStatus::OK;
}

void Control::update() {
    STLIB::update();
    Sensors::update();
    general_state_machine.check_transitions();
    operational_state_machine.check_transitions();
    if (powered_on) {
        ProtectionManager::check_protections();
        check_bms_status();

        if (Sensors::sdc().triggered) {
            ErrorHandler("SDC triggered");
        }

        if (!Sensors::imd().is_ok) {
            ErrorHandler("Insulation FAULT");
        }
    }

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