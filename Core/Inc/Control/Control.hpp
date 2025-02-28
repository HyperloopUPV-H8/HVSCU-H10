#pragma once

#include "Actuators/Actuators.hpp"
#include "Comms/HVSCUOrder.hpp"
#include "ST-LIB.hpp"

class Control {
   private:
    enum State : uint8_t { CONNECTING = 0, OPERATIONAL = 1, FAULT = 2 };

    StateMachine state_machine;
    std::unordered_map<State, std::vector<HVSCUOrderBase*>> orders;
    std::unordered_map<State, std::vector<HeapPacket*>> packets;
    bool send_packets_flag;

    void add_states();
    void add_transitions();
    void add_orders();
    void add_packets();

   public:
    Control();
    void update();
};