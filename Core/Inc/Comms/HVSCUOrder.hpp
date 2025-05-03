#pragma once

#include "Comms/Comms.hpp"
#include "ST-LIB.hpp"

namespace HVSCU {
class OrderBase {
   public:
    virtual void check_order() = 0;
};

template <Comms::IDOrder id>
class Order : public OrderBase {
   private:
    std::function<void()> callback;
    static bool received;
    HeapOrder* order;

   public:
    Order(std::function<void()> callback);
    void check_order() override;
};

template <Comms::IDOrder id>
bool Order<id>::received = false;

template <Comms::IDOrder id>
Order<id>::Order(std::function<void()> callback) : callback(callback) {
    order = new HeapOrder(static_cast<uint16_t>(id), []() { received = true; });
}

template <Comms::IDOrder id>
void Order<id>::check_order() {
    if (received) {
        callback();
        received = false;
    }
}
}