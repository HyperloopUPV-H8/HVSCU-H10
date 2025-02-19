#pragma once

#include "Comms/Comms.hpp"
#include "ST-LIB.hpp"

class HVSCUOrderBase {
   public:
    virtual void check_order() = 0;
};

template <Comms::IDOrder id>
class HVSCUOrder : public HVSCUOrderBase {
   private:
    void (*callback)(void);
    static bool received;
    HeapOrder* order;

   public:
    HVSCUOrder(void (*callback)(void));
    void check_order() override;
};

template <Comms::IDOrder id>
bool HVSCUOrder<id>::received = false;

template <Comms::IDOrder id>
HVSCUOrder<id>::HVSCUOrder(void (*callback)(void)) : callback(callback) {
    order = new HeapOrder(static_cast<uint16_t>(id), []() { received = true; });
}

template <Comms::IDOrder id>
void HVSCUOrder<id>::check_order() {
    if (received) {
        callback();
        received = false;
    }
}
