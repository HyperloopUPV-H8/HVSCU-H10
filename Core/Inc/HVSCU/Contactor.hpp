#pragma once

#include "ST-LIB.hpp"

class Contactor {
   private:
    DigitalOutput output;
    bool normally_opened;

   public:
    Contactor(DigitalOutput pin, bool normally_opened);
    void open();
    void close();
};