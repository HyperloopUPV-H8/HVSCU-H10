#pragma once

#include "ST-LIB.hpp"
#include "HVSCU/Contactor.hpp"
#include "HVSCU/HVSCU.hpp"

class HVSCUStateMachine {
   private:
    enum State { CONNECTING = 0, OPERATIONAL = 1, FAULT = 2 };
    StateMachine sm;
    HVSCU hvscu;

    void add_states();
    void add_transitions();
    void check_open_contactors_order();
    void check_close_contactors_order();
    void check_sdc_obbcu_order();

   public:
    HVSCUStateMachine();
    void update();
};