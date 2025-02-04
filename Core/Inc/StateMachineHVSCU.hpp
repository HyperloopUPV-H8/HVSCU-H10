#pragma once

#include "ST-LIB.hpp"
namespace HVSCU {
enum State { CONNECTING = 0, OPERATIONAL = 1, FAULT = 2 };

class SM {
   private:
    static StateMachine* sm;
    uint8_t timeout_id;

    void add_states();
    void add_transitions();
    void check_open_contactors_order();
    void check_close_contactors_order();

   public:
    SM();
    void update();
};
};  // namespace HVSCU