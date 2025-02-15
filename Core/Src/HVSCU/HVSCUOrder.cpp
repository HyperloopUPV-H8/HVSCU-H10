#include "HVSCU/HVSCUOrder.hpp"

HVSCUOrder::HVSCUOrder(uint16_t id, HVSCUStateMachine::State state, void (*callback)(void))
    : callback(callback), state(state) {
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