#pragma once

#include "ST-LIB.hpp"

class HVSCUOrder {
   private:

    void (*callback)(void);
    bool received;
    HeapOrder *order;

   public:
    HVSCUOrder(uint16_t id, void (*callback)(void));
    void check_order();
};
