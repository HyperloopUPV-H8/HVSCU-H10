#pragma once

#include "HVSCU/Contactor.hpp"
#include "HVSCU/HVSCU.hpp"
#include "ST-LIB.hpp"

class HVSCUStateMachine {
   private:
    StateMachine sm;
    HVSCU hvscu;

    void add_states();
    void add_transitions();
    void check_open_contactors_order();
    void check_close_contactors_order();
    void check_sdc_obbcu_order();

   public:
    enum State { CONNECTING = 0, OPERATIONAL = 1, FAULT = 2 };

    HVSCUStateMachine();
    void update();
};