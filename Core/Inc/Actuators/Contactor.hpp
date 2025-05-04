#pragma once

#include "ST-LIB.hpp"

class Contactor {
    enum class State { OPEN, CLOSED };

   private:
    DigitalOutput output;
    bool normally_opened;
    State state;

   public:
    Contactor(DigitalOutput pin, bool normally_opened);
    void open();
    bool is_open();

    void close();
    bool is_closed();
};