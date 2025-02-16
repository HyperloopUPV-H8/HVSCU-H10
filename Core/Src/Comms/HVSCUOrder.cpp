#include "Comms/HVSCUOrder.hpp"

HVSCUOrder::HVSCUOrder(uint16_t id, void (*callback)(void))
    : callback(callback) {
        received = false;
        order = new HeapOrder(id, [this](){
            received = true;
        });
    }

void HVSCUOrder::check_order() {
    if(received) {
        callback();
    }
}