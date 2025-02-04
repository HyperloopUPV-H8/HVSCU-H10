#include "StateMachineHVSCU.hpp"

#include "HVSCUComms.hpp"

StateMachine* hvscu_sm = nullptr;

StateMachineHVSCU::StateMachineHVSCU() {
    hvscu_sm = new StateMachine(0);
    add_states();
    add_transitions();
}

void StateMachineHVSCU::add_states() {
    hvscu_sm->add_state(StateHVSCU::CONNECTING);
    hvscu_sm->add_state(StateHVSCU::OPERATIONAL);
    hvscu_sm->add_state(StateHVSCU::FAULT);
}

void StateMachineHVSCU::add_transitions() {
    hvscu_sm->add_transition(
        StateHVSCU::CONNECTING, StateHVSCU::OPERATIONAL,
        []() { return HVSCUComms::control_station->is_connected(); });
    hvscu_sm->add_transition(StateHVSCU::OPERATIONAL, StateHVSCU::FAULT, []() {
        return !HVSCUComms::control_station->is_connected();
    });
}
