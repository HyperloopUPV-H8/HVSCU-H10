#pragma once

#include "ST-LIB.hpp"

enum StateHVSCU { CONNECTING = 0, OPERATIONAL = 1, FAULT = 2 };

class StateMachineHVSCU {
   private:
    static StateMachine* hvscu_sm;
    void add_states();
    void add_transitions();

   public:
    StateMachineHVSCU();
};