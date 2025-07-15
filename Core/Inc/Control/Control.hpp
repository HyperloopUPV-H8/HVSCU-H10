#pragma once

#include "Actuators/Actuators.hpp"
#include "Comms/HVSCUOrder.hpp"
#include "ST-LIB.hpp"

#define PERCENTAGE_TO_FINISH_PRECHARGE 0.95
#define SMART_PRECHARGE 1

namespace HVSCU {

class Control {
   private:
    enum GeneralSMState : uint8_t {
        CONNECTING = 0,
        OPERATIONAL = 1,
        FAULT = 2
    };
    enum OperationalSMState : uint8_t {
        HV_OPEN = 0,
        PRECHARGE = 1,
        HV_CLOSED = 2,
        CHARGING = 3
    };
    enum BmsStatus : uint8_t { OK, BMS_FAULT };

    StateMachine general_state_machine;
    StateMachine operational_state_machine;
    std::unordered_map<GeneralSMState, std::vector<OrderBase*>> orders;
    std::vector<Protection*> protections;

    BmsStatus bms_status{OK};
    bool send_packets_flag{false};
    bool powered_on{false};

#if SMART_PRECHARGE
    uint8_t precharge_timer_id;
    uint8_t precharge_timeout_id;
#else
    uint8_t contactors_timeout_id;
#endif
    void cancel_timeouts() {
#if SMART_PRECHARGE
        Time::cancel_timeout(precharge_timeout_id);
        Time::unregister_mid_precision_alarm(precharge_timer_id);
#else
        Time::cancel_timeout(contactors_timeout_id);
#endif
    }

    void set_state_machines();
    void add_orders();
    void add_packets();
    void add_protections();
    void check_bms_status();

   public:
    Control();
    void update();
};
}  // namespace HVSCU