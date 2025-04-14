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
    std::function<void()> callback;
    static bool received;
    HeapOrder* order;

   public:
    HVSCUOrder(std::function<void()> callback);
    void check_order() override;
};

template <Comms::IDOrder id>
bool HVSCUOrder<id>::received = false;

template <Comms::IDOrder id>
HVSCUOrder<id>::HVSCUOrder(std::function<void()> callback) : callback(callback) {
    order = new HeapOrder(static_cast<uint16_t>(id), []() { received = true; });
}

template <Comms::IDOrder id>
void HVSCUOrder<id>::check_order() {
    if (received) {
        callback();
        received = false;
    }
}
