#pragma once

#include "ST-LIB.hpp"

class Contactor {
    enum class State { OPEN, CLOSED };

    DigitalOutput output;
    bool normally_opened;
    State state;

   public:
    Contactor(Pin& pin, bool normally_opened);
    void open();
    bool is_open();

    void close();
    bool is_closed();
};